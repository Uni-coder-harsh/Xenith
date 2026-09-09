#include "xenith/solver/lp/revised_simplex_solver.hpp"
#include <iostream>
#include <sstream>
#include <cmath>
#include <limits>
#include <algorithm>

namespace xenith::solver {

RevisedSimplexSolver::RevisedSimplexSolver(SimplexOptions options)
    : m_options(std::move(options)) {}

SolveResult RevisedSimplexSolver::solve(const model::CanonicalModel& model) {
    SolveResult result;
    result.status = ModelStatus::UNINITIALIZED;

    Index m = model.numConstraints();
    Index n = model.numVariables();
    Index N = n + m; // Total expanded variables

    if (n == 0) {
        result.status = ModelStatus::OPTIMAL;
        result.objective_value = model.objectiveOffset();
        result.message = "Trivial model with 0 variables";
        return result;
    }

    // 1. Build Expanded Model Bounds and Objective
    std::vector<double> lower(N);
    std::vector<double> upper(N);
    std::vector<double> obj(N, 0.0);

    for (Index j = 0; j < n; ++j) {
        lower[j] = model.colLower()[j];
        upper[j] = model.colUpper()[j];
        double c_j = model.objective()[j];
        obj[j] = (model.sense() == ObjectiveSense::MINIMIZE) ? c_j : -c_j;
    }

    for (Index i = 0; i < m; ++i) {
        lower[n + i] = model.rowLower()[i];
        upper[n + i] = model.rowUpper()[i];
        obj[n + i] = 0.0;
    }

    // 2. Build Expanded Matrix A_bar [A  -I_m] of dimension m x N
    std::vector<numerics::Triplet> triplets;
    const auto& matrixA = model.matrixA();

    if (!matrixA.empty() && matrixA.nonZeros() > 0) {
        if (matrixA.format() == numerics::SparseStorageFormat::CSC) {
            auto col_ptr = matrixA.outerIndexPtr();
            auto row_idx = matrixA.innerIndices();
            auto vals = matrixA.values();
            Index active_cols = std::min(n, matrixA.cols());
            for (Index j = 0; j < active_cols; ++j) {
                if (j + 1 < col_ptr.size()) {
                    for (Index p = col_ptr[j]; p < col_ptr[j + 1]; ++p) {
                        triplets.push_back({row_idx[p], j, vals[p]});
                    }
                }
            }
        } else {
            auto row_ptr = matrixA.outerIndexPtr();
            auto col_idx = matrixA.innerIndices();
            auto vals = matrixA.values();
            Index active_rows = std::min(m, matrixA.rows());
            for (Index i = 0; i < active_rows; ++i) {
                if (i + 1 < row_ptr.size()) {
                    for (Index p = row_ptr[i]; p < row_ptr[i + 1]; ++p) {
                        triplets.push_back({i, col_idx[p], vals[p]});
                    }
                }
            }
        }
    }

    // Append -I_m slack columns
    for (Index i = 0; i < m; ++i) {
        triplets.push_back({i, n + i, -1.0});
    }

    numerics::SparseMatrix A_bar = numerics::SparseMatrix::fromTriplets(m, N, triplets, numerics::SparseStorageFormat::CSC);

    // 3. Initialize BasisManager
    BasisManager basis;
    basis.initializeDefault(m, N, lower, upper);

    // 4. Initialize Primal Variables x_bar
    std::vector<double> x_bar(N, 0.0);

    // Non-basic structural variables
    for (Index j = 0; j < n; ++j) {
        if (!isNegativeInfinity(lower[j])) {
            x_bar[j] = lower[j];
        } else if (!isPositiveInfinity(upper[j])) {
            x_bar[j] = upper[j];
        } else {
            x_bar[j] = 0.0;
        }
    }

    // Basic slack variables s_i = (A x)_i
    if (m > 0) {
        std::vector<double> Ax(m, 0.0);
        matrixA.multiply(std::span<const double>(x_bar.data(), n), std::span<double>(Ax.data(), m));
        for (Index i = 0; i < m; ++i) {
            x_bar[n + i] = Ax[i];
        }
    }

    // Check if initial slack basis is feasible or if Phase I is required
    bool phase_i = false;
    for (Index v = 0; v < N; ++v) {
        if (x_bar[v] < lower[v] - m_options.feasibility_tolerance ||
            x_bar[v] > upper[v] + m_options.feasibility_tolerance) {
            phase_i = true;
            break;
        }
    }

    Index iteration = 0;
    Index degenerate_count = 0;
    numerics::LuFactorization lu;

    // Helper to build Basis Matrix B from current basic variables
    auto buildBasisMatrix = [&]() -> numerics::SparseMatrix {
        std::vector<numerics::Triplet> b_triplets;
        auto basic_vars = basis.basicVariables();
        auto col_ptr = A_bar.outerIndexPtr();
        auto row_idx = A_bar.innerIndices();
        auto vals = A_bar.values();

        for (Index pos = 0; pos < m; ++pos) {
            Index var = basic_vars[pos];
            for (Index p = col_ptr[var]; p < col_ptr[var + 1]; ++p) {
                b_triplets.push_back({row_idx[p], pos, vals[p]});
            }
        }
        return numerics::SparseMatrix::fromTriplets(m, m, b_triplets, numerics::SparseStorageFormat::CSC);
    };

    // Helper to exact recompute x_B = - B^-1 N x_N to eliminate drift
    auto recomputeBasicVariables = [&](const numerics::LuFactorization& lu_fact) {
        std::vector<double> rhs(m, 0.0);
        auto col_ptr = A_bar.outerIndexPtr();
        auto row_idx = A_bar.innerIndices();
        auto vals = A_bar.values();

        for (Index j = 0; j < N; ++j) {
            if (basis.status(j) != BasisStatus::BASIC) {
                double val_j = x_bar[j];
                if (std::abs(val_j) > 1e-15) {
                    for (Index p = col_ptr[j]; p < col_ptr[j + 1]; ++p) {
                        rhs[row_idx[p]] -= vals[p] * val_j;
                    }
                }
            }
        }

        std::vector<double> x_B(m, 0.0);
        lu_fact.solveFtran(rhs, x_B);
        auto basic_vars = basis.basicVariables();
        for (Index i = 0; i < m; ++i) {
            x_bar[basic_vars[i]] = x_B[i];
        }
    };

    // Main Simplex Loop (Phase I and Phase II)
    bool in_phase_i = phase_i;

    while (iteration < m_options.max_iterations) {
        iteration++;

        // Step A: Factorize basis matrix B
        numerics::SparseMatrix B = buildBasisMatrix();
        if (!lu.factorize(B, m_options.pivot_tolerance)) {
            result.status = ModelStatus::ERROR;
            result.message = "Singular basis matrix encountered during LU factorization";
            return result;
        }

        // Recompute primal basic variables exactly from B^-1
        recomputeBasicVariables(lu);

        // Step B: Formulate Phase I / Phase II objective
        std::vector<double> c_phase(N, 0.0);
        if (in_phase_i) {
            // Check if primal values satisfy feasibility
            double total_infeasibility = 0.0;
            for (Index v = 0; v < N; ++v) {
                if (x_bar[v] < lower[v] - m_options.feasibility_tolerance) {
                    c_phase[v] = -1.0;
                    total_infeasibility += (lower[v] - x_bar[v]);
                } else if (x_bar[v] > upper[v] + m_options.feasibility_tolerance) {
                    c_phase[v] = 1.0;
                    total_infeasibility += (x_bar[v] - upper[v]);
                }
            }

            if (total_infeasibility <= m_options.feasibility_tolerance) {
                // Feasible basis found! Transition to Phase II
                in_phase_i = false;
                degenerate_count = 0;
                if (m_options.verbose) {
                    std::cout << "[XENITH Simplex] Phase I Complete at iteration " << iteration << "\n";
                }
                c_phase = obj;
            }
        } else {
            c_phase = obj;
        }

        // Step C: BTRAN Solve: B^T y = c_B
        std::vector<double> c_B(m, 0.0);
        auto basic_vars = basis.basicVariables();
        for (Index i = 0; i < m; ++i) {
            c_B[i] = c_phase[basic_vars[i]];
        }

        std::vector<double> y(m, 0.0);
        lu.solveBtran(c_B, y);

        // Step D: Compute reduced costs for non-basic variables r_j = c_j - A_col_j^T y
        Index entering_var = K_INVALID_INDEX;
        double max_violation = 0.0;

        auto col_ptr = A_bar.outerIndexPtr();
        auto row_idx = A_bar.innerIndices();
        auto vals = A_bar.values();

        bool use_bland = (degenerate_count > 10);

        for (Index j = 0; j < N; ++j) {
            BasisStatus st = basis.status(j);
            if (st == BasisStatus::BASIC || st == BasisStatus::FIXED) continue;
            if (upper[j] - lower[j] <= m_options.zero_tolerance) continue;

            // Dot product A_col_j^T y
            double dot_y = 0.0;
            for (Index p = col_ptr[j]; p < col_ptr[j + 1]; ++p) {
                dot_y += vals[p] * y[row_idx[p]];
            }
            double r_j = c_phase[j] - dot_y;

            double violation = 0.0;
            if (st == BasisStatus::NON_BASIC_AT_LOWER || (x_bar[j] <= lower[j] + m_options.feasibility_tolerance && !isNegativeInfinity(lower[j]))) {
                if (r_j < -m_options.optimality_tolerance) {
                    violation = -r_j;
                }
            } else if (st == BasisStatus::NON_BASIC_AT_UPPER || (x_bar[j] >= upper[j] - m_options.feasibility_tolerance && !isPositiveInfinity(upper[j]))) {
                if (r_j > m_options.optimality_tolerance) {
                    violation = r_j;
                }
            } else if (st == BasisStatus::FREE) {
                if (std::abs(r_j) > m_options.optimality_tolerance) {
                    violation = std::abs(r_j);
                }
            }

            if (violation > m_options.optimality_tolerance) {
                if (use_bland) {
                    // Bland's rule: select first eligible improving variable
                    entering_var = j;
                    max_violation = violation;
                    break;
                } else if (violation > max_violation) {
                    max_violation = violation;
                    entering_var = j;
                }
            }
        }

        // Step E: Check Optimality / Infeasibility Termination
        if (entering_var == K_INVALID_INDEX || max_violation <= m_options.optimality_tolerance) {
            if (in_phase_i) {
                result.status = ModelStatus::INFEASIBLE;
                result.iterations = iteration;
                result.message = "Problem is primal infeasible (Phase I terminated with non-zero infeasibility)";
                return result;
            } else {
                result.status = ModelStatus::OPTIMAL;
                result.iterations = iteration;
                result.message = "Optimal solution found";
                break;
            }
        }

        // Step F: FTRAN Step for entering variable q: B d_B = A_col_q
        std::vector<double> A_col_q(m, 0.0);
        for (Index p = col_ptr[entering_var]; p < col_ptr[entering_var + 1]; ++p) {
            A_col_q[row_idx[p]] = vals[p];
        }

        std::vector<double> d_B(m, 0.0);
        lu.solveFtran(A_col_q, d_B);

        // Determine direction of entering variable change delta_x_q
        double delta_x_q = 1.0;
        BasisStatus entering_st = basis.status(entering_var);
        if (entering_st == BasisStatus::NON_BASIC_AT_UPPER) {
            delta_x_q = -1.0;
        } else if (entering_st == BasisStatus::FREE) {
            // For free variable, direction depends on sign of reduced cost
            double dot_y = 0.0;
            for (Index p = col_ptr[entering_var]; p < col_ptr[entering_var + 1]; ++p) {
                dot_y += vals[p] * y[row_idx[p]];
            }
            double r_q = c_phase[entering_var] - dot_y;
            delta_x_q = (r_q < 0) ? 1.0 : -1.0;
        }

        // Step G: Ratio Test for step size theta*
        double theta_star = K_INFINITY;
        Index leaving_pos = K_INVALID_INDEX;
        BasisStatus leaving_new_st = BasisStatus::NON_BASIC_AT_LOWER;

        // Candidate 1: Entering variable's own bound limit
        if (delta_x_q > 0 && !isPositiveInfinity(upper[entering_var])) {
            theta_star = upper[entering_var] - x_bar[entering_var];
            leaving_new_st = BasisStatus::NON_BASIC_AT_UPPER;
        } else if (delta_x_q < 0 && !isNegativeInfinity(lower[entering_var])) {
            theta_star = x_bar[entering_var] - lower[entering_var];
            leaving_new_st = BasisStatus::NON_BASIC_AT_LOWER;
        }

        // Candidate 2: Basic variables limits
        for (Index i = 0; i < m; ++i) {
            Index basic_v = basic_vars[i];
            double d_i = -d_B[i] * delta_x_q; // Rate of change of basic_v per step theta

            double l_v = lower[basic_v];
            double u_v = upper[basic_v];
            double ratio = K_INFINITY;
            BasisStatus candidate_st = BasisStatus::NON_BASIC_AT_LOWER;

            if (in_phase_i) {
                if (x_bar[basic_v] < l_v - m_options.feasibility_tolerance) {
                    if (d_i < -m_options.pivot_tolerance) {
                        ratio = 0.0;
                        candidate_st = BasisStatus::NON_BASIC_AT_LOWER;
                    } else if (d_i > m_options.pivot_tolerance) {
                        ratio = (l_v - x_bar[basic_v]) / d_i;
                        candidate_st = BasisStatus::NON_BASIC_AT_LOWER;
                    }
                } else if (x_bar[basic_v] > u_v + m_options.feasibility_tolerance) {
                    if (d_i > m_options.pivot_tolerance) {
                        ratio = 0.0;
                        candidate_st = BasisStatus::NON_BASIC_AT_UPPER;
                    } else if (d_i < -m_options.pivot_tolerance) {
                        ratio = (u_v - x_bar[basic_v]) / d_i;
                        candidate_st = BasisStatus::NON_BASIC_AT_UPPER;
                    }
                } else {
                    if (d_i < -m_options.pivot_tolerance && !isNegativeInfinity(l_v)) {
                        ratio = std::max(0.0, (x_bar[basic_v] - l_v) / (-d_i));
                        candidate_st = BasisStatus::NON_BASIC_AT_LOWER;
                    } else if (d_i > m_options.pivot_tolerance && !isPositiveInfinity(u_v)) {
                        ratio = std::max(0.0, (u_v - x_bar[basic_v]) / d_i);
                        candidate_st = BasisStatus::NON_BASIC_AT_UPPER;
                    }
                }
            } else {
                if (d_i < -m_options.pivot_tolerance && !isNegativeInfinity(l_v)) {
                    ratio = std::max(0.0, (x_bar[basic_v] - l_v) / (-d_i));
                    candidate_st = BasisStatus::NON_BASIC_AT_LOWER;
                } else if (d_i > m_options.pivot_tolerance && !isPositiveInfinity(u_v)) {
                    ratio = std::max(0.0, (u_v - x_bar[basic_v]) / d_i);
                    candidate_st = BasisStatus::NON_BASIC_AT_UPPER;
                }
            }

            if (ratio < theta_star - m_options.zero_tolerance) {
                theta_star = ratio;
                leaving_pos = i;
                leaving_new_st = candidate_st;
            } else if (std::abs(ratio - theta_star) <= m_options.zero_tolerance && leaving_pos != K_INVALID_INDEX) {
                Index prev_leaving_var = basic_vars[leaving_pos];
                if (use_bland && basic_v < prev_leaving_var) {
                    theta_star = ratio;
                    leaving_pos = i;
                    leaving_new_st = candidate_st;
                }
            }
        }

        // Step H: Check Unboundedness
        if (isPositiveInfinity(theta_star) || theta_star >= K_INFINITY_THRESHOLD) {
            result.status = ModelStatus::UNBOUNDED;
            result.iterations = iteration;
            result.message = "Problem is primal unbounded";
            return result;
        }

        // Clamp theta_star to non-negative
        theta_star = std::max(0.0, theta_star);

        if (theta_star <= m_options.zero_tolerance) {
            degenerate_count++;
        } else {
            degenerate_count = 0;
        }

        if (m_options.verbose) {
            std::cout << "[XENITH Simplex] Iter " << iteration << ": entering=" << entering_var << " theta=" << theta_star << " leaving=" << leaving_pos << "\n";
        }

        // Step I: Perform Pivot and Update Primal Solution
        x_bar[entering_var] += theta_star * delta_x_q;
        for (Index i = 0; i < m; ++i) {
            Index basic_v = basic_vars[i];
            x_bar[basic_v] -= theta_star * d_B[i] * delta_x_q;
        }

        if (leaving_pos == K_INVALID_INDEX) {
            // Entering variable hit its opposite bound without changing basis
            basis.setStatus(entering_var, leaving_new_st);
        } else {
            // Basis pivot: entering_var replaces basic_vars[leaving_pos]
            basis.pivot(entering_var, leaving_pos, leaving_new_st);
        }
    }

    if (result.status == ModelStatus::UNINITIALIZED) {
        result.status = ModelStatus::ERROR;
        result.message = "Iteration limit reached without convergence";
        result.iterations = iteration;
        return result;
    }

    // Extract Primal Solution (first n variables)
    result.primal_solution.assign(x_bar.begin(), x_bar.begin() + n);

    // Compute Objective Value
    double raw_obj = 0.0;
    for (Index j = 0; j < n; ++j) {
        raw_obj += model.objective()[j] * result.primal_solution[j];
    }
    raw_obj += model.objectiveOffset();
    result.objective_value = raw_obj;

    // Compute Primal Infeasibility Residual: max |A x - s|
    double max_res = 0.0;
    if (m > 0) {
        std::vector<double> Ax(m, 0.0);
        matrixA.multiply(std::span<const double>(result.primal_solution.data(), n), std::span<double>(Ax.data(), m));
        for (Index i = 0; i < m; ++i) {
            double res = 0.0;
            if (Ax[i] < model.rowLower()[i] - m_options.feasibility_tolerance) {
                res = model.rowLower()[i] - Ax[i];
            } else if (Ax[i] > model.rowUpper()[i] + m_options.feasibility_tolerance) {
                res = Ax[i] - model.rowUpper()[i];
            }
            max_res = std::max(max_res, res);
        }
    }
    result.primal_residual = max_res;

    return result;
}

} // namespace xenith::solver
