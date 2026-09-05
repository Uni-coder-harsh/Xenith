# ADR-005: Separation of Solver Algorithms from Numerical Runtime

- **Status**: Accepted
- **Date**: 2026-09-05

---

## 📌 Context
In naive solver implementations, linear algebra operations (such as LU factorizations, matrix-vector products, inner products, and basis solves) are frequently embedded directly inside simplex iteration loops. This prevents replacing linear algebra routines with optimized sparse libraries or hardware acceleration backends.

## 💡 Decision
We strictly enforce **separation of optimization algorithms from numerical linear algebra**. Algorithm modules in `xenith/solver/` call sparse matrix and factorization interfaces defined in `xenith/numerics/`.

## ⚖️ Consequences
### Positive:
- Numerical linear algebra routines remain modular and independently testable.
- Hardware backends (SIMD, OpenMP, GPU) can be swapped inside `xenith/runtime/` without touching simplex logic.
- Promotes clean software architecture.

### Negative:
- Requires clean API boundaries and zero-overhead abstractions.
