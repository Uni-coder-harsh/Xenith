# XENITH Benchmarking Strategy

Benchmarking is a first-class subsystem in XENITH. Optimization solvers cannot be evaluated on toy problems alone; rigorous performance tracking against standard benchmark suites (such as Netlib LP) is essential for empirical verification.

---

## 🎯 Benchmark Objectives

XENITH avoids unsupported claims like "Xenith beats Gurobi." Benchmarking serves objective quantitative goals:

1. **Mathematical Correctness**: Verify objective values match published standard reference values to within numerical tolerance ($\epsilon < 10^{-5}$).
2. **Feasibility Verification**: Ensure primal and dual residual violations remain bounded.
3. **Iteration Counts**: Track simplex iteration efficiency across pricing strategies.
4. **Runtime Performance**: Measure wall-clock time, CPU cycles, and factorization overhead.
5. **Memory Footprint**: Monitor peak heap usage and sparse matrix memory efficiency.
6. **Numerical Scaling Behavior**: Measure performance degradation as problem size ($m, n, \text{nnz}$) increases.

---

## 📂 Benchmark Instance Isolation

- Large benchmark MPS files (e.g., Netlib LP problems like `afiro`, `25fv47`, `maros`, `fit2p`) are **never checked directly into the git source repository**.
- Benchmark datasets are fetched on-demand using scripts in `benchmarks/scripts/fetch_netlib.py` and stored locally in `benchmarks/instances/`.

---

## 📊 Automated Benchmark Metric Framework

```mermaid
flowchart TD
    A["Benchmark Instance (.mps)"] --> B["XENITH Benchmark Runner"]
    B --> C["Collect Execution Metrics"]
    
    subgraph Metrics Tracked
        M1["Primal/Dual Feasibility Residuals"]
        M2["Objective Error vs Reference Value"]
        M3["Solve Time (ms)"]
        M4["Simplex Iterations & Refactorizations"]
        M5["Peak RAM Usage (MB)"]
    end
    
    C --> Metrics Tracked
    Metrics Tracked --> D["JSON / CSV Benchmark Report"]
```

---

## ⚖️ Comparative Evaluation Methodology

- Benchmark runs must be **deterministic and reproducible**.
- Comparisons against standard open-source solvers (e.g., HiGHS) run under identical hardware parameters, single-threaded execution, and standardized numerical tolerance settings.
