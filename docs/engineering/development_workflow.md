# XENITH Development & Engineering Workflow

This document outlines the standard engineering workflow for building and contributing to XENITH.

---

## 🔄 Development Cycle

1. **Architecture First**: Before implementing non-trivial features, write an Architectural Decision Record (ADR) or design document.
2. **Interface Definition**: Define public headers in `include/xenith/` before writing implementation code in `src/xenith/`.
3. **Unit Test Creation**: Author unit tests alongside implementation code in `tests/unit/`.
4. **Code Quality Checks**: Run `clang-format` and `clang-tidy` before submitting PRs or commits.
5. **Continuous Verification**: Build and pass unit test suites under both Debug and Release configurations.

---

## 🛠️ Build Commands Workflow

### Build with CMake Presets:
```bash
# Configure debug build
cmake --preset debug

# Build targets
cmake --build --preset debug

# Run unit tests
ctest --preset unit-tests
```

---

## 📝 ADR Process

Major structural decisions (language standard, build system, mathematical representations, solver strategy) must follow the ADR process detailed in [`docs/decisions/README.md`](../decisions/README.md).
