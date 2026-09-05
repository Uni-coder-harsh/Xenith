#ifndef XENITH_COMMON_TYPES_HPP
#define XENITH_COMMON_TYPES_HPP

#include <cstddef>
#include <string>

namespace xenith {

/// Index type for matrix dimensions, rows, columns, and non-zeros.
using Index = std::size_t;

/// Objective function optimization sense.
enum class ObjectiveSense {
    MINIMIZE,
    MAXIMIZE
};

/// Decision variable integrality and domain type.
enum class VariableType {
    CONTINUOUS,
    GENERAL_INTEGER,
    BINARY,
    SEMI_CONTINUOUS,
    SEMI_INTEGER
};

/// High-level model lifecycle and solution status.
enum class ModelStatus {
    UNINITIALIZED,
    LOADED,
    VALIDATED,
    INFEASIBLE,
    OPTIMAL,
    UNBOUNDED,
    ERROR
};

/// Helper to convert ObjectiveSense to string representation.
inline std::string toString(ObjectiveSense sense) {
    switch (sense) {
        case ObjectiveSense::MINIMIZE: return "MINIMIZE";
        case ObjectiveSense::MAXIMIZE: return "MAXIMIZE";
    }
    return "UNKNOWN";
}

/// Helper to convert VariableType to string representation.
inline std::string toString(VariableType type) {
    switch (type) {
        case VariableType::CONTINUOUS: return "CONTINUOUS";
        case VariableType::GENERAL_INTEGER: return "GENERAL_INTEGER";
        case VariableType::BINARY: return "BINARY";
        case VariableType::SEMI_CONTINUOUS: return "SEMI_CONTINUOUS";
        case VariableType::SEMI_INTEGER: return "SEMI_INTEGER";
    }
    return "UNKNOWN";
}

} // namespace xenith

#endif // XENITH_COMMON_TYPES_HPP
