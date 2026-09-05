#include "xenith/solver/common/basis_manager.hpp"
#include <sstream>
#include <cmath>

namespace xenith::solver {

void BasisManager::initializeDefault(Index num_rows,
                                      Index total_vars,
                                      std::span<const double> lower_bounds,
                                      std::span<const double> upper_bounds) {
    if (total_vars < num_rows) {
        throw std::invalid_argument("Total variables N must be >= num_rows m");
    }

    m_numRows = num_rows;
    m_totalVars = total_vars;
    m_statuses.assign(m_totalVars, BasisStatus::NON_BASIC_AT_LOWER);
    m_basicVars.resize(m_numRows);
    m_varToBasisPos.assign(m_totalVars, K_INVALID_INDEX);

    Index num_structural = m_totalVars - m_numRows;

    // 1. Structural variables (j = 0 ... n-1)
    for (Index j = 0; j < num_structural; ++j) {
        double l = lower_bounds[j];
        double u = upper_bounds[j];

        if (std::abs(l - u) <= K_BOUND_TOLERANCE) {
            m_statuses[j] = BasisStatus::FIXED;
        } else if (isNegativeInfinity(l) && isPositiveInfinity(u)) {
            m_statuses[j] = BasisStatus::FREE;
        } else if (!isNegativeInfinity(l)) {
            m_statuses[j] = BasisStatus::NON_BASIC_AT_LOWER;
        } else if (!isPositiveInfinity(u)) {
            m_statuses[j] = BasisStatus::NON_BASIC_AT_UPPER;
        } else {
            m_statuses[j] = BasisStatus::FREE;
        }
    }

    // 2. Slack variables (i = 0 ... m-1, corresponding to variable index n + i)
    for (Index i = 0; i < m_numRows; ++i) {
        Index slack_var = num_structural + i;
        m_statuses[slack_var] = BasisStatus::BASIC;
        m_basicVars[i] = slack_var;
        m_varToBasisPos[slack_var] = i;
    }
}

BasisStatus BasisManager::status(Index var) const {
    if (var >= m_totalVars) {
        throw std::out_of_range("Variable index out of range in BasisManager::status");
    }
    return m_statuses[var];
}

void BasisManager::setStatus(Index var, BasisStatus status) {
    if (var >= m_totalVars) {
        throw std::out_of_range("Variable index out of range in BasisManager::setStatus");
    }
    m_statuses[var] = status;
}

Index BasisManager::basicVariable(Index basis_pos) const {
    if (basis_pos >= m_numRows) {
        throw std::out_of_range("Basis position out of range in BasisManager::basicVariable");
    }
    return m_basicVars[basis_pos];
}

Index BasisManager::basisPosition(Index var) const {
    if (var >= m_totalVars) {
        throw std::out_of_range("Variable index out of range in BasisManager::basisPosition");
    }
    return m_varToBasisPos[var];
}

void BasisManager::pivot(Index entering_var, Index leaving_pos, BasisStatus leaving_new_status) {
    if (entering_var >= m_totalVars) {
        throw std::out_of_range("Entering variable out of range in BasisManager::pivot");
    }
    if (leaving_pos >= m_numRows) {
        throw std::out_of_range("Leaving position out of range in BasisManager::pivot");
    }
    if (leaving_new_status == BasisStatus::BASIC) {
        throw std::invalid_argument("Leaving variable cannot be set to BASIC status");
    }

    Index leaving_var = m_basicVars[leaving_pos];

    // Update leaving variable
    m_statuses[leaving_var] = leaving_new_status;
    m_varToBasisPos[leaving_var] = K_INVALID_INDEX;

    // Update entering variable
    m_statuses[entering_var] = BasisStatus::BASIC;
    m_basicVars[leaving_pos] = entering_var;
    m_varToBasisPos[entering_var] = leaving_pos;
}

bool BasisManager::validateInvariants(std::string* out_error) const {
    if (m_basicVars.size() != m_numRows) {
        if (out_error) *out_error = "m_basicVars size does not match m_numRows";
        return false;
    }
    if (m_statuses.size() != m_totalVars || m_varToBasisPos.size() != m_totalVars) {
        if (out_error) *out_error = "Vector sizes do not match m_totalVars";
        return false;
    }

    Index count_basic = 0;
    for (Index v = 0; v < m_totalVars; ++v) {
        if (m_statuses[v] == BasisStatus::BASIC) {
            count_basic++;
            Index pos = m_varToBasisPos[v];
            if (pos >= m_numRows) {
                if (out_error) {
                    std::ostringstream oss;
                    oss << "Basic variable " << v << " has invalid basis position " << pos;
                    *out_error = oss.str();
                }
                return false;
            }
            if (m_basicVars[pos] != v) {
                if (out_error) {
                    std::ostringstream oss;
                    oss << "Mismatch between basicVars[" << pos << "]=" << m_basicVars[pos] << " and var " << v;
                    *out_error = oss.str();
                }
                return false;
            }
        } else {
            if (m_varToBasisPos[v] != K_INVALID_INDEX) {
                if (out_error) {
                    std::ostringstream oss;
                    oss << "Non-basic variable " << v << " has non-sentinel basis position " << m_varToBasisPos[v];
                    *out_error = oss.str();
                }
                return false;
            }
        }
    }

    if (count_basic != m_numRows) {
        if (out_error) {
            std::ostringstream oss;
            oss << "Count of basic variables (" << count_basic << ") != m_numRows (" << m_numRows << ")";
            *out_error = oss.str();
        }
        return false;
    }

    return true;
}

} // namespace xenith::solver
