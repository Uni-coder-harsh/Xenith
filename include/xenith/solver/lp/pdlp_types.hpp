#ifndef XENITH_SOLVER_LP_PDLP_TYPES_HPP
#define XENITH_SOLVER_LP_PDLP_TYPES_HPP

#include <vector>
#include <string>
#include "xenith/common/types.hpp"
#include "xenith/common/constants.hpp"

namespace xenith::solver {

/// Configuration options for the PDLP (Restarted PDHG) Solver.
struct PdlpOptions {
    double tolerance{1e-6};               ///< Target relative termination tolerance (primal, dual, gap)
    Index max_iterations{100000};          ///< Maximum outer/inner PDHG iterations
    Index check_interval{40};             ///< Iteration interval for convergence and restart checks
    double step_size_factor{0.9};         ///< Factor for step size eta = step_size_factor / ||K||_2
    bool enable_presolve{true};           ///< Enable Phase 4B presolve reductions
    bool enable_scaling{true};            ///< Enable Phase 4C Ruiz + Pock-Chambolle preconditioning
    Index ruiz_iterations{10};            ///< Ruiz equilibration iterations
    bool enable_pock_chambolle{true};     ///< Enable Pock-Chambolle scaling pass
    bool enable_adaptive_restart{true};   ///< Enable cuPDLP-C style adaptive restarts
    bool enable_primal_weight_update{true};///< Enable dynamic primal weight (omega) updates
    bool verbose{false};                  ///< Print progress during solve
};

/// Solution and diagnostic results returned by PdlpSolver.
struct PdlpResult {
    ModelStatus status{ModelStatus::UNINITIALIZED};
    std::vector<double> primal_solution;    ///< Unscaled, postsolved primal variables (dimension n)
    std::vector<double> dual_solution;      ///< Unscaled dual multipliers for constraints (dimension m)
    std::vector<double> reduced_costs;      ///< Unscaled reduced costs for variables (dimension n)
    double objective_value{0.0};            ///< Final objective value (respecting MIN/MAX sense and offset)
    Index iterations{0};                    ///< Total PDHG iterations performed
    Index restarts{0};                      ///< Total restarts performed
    double primal_residual{0.0};            ///< Relative primal residual ||Ax - s||_2 / (1 + ||q||_2)
    double dual_residual{0.0};              ///< Relative dual residual ||c - A^T y - lambda||_2 / (1 + ||c||_2)
    double duality_gap{0.0};                ///< Relative duality gap |primal_obj - dual_obj| / (1 + |p_obj| + |d_obj|)
    double solve_time_ms{0.0};              ///< Total solve time in milliseconds
    std::string message;                    ///< Human-readable status or diagnostic message
};

} // namespace xenith::solver

#endif // XENITH_SOLVER_LP_PDLP_TYPES_HPP
