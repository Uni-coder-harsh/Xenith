# Benchmark Instances Directory

This directory stores benchmark instances (e.g., Netlib LP dataset, MIPLIB) used during automated benchmarking runs.

---

## ⚠️ Repository Storage Policy

- Benchmark dataset files (`.mps`, `.mps.gz`, `.lp`) are **NOT** committed directly to the git repository to keep repository size small.
- Use `benchmarks/scripts/fetch_netlib.py` (to be introduced in Phase 5) to download reference instances locally into this directory.
- `benchmarks/instances/*.mps` is excluded by `.gitignore`.
