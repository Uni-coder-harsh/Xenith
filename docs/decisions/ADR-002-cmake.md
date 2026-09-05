# ADR-002: Build System Choice (CMake)

- **Status**: Accepted
- **Date**: 2026-09-05

---

## 📌 Context
XENITH must support multi-platform builds (Linux, macOS, Windows) across various C++20 compilers (GCC, Clang, MSVC) and integrate seamlessly with testing frameworks, static analysis tooling, and future language bindings.

## 💡 Decision
We adopt **CMake** (minimum version 3.20) as the native build configuration generator for XENITH, utilizing modern target-based CMake paradigms and `CMakePresets.json`.

## ⚖️ Consequences
### Positive:
- Industry-standard build system with native IDE support (VS Code, CLion, Visual Studio).
- Target-based property management (`target_include_directories`, `target_compile_features`).
- Unified test runner integration via `ctest`.

### Negative:
- Verbose CMake syntax.

## 🔄 Alternatives Considered
- **Bazel**: Fast, but introduces heavy external dependency setups for straightforward C++ core builds.
- **Meson**: Clean, but less universally supported across Windows MSVC toolchains compared to CMake.
