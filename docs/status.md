# XENITH Development Status

**Current Phase**: Phase 4 — High-Performance First-Order LP Core (Presolve, Diagonal Scaling & PDLP Engine)  
**Status**: Phase 4 Complete (Phase 4A, 4B, 4C, 4D & 4E Implemented & Verified — 17/17 Automated Tests Passing)  
**Date**: September 10, 2026

---

## 🎯 Strategic Direction: GPU-Accelerated First-Order LP (PDLP)

Following the strategic roadmap revision ([`sih_26119_research_roadmap.md`](../sih_26119_research_roadmap.md), [arXiv:2106.04756](https://arxiv.org/abs/2106.04756), [arXiv:2312.14832](https://arxiv.org/abs/2312.14832)), XENITH has pivoted its primary LP solving engine from CPU-sequential Simplex to **Primal-Dual Hybrid Gradient (PDHG / PDLP / cuPDLPx)**.

- **Primary LP Engine**: Matrix-free Restarted PDHG operating on Sparse Matrix-Vector products (SpMV: $K x$, $K^T y$) and box projections—ideal for massive parallel throughput on multi-core CPUs (AVX-512) and GPUs.
- **Secondary / Verification Engine**: Two-Phase Revised Simplex with Markowitz Sparse LU factorization (completed in Phase 3), serving as an exact vertex verifier and future basis refiner.

---

## ✅ Completed Milestones

### Phase 0: Repository Architecture Foundation
- Repository structure (50 directories, CMake C++20 build presets, `.clang-format`, `.clang-tidy`).
- Architecture documentation suite and ADRs 001–012.

### Phase 1: Canonical Model & Numerical Foundations
- Strongly typed types, constants, infinity policy, zero tolerance ($10^{-12}$).
- Vector operations (`dot`, `axpy`, `scale`, `vectorAdd`, `vectorSub`, `infinityNorm`, `euclideanNorm`).
- Dual CSC/CSR `SparseMatrix` abstraction (`fromTriplets`, format conversion, SpMV $Ax$, $A^T x$).
- `CanonicalModel` bounded-row representation and `ModelValidator` invariant engine.

### Phase 2: MPS Input Layer & Model Parser Integration
- `MpsReader` fixed and free format parser supporting `NAME`, `OBJSENSE`, `ROWS`, `COLUMNS`, `RHS`, `RANGES`, `BOUNDS`, `ENDATA`, integer markers, and structured parser diagnostics.
- `xenith_mps` CLI tool for model inspection with styled Neon ANSI Terminal UI.

### Phase 3: LP Solver Core (Revised Simplex Engine & Sparse LU Factorization)
- `BasisManager` tracking basic/non-basic status with $O(1)$ pivot updates and structural validation.
- Sparse $P B Q = L U$ factorization with Markowitz threshold pivoting, `solveFtran`, and `solveBtran`.
- Two-phase Revised Simplex solver (`RevisedSimplexSolver`) with Dantzig pricing, Bland's tie-breaking, and primal basic variable recomputation ($B x_B = -N x_N$).
- `LpSolutionValidator` independent mathematical verification engine.
- Netlib benchmark instance `afiro.mps` solved to exact optimal objective ($-464.753143$) in 17 iterations.

### Phase 4A: PDLP Numerics Prerequisites (Completed)
1. **Extended Vector Operations (`xenith/numerics/vector_ops`)**:
   - `projectBox`: Componentwise projection onto box bounds with infinite bound handling ($[-\infty, u]$, $[l, +\infty]$, $[-\infty, +\infty]$).
   - `componentwiseMul`: Vector Hadamard product ($z_i = x_i \cdot y_i$).
   - `componentwiseDiv`: Safe elementwise division ($z_i = x_i / y_i$ with zero-divisor protection).
   - `sumOfSquares`: Efficient Euclidean norm squared ($\sum x_i^2$).
   - `positivePartNorm`: Euclidean norm of positive part ($\| [x]^+ \|_2$).
2. **Extended Sparse Matrix Primitives (`xenith/numerics/sparse_matrix`)**:
   - `rowInfinityNorms`: Row-wise $\ell_\infty$ norm computation for both CSC and CSR formats.
   - `colInfinityNorms`: Column-wise $\ell_\infty$ norm computation for both CSC and CSR formats.
   - `scaleRows`: In-place row scaling ($A_{i,:} \leftarrow d_i \cdot A_{i,:}$).
   - `scaleCols`: In-place column scaling ($A_{:,j} \leftarrow d_j \cdot A_{:,j}$).
   - `spectralNormEstimate`: Power iteration on $A^T A$ to compute $\|A\|_2$ with reproducible deterministic seeding.
3. **Automated Unit Tests**: `unit_pdlp_numerics` passing 100%.

### Phase 4B: Basic Presolve System (Completed)
1. **Presolve Infrastructure (`xenith/presolve/presolve_types`)**:
   - `PresolveAction` enum (`REMOVE_EMPTY_ROW`, `REMOVE_EMPTY_COLUMN`, `FIX_VARIABLE`, `REMOVE_SINGLETON_ROW`, `TIGHTEN_BOUND`).
   - `PresolveRecord` and `PresolveResult` structures recording transformations for exact reversal.
2. **Presolver Implementation (`xenith/presolve/presolver`)**:
   - Rule 1: Remove empty rows (and flag primal infeasibility if bounds are inconsistent with 0).
   - Rule 2: Fix variables where $u_x - l_x \le \text{tol}$, propagate shifts to row bounds, remove columns.
   - Rule 3: Remove singleton rows (rows with 1 non-zero) and tighten variable bounds.
   - Rule 4: Remove empty columns (identify unboundedness or fix to optimal bound).
3. **Postsolve Reconstruction (`Presolver::postsolve`)**:
   - LIFO stack unwinding to map reduced-space solutions back to original variable dimensions.
4. **Automated Unit Tests**: `unit_presolver` passing 100%.

### Phase 4C: Diagonal Preconditioning & Scaling (Completed)
1. **Sparse Matrix $\ell_1$ Norms (`xenith/numerics/sparse_matrix`)**:
   - `rowL1Norms`: Row-wise $\ell_1$ norm calculation for both CSC and CSR.
   - `colL1Norms`: Column-wise $\ell_1$ norm calculation for both CSC and CSR.
2. **Diagonal Scaling Subsystem (`xenith/numerics/scaling`)**:
   - `ScalingOptions`: Configurable Ruiz iterations (default 10), Pock-Chambolle toggle, and numerical scale clamping $[10^{-8}, 10^8]$.
   - `DiagonalScaler`: Equilibrates matrix $A$ to $\tilde{A} = D_1 A D_2$ via Ruiz $\ell_\infty$ scaling followed by Pock-Chambolle $\ell_1$ scaling guaranteeing $\|\tilde{A}\|_2 \le 1$.
   - `ScaledModel`: Consistent scaling of objective $c$, variable bounds $[l_x, u_x]$, and constraint bounds $[l_r, u_r]$.
   - Unscaling utilities: Exact primal ($x = D_2 \tilde{x}$), dual ($y = D_1 \tilde{y}$), and reduced cost ($\lambda = D_2^{-1} \tilde{\lambda}$) reconstruction.
3. **Automated Unit Tests**: `unit_scaling` passing 100% (5 test cases).

### Phase 4D: Matrix-Free PDLP Solver Engine Core (Completed)
1. **Types & Options (`xenith/solver/lp/pdlp_types`)**:
   - `PdlpOptions`: Configurable tolerances, max iterations, check intervals, step size factors, presolve/scaling toggles, adaptive restart toggles.
   - `PdlpResult`: Full primal/dual/reduced-cost vectors, objective values, iterations, restarts, relative residuals, and execution timing.
2. **Restarted PDHG Core Engine (`xenith/solver/lp/pdlp_solver`)**:
   - **Equality Slack Conversion**: Constructs $K = [A \;\; -I_m]$, $X = [l_x, u_x] \times [l_r, u_r]$, free dual space $Y = \mathbb{R}^m$, eliminating dual projection overhead.
   - **Step Size Sizing**: Spectral norm estimation via power iteration: $\eta = 0.9 / \|K\|_2$.
   - **Dynamic Primal Weight Balancing ($\omega$)**: Logarithmic-ratio smoothing with clipping to dynamically adjust $\tau = \eta / \omega$ and $\sigma = \eta \cdot \omega$.
   - **Running Average Tracking**: Maintains weighted running averages $(\hat{x}_{\text{avg}}, y_{\text{avg}})$ for ergodic convergence.
   - **cuPDLP-C Style Adaptive Restarts**: Senses sufficient decay ($0.2 \times$), stagnation ($0.8 \times$), and long inner loops ($0.36 k$).
   - **Relative KKT Termination**: Verifies primal feasibility, dual feasibility (with normal-cone projection for bounds), and relative duality gap simultaneously.
   - **Full Integration Pipeline**: Seamlessly chains Presolve $\to$ Scaling $\to$ PDHG Core $\to$ Unscaling $\to$ Postsolve.
3. **CLI Integration (`xenith_mps`)**:
   - Added `--method pdlp` (default primary) and `--method simplex` dispatch with distinct neon dashboard reporting.
4. **Independent Benchmark Verification**:
   - Solved Netlib benchmark instance `afiro.mps` in **20.34 ms** (840 iterations) to exact objective **`-464.753164`** (relative KKT residuals $< 10^{-6}$), independently verified as 100% valid by `LpSolutionValidator`.
5. **Automated Unit Tests**: `unit_pdlp_solver` passing 100% (4 test cases).

### Phase 4E: Netlib Benchmark Suite Runner & Performance Analysis (Completed)
1. **Netlib Integration Suite (`tests/integration/test_netlib_benchmark_suite.cpp`)**:
   - Automated benchmark runner testing `afiro`, `sc50a`, `sc50b`, `blend`, and `adlittle`.
   - Compares Revised Simplex vs PDLP wall-clock time, iteration counts, and objective correctness.
   - Computes Hans Mittelmann Shifted Geometric Mean (SGM, $s = 10\text{ ms}$) metrics.
2. **Key Benchmark Results**:
   - `afiro.mps`: Both solve to `-464.7531` (1.27x speedup).
   - `sc50a.mps`: PDLP 59 ms vs Simplex 343 ms (**5.77x speedup**).
   - `sc50b.mps`: PDLP 62 ms vs Simplex 345 ms (**5.54x speedup**).
   - `blend.mps`: PDLP 250 ms vs Simplex 9,609 ms (**38.34x speedup**).
   - `adlittle.mps`: PDLP converges to exact optimal `225497.00` in 9.8s; Simplex fails due to Phase I basis cycling.
   - **Aggregate SGM Speedup**: **6.43x speedup** over Simplex.
3. **MPS Reader Netlib Hardening (`src/xenith/io/mps/mps_reader.cpp`)**:
   - Fixed column-0 indicator card detection preventing data records named "RHS" from masquerading as section headers.
   - Replaced fixed-width substr card slicer with robust token parser preventing decimal truncation.
4. **Benchmark Documentation**: Published detailed analysis in [`docs/benchmarks/netlib_report.md`](benchmarks/netlib_report.md).
5. **Automated Integration Tests**: `integration_netlib_benchmark_suite` passing 100% (6 test cases).

---

## 📊 Current Verification State

- **Automated Test Targets**: 17 executables (`unit_vector_ops`, `unit_sparse_matrix`, `unit_scaling`, `unit_pdlp_numerics`, `unit_canonical_model`, `unit_model_validator`, `unit_mps_reader`, `unit_basis_manager`, `unit_lu_factorization`, `unit_revised_simplex`, `unit_presolver`, `unit_pdlp_solver`, `integration_model_numerics`, `integration_mps_canonical_equivalence`, `integration_mps_cli_smoke`, `integration_lp_solver_afiro`, `integration_netlib_benchmark_suite`).
- **Pass Rate**: **100% (17/17 tests pass in 20.73s)**.

---

## 🚫 EXPLICITLY NOT IMPLEMENTED YET

- ❌ GPU / CUDA acceleration (Phase 5)
- ❌ MILP branch-and-bound / cutting planes (Phase 6)
- ❌ QP solver engines (Phase 7)

---

## ⏭️ Next Step

Proceed to **Phase 5: GPU Acceleration (CUDA / cuSPARSE Kernels)** (offloading SpMV and vector operations to GPU for $10\times$–$50\times$ acceleration on massive LP instances).

