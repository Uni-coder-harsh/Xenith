#ifndef XENITH_NUMERICS_VECTOR_OPS_HPP
#define XENITH_NUMERICS_VECTOR_OPS_HPP

#include <span>
#include <vector>
#include <stdexcept>
#include "xenith/common/types.hpp"
#include "xenith/common/constants.hpp"

namespace xenith::numerics {

/**
 * @brief Computes the dot product (inner product) of two vectors: sum(x[i] * y[i]).
 * @throws std::invalid_argument if x.size() != y.size().
 */
double dot(std::span<const double> x, std::span<const double> y);

/**
 * @brief Performs AXPY operation: y <- alpha * x + y.
 * @throws std::invalid_argument if x.size() != y.size().
 */
void axpy(double alpha, std::span<const double> x, std::span<double> y);

/**
 * @brief Scales vector in-place: x <- alpha * x.
 */
void scale(double alpha, std::span<double> x);

/**
 * @brief Vector addition: result <- x + y.
 * @throws std::invalid_argument if sizes mismatch.
 */
void vectorAdd(std::span<const double> x, std::span<const double> y, std::span<double> result);

/**
 * @brief Vector subtraction: result <- x - y.
 * @throws std::invalid_argument if sizes mismatch.
 */
void vectorSub(std::span<const double> x, std::span<const double> y, std::span<double> result);

/**
 * @brief Computes the infinity norm (max absolute entry) of vector x: max |x[i]|.
 */
double infinityNorm(std::span<const double> x);

/**
 * @brief Computes the Euclidean 2-norm of vector x: sqrt(sum(x[i]^2)).
 */
double euclideanNorm(std::span<const double> x);

/**
 * @brief Componentwise projection: result[i] = clamp(x[i], lower[i], upper[i]).
 * @throws std::invalid_argument if sizes mismatch.
 */
void projectBox(std::span<const double> x, std::span<const double> lower, std::span<const double> upper, std::span<double> result);

/**
 * @brief Componentwise multiplication: result[i] = x[i] * y[i].
 * @throws std::invalid_argument if sizes mismatch.
 */
void componentwiseMul(std::span<const double> x, std::span<const double> y, std::span<double> result);

/**
 * @brief Componentwise division: result[i] = x[i] / y[i]. Safe division (y[i] == 0 => result[i] = 0).
 * @throws std::invalid_argument if sizes mismatch.
 */
void componentwiseDiv(std::span<const double> x, std::span<const double> y, std::span<double> result);

/**
 * @brief Returns sum(x[i]^2) without sqrt.
 */
double sumOfSquares(std::span<const double> x);

/**
 * @brief Returns sqrt(sum(max(0, x[i])^2)).
 */
double positivePartNorm(std::span<const double> x);

} // namespace xenith::numerics

#endif // XENITH_NUMERICS_VECTOR_OPS_HPP
