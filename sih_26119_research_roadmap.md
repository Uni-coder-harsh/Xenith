# SIH 26119: Indigenous GPU-Accelerated Optimization Solver — Research & Roadmap

> **Last Updated:** 2026-09-08 | **Status:** Verified & Corrected (v2)

---

## 1. Problem Statement (Verified)

| Field | Detail |
| :--- | :--- |
| **Problem ID** | SIH26119 |
| **Title** | Indigenous GPU-Accelerated Optimization Solver (Sovereign Alternative to Xpress / CPLEX) |
| **Organization** | Mangalore Refinery and Petrochemicals Limited (MRPL) |
| **Domain** | Smart Automation |
| **Category** | Software Edition |
| **Initiative** | Atmanirbhar Bharat (Self-Reliant India) |

### What They're Asking For
Develop a **sovereign, numerically robust mathematical optimization solver core** that can:
1. Consistently find high-quality solutions for **large, sparse, and highly constrained industrial problems** (refinery scheduling, production planning, supply chain, blending, energy management) within practical computation times.
2. Leverage **GPU acceleration** to match or exceed the performance of open-source CPU-based alternatives.
3. Serve as a viable, indigenous alternative to foreign commercial solvers (IBM CPLEX, Gurobi, FICO Xpress).

> [!IMPORTANT]
> The official problem statement explicitly says the **focus should be on the optimization engine/core itself**, not on building a modeling interface or a front-end dashboard. Judges want algorithmic depth.

---

## 2. Technical Background

### 2.1 What is a Mathematical Optimization Solver?
A solver takes a mathematical model — an **objective function** (minimize cost / maximize profit), **decision variables** (quantities to determine), and **constraints** (limits and rules) — and finds the optimal solution.

| Problem Type | Variables | Constraints/Objective | Industrial Example |
| :--- | :--- | :--- | :--- |
| **Linear Programming (LP)** | Continuous (real-valued) | All linear | Refinery crude blending |
| **Mixed-Integer LP (MILP)** | Some integer, some continuous | All linear | Production scheduling (on/off decisions) |
| **Quadratic Programming (QP)** | Continuous | Quadratic objective, linear constraints | Portfolio optimization |
| **Non-Linear Programming (NLP)** | Continuous | Non-linear | Reactor modeling |

For MRPL's industrial use cases, **LP and MILP are the most critical**.

### 2.2 Why is Building a Solver Extremely Hard?
Commercial solvers like CPLEX (IBM) and Gurobi have **30+ years of continuous R&D investment**. Their advantage comes from:
- Highly optimized **Simplex** and **Interior Point Method (IPM)** implementations.
- Sophisticated **pre-solvers** that can reduce problem sizes by 50–90% before solving.
- Advanced **Branch & Bound / Branch & Cut** frameworks with proprietary heuristics and cutting planes for MILP.
- Decades of numerical stability engineering.

> [!NOTE]
> Even the best open-source CPU solvers (HiGHS, SCIP, CBC) are typically **1–2 orders of magnitude slower** than commercial solvers on complex, large-scale MILP problems. The GPU angle is the key to closing this gap.

### 2.3 Why GPU Acceleration?
Traditional algorithms are hard to parallelize on GPUs:
- **Simplex** relies on sequential pivot operations — nearly impossible to efficiently GPU-parallelize.
- **Interior Point Methods** require sparse matrix factorizations — partially parallelizable but memory-intensive.

Modern GPU-friendly approaches shift the algorithmic paradigm:

| Approach | How It Uses GPU | Maturity |
| :--- | :--- | :--- |
| **First-Order Methods (PDHG/PDLP)** | Core operations are massive sparse matrix-vector multiplications — perfect for GPU | Production-ready (Google PDLP, cuPDLP-C, NVIDIA cuOpt) |
| **GPU-Accelerated Barrier (IPM)** | Sparse Cholesky factorization on GPU via cuSolver/cuSPARSE | Production-ready (NVIDIA cuOpt 26.02+) |
| **Parallel Branch & Bound** | Expand the MILP search tree in parallel, solving thousands of LP sub-problems simultaneously | Active research / beta (cuOpt MIP is beta) |

