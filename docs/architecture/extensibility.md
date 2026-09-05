# XENITH System Extensibility & Modularity Matrix

XENITH is designed for long-term modular expansion across multiple mathematical optimization paradigms without requiring structural redesigns of existing subsystems.

---

## 🧪 Conceptual Extensibility Audit (Scenarios A – G)

To test the architecture before coding begins, we trace the impact of adding 7 distinct future optimization algorithms/capabilities across the codebase:

```mermaid
flowchart TD
    subgraph Core Shared Layer (Untouched)
        CM["Canonical Model (xenith/model/)"]
        IO["MPS Reader (xenith/io/)"]
        PS["Presolve (xenith/presolve/)"]
        NC["Numerical Core (xenith/numerics/)"]
        SV["Solution Validator (xenith/solution/)"]
    end

    subgraph Additions Across Scenarios
        SC_A["Scenario A: Dual Simplex"]
        SC_B["Scenario B: Primal Simplex"]
        SC_C["Scenario C: Interior Point"]
        SC_D["Scenario D: MILP Branch & Bound"]
        SC_E["Scenario E: Crossover LP Solver"]
        SC_F["Scenario F: GPU SpMV Execution"]
        SC_G["Scenario G: Quadratic Programming (QP)"]
    end

    CM --> SC_A
    CM --> SC_B
    CM --> SC_C
    CM --> SC_D
    CM --> SC_E
    CM --> SC_F
    CM --> SC_G
```

---

## 📊 Scenario Impact Matrix

| Scenario | Added / Modified Modules | Unaffected Core Subsystems | Coupling / Isolation Assessment |
| :--- | :--- | :--- | :--- |
| **Scenario A: Add Dual Simplex** | `xenith/solver/lp/DualSimplexEngine` | `model/`, `io/`, `presolve/`, `numerics/`, `runtime/` | **Perfect Isolation**. Reuses existing `BasisManager` and `LUFactorization`. Zero changes to model or numerics. |
| **Scenario B: Add Primal Simplex** | `xenith/solver/lp/PrimalSimplexEngine` | `model/`, `io/`, `presolve/`, `numerics/`, `runtime/` | **Perfect Isolation**. Shares basis and pricing structures with Dual Simplex. |
| **Scenario C: Add Interior Point (Barrier)** | `xenith/solver/lp/InteriorPointEngine` | `model/`, `io/`, `presolve/`, `solution/` | **Clean Isolation**. Uses `SparseMatrix` to construct KKT system ($A D^2 A^T \Delta y = r$). Bypasses basis matrix factorization. |
| **Scenario D: Add MILP Branch & Bound** | `xenith/solver/mip/BranchAndBoundEngine` | `numerics/`, `io/`, `presolve/`, `runtime/` | **Clean Modular Coupling**. Calls `LPSolverManager` to solve LP relaxations at tree nodes using warm-start basis headers (`BasisHeader`). Zero changes to LP linear algebra. |
| **Scenario E: Add Second LP Solver (Crossover)** | `xenith/solver/lp/CrossoverEngine` | `model/`, `io/`, `presolve/` | **Clean Isolation**. Converts interior point solution to vertex basis solution using simplex basis facilities. |
| **Scenario F: Add GPU SpMV Kernel** | `xenith/runtime/backends/cuda/` | `model/`, `io/`, `presolve/`, `solver/lp/`, `solver/mip/` | **Zero Algorithmic Leakage**. Vector/matrix operations dispatch through `xenith/runtime/`. Simplex loops remain 100% C++20 without CUDA code. |
| **Scenario G: Add Quadratic Programming (QP)** | `xenith/solver/qp/QPSolverEngine` | `io/`, `presolve/`, `solver/lp/`, `solver/mip/` | **Modular Extension**. Activates optional $Q$ matrix slot in `CanonicalModel`. LP solver pathways ignore $Q$ cleanly. |

---

## 🔑 Key Takeaways from Extensibility Audit

1. **Zero Monolithic Solver Coupling**: LP simplex, MILP branch-and-bound, and QP solvers live in separate subdirectories (`solver/lp/`, `solver/mip/`, `solver/qp/`) and communicate via clean API interfaces.
2. **GPU Code Isolation**: CUDA code lives exclusively inside `xenith/runtime/backends/cuda/`. Mathematical algorithms in `xenith/solver/` do not contain CUDA pragmas, headers, or platform directives.
3. **No Heavy Plugin Overhead**: Solvers are compiled into modular library targets without fragile dynamic shared object plugin frameworks.
