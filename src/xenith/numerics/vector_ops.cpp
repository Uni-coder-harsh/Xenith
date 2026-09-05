#include "xenith/numerics/vector_ops.hpp"
#include <cmath>
#include <algorithm>

namespace xenith::numerics {

double dot(std::span<const double> x, std::span<const double> y) {
    if (x.size() != y.size()) {
        throw std::invalid_argument("Vector dimension mismatch in dot product: " +
                                    std::to_string(x.size()) + " vs " + std::to_string(y.size()));
    }
    double result = 0.0;
    for (std::size_t i = 0; i < x.size(); ++i) {
        result += x[i] * y[i];
    }
    return result;
}

void axpy(double alpha, std::span<const double> x, std::span<double> y) {
    if (x.size() != y.size()) {
        throw std::invalid_argument("Vector dimension mismatch in axpy: " +
                                    std::to_string(x.size()) + " vs " + std::to_string(y.size()));
    }
    if (alpha == 0.0) {
        return;
    }
    for (std::size_t i = 0; i < x.size(); ++i) {
        y[i] += alpha * x[i];
    }
}

void scale(double alpha, std::span<double> x) {
    for (std::size_t i = 0; i < x.size(); ++i) {
        x[i] *= alpha;
    }
}

void vectorAdd(std::span<const double> x, std::span<const double> y, std::span<double> result) {
    if (x.size() != y.size() || x.size() != result.size()) {
        throw std::invalid_argument("Vector dimension mismatch in vectorAdd: x(" +
                                    std::to_string(x.size()) + "), y(" +
                                    std::to_string(y.size()) + "), result(" +
                                    std::to_string(result.size()) + ")");
    }
    for (std::size_t i = 0; i < x.size(); ++i) {
        result[i] = x[i] + y[i];
    }
}

void vectorSub(std::span<const double> x, std::span<const double> y, std::span<double> result) {
    if (x.size() != y.size() || x.size() != result.size()) {
        throw std::invalid_argument("Vector dimension mismatch in vectorSub: x(" +
                                    std::to_string(x.size()) + "), y(" +
                                    std::to_string(y.size()) + "), result(" +
                                    std::to_string(result.size()) + ")");
    }
    for (std::size_t i = 0; i < x.size(); ++i) {
        result[i] = x[i] - y[i];
    }
}

double infinityNorm(std::span<const double> x) {
    double max_val = 0.0;
    for (std::size_t i = 0; i < x.size(); ++i) {
        double abs_val = std::abs(x[i]);
        if (abs_val > max_val) {
            max_val = abs_val;
        }
    }
    return max_val;
}

double euclideanNorm(std::span<const double> x) {
    double sum_sq = 0.0;
    for (std::size_t i = 0; i < x.size(); ++i) {
        sum_sq += x[i] * x[i];
    }
    return std::sqrt(sum_sq);
}

} // namespace xenith::numerics
