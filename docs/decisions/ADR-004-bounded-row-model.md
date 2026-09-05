# ADR-004: Bounded-Row Canonical LP Representation

- **Status**: Accepted
- **Date**: 2026-09-05

---

## 📌 Context
Mathematical programs can be represented in standard form ($Ax = b, x \ge 0$) or general bounded form ($l_r \le Ax \le u_r, l_x \le x \le u_x$). Choosing the right canonical model representation determines how cleanly model modifications, range constraints, and presolve reductions are handled.

## 💡 Decision
We adopt the **Bounded-Row Canonical Representation** ($\min/\max c^T x$ s.t. $l_r \le A x \le u_r, l_x \le x \le u_x$) for XENITH's internal model representation.

## ⚖️ Consequences
### Positive:
- Losslessly encodes equality, inequality ($\le, \ge$), range, and free constraints without creating explicit slack columns during model setup.
- Enables direct bound tightening during presolve.
- Losslessly mirrors standard MPS `ROWS`, `RANGES`, and `BOUNDS` semantics.

### Negative:
- LP solver kernels must internally handle slacks and variable bounds during tableau/basis construction.

## 🔄 Alternatives Considered
- Standard Equality Form ($Ax = b, x \ge 0$): Requires introducing slacks upfront during parsing, inflating matrix size prematurely and complicating postsolve mapping.
