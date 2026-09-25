#ifndef XENITH_PRESOLVE_PRESOLVER_HPP
#define XENITH_PRESOLVE_PRESOLVER_HPP

#include "xenith/presolve/presolve_types.hpp"
#include "xenith/model/canonical_model.hpp"

namespace xenith::presolve {

/**
 * @brief Performs reversible presolve transformations on canonical models.
 */
class Presolver {
public:
    struct Options {
        bool remove_empty_rows{true};
        bool remove_empty_cols{true};
        bool fix_variables{true};
        bool remove_singletons{true};
        double feasibility_tolerance{1e-8};
        bool verbose{false};
    };

    Presolver();
    explicit Presolver(Options options);
    
    /**
     * @brief Applies presolve transformations to reduce model size.
     * @param model Original canonical model.
     * @return Result containing reduced model and record of transformations.
     */
    PresolveResult presolve(const model::CanonicalModel& model);

    /**
     * @brief Reverses presolve transformations to recover original solution.
     * @param reduced_solution Solution to the reduced model.
     * @param presolve_result Output of presolve() containing the record stack.
     * @return Recovered full-space solution.
     */
    static std::vector<double> postsolve(
        const std::vector<double>& reduced_solution,
        const PresolveResult& presolve_result);

private:
    Options m_options;
};

} // namespace xenith::presolve

#endif // XENITH_PRESOLVE_PRESOLVER_HPP
