#include <iostream>
#include <vector>
#include "xenith/solver/common/basis_manager.hpp"
#include "tests/test_harness.hpp"

using namespace xenith;
using namespace xenith::solver;

XENITH_TEST(TestBasisManagerDefaultInitialization) {
    BasisManager bm;
    Index m = 3;
    Index n = 4;
    Index total = n + m; // 7

    std::vector<double> lower = {0.0, -10.0, 5.0, -K_INFINITY, 0.0, 0.0, 0.0};
    std::vector<double> upper = {10.0, 10.0, 5.0, K_INFINITY, K_INFINITY, K_INFINITY, K_INFINITY};

    bm.initializeDefault(m, total, lower, upper);

    std::string err;
    XENITH_CHECK(bm.validateInvariants(&err));

    XENITH_CHECK_EQ(bm.numRows(), 3);
    XENITH_CHECK_EQ(bm.totalVars(), 7);

    // Slacks (indices 4, 5, 6) should be BASIC
    XENITH_CHECK(bm.status(4) == BasisStatus::BASIC);
    XENITH_CHECK(bm.status(5) == BasisStatus::BASIC);
    XENITH_CHECK(bm.status(6) == BasisStatus::BASIC);

    XENITH_CHECK_EQ(bm.basicVariable(0), 4);
    XENITH_CHECK_EQ(bm.basicVariable(1), 5);
    XENITH_CHECK_EQ(bm.basicVariable(2), 6);

    // Structurals
    XENITH_CHECK(bm.status(0) == BasisStatus::NON_BASIC_AT_LOWER);
    XENITH_CHECK(bm.status(1) == BasisStatus::NON_BASIC_AT_LOWER);
    XENITH_CHECK(bm.status(2) == BasisStatus::FIXED);
    XENITH_CHECK(bm.status(3) == BasisStatus::FREE);
}

XENITH_TEST(TestBasisManagerPivot) {
    BasisManager bm;
    Index m = 2;
    Index n = 3;
    Index total = n + m; // 5

    std::vector<double> lower = {0.0, 0.0, 0.0, 0.0, 0.0};
    std::vector<double> upper = {10.0, 10.0, 10.0, K_INFINITY, K_INFINITY};

    bm.initializeDefault(m, total, lower, upper);
    XENITH_CHECK(bm.validateInvariants());

    // Pivot entering var 0 into position 1 (replacing slack 4)
    bm.pivot(0, 1, BasisStatus::NON_BASIC_AT_LOWER);

    XENITH_CHECK(bm.validateInvariants());
    XENITH_CHECK(bm.status(0) == BasisStatus::BASIC);
    XENITH_CHECK(bm.status(4) == BasisStatus::NON_BASIC_AT_LOWER);
    XENITH_CHECK_EQ(bm.basicVariable(1), 0);
    XENITH_CHECK_EQ(bm.basisPosition(0), 1);
    XENITH_CHECK_EQ(bm.basisPosition(4), K_INVALID_INDEX);
}

int main() {
    return xenith::test::TestRunner::instance().runAll();
}
