# ADR-001: Core Language Choice (C++20)

- **Status**: Accepted
- **Date**: 2026-09-05

---

## 📌 Context
Building an industrial-grade, high-performance mathematical optimization solver requires precise low-level memory management, zero-overhead abstractions, direct vectorization/SIMD capabilities, and strong compile-time type safety.

## 💡 Decision
We adopt **C++20** as the primary implementation language for the core engine of XENITH.

## ⚖️ Consequences
### Positive:
- Zero-cost abstractions, concepts, and template metaprogramming for compile-time optimization.
- Standardized concurrency and std::span primitives.
- Deterministic memory management (RAII) with zero garbage collection pause overhead.
- Universal C ABI compatibility for future Python, C, and Rust language bindings.

### Negative:
- Manual memory management diligence required.
- Longer compile times compared to higher-level languages.

## 🔄 Alternatives Considered
- **C11**: Lacks template safety, concepts, and modern RAII abstractions.
- **Rust**: Excellent safety, but C++ ecosystem dominance in numerical linear algebra libraries (SuiteSparse, Eigen, LAPACK) makes C++20 standard more appropriate for initial core solver architecture.
