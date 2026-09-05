# XENITH Python Bindings (Roadmap Component)

This directory is reserved for future high-level Python bindings (`pyxenith`) for XENITH.

---

## 🗺️ Architectural Plan

- **Engine Decoupling**: The core solver will remain 100% pure C++20.
- **Binding Mechanism**: Python bindings will be implemented via `pybind11` or C ABI wrappers, binding to `CanonicalModel`, `SolverManager`, and `SolveResult`.
- **Target Integration**: `pyxenith` will allow model definition using Pythonic constructs and conversion to `CanonicalModel`.

*Python bindings will be developed in a future roadmap phase after the C++ LP core kernel is implemented and verified.*
