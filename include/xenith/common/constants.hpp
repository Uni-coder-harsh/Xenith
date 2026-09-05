#ifndef XENITH_COMMON_CONSTANTS_HPP
#define XENITH_COMMON_CONSTANTS_HPP

#include <cmath>
#include <limits>

namespace xenith {

/// Representation of numerical infinity in bound vectors.
constexpr double K_INFINITY = std::numeric_limits<double>::infinity();

/// Absolute threshold for values treated as mathematical infinity in bounds.
constexpr double K_INFINITY_THRESHOLD = 1e20;

/// Default numerical zero tolerance. Values below this are considered zero.
constexpr double K_ZERO_TOLERANCE = 1e-12;

/// Default feasibility tolerance for constraint and bound violations.
constexpr double K_FEASIBILITY_TOLERANCE = 1e-6;

/// Default bound check tolerance.
constexpr double K_BOUND_TOLERANCE = 1e-9;

/**
 * @brief Checks if a double value represents positive infinity (+inf).
 * Policy: val >= 1e20 or std::isinf(val) > 0.
 */
inline bool isPositiveInfinity(double val) {
    return val >= K_INFINITY_THRESHOLD || (std::isinf(val) && val > 0.0);
}

/**
 * @brief Checks if a double value represents negative infinity (-inf).
 * Policy: val <= -1e20 or std::isinf(val) < 0.
 */
inline bool isNegativeInfinity(double val) {
    return val <= -K_INFINITY_THRESHOLD || (std::isinf(val) && val < 0.0);
}

/**
 * @brief Checks if a double value is finite (neither +inf, -inf, nor NaN).
 */
inline bool isFinite(double val) {
    return !isPositiveInfinity(val) && !isNegativeInfinity(val) && !std::isnan(val);
}

/**
 * @brief Checks if a double value is NaN.
 */
inline bool isNaN(double val) {
    return std::isnan(val);
}

} // namespace xenith

#endif // XENITH_COMMON_CONSTANTS_HPP
