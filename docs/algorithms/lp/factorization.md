# Basis LU Factorization & Fast Updates

Efficiently solving $B d = a_{j_{\text{in}}}$ (FTRAN) and $B^T y = c_B$ (BTRAN) is the single most computationally intensive task in revised simplex. Direct inversion $B^{-1}$ is computationally prohibitive and numerically unstable; sparse LU decomposition is used instead.

---

## 🧩 Basis LU Decomposition

The basis matrix $B$ is factorized as:
$$P B Q = L U$$
where:
- $P, Q$ are permutation matrices chosen via sparse Markowitz pivoting to preserve sparsity and limit fill-in.
- $L$ is lower triangular with unit diagonal.
- $U$ is upper triangular.

---

## ⚡ Rank-1 Basis Updates (Forrest-Tomlin / Bartels-Golub)

When column $i_{\text{out}}$ of $B$ is replaced by column $a_{j_{\text{in}}}$, full refactorization from scratch ($O(m^3)$ or sparse equivalent) is avoided by performing fast rank-1 updates ($O(m)$ sparse operations).

### Update Mechanism:
Replaced column creates an Eta matrix $E_k$:
$$B_{k} = B_{k-1} E_k \implies B_k^{-1} = E_k^{-1} B_{k-1}^{-1}$$

- **Forrest-Tomlin Update**: Modifies rows of $U$ and accumulates row elimination vectors.
- **Bartels-Golub Update**: Maintains numerical stability via localized pivot permutations on $U$.

---

## 🔁 Refactorization Policy

Factorization update sequences accumulate fill-in entries and numerical error over time. Full LU refactorization is triggered when:

1. Number of pivots since last full LU reaches limit (e.g., $K = 50$ to $100$ iterations).
2. Numerical residual ratio exceeds tolerance:
   $$\frac{\|B d - a_{j_{\text{in}}}\|_{\infty}}{\|a_{j_{\text{in}}}\|_{\infty}} > \epsilon_{\text{refactor}}$$
3. Pivot element in update drops below absolute stability threshold ($|\text{pivot}| < 10^{-10}$).
