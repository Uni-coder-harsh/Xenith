#ifndef XENITH_SOLUTION_LP_SOLUTION_VALIDATOR_HPP
#define XENITH_SOLUTION_LP_SOLUTION_VALIDATOR_HPP

#include "xenith/model/canonical_model.hpp"
#include "xenith/model/model_validator.hpp"
#include "xenith/solver/lp/revised_simplex_solver.hpp"

namespace xenith::solution {

/**
 * @brief Independent Solution Validator for verifying LP solver primal feasibility and objective accuracy.
 */
class LpSolutionValidator {
public:
    /**
     * @brief Performs independent mathematical validation of a solver result against the CanonicalModel.
     * @param model CanonicalModel instance.
     * @param result SolveResult produced by RevisedSimplexSolver.
     * @param tolerance Numerical tolerance for bounds and objective checks (default 1e-5).
     * @return ValidationResult containing validity flag and diagnostic list.
     */
    static model::ValidationResult validate(const model::CanonicalModel& model,
                                             const solver::SolveResult& result,
                                             double tolerance = 1e-5);
};

} // namespace xenith::solution

#endif // XENITH_SOLUTION_LP_SOLUTION_VALIDATOR_HPP
