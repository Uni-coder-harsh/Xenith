# XENITH PDLP Solver Engine Design

> **Primal-Dual Hybrid Gradient for Linear Programming (PDLP / cuPDLPx)**  
> **Status**: Specification & Numerical Foundation Complete (Phases 4A & 4B Verified)  
> **Primary References**: Applegate et al. (Google Research, [arXiv:2106.04756](https://arxiv.org/abs/2106.04756)), Lu & Yang (cuPDLP-C, [arXiv:2312.14832](https://arxiv.org/abs/2312.14832)), MIT Lu Lab (cuPDLPx, 2025)

---

## 1. Mathematical Formulation

### 1.1 Canonical Model to Equality Slack Representation
XENITH's [`CanonicalModel`](../../../include/xenith/model/canonical_model.hpp) represents general linear optimization problems in bounded-row form:

$$\min_{x \in \mathbb{R}^n} c^T x \quad \text{s.t.} \quad l_r \le A x \le u_r, \quad l_x \le x \le u_x$$

To obtain a formulation whose primal domain is a simple hyper-rectangle and whose dual domain is unconstrained ($\mathbb{R}^m$), XENITH introduces slack variables $s \in \mathbb{R}^m$ bounded by $[l_r, u_r]$:

$$\min_{\hat{x} \in X} \hat{c}^T \hat{x} \quad \text{s.t.} \quad K \hat{x} = 0$$

where:
- Expanded primal vector: $\hat{x} = \begin{pmatrix} x \\ s \end{pmatrix} \in \mathbb{R}^{n+m}$
- Expanded cost vector: $\hat{c} = \begin{pmatrix} c \\ 0_m \end{pmatrix} \in \mathbb{R}^{n+m}$
- Expanded constraint matrix: $K = \begin{pmatrix} A & -I_m \end{pmatrix} \in \mathbb{R}^{m \times (n+m)}$
- Primal bound domain: $X = [l_x, u_x] \times [l_r, u_r] \subset \mathbb{R}^{n+m}$
- Dual domain: $Y = \mathbb{R}^m$ (free vector space)

### 1.2 Saddle-Point Formulation
The primal problem is equivalent to the minimax saddle-point problem:

$$\min_{\hat{x} \in X} \max_{y \in \mathbb{R}^m} \mathcal{L}(\hat{x}, y) = \hat{c}^T \hat{x} + y^T (0 - K \hat{x}) = \hat{c}^T \hat{x} - y^T K \hat{x}$$

The gradients of the Lagrangian are:
- $\nabla_{\hat{x}} \mathcal{L}(\hat{x}, y) = \hat{c} - K^T y$
- $\nabla_y \mathcal{L}(\hat{x}, y) = -K \hat{x}$

---

## 2. The Restarted PDHG Algorithm

### 2.1 Core Primal-Dual Iteration
Given current iterates $(\hat{x}^k, y^k)$, primal step size $\tau = \eta / \omega$, and dual step size $\sigma = \eta \cdot \omega$:

1. **Primal Step (Gradient Descent on $\hat{x}$ + Box Projection)**:
   $$\hat{x}^{k+1} = \text{proj}_X \left( \hat{x}^k - \tau (\hat{c} - K^T y^k) \right)$$
   where $\text{proj}_X(v)_i = \text{clamp}(v_i, l_i, u_i)$ (handled in parallel via `xenith::numerics::projectBox`).

2. **Extrapolation (Over-relaxation)**:
   $$\bar{x}^{k+1} = 2 \hat{x}^{k+1} - \hat{x}^k$$

3. **Dual Step (Gradient Ascent on $y$)**:
   $$y^{k+1} = y^k + \sigma (0 - K \bar{x}^{k+1}) = y^k - \sigma K \bar{x}^{k+1}$$
   *(Note: Because $Y = \mathbb{R}^m$, projection is the identity operation).*

4. **Running Average Accumulation**:
   Maintain running weighted averages $(\hat{x}_{\text{avg}}, y_{\text{avg}})$ as the candidate solution for restarts and termination.

---

## 3. Preconditioning & Scaling Pipeline

To guarantee rapid convergence, the constraint matrix $K$ must be well-conditioned ($\|K\|_2 \approx 1$ with balanced row and column scales):

1. **Ruiz $\ell_\infty$ Equilibration (10 iterations)**:
   Iteratively compute:
   $$r_i = \frac{1}{\sqrt{\|K_{i,:}\|_\infty}}, \quad c_j = \frac{1}{\sqrt{\|K_{:,j}\|_\infty}}$$
   Scale rows and columns in-place via `SparseMatrix::scaleRows` and `scaleCols`.
2. **Pock-Chambolle $\ell_1$ Scaling (1 pass)**:
   $$(D_1)_{ii} = \frac{1}{\sqrt{\|K_{i,:}\|_1}}, \quad (D_2)_{jj} = \frac{1}{\sqrt{\|K_{:,j}\|_1}}$$
3. **Spectral Norm Estimation**:
   Estimate $\|K\|_2$ using power iteration (`SparseMatrix::spectralNormEstimate`) to set the initial step size:
   $$\eta_0 = \frac{0.9}{\|K\|_2}$$

---

## 4. Adaptive Restarts & Primal Weight Updates (cuPDLPx-Style)

### 4.1 GPU-Vectorized KKT Error Metric
Every 40 iterations, compute the normalized KKT error:
$$\text{KKT}_\omega(\hat{x}, y) = \max \left\{ \frac{\|K \hat{x}\|_2}{1 + \|q\|_2}, \; \frac{\|\hat{c} - K^T y - \lambda\|_2}{1 + \|\hat{c}\|_2}, \; \frac{|\text{DualityGap}|}{1 + |\hat{c}^T \hat{x}| + |y^T q|} \right\}$$
where $\lambda = \text{proj}_\Lambda(\hat{c} - K^T y)$ represents the reduced costs projected onto the normal cone of $X$.

### 4.2 Restart Conditions
A restart is triggered if any of the following three conditions holds:
1. **Sufficient Decay**: $\text{KKT}(z_c) \le 0.2 \cdot \text{KKT}(z_0)$
2. **Necessary Decay + Stagnation**: $\text{KKT}(z_c) \le 0.8 \cdot \text{KKT}(z_0)$ and $\text{KKT}(z_c) > \text{KKT}(z_{c,\text{prev}})$
3. **Long Inner Loop**: $t \ge 0.36 k_{\text{total}}$

On restart:
- Reset the iterate to the running average candidate: $(\hat{x}, y) \leftarrow (\hat{x}_{\text{avg}}, y_{\text{avg}})$.
- Update the primal weight $\omega$ using logarithmic smoothing:
  $$\omega^+ = \omega \cdot \exp \left( \theta \ln \left( \frac{\Delta y}{\Delta x} \right) \right), \quad \theta = 0.5$$

---

## 5. Convergence & Termination

The solver terminates with `ModelStatus::OPTIMAL` when all three relative residuals fall below tolerance $\epsilon$ (default $10^{-6}$ for industrial scale, $10^{-8}$ for high-precision):
1. **Primal Feasibility**: $\frac{\|K \hat{x}\|_2}{1 + \|q\|_2} \le \epsilon$
2. **Dual Feasibility**: $\frac{\|\hat{c} - K^T y - \lambda\|_2}{1 + \|\hat{c}\|_2} \le \epsilon$
3. **Duality Gap**: $\frac{|\hat{c}^T \hat{x} - \text{DualObj}|}{1 + |\hat{c}^T \hat{x}| + |\text{DualObj}|} \le \epsilon$

The solution is unscaled back to the original model's coordinate frame and passed through `Presolver::postsolve` to recover eliminated variables.
