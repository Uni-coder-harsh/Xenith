# XENITH Strategic Multi-Phase Roadmap

XENITH is built incrementally from mathematical and engineering foundations.

---

## 🗺️ Multi-Phase Strategic Execution Plan

```mermaid
timeline
    title XENITH Project Roadmap
    Phase 0 (Complete) : Repository Architecture Foundation : Complete ADRs 001-012 : Scaffold C++20 CMake infrastructure : Document Canonical Model & System Data Flow
    Phase 1 (Complete) : Canonical Model & Numerics Implementation : C++20 CanonicalModel & ModelValidator implementation : Sparse matrix (CSC/CSR) primitives : SpMV & SpMV-transpose (Ax, ATx) : Automated test suite (100% pass)
    Phase 2 (Complete) : MPS Input & Validation Pipeline : MPS parser implementation (Fixed & Free format) : Bound type mappings (LO, UP, FX, FR, MI, PL, BV, LI, UI) : Integer marker block handling : Equivalence & unit test suite (100% pass)
    Phase 3 : LP Solver Core (Revised Simplex) : BasisManager & LU Factorization : Phase I / II Revised Simplex engine : Steepest-edge pricing & Forrest-Tomlin updates
    Phase 4 : Presolve & Postsolve Transformations : Fixed variable & singleton row elimination : Dual presolve rules : Reversible Postsolve mapping
    Phase 5 : Benchmark Suite & LP Verification : Netlib LP benchmark integration : Comparative performance metrics vs standard reference outputs : Numerical residual tuning
    Phase 6 : Advanced LP & Dual Simplex : High-performance Dual Simplex engine : Interior point (Barrier) LP solver : Multi-threaded column pricing
    Phase 7 : MILP Expansion : Branch-and-bound tree manager : Cutting plane generators (Gomory, MIR) : Primal heuristics & node selection
    Phase 8 : QP & Hardware Backends : Quadratic objective support ($Q$) : SIMD / AVX-512 vectorization backends : Initial CUDA GPU execution backend
```

---

## 🔒 Phase Boundaries & Scope Enforcement

- **Current Phase**: **Phase 2 Complete**. Preparing for **Phase 3** (Basis Management, LU Factorization & FTRAN/BTRAN Solves).
- **Rule**: Do not begin Phase 3+ solver algorithms until Phase 2 MPS input parsing and canonical model creation are established and tested.

