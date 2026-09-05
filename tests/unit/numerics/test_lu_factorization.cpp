#include <iostream>
#include <vector>
#include "xenith/numerics/sparse_matrix.hpp"
#include "xenith/numerics/lu_factorization.hpp"
#include "tests/test_harness.hpp"

using namespace xenith;
using namespace xenith::numerics;

XENITH_TEST(TestLuFactorization2x2) {
    // B = [ 2  1 ]
    //     [ 1  3 ]
    std::vector<Triplet> triplets = {
        {0, 0, 2.0}, {0, 1, 1.0},
        {1, 0, 1.0}, {1, 1, 3.0}
    };
    SparseMatrix B = SparseMatrix::fromTriplets(2, 2, triplets, SparseStorageFormat::CSC);

    LuFactorization lu;
    XENITH_CHECK(lu.factorize(B));
    XENITH_CHECK(!lu.isSingular());

    // Test FTRAN: B * y = [5, 10]^T
    // 2 y1 + y2 = 5
    // y1 + 3 y2 = 10
    // => y1 = 1, y2 = 3.
    std::vector<double> rhs = {5.0, 10.0};
    std::vector<double> sol(2, 0.0);
    lu.solveFtran(rhs, sol);

    XENITH_CHECK_NEAR(sol[0], 1.0, 1e-9);
    XENITH_CHECK_NEAR(sol[1], 3.0, 1e-9);

    // Test BTRAN: B^T * y = [4, 7]^T
    // Since B is symmetric, B^T = B.
    // 2 y1 + y2 = 4
    // y1 + 3 y2 = 7
    // => y1 = 1, y2 = 2.
    std::vector<double> rhs_b = {4.0, 7.0};
    std::vector<double> sol_b(2, 0.0);
    lu.solveBtran(rhs_b, sol_b);

    XENITH_CHECK_NEAR(sol_b[0], 1.0, 1e-9);
    XENITH_CHECK_NEAR(sol_b[1], 2.0, 1e-9);
}

XENITH_TEST(TestLuFactorizationAsymmetric3x3) {
    // B = [ 1  2  0 ]
    //     [ 0  1  3 ]
    //     [ 2  0  1 ]
    std::vector<Triplet> triplets = {
        {0, 0, 1.0}, {0, 1, 2.0},
        {1, 1, 1.0}, {1, 2, 3.0},
        {2, 0, 2.0}, {2, 2, 1.0}
    };
    SparseMatrix B = SparseMatrix::fromTriplets(3, 3, triplets, SparseStorageFormat::CSC);

    LuFactorization lu;
    XENITH_CHECK(lu.factorize(B));

    // Test FTRAN: B * y = [4, 7, 6]^T
    // y1 + 2 y2 = 4
    // y2 + 3 y3 = 7
    // 2 y1 + y3 = 6
    // Solution: y = [2, 1, 2]^T.
    std::vector<double> rhs = {4.0, 7.0, 6.0};
    std::vector<double> sol(3, 0.0);
    lu.solveFtran(rhs, sol);

    XENITH_CHECK_NEAR(sol[0], 2.0, 1e-9);
    XENITH_CHECK_NEAR(sol[1], 1.0, 1e-9);
    XENITH_CHECK_NEAR(sol[2], 2.0, 1e-9);

    // Test BTRAN: B^T * y = [5, 3, 5]^T
    // y1 + 2 y3 = 5
    // 2 y1 + y2 = 3
    // 3 y2 + y3 = 5
    // Solution: y1 = 1, y2 = 1, y3 = 2.
    std::vector<double> rhs_b = {5.0, 3.0, 5.0};
    std::vector<double> sol_b(3, 0.0);
    lu.solveBtran(rhs_b, sol_b);

    XENITH_CHECK_NEAR(sol_b[0], 1.0, 1e-9);
    XENITH_CHECK_NEAR(sol_b[1], 1.0, 1e-9);
    XENITH_CHECK_NEAR(sol_b[2], 2.0, 1e-9);
}

XENITH_TEST(TestLuFactorizationSingularMatrix) {
    // B = [ 1  2 ]
    //     [ 2  4 ]  (Dependent rows)
    std::vector<Triplet> triplets = {
        {0, 0, 1.0}, {0, 1, 2.0},
        {1, 0, 2.0}, {1, 1, 4.0}
    };
    SparseMatrix B = SparseMatrix::fromTriplets(2, 2, triplets, SparseStorageFormat::CSC);

    LuFactorization lu;
    XENITH_CHECK(!lu.factorize(B));
    XENITH_CHECK(lu.isSingular());
}

int main() {
    return xenith::test::TestRunner::instance().runAll();
}
