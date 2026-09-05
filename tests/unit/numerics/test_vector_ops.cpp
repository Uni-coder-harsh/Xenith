#include "tests/test_harness.hpp"
#include "xenith/numerics/vector_ops.hpp"
#include <vector>

using namespace xenith;
using namespace xenith::numerics;

XENITH_TEST(TestVectorDotProduct) {
    std::vector<double> x = {1.0, 2.0, 3.0};
    std::vector<double> y = {4.0, -5.0, 6.0};

    // 1*4 + 2*(-5) + 3*6 = 4 - 10 + 18 = 12.0
    double val = dot(x, y);
    XENITH_CHECK_NEAR(val, 12.0, 1e-12);

    std::vector<double> z = {1.0, 2.0};
    XENITH_CHECK_THROW(dot(x, z), std::invalid_argument);
}

XENITH_TEST(TestVectorAXPY) {
    std::vector<double> x = {1.0, -2.0, 3.0};
    std::vector<double> y = {10.0, 20.0, 30.0};

    axpy(2.5, x, y);
    // y = 2.5*x + y = [12.5, 15.0, 37.5]
    XENITH_CHECK_NEAR(y[0], 12.5, 1e-12);
    XENITH_CHECK_NEAR(y[1], 15.0, 1e-12);
    XENITH_CHECK_NEAR(y[2], 37.5, 1e-12);

    std::vector<double> z = {1.0};
    XENITH_CHECK_THROW(axpy(1.0, x, z), std::invalid_argument);
}

XENITH_TEST(TestVectorScale) {
    std::vector<double> x = {2.0, -4.0, 8.0};
    scale(0.5, x);
    XENITH_CHECK_NEAR(x[0], 1.0, 1e-12);
    XENITH_CHECK_NEAR(x[1], -2.0, 1e-12);
    XENITH_CHECK_NEAR(x[2], 4.0, 1e-12);
}

XENITH_TEST(TestVectorAddSub) {
    std::vector<double> x = {5.0, 10.0, 15.0};
    std::vector<double> y = {1.0, 2.0, 3.0};
    std::vector<double> res(3, 0.0);

    vectorAdd(x, y, res);
    XENITH_CHECK_NEAR(res[0], 6.0, 1e-12);
    XENITH_CHECK_NEAR(res[1], 12.0, 1e-12);
    XENITH_CHECK_NEAR(res[2], 18.0, 1e-12);

    vectorSub(x, y, res);
    XENITH_CHECK_NEAR(res[0], 4.0, 1e-12);
    XENITH_CHECK_NEAR(res[1], 8.0, 1e-12);
    XENITH_CHECK_NEAR(res[2], 12.0, 1e-12);
}

XENITH_TEST(TestVectorNorms) {
    std::vector<double> x = {-3.0, 4.0, -2.0};
    XENITH_CHECK_NEAR(infinityNorm(x), 4.0, 1e-12);
    // sqrt(9 + 16 + 4) = sqrt(29) ~= 5.385164807134504
    XENITH_CHECK_NEAR(euclideanNorm(x), std::sqrt(29.0), 1e-12);
}

int main() {
    return xenith::test::TestRunner::instance().runAll();
}
