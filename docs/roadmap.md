# XENITH Strategic Multi-Phase Roadmap

XENITH is built incrementally from mathematical and engineering foundations.

---

## 🗺️ Multi-Phase Strategic Execution Plan

```mermaid
timeline
    title XENITH Project Roadmap
    Phase 0 : Repository Architecture Foundation : Complete ADRs 001-010 : Scaffold C++20 CMake infrastructure : Document Canonical Model & System Data Flow
    Phase 1 : Canonical Model & Numerics Implementation : C++20 CanonicalModel implementation : Sparse matrix (CSC/CSR) primitives : Basic SpMV & inner products
    Phase 2 : MPS Input & Validation Pipeline : MPS parser implementation (Fixed & Free format) : Model validator & invariant checker : Unit testing on synthetic MPS inputs
    Phase 3 : LP Solver Core (Revised Simplex) : BasisManager & LU Factorization : Phase I / II Revised Simplex engine : Steepest-edge pricing & Forrest-Tomlin updates
    Phase 4 : Presolve & Postsolve Transformations : Fixed variable & singleton row elimination : Dual presolve rules : Reversible Postsolve mapping
    Phase 5 : Benchmark Suite & LP Verification : Netlib LP benchmark integration : Comparative performance metrics vs standard reference outputs : Numerical residual tuning
    Phase 6 : Advanced LP & Dual Simplex : High-performance Dual Simplex engine : Interior point (Barrier) LP solver : Multi-threaded column pricing
    Phase 7 : MILP Expansion : Branch-and-bound tree manager : Cutting plane generators (Gomory, MIR) : Primal heuristics & node selection
    Phase 8 : QP & Hardware Backends : Quadratic objective support ($Q$) : SIMD / AVX-512 vectorization backends : Initial CUDA GPU execution backend
```

---

## 🔒 Phase Boundaries & Scope Enforcement

- **Current Phase**: **Phase 0** (Repository Architecture Foundation).
- **Rule**: Do not begin Phase 1+ code implementation until Phase 0 architecture foundation and design documents are fully established and committed.
