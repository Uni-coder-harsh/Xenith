# XENITH Canonical Model Specification

The **Canonical Model** (`xenith/model/CanonicalModel`) is the central mathematical representation in XENITH. All input formats (MPS, LP format, Python/C++ APIs) map into this uniform intermediate structure before model validation, presolve, or solving occurs.

---

## 📐 Mathematical Formulation

XENITH adopts a **Bounded-Row Canonical LP Form** with extensions for Quadratic Objective matrices ($Q$) and Integrality Metadata:

$$\begin{aligned}
\min \quad & c^T x + \frac{1}{2} x^T Q x \quad (\text{or } \max) \\
\text{subject to} \quad & l_r \le A x \le u_r \\
\text{and} \quad & l_x \le x \le u_x
\end{aligned}$$

where:
- $x \in \mathbb{R}^n$ is the decision variable vector of length $n$.
- $c \in \mathbb{R}^n$ is the linear objective coefficient vector.
- $Q \in \mathbb{R}^{n \times n}$ is an optional sparse symmetric matrix representing quadratic objective terms (empty for pure LP/MILP).
- $A \in \mathbb{R}^{m \times n}$ is the sparse constraint matrix with $m$ rows and $n$ columns.
- $l_r, u_r \in (\mathbb{R} \cup \{-\infty, +\infty\})^m$ are row lower and upper bound vectors.
- $l_x, u_x \in (\mathbb{R} \cup \{-\infty, +\infty\})^n$ are variable lower and upper bound vectors.

---

## 💡 Why the Bounded-Row Representation?

Traditional textbook LP implementations convert constraints into standard form $A x = b, x \ge 0$ using slack variables prior to solver entry. XENITH maintains the bounded-row form in its canonical model for several key reasons:

1. **Native Constraint Representation**:
   - Less-than-or-equal ($\le$): $l_r = -\infty, u_r = b_i$
   - Greater-than-or-equal ($\ge$): $l_r = b_i, u_r = +\infty$
   - Equality ($=$): $l_r = u_r = b_i$
   - Range constraints: $-\infty < l_r < u_r < +\infty$
   - Free rows (objective or unconstrained): $l_r = -\infty, u_r = +\infty$
2. **Compact Storage**: Range constraints do not require adding explicit artificial variables or extra constraint rows during model setup.
3. **Presolve Friendly**: Bound tightening on rows and variables directly modifies $l_r, u_r, l_x, u_x$ without changing matrix topology.
4. **Unified Mapping**: MPS `ROWS`, `RANGES`, and `BOUNDS` sections map losslessly into this formulation.

---

## 🏷️ Future MILP & QP Extensions

1. **Integrality Metadata (MILP)**:
   Each variable $x_j$ carries a type metadata field (`CONTINUOUS`, `GENERAL_INTEGER`, `BINARY`, `SEMI_CONTINUOUS`, `SEMI_INTEGER`).
   *Note: LP solver routines ignore integrality metadata, treating all variables as continuous. MILP branch-and-bound inspects integrality flags to drive branching decisions.*

2. **Quadratic Objective Matrix $Q$ (QP)**:
   The optional $Q$ matrix stores quadratic terms $\frac{1}{2} x_i Q_{ij} x_j$. When empty ($\text{nnz}(Q) = 0$), the model defaults cleanly to pure Linear Programming without runtime checks or memory overhead in LP code paths.

---

## 🔒 Invariants Maintained by Canonical Model

| Invariant Category | Rule & Enforcement |
| :--- | :--- |
| **Dimensional Consistency** | $\text{len}(c) = \text{len}(l_x) = \text{len}(u_x) = \text{len}(\text{var\_types}) = n$; $\text{len}(l_r) = \text{len}(u_r) = m$; $\text{dim}(A) = m \times n$. |
| **Bound Validity** | $l_x[j] \le u_x[j] \ \forall j$; $l_r[i] \le u_r[i] \ \forall i$. Invalid bounds flag model infeasible during validation. |
| **Finite Matrix Entries** | Matrix $A$ non-zeros must be finite non-zero real numbers ($|A_{ij}| > 0$, no $\text{NaN}$ or $\pm\infty$). |
| **Unique Identifiers** | Variable and constraint string names must be unique; bidirectional string-to-index maps are maintained. |
| **Symmetric Positive Semi-Definite $Q$** | If $Q$ is non-empty, $Q$ must be symmetric ($Q_{ij} = Q_{ji}$). |

---

## 🧠 Memory Ownership & Mutability Rules

- `CanonicalModel` owns its numerical buffers exclusively (`std::vector<double>`, `SparseMatrix`).
- Model ownership is **immutable** once passed into `Presolve` or `SolverManager`. Presolve transforms `CanonicalModel` by creating a new reduced instance rather than mutating original data in place.
