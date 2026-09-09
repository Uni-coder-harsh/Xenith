#ifndef XENITH_NUMERICS_SCALING_HPP
#define XENITH_NUMERICS_SCALING_HPP

#include <vector>
#include <span>
#include "xenith/model/canonical_model.hpp"

namespace xenith::numerics {

struct ScalingOptions {
    Index ruiz_iterations{10};
    bool enable_pock_chambolle{true};
    double min_scale{1e-8};
    double max_scale{1e8};
    double zero_tolerance{1e-12};
};

struct ScaledModel {
    model::CanonicalModel model;         // Scaled model
    std::vector<double> row_scale;       // D1 (size m)
    std::vector<double> col_scale;       // D2 (size n)
    std::vector<double> inv_row_scale;   // 1 / D1 (size m)
    std::vector<double> inv_col_scale;   // 1 / D2 (size n)
};

class DiagonalScaler {
public:
    explicit DiagonalScaler(ScalingOptions options = {});

    /// Scales a CanonicalModel, returning a ScaledModel with scaling factors
    ScaledModel scale(const model::CanonicalModel& model) const;

    /// Unscales primal solution: x = D2 * x_scaled
    static void unscalePrimal(std::span<const double> x_scaled,
                              std::span<const double> col_scale,
                              std::span<double> x_orig);

    /// Unscales dual solution: y = D1 * y_scaled
    static void unscaleDual(std::span<const double> y_scaled,
                            std::span<const double> row_scale,
                            std::span<double> y_orig);

    /// Unscales reduced costs: lambda = lambda_scaled / D2
    static void unscaleReducedCosts(std::span<const double> lambda_scaled,
                                   std::span<const double> inv_col_scale,
                                   std::span<double> lambda_orig);

private:
    ScalingOptions m_options;
};

} // namespace xenith::numerics

#endif // XENITH_NUMERICS_SCALING_HPP
