#include "tests/test_harness.hpp"
#include "xenith/numerics/scaling.hpp"
#include "xenith/numerics/sparse_matrix.hpp"
#include "xenith/model/canonical_model.hpp"
#include "xenith/common/constants.hpp"

#include <vector>
#include <cmath>
#include <iostream>

using namespace xenith::numerics;
using namespace xenith::model;
using namespace xenith;

XENITH_TEST(SparseMatrixL1Norms) {
    std::vector<Triplet> triplets = {
        {0, 0, -2.0}, {0, 1, 1.5},
        {1, 0, 3.0},  {1, 1, -4.5}
    };
    SparseMatrix mat_csc = SparseMatrix::fromTriplets(2, 2, triplets, SparseStorageFormat::CSC);
    SparseMatrix mat_csr = SparseMatrix::fromTriplets(2, 2, triplets, SparseStorageFormat::CSR);
    
    std::vector<double> r_csc(2), r_csr(2);
    mat_csc.rowL1Norms(r_csc);
    mat_csr.rowL1Norms(r_csr);
    
    XENITH_CHECK_NEAR(r_csc[0], 3.5, 1e-9);
    XENITH_CHECK_NEAR(r_csc[1], 7.5, 1e-9);
    XENITH_CHECK_NEAR(r_csr[0], 3.5, 1e-9);
    XENITH_CHECK_NEAR(r_csr[1], 7.5, 1e-9);
    
    std::vector<double> c_csc(2), c_csr(2);
    mat_csc.colL1Norms(c_csc);
    mat_csr.colL1Norms(c_csr);
    
    XENITH_CHECK_NEAR(c_csc[0], 5.0, 1e-9);
    XENITH_CHECK_NEAR(c_csc[1], 6.0, 1e-9);
    XENITH_CHECK_NEAR(c_csr[0], 5.0, 1e-9);
    XENITH_CHECK_NEAR(c_csr[1], 6.0, 1e-9);
}

XENITH_TEST(RuizEquilibration) {
    CanonicalModel model;
    model.addVariable("x1");
    model.addVariable("x2");
    model.addRow("r1", 0, 0);
    model.addRow("r2", 0, 0);
    
    std::vector<Triplet> triplets = {
        {0, 0, 1e-4}, {0, 1, 1e-4},
        {1, 0, 1e4},  {1, 1, 1e4}
    };
    model.setMatrixA(SparseMatrix::fromTriplets(2, 2, triplets));
    
    ScalingOptions opts;
    opts.enable_pock_chambolle = false; // test ruiz alone
    opts.ruiz_iterations = 50;
    
    DiagonalScaler scaler(opts);
    ScaledModel sm = scaler.scale(model);
    
    std::vector<double> row_inf(2);
    std::vector<double> col_inf(2);
    
    sm.model.matrixA().rowInfinityNorms(row_inf);
    sm.model.matrixA().colInfinityNorms(col_inf);
    
    XENITH_CHECK_NEAR(row_inf[0], 1.0, 1e-5);
    XENITH_CHECK_NEAR(row_inf[1], 1.0, 1e-5);
    XENITH_CHECK_NEAR(col_inf[0], 1.0, 1e-5);
    XENITH_CHECK_NEAR(col_inf[1], 1.0, 1e-5);
}

XENITH_TEST(PockChambolleSpectralNorm) {
    CanonicalModel model;
    model.addVariable("x1");
    model.addVariable("x2");
    model.addRow("r1", 0, 0);
    model.addRow("r2", 0, 0);
    
    std::vector<Triplet> triplets = {
        {0, 0, 2.0}, {0, 1, 1.0},
        {1, 0, 1.0}, {1, 1, 3.0}
    };
    model.setMatrixA(SparseMatrix::fromTriplets(2, 2, triplets));
    
    ScalingOptions opts;
    opts.enable_pock_chambolle = true;
    
    DiagonalScaler scaler(opts);
    ScaledModel sm = scaler.scale(model);
    
    double norm = sm.model.matrixA().spectralNormEstimate(50);
    XENITH_CHECK(norm <= 1.0 + 1e-5);
}

