# Revised Simplex Algorithm Specification

The **Revised Simplex Method** maintains the basis matrix $B$ implicitly or explicitly via factorizations ($B = L U$), avoiding full tableau updates.

---

## 🧮 Mathematical Formulation

Given an LP in standard form:
$$\min c^T x \quad \text{s.t.} \quad A x = b, \quad l_x \le x \le u_x$$

Partition variables into basic ($B$) and nonbasic ($N$) sets:
$$A_B x_B + A_N x_N = b \implies x_B = B^{-1}(b - A_N x_N)$$

Objective function value:
$$z = c_B^T x_B + c_N^T x_N = c_B^T B^{-1} b + (c_N^T - c_B^T B^{-1} A_N) x_N$$

Define dual vector $y$:
$$B^T y = c_B \implies y^T = c_B^T B^{-1}$$

Define reduced cost vector for nonbasic variable $j$:
$$\bar{c}_j = c_j - y^T A_{:,j}$$

---

## 🔄 Simplex Iteration Execution Steps

Each pivot step follows a deterministic sequence:

```mermaid
flowchart TD
    Step1["1. BTRAN (Dual Vector Solve)\nSolve B^T y = c_B"]
    Step2["2. Column Pricing (Reduced Cost Calculation)\nCompute c_j_bar = c_j - y^T a_j for nonbasic j"]
    Step3{"Optimality Check\nAre all c_j_bar >= -eps?"}
    Step4["3. Entering Variable Selection\nSelect entering column j_in (Dantzig / Steepest Edge)"]
    Step5["4. FTRAN (Direction Vector Solve)\nSolve B d = a_(j_in)"]
    Step6["5. Ratio Test (Leaving Variable Selection)\nDetermine max step length alpha and leaving variable i_out"]
    Step7{"Bounded / Unbounded?"}
    Step8["6. Basis Update & Factorization Refinements\nUpdate basis status, perform FT update on LU"]

    Step1 --> Step2
    Step2 --> Step3
    Step3 -- Yes --> Optimal["Return OPTIMAL"]
    Step3 -- No --> Step4
    Step4 --> Step5
    Step5 --> Step6
    Step6 --> Step7
    Step7 -- Unbounded --> Unbounded["Return UNBOUNDED"]
    Step7 -- Bounded --> Step8
    Step8 --> Step1
```

---

## 📊 Phase I & Phase II Strategy

1. **Phase I (Feasibility Search)**:
   - If initial starting point violates variable bounds or constraint ranges, introduce artificial variables or solve Phase I objective minimizing sum of primal infeasibilities.
2. **Phase II (Optimality Search)**:
   - Once primal feasibility is achieved ($\text{infeasibility} < \epsilon_{\text{primal}}$), transition seamlessly to optimizing the original objective function $c^T x$.
