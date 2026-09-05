# XENITH Reversible Presolve & Postsolve Architecture

The **Presolve Subsystem** (`xenith/presolve/`) performs model reductions to decrease row, column, and non-zero counts before solver execution. Crucially, presolve in XENITH is strictly designed as a **reversible transformation pipeline**.

---

## 🔄 Reversible Transformation Philosophy

Presolve must **never** be a one-way destructive mutation of the model.

The exact mapping between original and reduced model spaces is preserved using a stack of atomic `PresolveOperation` objects (`ReversiblePresolveStack`):

```
Original Model (M_0)
      │
  Presolve Step 1  ── Push Op_1 to Stack
      │
  Presolve Step 2  ── Push Op_2 to Stack
      │
      ...
      │
  Presolve Step K  ── Push Op_K to Stack
      │
      ▼
Reduced Model (M_K)  ──────> Solver Engine
                                    │
                              Reduced Solution (x_K*, y_K*)
                                    │
  Postsolve Step K ── Pop Op_K & Unwind
      │
      ...
      │
  Postsolve Step 1 ── Pop Op_1 & Unwind
      │
      ▼
Original Space Solution (x_0*, y_0*)  ──────> Independent SolutionValidator
```

---

## 🧱 Reversible Presolve Operations

Each reduction type implements a bidirectional interface:

1. **Fixed Variable Elimination**:
   - *Presolve*: Variable $x_j$ pinned ($l_x[j] = u_x[j] = v$). Remove column $j$, update RHS vector $b' = b - A_{:, j} v$, accumulate constant objective offset $c_j v$.
   - *Postsolve*: Re-insert $x_j^* = v$ into primal solution vector.

2. **Singleton Row Elimination**:
   - *Presolve*: Constraint row $i$ has single non-zero entry $A_{i, j}$. Compute fixed bound for $x_j$, tighten variable bounds $l_x[j], u_x[j]$, remove row $i$.
   - *Postsolve*: Recompute dual multiplier $y_i^*$ from reduced costs of eliminated constraint.

3. **Singleton Column Substitution**:
   - *Presolve*: Variable $x_j$ appears in single constraint $i$. Express $x_j$ in terms of other row variables, substitute $x_j$ out.
   - *Postsolve*: Calculate $x_j^*$ using values of remaining row variables and original row bounds.

4. **Redundant Row Removal**:
   - *Presolve*: Constraint row $i$ implied by variable bounds ($l_{\text{implied}} \ge l_r[i]$ and $u_{\text{implied}} \le u_r[i]$). Drop row $i$.
   - *Postsolve*: Set dual multiplier $y_i^* = 0$.

5. **Coefficient Scaling**:
   - *Presolve*: Multiply row $i$ by scalar $\gamma_i$ and col $j$ by scalar $\delta_j$.
   - *Postsolve*: Unscale primal $x_j^* \leftarrow x_j^* / \delta_j$ and dual $y_i^* \leftarrow y_i^* / \gamma_i$.

---

## 🔒 Presolve Invariants

1. **Primal & Dual Equivalence**:
   $$c^T x_0^* = c_{\text{reduced}}^T x_K^* + \text{offset}_{\text{obj}}$$
2. **Stack Order Determinism**: Postsolve executes operations in strict **Last-In, First-Out (LIFO)** reverse order relative to presolve creation.
3. **Immutability of Original Model**: Presolve takes a const reference to `CanonicalModel` and outputs a distinct reduced `CanonicalModel` along with `ReversiblePresolveStack`.