XENITH_TEST(ModelDataScalingAndUnscaling) {
    CanonicalModel model;
    model.addVariable("x1", 10.0, K_INFINITY, 2.0);
    model.addVariable("x2", -K_INFINITY, 20.0, -3.0);
    model.addRow("r1", -5.0, 5.0);
    model.addRow("r2", 0.0, K_INFINITY);
    
    std::vector<Triplet> triplets = {
        {0, 0, 2.0}, {0, 1, -1.0},
        {1, 0, 1.0}, {1, 1, 0.5}
    };
    model.setMatrixA(SparseMatrix::fromTriplets(2, 2, triplets));
    
    ScalingOptions opts;
    DiagonalScaler scaler(opts);
    ScaledModel sm = scaler.scale(model);
    
    XENITH_CHECK_EQ(sm.row_scale.size(), 2);
    XENITH_CHECK_EQ(sm.col_scale.size(), 2);
    
    // Bounds check
    auto cL = sm.model.colLower();
    auto cU = sm.model.colUpper();
    XENITH_CHECK_NEAR(cL[0], 10.0 / sm.col_scale[0], 1e-9);
    XENITH_CHECK(isPositiveInfinity(cU[0]));
    XENITH_CHECK(isNegativeInfinity(cL[1]));
    XENITH_CHECK_NEAR(cU[1], 20.0 / sm.col_scale[1], 1e-9);
    
    auto rL = sm.model.rowLower();
    auto rU = sm.model.rowUpper();
    XENITH_CHECK_NEAR(rL[0], -5.0 * sm.row_scale[0], 1e-9);
    XENITH_CHECK_NEAR(rU[0], 5.0 * sm.row_scale[0], 1e-9);
    XENITH_CHECK_NEAR(rL[1], 0.0, 1e-9);
    XENITH_CHECK(isPositiveInfinity(rU[1]));
    
    auto obj = sm.model.objective();
    XENITH_CHECK_NEAR(obj[0], 2.0 * sm.col_scale[0], 1e-9);
    XENITH_CHECK_NEAR(obj[1], -3.0 * sm.col_scale[1], 1e-9);
    
    // Test Unscaling
    std::vector<double> x_scaled = { 1.0, 2.0 };
    std::vector<double> x_orig(2);
    DiagonalScaler::unscalePrimal(x_scaled, sm.col_scale, x_orig);
    XENITH_CHECK_NEAR(x_orig[0], 1.0 * sm.col_scale[0], 1e-9);
    XENITH_CHECK_NEAR(x_orig[1], 2.0 * sm.col_scale[1], 1e-9);
    
    std::vector<double> y_scaled = { -1.0, 3.0 };
    std::vector<double> y_orig(2);
    DiagonalScaler::unscaleDual(y_scaled, sm.row_scale, y_orig);
    XENITH_CHECK_NEAR(y_orig[0], -1.0 * sm.row_scale[0], 1e-9);
    XENITH_CHECK_NEAR(y_orig[1], 3.0 * sm.row_scale[1], 1e-9);
    
    std::vector<double> lambda_scaled = { 0.5, -0.5 };
    std::vector<double> lambda_orig(2);
    DiagonalScaler::unscaleReducedCosts(lambda_scaled, sm.inv_col_scale, lambda_orig);
    XENITH_CHECK_NEAR(lambda_orig[0], 0.5 / sm.col_scale[0], 1e-9);
    XENITH_CHECK_NEAR(lambda_orig[1], -0.5 / sm.col_scale[1], 1e-9);
}

XENITH_TEST(EmptyMatrixHandling) {
    CanonicalModel model; // empty
    ScalingOptions opts;
    DiagonalScaler scaler(opts);
    ScaledModel sm = scaler.scale(model);
    XENITH_CHECK(sm.model.matrixA().empty());
}

int main() {
    return xenith::test::TestRunner::instance().runAll();
}
