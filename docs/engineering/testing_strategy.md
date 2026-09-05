# XENITH Testing Strategy

Testing in XENITH is structured in tiers to guarantee correctness across model validation, numerical linear algebra, input parsing, and algorithm execution.

---

## 🧪 Testing Pyramid

```
                       ┌─────────────────────────┐
                       │  Regression Tests       │  (Netlib LP Suite)
                       ├─────────────────────────┤
                       │  Integration Tests      │  (End-to-end solve pipeline)
                       ├─────────────────────────┤
                       │  Unit Tests             │  (Matrices, Vectors, Presolve,
                       └─────────────────────────┘   Model validation, Factorizations)
```

---

## 🔬 Test Categories & Directories

### 1. Unit Tests (`tests/unit/`)
- **`model/`**: Test `CanonicalModel` bound checks, row additions, and invariant assertions.
- **`io/`**: Test MPS parsing of fixed and free formats on synthetic test strings.
- **`presolve/`**: Test individual presolve rules (fixed variable removal, singleton row reduction).
- **`numerics/`**: Test sparse matrix SpMV ($A x$), transpose SpMV ($A^T y$), inner products, and LU factorizations.
- **`solver/`**: Test pivot rules, pricing calculations, ratio tests, and basis updates on small numeric matrices.
- **`solution/`**: Test independent `SolutionValidator` checks.

### 2. Integration Tests (`tests/integration/`)
- Test full end-to-end pipeline: MPS Input $\rightarrow$ Canonical Model $\rightarrow$ Presolve $\rightarrow$ LP Solver $\rightarrow$ Postsolve $\rightarrow$ Solution Validation.

### 3. Regression Tests (`tests/regression/`)
- Test against known ill-conditioned matrices, degenerate LPs, and edge-case bound setups to prevent numerical regressions.
