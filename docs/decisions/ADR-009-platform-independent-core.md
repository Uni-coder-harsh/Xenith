# ADR-009: Platform-Independent Core with Isolated Hardware Backends

- **Status**: Accepted
- **Date**: 2026-09-05

---

## 📌 Context
Mathematical optimization solvers are deployed across heterogeneous compute environments (Linux servers, macOS ARM workstations, cloud instances, embedded edge devices, future GPU acceleration clusters).

## 💡 Decision
We maintain a **Strictly Platform-Independent Core** in ISO C++20. All platform-specific system calls, hardware vectorization intrinsics (AVX2, AVX-512, NEON), thread management, and hardware acceleration (CUDA) are isolated inside `xenith/runtime/`.

## ⚖️ Consequences
### Positive:
- Mathematical code remains completely portable across operating systems and hardware architectures.
- Simplifies cross-compilation and automated CI testing.

### Negative:
- Abstraction wrappers must be designed carefully to avoid runtime overhead.
