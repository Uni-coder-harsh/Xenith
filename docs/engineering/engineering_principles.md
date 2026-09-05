# XENITH Core Engineering Principles

Every engineer and contributor to XENITH must adhere to these 15 core engineering principles:

---

1. **Correctness Before Optimization**: Mathematical correctness and numerical stability take absolute priority over raw execution speed.
2. **Explicit Mathematical Invariants**: Mathematical invariants (such as basis size, bound validity, and matrix dimension consistency) must be explicitly checked and enforced.
3. **Domain Independence**: The solver core remains strictly domain-independent. Application domains (e.g., refinery scheduling, logistics) are validation targets, not hardcoded assumptions.
4. **Input Formats $\neq$ Canonical Representation**: File formats like MPS or LP are interchange representations and must never leak into algorithm implementations.
5. **Reusable Numerical Linear Algebra**: Numerical linear algebra components (sparse matrices, vectors, factorizations) are decoupled from optimization solver algorithms.
6. **Extensible Architecture**: Future algorithm additions (MILP, QP, Interior Point) must not require rewriting model, numerical, or IO subsystems.
7. **Benchmark-Backed Performance Claims**: All performance claims must be supported by empirical, reproducible benchmark data.
8. **No Unsupported Solver Claims**: Never make baseless claims regarding commercial solvers (e.g., CPLEX, Gurobi). Compare strictly against verifiable metrics.
9. **No Presentation-Only Features**: Avoid fake implementations, mock placeholders, or gimmick "AI" features.
10. **CPU Correctness Before GPU Acceleration**: Establish rock-solid CPU correctness before attempting hardware/GPU execution backends.
11. **Solid LP Foundation Before MILP Expansion**: Never implement MILP branch-and-bound prior to establishing a dependable LP engine.
12. **Document Major Architectural Decisions**: All non-trivial structural changes require an Architectural Decision Record (ADR).
13. **Tests as First-Class Code**: Unit tests, integration tests, and numerical regression checks are integral to solver code development.
14. **Benchmark Reproducibility**: Benchmark execution environments and configurations must be deterministic and fully reproducible.
15. **No Single-Instance Micro-Optimizations**: Never optimize the system architecture for a single benchmark problem at the expense of general robustness.
