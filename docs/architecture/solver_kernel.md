# XENITH Solver Kernel Architecture

The **Solver Kernel** is responsible for orchestrating optimization algorithms, managing solver execution state, enforcing timeout/iteration limits, and mediating between algorithmic logic and the numerical core.

---

## 🏛️ Kernel Component Overview

```mermaid
flowchart TD
    subgraph Solver Kernel
        SM["SolverManager"]
        SC["SolverControl / Parameters"]
        ST["Telemetry & Progress Monitor"]
        
        subgraph Algorithm Modules
            RS["RevisedSimplexEngine (LP)"]
            DS["DualSimplexEngine (LP)"]
            IP["InteriorPointEngine (Future LP)"]
            BB["BranchAndBoundEngine (Future MILP)"]
        end
        
        BM["BasisManager"]
        SE["State & Phase Controller"]
    end

    NC["Numerical Core (Sparse Linear Algebra)"]

    SM --> SC
    SM --> ST
    SM --> SE
    SE --> RS
    SE --> DS
    SE --> IP
    SE --> BB
    RS <--> BM
    DS <--> BM
    RS <--> NC
    DS <--> NC
```

---

## 🔄 Solver Execution Lifecycle

The solver kernel manages execution via a strict state machine:

1. **Uninitialized**: No model loaded.
2. **Ready**: Model loaded, validated, and presolved.
3. **Solving (Phase I)**: Searching for a initial primal/dual feasible basis.
4. **Solving (Phase II)**: Optimizing objective function value while maintaining feasibility.
5. **Terminated**: Algorithm finished. Status set to `OPTIMAL`, `INFEASIBLE`, `UNBOUNDED`, `ITERATION_LIMIT`, `TIME_LIMIT`, or `NUMERICAL_ERROR`.

---

## 🎛️ Solver Control & Parameters

Solver behavior is configured via `SolverParameters`:

- `primal_feasibility_tolerance` (default: $10^{-6}$)
- `dual_feasibility_tolerance` (default: $10^{-6}$)
- `pivot_tolerance` (default: $10^{-10}$)
- `refactorization_frequency` (default: 50–100 iterations)
- `max_iterations` (default: unlimited / $10^6$)
- `max_time_seconds` (default: unlimited)
- `pricing_strategy` (Steepest Edge, Devex, Dantzig)
- `algorithm_choice` (Auto, PrimalSimplex, DualSimplex)

---

## 🧱 Separation of Algorithm & Numerics

To ensure modularity and clean engineering:

- Algorithmic loops (e.g., pricing step, ratio test, variable selection) reside in the algorithm modules (`xenith/solver/lp/`).
- Matrix operations, B-solve ($B x = b$), B-transpose solve ($B^T y = c$), and LU factorizations are requested through abstract numerical calls (`xenith/numerics/`).
- The algorithm modules do **not** directly inspect internal raw arrays of LU matrices.
