# ADR-010: Benchmarking Subsystem as a First-Class Citizen

- **Status**: Accepted
- **Date**: 2026-09-05

---

## 📌 Context
Optimization solver quality can only be measured empirically. Treating benchmarking as an afterthought or manual script leads to undetected numerical regressions and unverified performance claims.

## 💡 Decision
We establish **Benchmarking as a First-Class Subsystem** within XENITH (`benchmarks/`). The benchmark framework automates correctness, feasibility, iteration count, memory, and runtime metrics collection against standard LP datasets.

## ⚖️ Consequences
### Positive:
- Empirical validation drives optimization work.
- Prevents numerical degradation and performance regressions.

### Negative:
- Requires setup and maintenance of automated benchmark evaluation infrastructure.
