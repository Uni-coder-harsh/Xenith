# XENITH

**XENITH** is a sovereign, high-performance mathematical optimization solver engine built from first principles in modern C++20.

---

## 🎯 Project Overview & Purpose

XENITH is designed as an independent, domain-agnostic solver system for large-scale mathematical optimization problems. It aims to deliver industrial-grade correctness, numerical reliability, high performance, and architectural extensibility.

While XENITH is intended to eventually power demanding industrial workloads—such as refinery scheduling, supply chain optimization, power grid dispatch, crude blending, and logistics—the **solver core remains completely domain-independent**. Application domains serve purely as validation targets and drive zero domain-specific assumptions within the solver architecture.

---

## 🚫 What XENITH Is and Is Not

### XENITH IS:
- **A sovereign mathematical optimization engine** built directly from first principles.
- **Domain-independent** mathematical software.
- **Architected as a solver system**, cleanly decoupling input parsing, canonical model representation, presolve transformations, solver algorithms, numerical linear algebra, and hardware backends.
- **Focused on LP correctness and numerical stability first**, progressing methodically toward MILP, QP, and advanced optimization paradigms.

### XENITH IS NOT:
- A wrapper or thin API over existing commercial or open-source solvers (e.g., CPLEX, Gurobi, HiGHS, SCIP).
- A domain-specific application (e.g., a refinery dashboard or supply-chain tool).
- A GUI-first or web application.
- A collection of placeholder or "fake" implementations.

---

## 📐 Current Development Priority: LP Core Kernel

The immediate focus of XENITH is building a **robust, high-performance Linear Programming (LP) core kernel**. 

### Progression Plan:
```
Canonical Model Representation
          ↓
Sparse Matrix & Numerical Foundations
          ↓
MPS Input Parser
          ↓
Model Validation & Invariant Checking
          ↓
Presolve System (Reversible Transformations)
          ↓
LP Solver Engine (Revised & Dual Simplex)
          ↓
Solution Validation & Postsolve
          ↓
Benchmarking & Numerical Verification
          ↓
MILP & Branch-and-Bound / Cutting Planes
          ↓
Quadratic Programming (QP) & Beyond
```

---

## 🏗️ Core Architectural Principles

XENITH avoids monolithic solver designs by enforcing strict boundary separation across independent subsystems:

1. **Input Format $\neq$ Mathematical Model**: Input formats like MPS or LP are interchange representations. They translate into the central **Canonical Model** and never leak into solver algorithms.
2. **Canonical Model $\neq$ Solver State**: The internal model uses a bounded-row representation ($\min/\max c^T x$ s.t. $l_r \le A x \le u_r, l_x \le x \le u_x$) that natively represents general constraints and bounds while supporting integrality metadata for future MILP extensions.
3. **Solver Algorithm $\neq$ Numerical Core**: Optimization algorithms (e.g., Revised Simplex) interact with numerical linear algebra through clean abstractions. Dense/sparse vector operations, LU factorization, and basis updates are independent reusable components.
4. **Numerical Core $\neq$ Hardware Execution**: Algorithm logic remains agnostic of hardware backends (CPU, AVX-512, future GPU execution).

---

## 📂 Repository Structure

```
xenith/
├── README.md                 # Project introduction and philosophy
├── LICENSE                   # Apache-2.0 License
├── CMakeLists.txt            # Root build configuration (C++20)
├── CMakePresets.json         # Standard build presets
├── .gitignore                # Git exclusion rules
├── .clang-format             # Code formatting guidelines
├── .clang-tidy               # Static analysis rules
├── docs/                     # Comprehensive architecture and design documentation
│   ├── architecture/         # System design, canonical model, numerics, extensibility
│   ├── input/                # MPS parser design
│   ├── algorithms/lp/        # LP solver algorithms, basis management, factorization
│   ├── validation/           # Solution validation & postsolve specification
│   ├── benchmarks/           # Benchmark methodology (Netlib, etc.)
│   ├── engineering/          # Coding standards, testing, workflow, principles
│   ├── decisions/            # Architectural Decision Records (ADRs 001–010)
│   ├── roadmap.md            # Strategic multi-phase project roadmap
│   └── status.md             # Development status and implementation boundaries
├── include/xenith/           # Public C++ header files
│   ├── core/                 # Core definitions, types, telemetry
│   ├── model/                # Canonical model data structures
│   ├── io/                   # Readers/writers interfaces
│   ├── presolve/             # Presolve infrastructure
│   ├── solver/               # LP, MIP, QP solver modules
│   ├── numerics/             # Vectors, sparse matrices, factorizations
│   ├── runtime/              # Execution backends & hardware abstraction
│   ├── solution/             # Solution result & validation types
│   └── common/               # General utilities & constants
├── src/xenith/               # Private C++ implementations
├── tests/                    # Unit, integration, and regression tests
├── benchmarks/               # Benchmark configurations, scripts, and results
├── examples/                 # Target usage examples (LP, MPS, C++ API)
├── tools/                    # Inspection and diagnostic tooling
├── scripts/                  # Repository tooling (scaffolding, etc.)
└── python/xenith/            # Future Python binding specifications
```

