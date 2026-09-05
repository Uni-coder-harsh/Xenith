#ifndef XENITH_SOLVER_LP_REVISED_SIMPLEX_SOLVER_HPP
#define XENITH_SOLVER_LP_REVISED_SIMPLEX_SOLVER_HPP

#include <vector>
#include <string>
#include <memory>
#include "xenith/common/types.hpp"
#include "xenith/common/constants.hpp"
#include "xenith/model/canonical_model.hpp"
#include "xenith/solver/common/basis_manager.hpp"
#include "xenith/numerics/lu_factorization.hpp"

namespace xenith::solver {

/// Structured configuration options for RevisedSimplexSolver.
struct SimplexOptions {
    double feasibility_tolerance{1e-6};
    double optimality_tolerance{1e-7};
    double pivot_tolerance{1e-10};
    double zero_tolerance{1e-12};
    Index max_iterations{100000};
    bool verbose{false};
};

/// Aggregated solution result from RevisedSimplexSolver.
struct SolveResult {
    ModelStatus status{ModelStatus::UNINITIALIZED};
    std::vector<double> primal_solution; // Size n (original variables)
    double objective_value{0.0};         // Original model objective sense value
    Index iterations{0};
    double primal_residual{0.0};
    double dual_residual{0.0};
    std::string message;
};

/**
 * @brief High-performance, mathematically rigorous Revised Simplex Solver.
 */
class RevisedSimplexSolver {
public:
    explicit RevisedSimplexSolver(SimplexOptions options = {});

    /**
     * @brief Solves the given CanonicalModel using Revised Simplex (Phase I / Phase II).
     * @param model CanonicalModel instance.
     * @return SolveResult containing lifecycle status, primal solution, and metrics.
     */
    SolveResult solve(const model::CanonicalModel& model);

    const SimplexOptions& options() const { return m_options; }
    void setOptions(SimplexOptions options) { m_options = std::move(options); }

private:
    SimplexOptions m_options;
};

} // namespace xenith::solver

#endif // XENITH_SOLVER_LP_REVISED_SIMPLEX_SOLVER_HPP
