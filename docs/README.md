# XENITH Documentation Index

Welcome to the documentation for **XENITH**, a sovereign mathematical optimization solver engine.

---

## 🗺️ Documentation Sitemap

### 🏛️ Architecture & Design
- [System Architecture Overview](architecture/overview.md) — High-level architecture, subsystem boundaries, data flow pipelines, and separation of concerns.
- [Canonical Model Specification](architecture/canonical_model.md) — In-depth details of the bounded-row canonical model representation, invariants, and mapping.
- [Solver Kernel Architecture](architecture/solver_kernel.md) — Design of the solver control engine, state machine, and algorithm lifecycle.
- [Numerical Core Architecture](architecture/numerical_architecture.md) — Linear algebra abstractions, sparse matrix formats, vectors, factorizations, and tolerances.
- [Hardware & Runtime Architecture](architecture/runtime_architecture.md) — Execution backend isolation, SIMD/AVX vectorization, thread management, and platform strategy.
- [System Extensibility & Modularity](architecture/extensibility.md) — Architectural provisions for LP, MILP, QP, NLP expansion without refactoring core layers.
- [Reversible Presolve & Postsolve Architecture](architecture/presolve_design.md) — Stack-based reversible presolve transformation pipeline.

---

### 📥 Input Subsystem
- [MPS Parser & Input Pipeline](input/mps_design.md) — MPS parsing strategy, fixed vs. free format, validation, and mapping to canonical model.

---

### ⚡ Optimization Algorithms
- [LP Solver Overview](algorithms/lp/overview.md) — Strategic roadmap for LP algorithms (Revised Simplex, Dual Simplex, Interior Point).
- [Revised Simplex Algorithm](algorithms/lp/revised_simplex.md) — Mathematical specification, phase I/II execution, pricing, ratio test, and pivoting.
- [Basis Management & Warm Starts](algorithms/lp/basis.md) — First-class basis matrix abstraction, status tracking, refactorization criteria, and warm-starts.
- [Basis Factorization & Updates](algorithms/lp/factorization.md) — LU factorization algorithms, Forrest-Tomlin / Bartels-Golub update schemes, and precision recovery.

---

### 🔬 Solution & Validation
- [Solution Validation & Postsolve](validation/solution_validation.md) — Independent solution checking ($A x - s = 0$, primal/dual feasibility), `SolveResult` structure, and postsolve mapping.

---

### 📊 Benchmarking & Telemetry
- [Benchmarking Strategy](benchmarks/benchmark_strategy.md) — Benchmark philosophy, Netlib suite integration, metrics (runtime, iterations, residual, memory), and solver comparisons.

---

### 🛠️ Engineering Standards & Workflow
- [Engineering Principles](engineering/engineering_principles.md) — Core principles (Correctness over premature optimization, Domain independence, Invariant enforcement).
- [Coding Standards](engineering/coding_standards.md) — Modern C++20 conventions, RAII, safety rules, memory guidelines, naming rules.
- [Testing Strategy](engineering/testing_strategy.md) — Unit testing, integration tests, numerical stability validation, regression suites.
- [Development Workflow](engineering/development_workflow.md) — Git workflow, code reviews, documentation rules, ADR lifecycle.

---

### 📋 Architectural Decision Records (ADRs)
- [ADR Index & Process](decisions/README.md)
  - [ADR-001: Core Language C++20](decisions/ADR-001-cpp20.md)
  - [ADR-002: Build System CMake](decisions/ADR-002-cmake.md)
  - [ADR-003: Central Canonical Model](decisions/ADR-003-canonical-model.md)
  - [ADR-004: Bounded-Row LP Model Representation](decisions/ADR-004-bounded-row-model.md)
  - [ADR-005: Separation of Solver Algorithms from Numerical Runtime](decisions/ADR-005-solver-numerics-separation.md)
  - [ADR-006: Reusable Sparse Matrix Abstraction](decisions/ADR-006-sparse-matrix-abstraction.md)
  - [ADR-007: LP-First Development Strategy](decisions/ADR-007-lp-first.md)
  - [ADR-008: MPS as Initial Benchmark and Interchange Interface](decisions/ADR-008-mps-input.md)
  - [ADR-009: Platform-Independent Core with Isolated Hardware Backends](decisions/ADR-009-platform-independent-core.md)
  - [ADR-010: Benchmarking as a First-Class Subsystem](decisions/ADR-010-benchmarking.md)
  - [ADR-011: Warm-Start Basis and Re-optimization Support](decisions/ADR-011-warm-start-reoptimization.md)
  - [ADR-012: Reversible Stack-Based Presolve Architecture](ADR-012-reversible-presolve-stack.md)

---

### 📍 Status & Roadmap
- [Strategic Roadmap](roadmap.md) — Multi-phase vision from LP Core Kernel to MILP, QP, and advanced optimization.
- [Current Development Status](status.md) — Detailed status report for Phase 0 and boundaries of current implementation.
- [Agent Work & Task Execution Log](work_log.md) — Persistent audit log tracking tasks, successes, failures, and major milestones.
