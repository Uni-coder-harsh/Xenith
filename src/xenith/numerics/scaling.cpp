#include "xenith/numerics/scaling.hpp"
#include "xenith/common/constants.hpp"
#include <cmath>
#include <algorithm>
#include <stdexcept>

namespace xenith::numerics {

DiagonalScaler::DiagonalScaler(ScalingOptions options) : m_options(options) {}

ScaledModel DiagonalScaler::scale(const model::CanonicalModel& model) const {
    ScaledModel result;
    result.model = model;
    
    Index m = result.model.numConstraints();
    Index n = result.model.numVariables();
    
    result.row_scale.assign(m, 1.0);
    result.col_scale.assign(n, 1.0);
    result.inv_row_scale.assign(m, 1.0);
    result.inv_col_scale.assign(n, 1.0);
    
    // Copy the matrix to perform scaling on it
    SparseMatrix A = result.model.matrixA();
    if (A.empty()) {
        return result; // Nothing to scale
    }
    
    std::vector<double> r(m, 0.0);
    std::vector<double> c(n, 0.0);
    std::vector<double> d1(m, 1.0);
    std::vector<double> d2(n, 1.0);

    // Step 1: Ruiz l_infinity Equilibration
    for (Index iter = 0; iter < m_options.ruiz_iterations; ++iter) {
        A.rowInfinityNorms(r);
        for (Index i = 0; i < m; ++i) {
            if (r[i] > m_options.zero_tolerance) {
                d1[i] = 1.0 / std::sqrt(r[i]);
            } else {
                d1[i] = 1.0;
            }
        }
        
        A.colInfinityNorms(c);
        for (Index j = 0; j < n; ++j) {
            if (c[j] > m_options.zero_tolerance) {
                d2[j] = 1.0 / std::sqrt(c[j]);
            } else {
                d2[j] = 1.0;
            }
        }
        
        A.scaleRows(d1);
        A.scaleCols(d2);
        
        for (Index i = 0; i < m; ++i) {
            result.row_scale[i] *= d1[i];
        }
        for (Index j = 0; j < n; ++j) {
            result.col_scale[j] *= d2[j];
        }
    }
    
    // Step 2: Pock-Chambolle l_1 Scaling
    if (m_options.enable_pock_chambolle) {
        A.rowL1Norms(r);
        for (Index i = 0; i < m; ++i) {
            if (r[i] > m_options.zero_tolerance) {
                d1[i] = 1.0 / std::sqrt(r[i]);
            } else {
                d1[i] = 1.0;
            }
        }
        
        A.colL1Norms(c);
        for (Index j = 0; j < n; ++j) {
            if (c[j] > m_options.zero_tolerance) {
                d2[j] = 1.0 / std::sqrt(c[j]);
            } else {
                d2[j] = 1.0;
            }
        }
        
        A.scaleRows(d1);
        A.scaleCols(d2);
        
        for (Index i = 0; i < m; ++i) {
            result.row_scale[i] *= d1[i];
        }
        for (Index j = 0; j < n; ++j) {
            result.col_scale[j] *= d2[j];
        }
    }
    
    // Clamp values and calculate inverses
    for (Index i = 0; i < m; ++i) {
        result.row_scale[i] = std::clamp(result.row_scale[i], m_options.min_scale, m_options.max_scale);
        result.inv_row_scale[i] = 1.0 / result.row_scale[i];
    }
    for (Index j = 0; j < n; ++j) {
        result.col_scale[j] = std::clamp(result.col_scale[j], m_options.min_scale, m_options.max_scale);
        result.inv_col_scale[j] = 1.0 / result.col_scale[j];
    }
    
    // Scale A properly based on clamped scales (in case clamping occurred, recalculate A)
    // Actually, A was already scaled by incremental d1/d2. Let's rebuild A from original to ensure exact match with clamped scales.
    A = result.model.matrixA();
    A.scaleRows(result.row_scale);
    A.scaleCols(result.col_scale);
    result.model.setMatrixA(std::move(A));
    
    // Scale Objective
    auto obj = result.model.objective();
    for (Index j = 0; j < n; ++j) {
        obj[j] *= result.col_scale[j];
    }
    
    // Scale Variable Bounds
    auto colLower = result.model.colLower();
    auto colUpper = result.model.colUpper();
    for (Index j = 0; j < n; ++j) {
        if (!isNegativeInfinity(colLower[j])) {
            colLower[j] /= result.col_scale[j];
        }
        if (!isPositiveInfinity(colUpper[j])) {
            colUpper[j] /= result.col_scale[j];
        }
    }
    
    // Scale Constraint Bounds
    auto rowLower = result.model.rowLower();
    auto rowUpper = result.model.rowUpper();
    for (Index i = 0; i < m; ++i) {
        if (!isNegativeInfinity(rowLower[i])) {
            rowLower[i] *= result.row_scale[i];
        }
        if (!isPositiveInfinity(rowUpper[i])) {
            rowUpper[i] *= result.row_scale[i];
        }
    }
    
    return result;
}

void DiagonalScaler::unscalePrimal(std::span<const double> x_scaled,
                                   std::span<const double> col_scale,
                                   std::span<double> x_orig) {
    if (x_scaled.size() != col_scale.size() || x_orig.size() != x_scaled.size()) {
        throw std::invalid_argument("Size mismatch in unscalePrimal");
    }
    for (std::size_t j = 0; j < x_scaled.size(); ++j) {
        x_orig[j] = x_scaled[j] * col_scale[j];
    }
}

void DiagonalScaler::unscaleDual(std::span<const double> y_scaled,
                                 std::span<const double> row_scale,
                                 std::span<double> y_orig) {
    if (y_scaled.size() != row_scale.size() || y_orig.size() != y_scaled.size()) {
        throw std::invalid_argument("Size mismatch in unscaleDual");
    }
    for (std::size_t i = 0; i < y_scaled.size(); ++i) {
        y_orig[i] = y_scaled[i] * row_scale[i];
    }
}

void DiagonalScaler::unscaleReducedCosts(std::span<const double> lambda_scaled,
                                         std::span<const double> inv_col_scale,
                                         std::span<double> lambda_orig) {
    if (lambda_scaled.size() != inv_col_scale.size() || lambda_orig.size() != lambda_scaled.size()) {
        throw std::invalid_argument("Size mismatch in unscaleReducedCosts");
    }
    for (std::size_t j = 0; j < lambda_scaled.size(); ++j) {
        lambda_orig[j] = lambda_scaled[j] * inv_col_scale[j]; // since it's lambda / D2
    }
}

} // namespace xenith::numerics