### 2.4 Accuracy vs. Speed Tradeoff (Critical!)

> [!WARNING]
> First-order methods (like PDHG) trade **precision for speed**. They converge to an approximate solution very quickly but getting the last few decimal places of accuracy can be much slower than traditional methods. For industrial LP problems where solutions must be numerically tight, you must implement:
> - **Adaptive restart schemes** (to reset stalled convergence)
> - **Iterative refinement** (to polish solutions post-convergence)
> - **FP64 (double precision)** throughout — GPUs default to FP32, but optimization demands FP64 to avoid catastrophic numerical drift.
>
> Judges **will** ask about this. Have a clear answer.

---

## 3. Competitive Landscape (Know What Already Exists)

> [!CAUTION]
> Ignoring existing work will destroy your credibility with the jury. You must demonstrate awareness of these projects and clearly articulate what your solver does differently or better.

### Open-Source CPU Solvers

| Solver | Strengths | Weaknesses |
| :--- | :--- | :--- |
| **HiGHS** | Fastest open-source LP/MILP/QP solver; integrated into SciPy; modern codebase | CPU-only; no GPU acceleration |
| **SCIP** | Best open-source MILP solver (non-commercial); rich constraint programming features | CPU-only; complex codebase |
| **CBC (COIN-OR)** | Mature; deeply integrated into PuLP/Pyomo | Slower than HiGHS on most benchmarks |
| **GLPK** | Simple, stable, educational | Significantly slower; not competitive for industrial scale |

### GPU-Accelerated Solvers

| Solver | Details | License |
| :--- | :--- | :--- |
| **NVIDIA cuOpt** | World's fastest open-source LP solver (2026 Mittelmann benchmarks). Includes GPU barrier method, PDHG, multi-GPU support. MIP solver in beta. | **Apache 2.0** (open-source as of 2025) |
| **cuPDLP-C** | Focused GPU PDHG solver for LP. C/CUDA implementation. Python interface (`pycupdlp`). Requires HiGHS ≥1.6.0. | Open-source (GitHub: COPT-Public/cuPDLP-C) |
| **cuPDLP.jl** | Original Julia implementation of GPU PDHG. Research-oriented. | Open-source |
| **Google PDLP** | CPU-based first-order LP solver inside OR-Tools. Highly scalable. | Apache 2.0 (part of OR-Tools) |

### What This Means for Your Strategy
NVIDIA cuOpt is now open-source and already solves the LP part of this problem. Your sovereign solver must either:
1. **Build on top of cuOpt/cuPDLP-C** and add value (better MILP, Indian industrial benchmarks, Pyomo/PuLP integration, custom pre-solvers for refinery problems), or
2. **Build from scratch** using only open-source math libraries (cuBLAS, cuSPARSE) to demonstrate true algorithmic innovation.

Option 2 is harder but more impressive for the hackathon jury.

---

## 4. Sovereignty vs. NVIDIA Dependency — An Honest Assessment

> [!IMPORTANT]
> The original roadmap claimed "zero dependencies on proprietary foreign math libraries." This is **misleading**. CUDA, cuBLAS, and cuSPARSE are all NVIDIA (American company) proprietary ecosystems. While freely available, they are not "sovereign."
>
> **How to handle this with judges:**
> - Acknowledge the CUDA dependency honestly. The sovereignty claim applies to the **solver algorithm and codebase** — your optimization logic, pre-solvers, and B&B framework are 100% Indian-developed and open-source.
> - Frame it as: "We own and control the optimization intelligence. The hardware abstraction layer (CUDA) is a standard, replaceable interface — analogous to how Indian software runs on Intel CPUs without compromising sovereignty."
> - **Bonus:** Mention that a future roadmap includes OpenCL or SYCL backends for hardware-agnostic GPU support (Intel, AMD, Indian-designed GPUs).

---

## 5. Roadmap — Restructured for SIH Timeline

