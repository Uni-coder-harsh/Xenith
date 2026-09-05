# ADR-007: LP-First Solver Development Strategy

- **Status**: Accepted
- **Date**: 2026-09-05

---

## 📌 Context
Building a complete mathematical solver suite (LP, MILP, QP, MINLP) is a massive undertaking. Attempting to build MILP branch-and-bound or QP algorithms before establishing a reliable, numerically stable LP solver leads to poor solver quality.

## 💡 Decision
We adopt an **LP-First Development Strategy**. All initial engineering efforts focus exclusively on building a rock-solid, verified LP core kernel (Revised Simplex and Dual Simplex) before expanding into MILP or QP.

## ⚖️ Consequences
### Positive:
- Ensures high numerical quality, correctness, and benchmark verification on LP foundations.
- Provides the essential relaxation engine required for future MILP branch-and-bound solving.

### Negative:
- Non-LP features (integer variables, quadratic objectives) are deferred to later roadmap phases.
