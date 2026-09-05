#include "tests/test_harness.hpp"
#include "xenith/numerics/sparse_matrix.hpp"
#include <vector>

using namespace xenith;
using namespace xenith::numerics;

XENITH_TEST(TestSparseMatrixEmpty) {
    SparseMatrix mat_empty;
    XENITH_CHECK(mat_empty.empty());
    XENITH_CHECK_EQ(mat_empty.rows(), 0);
    XENITH_CHECK_EQ(mat_empty.cols(), 0);
    XENITH_CHECK_EQ(mat_empty.nonZeros(), 0);
    XENITH_CHECK(mat_empty.validate());
}

XENITH_TEST(TestSparseMatrixTripletCSCAndCSR) {
    // 3x3 Matrix:
    // [ 1.0  0.0 -2.0 ]
    // [ 0.0  4.0  0.0 ]
    // [ 3.0  0.0  5.0 ]
    std::vector<Triplet> triplets = {
        {0, 0, 1.0},
        {0, 2, -2.0},
        {1, 1, 4.0},
        {2, 0, 3.0},
        {2, 2, 5.0}
    };

    SparseMatrix csc_mat = SparseMatrix::fromTriplets(3, 3, triplets, SparseStorageFormat::CSC);
    XENITH_CHECK_EQ(csc_mat.rows(), 3);
    XENITH_CHECK_EQ(csc_mat.cols(), 3);
    XENITH_CHECK_EQ(csc_mat.nonZeros(), 5);
    XENITH_CHECK_EQ(static_cast<int>(csc_mat.format()), static_cast<int>(SparseStorageFormat::CSC));
    XENITH_CHECK(csc_mat.validate());

    SparseMatrix csr_mat = SparseMatrix::fromTriplets(3, 3, triplets, SparseStorageFormat::CSR);
    XENITH_CHECK_EQ(csr_mat.rows(), 3);
    XENITH_CHECK_EQ(csr_mat.cols(), 3);
    XENITH_CHECK_EQ(csr_mat.nonZeros(), 5);
    XENITH_CHECK_EQ(static_cast<int>(csr_mat.format()), static_cast<int>(SparseStorageFormat::CSR));
    XENITH_CHECK(csr_mat.validate());
}

XENITH_TEST(TestSparseMatrixMultiplyAx) {
    // 3x3 Matrix A:
    // [ 1.0  0.0 -2.0 ]
    // [ 0.0  4.0  0.0 ]
    // [ 3.0  0.0  5.0 ]
    // Input x = [2.0, 3.0, 4.0]^T
    // A * x = [ 1*2 + 0*3 + (-2)*4 , 0*2 + 4*3 + 0*4 , 3*2 + 0*3 + 5*4 ]
    //       = [ 2 - 8, 12, 6 + 20 ] = [ -6.0, 12.0, 26.0 ]
    std::vector<Triplet> triplets = {
        {0, 0, 1.0}, {0, 2, -2.0},
        {1, 1, 4.0},
        {2, 0, 3.0}, {2, 2, 5.0}
    };

    SparseMatrix csc_mat = SparseMatrix::fromTriplets(3, 3, triplets, SparseStorageFormat::CSC);
    SparseMatrix csr_mat = SparseMatrix::fromTriplets(3, 3, triplets, SparseStorageFormat::CSR);

    std::vector<double> x = {2.0, 3.0, 4.0};
    std::vector<double> y_csc(3, 0.0);
    std::vector<double> y_csr(3, 0.0);

    csc_mat.multiply(x, y_csc);
    csr_mat.multiply(x, y_csr);

    XENITH_CHECK_NEAR(y_csc[0], -6.0, 1e-12);
    XENITH_CHECK_NEAR(y_csc[1], 12.0, 1e-12);
    XENITH_CHECK_NEAR(y_csc[2], 26.0, 1e-12);

    // Cross-representation verification
    XENITH_CHECK_NEAR(y_csr[0], y_csc[0], 1e-12);
    XENITH_CHECK_NEAR(y_csr[1], y_csc[1], 1e-12);
    XENITH_CHECK_NEAR(y_csr[2], y_csc[2], 1e-12);
}

XENITH_TEST(TestSparseMatrixMultiplyTranspose) {
    // 3x3 Matrix A:
    // [ 1.0  0.0 -2.0 ]
    // [ 0.0  4.0  0.0 ]
    // [ 3.0  0.0  5.0 ]
    // A^T:
    // [ 1.0  0.0  3.0 ]
    // [ 0.0  4.0  0.0 ]
    // [-2.0  0.0  5.0 ]
    // Input x = [1.0, 2.0, 3.0]^T
    // A^T * x = [ 1*1 + 0*2 + 3*3 , 0*1 + 4*2 + 0*3 , -2*1 + 0*2 + 5*3 ]
    //         = [ 1 + 9, 8, -2 + 15 ] = [ 10.0, 8.0, 13.0 ]
    std::vector<Triplet> triplets = {
        {0, 0, 1.0}, {0, 2, -2.0},
        {1, 1, 4.0},
        {2, 0, 3.0}, {2, 2, 5.0}
    };

    SparseMatrix csc_mat = SparseMatrix::fromTriplets(3, 3, triplets, SparseStorageFormat::CSC);
    SparseMatrix csr_mat = SparseMatrix::fromTriplets(3, 3, triplets, SparseStorageFormat::CSR);

    std::vector<double> x = {1.0, 2.0, 3.0};
    std::vector<double> y_csc(3, 0.0);
    std::vector<double> y_csr(3, 0.0);

    csc_mat.multiplyTranspose(x, y_csc);
    csr_mat.multiplyTranspose(x, y_csr);

    XENITH_CHECK_NEAR(y_csc[0], 10.0, 1e-12);
    XENITH_CHECK_NEAR(y_csc[1], 8.0, 1e-12);
    XENITH_CHECK_NEAR(y_csc[2], 13.0, 1e-12);

    // Cross-representation verification
    XENITH_CHECK_NEAR(y_csr[0], y_csc[0], 1e-12);
    XENITH_CHECK_NEAR(y_csr[1], y_csc[1], 1e-12);
    XENITH_CHECK_NEAR(y_csr[2], y_csc[2], 1e-12);
}

XENITH_TEST(TestSparseMatrixConversion) {
    std::vector<Triplet> triplets = {
        {0, 1, 3.5}, {1, 0, -1.2}, {2, 2, 7.0}
    };
    SparseMatrix csc_mat = SparseMatrix::fromTriplets(3, 3, triplets, SparseStorageFormat::CSC);
    SparseMatrix converted_csr = csc_mat.toCSR();

    XENITH_CHECK_EQ(static_cast<int>(converted_csr.format()), static_cast<int>(SparseStorageFormat::CSR));
    XENITH_CHECK(converted_csr.validate());

    SparseMatrix converted_back_csc = converted_csr.toCSC();
    XENITH_CHECK_EQ(static_cast<int>(converted_back_csc.format()), static_cast<int>(SparseStorageFormat::CSC));
    XENITH_CHECK(converted_back_csc.validate());
}

XENITH_TEST(TestSparseMatrixInvalidIndexDetection) {
    std::vector<Triplet> invalid_triplets = {
        {0, 0, 1.0},
        {5, 0, 2.0} // Row index 5 is out of bounds for 3x3 matrix
    };

    XENITH_CHECK_THROW(SparseMatrix::fromTriplets(3, 3, invalid_triplets, SparseStorageFormat::CSC), std::out_of_range);
}

int main() {
    return xenith::test::TestRunner::instance().runAll();
}