---

## 🛠️ Building & Tooling Strategy

- **Language Standard**: Modern C++20 (`-std=c++20`).
- **Build System**: CMake (minimum 3.20) with standard presets.
- **Compilers**: GCC 10+, Clang 12+, MSVC 2019+.
- **Formatting & Analysis**: `clang-format` and `clang-tidy`.

---

## 🚀 Running XENITH on an MPS File

XENITH provides a user-facing command-line tool `xenith_mps` to parse, inspect, and validate any Mathematical Programming System (MPS) optimization model file:

```bash
# Build XENITH executable
cmake -S . -B build
cmake --build build

# Inspect an MPS file
./build/xenith_mps tests/data/mps/afiro.mps
```

### Demonstration Pipeline:

```text
Input (.mps file)
        ↓
   MPS Reader
        ↓
 CanonicalModel
        ↓
 Model Validator
        ↓
  Model Summary
```

> **[!NOTE]**
> At this stage, XENITH parses, tokenizes, and validates optimization models, producing canonical structural representations and invariant checks, but does **not** solve them yet (solvers are implemented in Phase 3+).

---

## 🎬 How to Demonstrate an Arbitrary MPS File

To test or demonstrate XENITH on any standard `.mps` file without changing source code or rebuilding:

1. **Build XENITH**:
   ```bash
   cmake --build build
   ```
2. **Execute `xenith_mps` with any file path**:
   ```bash
   ./build/xenith_mps /path/to/any_problem.mps
   ```
3. **Observe the model summary and validation check**:
   ```text
   XENITH MPS Reader
   -----------------
   File: /path/to/any_problem.mps

   Parse status: SUCCESS

   Model:
     Name: PROBLEM_NAME
     Variables: 32
     Constraints: 27
     Nonzeros: 83
     Objective sense: MINIMIZE
     Integer variables: 0
     Binary variables: 0

   Validation: PASSED
   ```

*Note: Changing the input `.mps` file only requires running the command again with a new file path. No source code modifications, CMake changes, or rebuilds are required.*

---

## 📌 Development Status

Current Phase: **Phase 2 Complete — MPS Input Layer & Validation Acceptance Verified**

- **Completed**:
  - Repository structure, architectural documentation, ADRs (001–012), build setup.
  - C++20 `CanonicalModel` representation & `ModelValidator` invariant engine.
  - Sparse Matrix CSC/CSR abstractions & vector numerics primitives.
  - Section-aware `MpsReader` engine supporting fixed and free format MPS files.
  - 8 automated test suites passing 100% clean (`unit_vector_ops`, `unit_sparse_matrix`, `unit_canonical_model`, `unit_model_validator`, `unit_mps_reader`, `integration_model_numerics`, `integration_mps_canonical_equivalence`, `integration_mps_cli_smoke`).
- **In Progress**: Preparing for Phase 3 (Basis Management, LU Factorization & FTRAN/BTRAN Solves).
- **Not Implemented Yet**: Simplex algorithms, presolve transformations, factorization update routines, GPU execution, MILP/QP solvers.

For full status and phase boundaries, see [`docs/status.md`](docs/status.md) and [`docs/roadmap.md`](docs/roadmap.md).
For a detailed task execution history tracking completed work, successes, and resolved issues, see [`docs/work_log.md`](docs/work_log.md).

---

## 📜 License

XENITH is distributed under the terms of the [Apache License (Version 2.0)](LICENSE).

