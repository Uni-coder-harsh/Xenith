#include "tests/test_harness.hpp"
#include "xenith/model/canonical_model.hpp"
#include "xenith/model/model_validator.hpp"

using namespace xenith;
using namespace xenith::model;
using namespace xenith::numerics;

XENITH_TEST(TestModelValidatorValidModel) {
    CanonicalModel model("valid_model");
    model.addVariable("x1", 0.0, 10.0, 1.0);
    model.addVariable("x2", 0.0, 5.0, 2.0);
    model.addLessOrEqualRow("r1", 8.0);

    std::vector<Triplet> triplets = {{0, 0, 1.0}, {0, 1, 2.0}};
    model.setMatrixA(SparseMatrix::fromTriplets(1, 2, triplets));

    ValidationResult result = ModelValidator::validate(model);
    XENITH_CHECK(result.isValid);
    XENITH_CHECK_EQ(result.errorCount(), 0);
}

XENITH_TEST(TestModelValidatorDimensionMismatch) {
    CanonicalModel model("invalid_dim_model");
    model.addVariable("x1", 0.0, 10.0, 1.0);
    model.addLessOrEqualRow("r1", 8.0);

    // Matrix size 2x2 does not match model 1 constraint x 1 variable (1x1)
    std::vector<Triplet> triplets = {{0, 0, 1.0}, {1, 1, 2.0}};
    model.setMatrixA(SparseMatrix::fromTriplets(2, 2, triplets));

    ValidationResult result = ModelValidator::validate(model);
    XENITH_CHECK(!result.isValid);
    XENITH_CHECK(result.errorCount() > 0);
}

XENITH_TEST(TestModelValidatorReversedVariableBound) {
    CanonicalModel model("reversed_bound_model");
    model.addVariable("x1", 10.0, 5.0, 1.0); // Invalid lower 10 > upper 5
    model.addLessOrEqualRow("r1", 8.0);

    std::vector<Triplet> triplets = {{0, 0, 1.0}};
    model.setMatrixA(SparseMatrix::fromTriplets(1, 1, triplets));

    ValidationResult result = ModelValidator::validate(model);
    XENITH_CHECK(!result.isValid);
    XENITH_CHECK(result.errorCount() >= 1);

    std::string summary = result.summary();
    XENITH_CHECK(summary.find("Invalid variable bound") != std::string::npos);
    XENITH_CHECK(summary.find("lower_bound = 10") != std::string::npos);
    XENITH_CHECK(summary.find("upper_bound = 5") != std::string::npos);
}

XENITH_TEST(TestModelValidatorReversedRowBound) {
    CanonicalModel model("reversed_row_model");
    model.addVariable("x1", 0.0, 10.0, 1.0);
    model.addRangeRow("r1", 15.0, 2.0); // Invalid lower 15 > upper 2

    std::vector<Triplet> triplets = {{0, 0, 1.0}};
    model.setMatrixA(SparseMatrix::fromTriplets(1, 1, triplets));

    ValidationResult result = ModelValidator::validate(model);
    XENITH_CHECK(!result.isValid);
    XENITH_CHECK(result.errorCount() >= 1);

    std::string summary = result.summary();
    XENITH_CHECK(summary.find("Invalid constraint row bound") != std::string::npos);
}

int main() {
    return xenith::test::TestRunner::instance().runAll();
}
