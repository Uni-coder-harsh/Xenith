#ifndef XENITH_SOLVER_COMMON_BASIS_TYPES_HPP
#define XENITH_SOLVER_COMMON_BASIS_TYPES_HPP

#include <string>
#include <limits>
#include "xenith/common/types.hpp"

namespace xenith::solver {

/// Sentinel index representing an unassigned or invalid basis position/variable.
constexpr Index K_INVALID_INDEX = std::numeric_limits<Index>::max();

/// Variable status in the Simplex basis representation.
enum class BasisStatus {
    BASIC,               ///< Variable is basic (part of basis matrix B).
    NON_BASIC_AT_LOWER,  ///< Non-basic variable fixed/at its lower bound.
    NON_BASIC_AT_UPPER,  ///< Non-basic variable at its upper bound.
    FIXED,               ///< Non-basic variable fixed (lower bound == upper bound).
    FREE                 ///< Non-basic unconstrained variable (at value 0).
};

/// Helper to convert BasisStatus to human-readable string.
inline std::string toString(BasisStatus status) {
    switch (status) {
        case BasisStatus::BASIC: return "BASIC";
        case BasisStatus::NON_BASIC_AT_LOWER: return "NON_BASIC_AT_LOWER";
        case BasisStatus::NON_BASIC_AT_UPPER: return "NON_BASIC_AT_UPPER";
        case BasisStatus::FIXED: return "FIXED";
        case BasisStatus::FREE: return "FREE";
    }
    return "UNKNOWN";
}

} // namespace xenith::solver

#endif // XENITH_SOLVER_COMMON_BASIS_TYPES_HPP
