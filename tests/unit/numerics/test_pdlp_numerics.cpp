#include "tests/test_harness.hpp"
#include "xenith/numerics/vector_ops.hpp"
#include "xenith/numerics/sparse_matrix.hpp"
#include "xenith/common/constants.hpp"
#include <vector>
#include <cmath>
#include <limits>

using namespace xenith;
using namespace xenith::numerics;

XENITH_TEST(TestPDLPVectorOps) {
    // ProjectBox
    std::vector<double> x = {-5.0, 0.0, 5.0, 10.0, -10.0};
    std::vector<double> lower = {-2.0, -1.0, -K_INFINITY, -K_INFINITY, 0.0};
    std::vector<double> upper = {2.0, 1.0, K_INFINITY, 5.0, K_INFINITY};
    std::vector<double> result(5, 0.0);
    projectBox(x, lower, upper, result);
    XENITH_CHECK_NEAR(result[0], -2.0, 1e-12);
    XENITH_CHECK_NEAR(result[1], 0.0, 1e-12);
    XENITH_CHECK_NEAR(result[2], 5.0, 1e-12);
    XENITH_CHECK_NEAR(result[3], 5.0, 1e-12);
    XENITH_CHECK_NEAR(result[4], 0.0, 1e-12);

    // ComponentwiseMul
    std::vector<double> y = {1.0, 2.0, 3.0};
    std::vector<double> z = {4.0, 5.0, 6.0};
    std::vector<double> mul_res(3, 0.0);
    componentwiseMul(y, z, mul_res);
    XENITH_CHECK_NEAR(mul_res[0], 4.0, 1e-12);
    XENITH_CHECK_NEAR(mul_res[1], 10.0, 1e-12);
    XENITH_CHECK_NEAR(mul_res[2], 18.0, 1e-12);

    // ComponentwiseDiv
    std::vector<double> div_y = {10.0, 20.0, 30.0};
    std::vector<double> div_z = {2.0, 0.0, 3.0};
    std::vector<double> div_res(3, 0.0);
    componentwiseDiv(div_y, div_z, div_res);
    XENITH_CHECK_NEAR(div_res[0], 5.0, 1e-12);
    XENITH_CHECK_NEAR(div_res[1], 0.0, 1e-12); // Safe division
    XENITH_CHECK_NEAR(div_res[2], 10.0, 1e-12);

    // sumOfSquares
    std::vector<double> sq_x = {-3.0, 4.0, -2.0};
    XENITH_CHECK_NEAR(sumOfSquares(sq_x), 29.0, 1e-12);

    // positivePartNorm
    std::vector<double> pos_x = {-3.0, 4.0, -2.0, 3.0};
    XENITH_CHECK_NEAR(positivePartNorm(pos_x), 5.0, 1e-12); // sqrt(16 + 9) = 5
}

XENITH_TEST(TestPDLPSparseMatrixOps) {
    // 3x3 matrix:
    // 1.0  0.0  2.0
    // 0.0 -3.0  4.0
    // 5.0  6.0  0.0
    std::vector<Triplet> triplets = {
        {0, 0, 1.0}, {0, 2, 2.0},
        {1, 1, -3.0}, {1, 2, 4.0},
        {2, 0, 5.0}, {2, 1, 6.0}
    };
    SparseMatrix A = SparseMatrix::fromTriplets(3, 3, triplets, SparseStorageFormat::CSC);

    std::vector<double> r_norm(3, 0.0);
    A.rowInfinityNorms(r_norm);
    XENITH_CHECK_NEAR(r_norm[0], 2.0, 1e-12);
    XENITH_CHECK_NEAR(r_norm[1], 4.0, 1e-12);
    XENITH_CHECK_NEAR(r_norm[2], 6.0, 1e-12);

    std::vector<double> c_norm(3, 0.0);
    A.colInfinityNorms(c_norm);
    XENITH_CHECK_NEAR(c_norm[0], 5.0, 1e-12);
    XENITH_CHECK_NEAR(c_norm[1], 6.0, 1e-12);
    XENITH_CHECK_NEAR(c_norm[2], 4.0, 1e-12);

    // Scale rows
    std::vector<double> r_scale = {2.0, 0.5, 1.0};
    A.scaleRows(r_scale);
    // Row 0 * 2 => 2.0, 0, 4.0
    // Row 1 * 0.5 => 0, -1.5, 2.0
    // Row 2 * 1 => 5.0, 6.0, 0
    A.rowInfinityNorms(r_norm);
    XENITH_CHECK_NEAR(r_norm[0], 4.0, 1e-12);
    XENITH_CHECK_NEAR(r_norm[1], 2.0, 1e-12);
    XENITH_CHECK_NEAR(r_norm[2], 6.0, 1e-12);

    // Scale cols
    std::vector<double> c_scale = {0.1, 2.0, 0.5};
    A.scaleCols(c_scale);
    // Col 0 * 0.1 => 0.2, 0, 0.5
    // Col 1 * 2.0 => 0, -3.0, 12.0
    // Col 2 * 0.5 => 2.0, 1.0, 0
    A.colInfinityNorms(c_norm);
    XENITH_CHECK_NEAR(c_norm[0], 0.5, 1e-12);
    XENITH_CHECK_NEAR(c_norm[1], 12.0, 1e-12);
    XENITH_CHECK_NEAR(c_norm[2], 2.0, 1e-12);
    
    // Spectral Norm on Diagonal Matrix
    std::vector<Triplet> diag_triplets = {
        {0, 0, 3.0}, {1, 1, -4.0}, {2, 2, 2.0}
    };
    SparseMatrix D = SparseMatrix::fromTriplets(3, 3, diag_triplets, SparseStorageFormat::CSC);
    double spec_norm = D.spectralNormEstimate(50);
    XENITH_CHECK_NEAR(spec_norm, 4.0, 1e-6); // Approx max abs eigenvalue
}

int main() {
    return xenith::test::TestRunner::instance().runAll();
}