> [!NOTE]
> SIH is a **36-hour hackathon** at the grand finale, with a few weeks of preparation time beforehand. The original 12-week phased timeline was unrealistic for this format. Below is a restructured plan.

### Pre-Hackathon Preparation (4–6 Weeks Before Grand Finale)

#### Week 1–2: Research & Architecture
- [ ] Deep-dive into the PDHG algorithm (read the Google PDLP paper + cuPDLP-C source code).
- [ ] Study the MPS/LP file format specification.
- [ ] Set up the development environment: C++ 17, CUDA Toolkit 12+, CMake, Python 3.10+, pybind11.
- [ ] Design the solver architecture (modular: Parser → Pre-solver → LP Engine → MILP Engine → API).

#### Week 3–4: Core LP Engine (GPU)
- [ ] Implement a CPU-based PDHG solver in C++ for correctness testing.
- [ ] Port core operations (sparse matrix-vector multiply, vector updates, projection) to CUDA kernels.
- [ ] Use **CSR (Compressed Sparse Row)** format for sparse matrices — industrial problems are >99% sparse.
- [ ] Validate against Netlib LP benchmark instances (small problems, verify correctness).

#### Week 5–6: Pre-solver + MPS Parser + Basic MILP
- [ ] Build an MPS file parser in C++.
- [ ] Implement CPU-based pre-solve routines:
  - Remove empty rows/columns.
  - Fix variables with equal bounds.
  - Remove singleton rows.
  - Bound tightening.
- [ ] Implement a basic **Branch & Bound** framework for MILP:
  - Variable selection (most fractional).
  - Node LP relaxation solved by your GPU engine.
  - Simple rounding heuristic for feasible solutions.
- [ ] Build Python bindings with `pybind11`.

### During the 36-Hour Hackathon

#### Hours 1–4: Setup & Integration Demo
- [ ] Set up the environment on hackathon hardware (ensure GPU drivers + CUDA work).
- [ ] Run a pre-prepared demo solving a Netlib LP problem on GPU.
- [ ] Show Python API: `from indig_solver import Solver; s = Solver(); s.read("problem.mps"); s.solve()`.

#### Hours 5–18: Polish & Benchmarks
- [ ] Run benchmarks against HiGHS and CBC on 10–20 Netlib/MIPLIB instances.
- [ ] Generate speedup charts (GPU vs CPU solve times).
- [ ] Write PuLP/Pyomo adapter so judges can see drop-in replacement capability.
- [ ] Handle edge cases: infeasible problems, unbounded problems, timeout handling.

#### Hours 19–28: MILP + Refinery Use Case
- [ ] Demonstrate MILP solving on a simplified refinery scheduling problem.
- [ ] If time permits, implement a **cutting plane** (Gomory cuts) to strengthen the MILP solver.
- [ ] Build a simple web dashboard showing: problem stats, solve progress, solution visualization.

#### Hours 29–36: Presentation & Documentation
- [ ] Prepare the PPT/demo with:
  - Architecture diagram.
  - Benchmark results (tables + charts).
  - Live demo solving a refinery-scale problem.
  - Sovereignty narrative + future roadmap.
- [ ] Record a backup demo video in case of hardware failure.

---

## 6. Tech Stack Summary

| Layer | Technology | Purpose |
| :--- | :--- | :--- |
| **Language** | C++ 17 | Core solver engine |
| **GPU** | CUDA 12+ | GPU kernel programming |
| **GPU Math** | cuBLAS, cuSPARSE | Sparse/dense linear algebra on GPU |
| **Build** | CMake | Cross-platform build system |
| **Python API** | pybind11 | Python bindings for the C++ solver |
| **Modeling** | PuLP / Pyomo adapter | Drop-in replacement interface |
| **Benchmarks** | Netlib LP, MIPLIB 2017 | Standard problem libraries |
| **Visualization** | Matplotlib / Plotly | Benchmark charts for presentation |
| **Dashboard (optional)** | Flask + HTML/JS | Live solve monitoring |

---

## 7. Key Differentiators to Win SIH

