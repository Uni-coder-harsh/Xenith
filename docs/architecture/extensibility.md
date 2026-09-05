# XENITH System Extensibility & Modularity

XENITH is designed for long-term modular expansion across multiple mathematical optimization paradigms without requiring structural redesigns of existing subsystems.

---

## 🔌 Plug-and-Play Solver Architecture

Adding a new optimization algorithm family (e.g., Interior Point LP, MILP Branch-and-Bound, or Quadratic Programming) requires implementing specific algorithm interfaces while seamlessly reusing the existing infrastructure:

```mermaid
flowchart TD
    subgraph Core Infrastructure (Shared & Reused)
        CM["Canonical Model (xenith/model/)"]
        IO["MPS / API Readers (xenith/io/)"]
        PS["Presolve System (xenith/presolve/)"]
        NC["Numerical Core (xenith/numerics/)"]
        RT["Execution Runtime (xenith/runtime/)"]
        SV["Solution Validator (xenith/solution/)"]
    end

    subgraph Modular Solver Engines
        LP_RS["Revised Simplex (LP)"]
        LP_DS["Dual Simplex (LP)"]
        LP_IP["Interior Point (LP - Roadmap)"]
        MILP_BB["Branch & Bound (MILP - Roadmap)"]
        QP_ACT["Active Set / Interior Point (QP - Roadmap)"]
    end

    CM --> LP_RS
    CM --> LP_DS
    CM --> LP_IP
    CM --> MILP_BB
    CM --> QP_ACT

    LP_RS <--> NC
    LP_DS <--> NC
    LP_IP <--> NC
    MILP_BB <--> NC
    QP_ACT <--> NC

    LP_RS --> SV
    LP_DS --> SV
    LP_IP --> SV
    MILP_BB --> SV
    QP_ACT --> SV
```

---

## 📑 Incremental Expansion Rules

1. **Model Layer Stability**: Adding MILP or QP capabilities extends `CanonicalModel` through metadata (e.g., integrality flags for MILP, sparse matrix $Q$ for QP) without breaking existing LP code paths.
2. **Presolve Pipeline Extensions**: New presolve techniques register as modular transformations adhering to a standard `PresolveRule` interface.
3. **No Heavy Plugin Framework Overhead**: XENITH avoids complex dynamic shared object plugin managers at this early stage. Interfaces use standard C++ compile-time templates and polymorphic base classes where justified.
