#include "xenith/solver/lp/pdlp_solver.hpp"
#include "xenith/numerics/vector_ops.hpp"
#include "xenith/numerics/sparse_matrix.hpp"
#include "xenith/numerics/scaling.hpp"
#include "xenith/presolve/presolver.hpp"
#include <chrono>
#include <cmath>
#include <algorithm>
#include <iostream>

namespace xenith::solver {

PdlpSolver::PdlpSolver(PdlpOptions options)
    : m_options(std::move(options)) {}

PdlpResult PdlpSolver::solve(const model::CanonicalModel& model) {
    auto start_time = std::chrono::high_resolution_clock::now();
    PdlpResult result;

    Index n = model.numVariables();

    if (n == 0) {
        result.status = ModelStatus::OPTIMAL;
        result.objective_value = model.objectiveOffset();
        result.message = "Trivial model with 0 variables";
        auto end_time = std::chrono::high_resolution_clock::now();
        result.solve_time_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();
        return result;
    }

    // Step 1: Presolve (if enabled)
    model::CanonicalModel active_model = model;
    presolve::PresolveResult presolve_res;
    bool presolve_applied = false;

    if (m_options.enable_presolve) {
        presolve::Presolver presolver;
        presolve_res = presolver.presolve(model);
        if (presolve_res.was_infeasible) {
            result.status = ModelStatus::INFEASIBLE;
            result.message = "Model proven infeasible during presolve: " + presolve_res.message;
            auto end_time = std::chrono::high_resolution_clock::now();
            result.solve_time_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();
            return result;
        }
        active_model = presolve_res.reduced_model;
        presolve_applied = true;
    }

    // Step 2: Scaling / Preconditioning (if enabled)
    std::vector<double> raw_primal;
    std::vector<double> raw_dual;
    std::vector<double> raw_rc;
    PdlpResult core_res;

    if (m_options.enable_scaling) {
        numerics::ScalingOptions scale_opts;
        scale_opts.ruiz_iterations = m_options.ruiz_iterations;
        scale_opts.enable_pock_chambolle = m_options.enable_pock_chambolle;
        numerics::DiagonalScaler scaler(scale_opts);
        numerics::ScaledModel scaled_model = scaler.scale(active_model);

        core_res = solveCore(scaled_model.model);

        if (core_res.status == ModelStatus::OPTIMAL) {
            // Unscale solutions
            raw_primal.resize(scaled_model.model.numVariables());
            numerics::DiagonalScaler::unscalePrimal(core_res.primal_solution, scaled_model.col_scale, raw_primal);

            raw_dual.resize(scaled_model.model.numConstraints());
            numerics::DiagonalScaler::unscaleDual(core_res.dual_solution, scaled_model.row_scale, raw_dual);

            raw_rc.resize(scaled_model.model.numVariables());
            numerics::DiagonalScaler::unscaleReducedCosts(core_res.reduced_costs, scaled_model.inv_col_scale, raw_rc);
        } else {
            result = core_res;
            auto end_time = std::chrono::high_resolution_clock::now();
            result.solve_time_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();
            return result;
        }
    } else {
        core_res = solveCore(active_model);
        if (core_res.status == ModelStatus::OPTIMAL) {
            raw_primal = core_res.primal_solution;
            raw_dual = core_res.dual_solution;
            raw_rc = core_res.reduced_costs;
        } else {
            result = core_res;
            auto end_time = std::chrono::high_resolution_clock::now();
            result.solve_time_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();
            return result;
        }
    }

    // Step 3: Postsolve (if presolve was applied)
    if (presolve_applied) {
        result.primal_solution = presolve::Presolver::postsolve(raw_primal, presolve_res);
    } else {
        result.primal_solution = std::move(raw_primal);
    }

    result.dual_solution = std::move(raw_dual);
    result.reduced_costs = std::move(raw_rc);
    result.status = core_res.status;
    result.iterations = core_res.iterations;
    result.restarts = core_res.restarts;
    result.primal_residual = core_res.primal_residual;
    result.dual_residual = core_res.dual_residual;
    result.duality_gap = core_res.duality_gap;
    result.message = core_res.message;

    // Compute exact objective value on the original model
    double obj = 0.0;
    for (Index j = 0; j < n; ++j) {
        obj += model.objective()[j] * result.primal_solution[j];
    }
    obj += model.objectiveOffset();
    result.objective_value = obj;

    auto end_time = std::chrono::high_resolution_clock::now();
    result.solve_time_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();
    return result;
}

PdlpResult PdlpSolver::solveCore(const model::CanonicalModel& model) {
    PdlpResult result;
    result.status = ModelStatus::UNINITIALIZED;

    Index n = model.numVariables();
    Index m = model.numConstraints();
    Index N = n + m; // Expanded variables: [x, s]

    if (n == 0) {
        result.status = ModelStatus::OPTIMAL;
        result.objective_value = 0.0;
        return result;
    }

    // Formulate expanded problem: min c_hat^T x_hat s.t. K x_hat = 0, l <= x_hat <= u
    std::vector<double> c_hat(N, 0.0);
    std::vector<double> lower(N);
    std::vector<double> upper(N);

    for (Index j = 0; j < n; ++j) {
        double c_j = model.objective()[j];
        c_hat[j] = (model.sense() == ObjectiveSense::MINIMIZE) ? c_j : -c_j;
        lower[j] = model.colLower()[j];
        upper[j] = model.colUpper()[j];
    }

    for (Index i = 0; i < m; ++i) {
        c_hat[n + i] = 0.0;
        lower[n + i] = model.rowLower()[i];
        upper[n + i] = model.rowUpper()[i];
    }

    // Build K = [A  -I_m] of dimension m x N
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

    for (Index i = 0; i < m; ++i) {
        triplets.push_back({i, n + i, -1.0});
    }

    numerics::SparseMatrix K = numerics::SparseMatrix::fromTriplets(m, N, triplets, numerics::SparseStorageFormat::CSC);

    // Compute spectral norm of K via power iteration
    double norm_K = K.spectralNormEstimate(50);
    if (norm_K < 1e-4) norm_K = 1.0;

    double eta = m_options.step_size_factor / norm_K;

    // Estimate scale of objective and bounds for primal weight initialization
    double norm_c = numerics::euclideanNorm(model.objective());
    double norm_b = 0.0;
    for (Index i = 0; i < m; ++i) {
        double val = 0.0;
        if (isFinite(model.rowLower()[i])) val = std::max(val, std::abs(model.rowLower()[i]));
        if (isFinite(model.rowUpper()[i])) val = std::max(val, std::abs(model.rowUpper()[i]));
        norm_b += val * val;
    }
    norm_b = std::sqrt(norm_b);

    double omega = (norm_c > 1e-4 && norm_b > 1e-4) ? (norm_c / norm_b) : 1.0;
    omega = std::clamp(omega, 1e-4, 1e4);

    double tau = eta / omega;
    double sigma = eta * omega;

    // Initialize iterates x_hat and y
    std::vector<double> x(N, 0.0);
    std::vector<double> zero_vec(N, 0.0);
    numerics::projectBox(zero_vec, lower, upper, x);

    std::vector<double> y(m, 0.0);

    // Running average accumulator
    std::vector<double> x_avg = x;
    std::vector<double> y_avg = y;
    double avg_weight_sum = 1.0;

    std::vector<double> x_last_restart = x;
    std::vector<double> y_last_restart = y;

    // Working buffers to avoid reallocations in the inner loop
    std::vector<double> K_T_y(N, 0.0);
    std::vector<double> step_x(N, 0.0);
    std::vector<double> x_next(N, 0.0);
    std::vector<double> x_bar(N, 0.0);
    std::vector<double> K_x_bar(m, 0.0);

    // Diagnostics buffers
    std::vector<double> K_x_avg(m, 0.0);
    std::vector<double> K_T_y_avg(N, 0.0);

    Index iter = 0;
    Index last_restart_iter = 0;
    Index restart_count = 0;
    double last_restart_kkt = 1e10;
    double prev_check_kkt = 1e10;

    double best_kkt = 1e10;
    std::vector<double> best_x = x;
    std::vector<double> best_y = y;

    for (iter = 1; iter <= m_options.max_iterations; ++iter) {
        // 1. K^T * y
        K.multiplyTranspose(y, K_T_y);

        // 2. Primal update: x_{k+1} = proj_X( x_k - tau * (c_hat - K^T y_k) )
        for (Index j = 0; j < N; ++j) {
            step_x[j] = x[j] - tau * (c_hat[j] - K_T_y[j]);
        }
        numerics::projectBox(step_x, lower, upper, x_next);

        // 3. Extrapolation: x_bar = 2 * x_{k+1} - x_k
        for (Index j = 0; j < N; ++j) {
            x_bar[j] = 2.0 * x_next[j] - x[j];
        }

        // 4. Dual update: y_{k+1} = y_k - sigma * K * x_bar
        K.multiply(x_bar, K_x_bar);
        for (Index i = 0; i < m; ++i) {
            y[i] -= sigma * K_x_bar[i];
        }

        // 5. Advance iterate
        x = x_next;

        // 6. Running average update
        avg_weight_sum += 1.0;
        double frac = 1.0 / avg_weight_sum;
        for (Index j = 0; j < N; ++j) {
            x_avg[j] += frac * (x[j] - x_avg[j]);
        }
        for (Index i = 0; i < m; ++i) {
            y_avg[i] += frac * (y[i] - y_avg[i]);
        }

        // 7. Periodic convergence and restart evaluation
        if (iter % m_options.check_interval == 0 || iter == m_options.max_iterations) {
            // Primal residual: ||K * x_avg||_2 / (1 + ||b||_2)
            K.multiply(x_avg, K_x_avg);
            double prim_res_norm = numerics::euclideanNorm(K_x_avg);
            double rel_prim_res = prim_res_norm / (1.0 + norm_b);

            // Dual residual & reduced costs
            K.multiplyTranspose(y_avg, K_T_y_avg);
            double dual_res_sq = 0.0;
            std::vector<double> lambda(N, 0.0);

            for (Index j = 0; j < N; ++j) {
                double g_j = c_hat[j] - K_T_y_avg[j];
                double lj = lower[j];
                double uj = upper[j];

                if (isFinite(lj) && isFinite(uj) && (uj - lj <= 1e-9)) {
                    // Fixed variable or equality slack: normal cone is R, reduced cost is free
                    lambda[j] = g_j;
                } else if (x_avg[j] <= lj + 1e-6 && !isNegativeInfinity(lj)) {
                    lambda[j] = std::max(0.0, g_j);
                } else if (x_avg[j] >= uj - 1e-6 && !isPositiveInfinity(uj)) {
                    lambda[j] = std::min(0.0, g_j);
                } else {
                    lambda[j] = 0.0;
                }

                double viol = g_j - lambda[j];
                dual_res_sq += viol * viol;
            }
            double dual_res_norm = std::sqrt(dual_res_sq);
            double rel_dual_res = dual_res_norm / (1.0 + norm_c);

            // Duality gap
            double prim_obj = 0.0;
            for (Index j = 0; j < n; ++j) {
                prim_obj += c_hat[j] * x_avg[j];
            }

            double dual_obj = 0.0;
            for (Index j = 0; j < N; ++j) {
                double lj = lower[j];
                double uj = upper[j];
                double lam = lambda[j];

                if (isFinite(lj) && isFinite(uj) && (uj - lj <= 1e-9)) {
                    dual_obj += lam * lj;
                } else if (lam > 0.0 && !isNegativeInfinity(lj)) {
                    dual_obj += lam * lj;
                } else if (lam < 0.0 && !isPositiveInfinity(uj)) {
                    dual_obj += lam * uj;
                }
            }

            double gap = std::abs(prim_obj - dual_obj);
            double rel_gap = gap / (1.0 + std::abs(prim_obj) + std::abs(dual_obj));

            double current_kkt = std::max({rel_prim_res, rel_dual_res, rel_gap});

            if (current_kkt < best_kkt) {
                best_kkt = current_kkt;
                best_x = x_avg;
                best_y = y_avg;
            }

            if (m_options.verbose) {
                std::cout << "[XENITH PDLP] Iter " << iter
                          << ": KKT=" << current_kkt
                          << " (prim=" << rel_prim_res
                          << ", dual=" << rel_dual_res
                          << ", gap=" << rel_gap << ")"
                          << " omega=" << omega << "\n";
            }

            // Check optimality termination
            if (current_kkt <= m_options.tolerance) {
                result.status = ModelStatus::OPTIMAL;
                result.iterations = iter;
                result.restarts = restart_count;
                result.primal_residual = rel_prim_res;
                result.dual_residual = rel_dual_res;
                result.duality_gap = rel_gap;
                result.message = "Optimal solution found within tolerance";
                break;
            }

            // Adaptive Restart Logic
            if (m_options.enable_adaptive_restart) {
                bool trigger_restart = false;

                if (current_kkt <= 0.2 * last_restart_kkt) {
                    trigger_restart = true;
                } else if (current_kkt <= 0.8 * last_restart_kkt && current_kkt > prev_check_kkt) {
                    trigger_restart = true;
                } else if (iter - last_restart_iter >= static_cast<Index>(0.36 * static_cast<double>(iter))) {
                    trigger_restart = true;
                }

                if (trigger_restart) {
                    restart_count++;

                    // Primal weight update
                    if (m_options.enable_primal_weight_update) {
                        double dx = 0.0, dy = 0.0;
                        for (Index j = 0; j < N; ++j) {
                            double diff = x_avg[j] - x_last_restart[j];
                            dx += diff * diff;
                        }
                        for (Index i = 0; i < m; ++i) {
                            double diff = y_avg[i] - y_last_restart[i];
                            dy += diff * diff;
                        }
                        dx = std::sqrt(dx);
                        dy = std::sqrt(dy);

                        if (dx > 1e-8 && dy > 1e-8) {
                            double log_ratio = std::clamp(std::log(dy / dx), -0.5, 0.5);
                            omega *= std::exp(0.5 * log_ratio);
                            omega = std::clamp(omega, 0.01, 100.0);
                            tau = eta / omega;
                            sigma = eta * omega;
                        }
                    }

                    x = x_avg;
                    y = y_avg;
                    x_last_restart = x_avg;
                    y_last_restart = y_avg;

                    avg_weight_sum = 1.0;
                    last_restart_iter = iter;
                    last_restart_kkt = current_kkt;
                }

                prev_check_kkt = current_kkt;
            }
        }
    }

    if (result.status != ModelStatus::OPTIMAL) {
        if (best_kkt <= m_options.tolerance * 10.0) {
            // Near-optimal acceptance if close enough at iteration limit
            result.status = ModelStatus::OPTIMAL;
            result.message = "Converged to near-optimal precision at iteration limit";
        } else {
            result.status = ModelStatus::ERROR;
            result.message = "Iteration limit reached without full convergence";
        }
        result.iterations = m_options.max_iterations;
        result.restarts = restart_count;
        result.primal_residual = best_kkt;
        x_avg = best_x;
        y_avg = best_y;
    }

    // Extract Primal Solution (original n variables)
    result.primal_solution.assign(x_avg.begin(), x_avg.begin() + n);

    // Extract Dual Solution (m constraints)
    result.dual_solution = y_avg;

    // Extract Reduced Costs (original n variables)
    K.multiplyTranspose(y_avg, K_T_y_avg);
    result.reduced_costs.resize(n);
    for (Index j = 0; j < n; ++j) {
        result.reduced_costs[j] = c_hat[j] - K_T_y_avg[j];
    }

    return result;
}

} // namespace xenith::solver