1. **Algorithmic Depth > UI Polish:** This is a solver problem. Judges want to see that you understand PDHG, convergence theory, and numerical stability — not a fancy React dashboard.
2. **Benchmarks are Everything:** Show concrete numbers. "Our GPU solver solves Netlib instance `pilot87` in 0.3s vs HiGHS at 1.2s" is worth more than any slide.
3. **Sparse Matrix Mastery:** Real MRPL problems have millions of variables but >99% sparsity. Your solver *must* use sparse representations (CSR/CSC) or it will crash on real data from VRAM exhaustion.
4. **Seamless Python API + PuLP/Pyomo Integration:** If a refinery data scientist can change `solver=CPLEX()` → `solver=IndigSolver()` in one line, that's a winning demo.
5. **Honest Sovereignty Narrative:** Acknowledge CUDA dependency, emphasize algorithmic sovereignty, mention OpenCL/SYCL future roadmap.
6. **Know Your Competition:** Reference cuOpt, cuPDLP-C, HiGHS by name. Show you know the landscape and explain what your solver adds.

---

## 8. Resources & References

### Must-Read Papers
- **"Practical Large-Scale Linear Programming using Primal-Dual Hybrid Gradient"** — Applegate et al. (Google Research). The foundational paper for GPU-friendly LP solving.
- **"cuPDLP Revisited: Yet Faster First-Order LP Solvers"** — Lu & Yang. Details on the C/CUDA implementation and performance optimizations.

### Open-Source Code to Study
- **cuPDLP-C** — [github.com/COPT-Public/cuPDLP-C](https://github.com/COPT-Public/cuPDLP-C) — The closest reference implementation to what you're building.
- **NVIDIA cuOpt** — [github.com/NVIDIA/cuOpt](https://github.com/NVIDIA/cuOpt) — World's fastest open-source LP solver. Apache 2.0.
- **HiGHS** — [github.com/ERGO-Code/HiGHS](https://github.com/ERGO-Code/HiGHS) — Best open-source CPU solver. Study its architecture, pre-solver, and MPS parser.
- **Google OR-Tools PDLP** — [github.com/google/or-tools](https://github.com/google/or-tools) — CPU-based PDLP implementation inside OR-Tools.

### Benchmark Datasets
- **Netlib LP** — [netlib.org/lp](http://www.netlib.org/lp/) — Standard LP benchmark set (~100 instances).
- **MIPLIB 2017** — [miplib.zib.de](https://miplib.zib.de/) — Standard MILP benchmark set.
- **Hans Mittelmann Benchmarks** — [plato.asu.edu/bench.html](http://plato.asu.edu/bench.html) — The gold standard for solver performance comparison.

### CUDA Development
- **CUDA Toolkit Documentation** — Specifically cuSPARSE (sparse matrix ops) and cuBLAS (dense BLAS).
- **CUDA C++ Programming Guide** — For writing custom kernels (vector updates, projections).

---

## 9. Corrections Log (v1 → v2)

| # | What Was Wrong | Correction |
| :--- | :--- | :--- |
| 1 | Title had "CEPLEX" (typo) | Corrected to **CPLEX** |
| 2 | Domain listed as "Software / Computational Mathematics" | Corrected to **Smart Automation** (category: Software Edition) |
| 3 | 12-week roadmap — unrealistic for SIH format | Restructured to **6-week prep + 36-hour hackathon** |
| 4 | No mention of existing GPU solvers (cuOpt, cuPDLP-C) | Added full **Competitive Landscape** section |
| 5 | Claimed "zero dependencies on proprietary foreign math libraries" | Added honest **Sovereignty vs NVIDIA Dependency** analysis |
| 6 | Missing accuracy vs. speed tradeoff for first-order methods | Added **critical warning** about PDHG precision limitations |
| 7 | Resources section was too thin | Expanded with specific papers, GitHub repos, and benchmark datasets |
| 8 | Missing SCIP in open-source solver landscape | Added SCIP as leading open-source MILP solver |
| 9 | Problem statement focus not explicit | Added note that judges want the **engine/core**, not a dashboard |
