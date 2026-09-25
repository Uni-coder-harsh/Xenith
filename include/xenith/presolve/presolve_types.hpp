#ifndef XENITH_PRESOLVE_PRESOLVE_TYPES_HPP
#define XENITH_PRESOLVE_PRESOLVE_TYPES_HPP

#include <vector>
#include <string>
#include "xenith/common/types.hpp"
#include "xenith/model/canonical_model.hpp"

namespace xenith::presolve {

/// Type of presolve transformation applied
enum class PresolveAction {
    REMOVE_EMPTY_ROW,        // Row with all zero coefficients
    REMOVE_EMPTY_COLUMN,     // Column with all zero coefficients
    FIX_VARIABLE,            // Variable with l_x == u_x
    REMOVE_SINGLETON_ROW,    // Row with exactly one nonzero
    TIGHTEN_BOUND            // Implied bound tightening
};

/// Record of a single presolve transformation (for postsolve reversal)
struct PresolveRecord {
    PresolveAction action;
    Index original_index;    // Original row or column index
    double value;            // Fixed value, bound value, or coefficient
    double aux_value;        // Secondary value if needed
};

/// Result of presolve: a reduced model plus the transformation stack
struct PresolveResult {
    model::CanonicalModel reduced_model;
    std::vector<PresolveRecord> records;  // LIFO stack for postsolve
    Index original_vars{0};
    Index original_constraints{0};
    Index vars_removed{0};
    Index constraints_removed{0};
    bool was_infeasible{false};
    std::string message;
};

} // namespace xenith::presolve

#endif // XENITH_PRESOLVE_PRESOLVE_TYPES_HPP
