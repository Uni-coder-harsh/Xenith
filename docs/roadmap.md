# XENITH Strategic Multi-Phase Roadmap

XENITH is built incrementally from mathematical and engineering foundations. Following our algorithmic research audit ([`sih_26119_research_roadmap.md`](../sih_26119_research_roadmap.md)), the primary continuous solver core has transitioned to **First-Order Matrix-Free Primal-Dual Hybrid Gradient (PDHG / PDLP / cuPDLPx)** to enable massive parallel throughput and native GPU execution.

---

## 🗺️ Multi-Phase Strategic Execution Plan

```mermaid
timeline
    title XENITH Strategic Project Roadmap (v3 - GPU-First PDLP Pivot)
    Phase 0 (Complete) : Architecture Foundation : Complete ADRs 001-012 : C++20 CMake Presets : System Data Flow
    Phase 1 (Complete) : Canonical Model & Numerics : Bounded-row CanonicalModel & Validator : Sparse matrix (CSC/CSR) : SpMV & SpMV-T : VectorOps (100% pass)
    Phase 2 (Complete) : MPS Input Pipeline : Fixed & Free format MpsReader : 9 bound cards & integer markers : xenith_mps CLI & Neon UI (100% pass)
    Phase 3 (Complete) : Baseline Simplex LP Core : BasisManager : Sparse PBQ=LU Markowitz factorization : 2-Phase Revised Simplex : afiro.mps validation (100% pass)
    Phase 4A (Complete) : PDLP Numerics Prerequisites : projectBox bound projections : componentwiseMul/Div : sumOfSquares : Sparse row/col norms & in-place scaling : spectralNormEstimate (100% pass)
    Phase 4B (Complete) : Reversible Presolve System : Empty rows/cols elimination : Fixed variable substitution : Singleton row bound tightening : Reversible LIFO Postsolve stack (100% pass)
    Phase 4C (Complete) : Diagonal Preconditioning : Ruiz l_inf matrix equilibration : Pock-Chambolle l_1 scaling : Invertible model scaling wrapper (100% pass)
    Phase 4D (Complete) : PDLP Core Solver Engine : Equality slack conversion [A -I] : Restarted PDHG saddle-point loop : cuPDLP-C KKT restart criteria : Dynamic primal weight balancing : afiro.mps validated (100% pass)
    Phase 4E (Complete) : Benchmarking & CLI : CLI --method pdlp/simplex : Netlib LP benchmark suite : Shifted geometric mean metrics (6.43x speedup)
    Phase 5 (Active) : GPU Hardware Acceleration : cuSPARSE SpMV kernels : Custom CUDA vector update & projection kernels : Native GPU execution
    Phase 6 : MILP Expansion : Parallel Branch-and-Bound tree manager : Cutting planes (Gomory, MIR) : Heuristics
    Phase 7 : QP & Industrial Refinery Targets : Quadratic objective support (Q) : MRPL refinery scheduling validation
```

---

## 🔒 Current Phase Boundaries & Scope Enforcement

- **Current Status**: **Phase 4 Complete (Phase 4A, 4B, 4C, 4D & 4E Complete & Verified)**.
- **Active Sprint**: **Phase 5: GPU Hardware Acceleration (CUDA / cuSPARSE)**.
- **Rule**: Maintain 100% automated test pass rate across all solver capabilities (currently 17/17 tests passing).



