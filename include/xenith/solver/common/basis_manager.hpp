#ifndef XENITH_SOLVER_COMMON_BASIS_MANAGER_HPP
#define XENITH_SOLVER_COMMON_BASIS_MANAGER_HPP

#include <span>
#include <vector>
#include <string>
#include <optional>
#include <stdexcept>
#include "xenith/common/types.hpp"
#include "xenith/common/constants.hpp"
#include "xenith/solver/common/basis_types.hpp"

namespace xenith::solver {

/**
 * @brief Manages basis state, variable statuses, and basis indexing for Revised Simplex.
 */
class BasisManager {
public:
    BasisManager() = default;

    /**
     * @brief Initializes basis manager with default slack basis.
     * @param num_rows Number of constraint rows m.
     * @param total_vars Total expanded variables N = n + m.
     * @param lower_bounds Span of variable lower bounds (length N).
     * @param upper_bounds Span of variable upper bounds (length N).
     */
    void initializeDefault(Index num_rows,
                           Index total_vars,
                           std::span<const double> lower_bounds,
                           std::span<const double> upper_bounds);

    Index numRows() const { return m_numRows; }
    Index totalVars() const { return m_totalVars; }

    /// Gets status of variable var.
    BasisStatus status(Index var) const;

    /// Sets status of variable var.
    void setStatus(Index var, BasisStatus status);

    /// Gets variable index occupying basis position basis_pos in [0, m-1].
    Index basicVariable(Index basis_pos) const;

    /// Gets basis position in [0, m-1] for a basic variable, or K_INVALID_INDEX if non-basic.
    Index basisPosition(Index var) const;

    /// Exposes read-only span of all basic variable indices (length m).
    std::span<const Index> basicVariables() const { return m_basicVars; }

    /// Exposes read-only span of all variable statuses (length N).
    std::span<const BasisStatus> statuses() const { return m_statuses; }

    /**
     * @brief Performs basis pivot replacing leaving variable at leaving_pos with entering_var.
     * @param entering_var Variable index entering the basis.
     * @param leaving_pos Basis position (0 to m-1) being replaced.
     * @param leaving_new_status New non-basic status for the leaving variable.
     */
    void pivot(Index entering_var, Index leaving_pos, BasisStatus leaving_new_status);

    /**
     * @brief Validates structural invariants of the basis state.
     * @param out_error Optional diagnostic string output.
     * @return true if valid, false otherwise.
     */
    bool validateInvariants(std::string* out_error = nullptr) const;

private:
    Index m_numRows{0};
    Index m_totalVars{0};
    std::vector<BasisStatus> m_statuses;
    std::vector<Index> m_basicVars;       // Size m: basis_pos -> var_index
    std::vector<Index> m_varToBasisPos;   // Size N: var_index -> basis_pos (or K_INVALID_INDEX)
};

} // namespace xenith::solver

#endif // XENITH_SOLVER_COMMON_BASIS_MANAGER_HPP
