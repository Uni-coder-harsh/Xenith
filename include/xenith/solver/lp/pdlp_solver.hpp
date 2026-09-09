#ifndef XENITH_SOLVER_LP_PDLP_SOLVER_HPP
#define XENITH_SOLVER_LP_PDLP_SOLVER_HPP

#include "xenith/model/canonical_model.hpp"
#include "xenith/solver/lp/pdlp_types.hpp"

namespace xenith::solver {

/**
 * @brief High-performance First-Order LP Solver using Restarted Primal-Dual Hybrid Gradient (PDLP).
 *
 * Implements Google PDLP and cuPDLP-C algorithmic principles:
 * - Matrix-free saddle-point equality slack formulation: K = [A  -I_m]
 * - Ruiz l_inf matrix equilibration & Pock-Chambolle l_1 scaling
 * - GPU-vectorized KKT error restart criteria
 * - Dynamic primal weight (omega) balancing
 * - Reversible presolve and postsolve unwinding
 */
class PdlpSolver {
public:
    explicit PdlpSolver(PdlpOptions options = {});

    /**
     * @brief Solves a CanonicalModel using the complete PDLP pipeline (Presolve -> Scale -> PDHG -> Unscale -> Postsolve).
     * @param model Input CanonicalModel.
     * @return PdlpResult containing solution status, variables, objective, residuals, and metrics.
     */
    PdlpResult solve(const model::CanonicalModel& model);

    const PdlpOptions& options() const { return m_options; }
    void setOptions(PdlpOptions options) { m_options = std::move(options); }

private:
    PdlpOptions m_options;

    // Internal core PDHG solve on a prepared (presolved + scaled) model
    PdlpResult solveCore(const model::CanonicalModel& model);
};

} // namespace xenith::solver

#endif // XENITH_SOLVER_LP_PDLP_SOLVER_HPP
