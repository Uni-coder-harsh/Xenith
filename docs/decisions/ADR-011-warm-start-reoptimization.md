# ADR-011: Warm-Start Basis and Re-optimization Support

- **Status**: Accepted
- **Date**: 2026-09-05

---

## 📌 Context
MILP branch-and-bound solving requires solving thousands of sequence LP relaxations. Solving each relaxation from scratch without warm-starting causes prohibitively expensive solve runtimes.

## 💡 Decision
We make **Warm-Start Basis loading** (`xenith::solver::lp::BasisHeader`) a native feature of the LP solver kernel and `BasisManager`.

## ⚖️ Consequences
### Positive:
- Enables Dual Simplex to re-optimize branch-and-bound nodes in very few pivots.
- Enables fast resolve after cutting plane addition or presolve bound tightening.

### Negative:
- `BasisManager` must validate warm-start basis dimension compatibility before solve entry.
