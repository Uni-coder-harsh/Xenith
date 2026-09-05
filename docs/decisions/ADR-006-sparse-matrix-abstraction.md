# ADR-006: Reusable Sparse Matrix Abstraction Component

- **Status**: Accepted
- **Date**: 2026-09-05

---

## 📌 Context
Optimization models operate on sparse matrices where $99\%+$ of entries are zero. A robust, reusable sparse matrix data structure is needed to support row/column access, SpMV operations, scaling, and factorizations.

## 💡 Decision
We implement a **Reusable Sparse Matrix Abstraction** (`xenith::numerics::SparseMatrix`) supporting dual Compressed Sparse Column (CSC) and Compressed Sparse Row (CSR) representations as core numerical components.

## ⚖️ Consequences
### Positive:
- Highly efficient column access for Simplex pricing ($A_{:, j}$) and row access for BTRAN ($A_{i, :}^T$).
- Contiguous array allocations maximize memory locality and CPU cache utilization.

### Negative:
- Dual CSC/CSR representations require synchronization when matrix values are mutated.
