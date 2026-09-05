# XENITH Numerical Architecture

The **Numerical Core** (`xenith/numerics/`) provides reusable, highly optimized sparse linear algebra routines and factorization algorithms. It operates independently of high-level solver logic (simplex, presolve, MILP).

---

## 🧮 Numerical Core Decoupling

```
         ┌──────────────────────────────────────┐
         │     LP / Simplex Solver Engines      │
         └──────────────────┬───────────────────┘
                            │
              Numerical Linear Algebra API
                            │
         ┌──────────────────▼───────────────────┐
         │       XENITH Numerical Core          │
         │  (Sparse Matrices, Vectors, Solves) │
         └──────────────────┬───────────────────┘
                            │
             Execution Backend Abstraction
                            │
         ┌──────────────────▼───────────────────┐
         │    Hardware Backend (CPU/SIMD/GPU)   │
         └──────────────────────────────────────┘
```

---

## 📊 Sparse Matrix Representations

XENITH provides two primary sparse matrix representations:

1. **Compressed Sparse Column (CSC)**:
   - Optimized for column-oriented operations such as column pricing, matrix-vector multiplication $A x$, and basis column insertion.
   - Arrays: `col_ptr` (size $n+1$), `row_ind` (size $\text{nnz}$), `values` (size $\text{nnz}$).
2. **Compressed Sparse Row (CSR)**:
   - Optimized for row-oriented operations such as row pricing, range constraint checks, and matrix-transpose-vector multiplication $A^T y$.
   - Arrays: `row_ptr` (size $m+1$), `col_ind` (size $\text{nnz}$), `values` (size $\text{nnz}$).

Both CSC and CSR formats support zero-copy reference creation from dual-stored matrix representations.

---

## 🔍 LU Factorization & Basis Linear Solves

Simplex algorithms require solving two system types at each iteration:

1. **FTRAN (Forward Transformation / Column Solve)**:
   $$\text{Solve } B d = a_j \quad \text{for direction vector } d$$
2. **BTRAN (Backward Transformation / Row Solve)**:
   $$\text{Solve } B^T y = c_B \quad \text{for dual vector } y$$

### Factorization Scheme:
- **Base LU Factorization**: Sparse Markowitz pivoting LU decomposition computed periodically (every $K$ iterations).
- **Factorization Updates**: Fast rank-1 basis updates during simplex iterations using Forrest-Tomlin or Bartels-Golub update methods to avoid full refactorization at every pivot.
- **Refactorization Triggers**:
  - Exceeding iteration limit threshold (e.g., $K = 50$ to $100$ pivots).
  - Accumulation of numerical error beyond tolerance thresholds ($\|B d - a_j\| > \epsilon_{\text{resid}}$).
  - Encountering zero or near-zero pivot values ($|\text{pivot}| < \epsilon_{\text{pivot}}$).

---

## ⚖️ Numerical Stability & Tolerances

Floating-point operations in optimization solvers are vulnerable to numerical instability. XENITH establishes explicit, configurable numerical tolerances:

| Parameter | Default Value | Description |
| :--- | :--- | :--- |
| `zero_tolerance` | $10^{-12}$ | Values below this threshold are treated as exact zero. |
| `pivot_tolerance` | $10^{-10}$ | Minimum absolute magnitude for a valid pivot element. |
| `primal_feasibility_tolerance` | $10^{-6}$ | Maximum allowed primal constraint violation $\|A x - s\|_{\infty}$. |
| `dual_feasibility_tolerance` | $10^{-6}$ | Maximum allowed reduced cost violation for nonbasic variables. |
| `residual_refactor_threshold` | $10^{-8}$ | Relative linear solve residual threshold triggering basis refactorization. |

---

## ⚡ Matrix Scaling Primitives

Before entering the solver kernel, canonical models undergo numerical scaling to reduce matrix condition number $\kappa(A)$:

- **Equilibration Scaling**: Row and column scaling such that max entry magnitude in each row and column approaches 1.
- **Geometric Mean Scaling**: Iterative scaling using row and column geometric means.
- Scaling multipliers are recorded to allow exact unscaling of primal and dual solution vectors during postsolve.
