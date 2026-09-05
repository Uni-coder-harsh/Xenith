#include "tests/test_harness.hpp"
#include "xenith/io/mps/mps_reader.hpp"
#include "xenith/model/model_validator.hpp"
#include <filesystem>

using namespace xenith;
using namespace xenith::model;
using namespace xenith::io::mps;

static std::string resolveFixturePath(const std::string& filename) {
    std::vector<std::string> candidates = {
        "tests/data/mps/" + filename,
        "../tests/data/mps/" + filename,
        "../../tests/data/mps/" + filename,
        "../../../tests/data/mps/" + filename
    };
    for (const auto& path : candidates) {
        if (std::filesystem::exists(path)) {
            return path;
        }
    }
    return "tests/data/mps/" + filename;
}

XENITH_TEST(TestMpsReaderMinimalLpFromString) {
    std::string content = R"(NAME          MINIMAL_LP
OBJSENSE
    MAXIMIZE
ROWS
 N  OBJ
 L  C1
 L  C2
COLUMNS
    X1        OBJ       3.0       C1        1.0
    X1        C2        3.0
    X2        OBJ       5.0       C1        2.0
    X2        C2        2.0
RHS
    RHS1      C1        8.0       C2        12.0
BOUNDS
ENDATA
)";

    CanonicalModel model = MpsReader::readFromString(content, "test_string");

    XENITH_CHECK_EQ(model.name(), "MINIMAL_LP");
    XENITH_CHECK_EQ(static_cast<int>(model.sense()), static_cast<int>(ObjectiveSense::MAXIMIZE));
    XENITH_CHECK_EQ(model.numVariables(), 2);
    XENITH_CHECK_EQ(model.numConstraints(), 2);

    XENITH_CHECK_NEAR(model.objective()[0], 3.0, 1e-12);
    XENITH_CHECK_NEAR(model.objective()[1], 5.0, 1e-12);

    XENITH_CHECK_NEAR(model.rowUpper()[0], 8.0, 1e-12);
    XENITH_CHECK_NEAR(model.rowUpper()[1], 12.0, 1e-12);

    ValidationResult val_res = ModelValidator::validate(model);
    XENITH_CHECK(val_res.isValid);
}

XENITH_TEST(TestMpsReaderBoundTypes) {
    std::string content = R"(NAME          BOUND_TYPES
ROWS
 N  COST
 L  ROW1
COLUMNS
    X_LO      COST      1.0       ROW1      1.0
    X_UP      COST      2.0       ROW1      1.0
    X_FX      COST      3.0       ROW1      1.0
    X_FR      COST      4.0       ROW1      1.0
    X_MI      COST      5.0       ROW1      1.0
    X_PL      COST      6.0       ROW1      1.0
    X_BV      COST      7.0       ROW1      1.0
RHS
    RHS1      ROW1      100.0
BOUNDS
 LO BNDSET    X_LO      2.0
 UP BNDSET    X_UP      10.0
 FX BNDSET    X_FX      5.0
 FR BNDSET    X_FR
 MI BNDSET    X_MI
 PL BNDSET    X_PL
 BV BNDSET    X_BV
ENDATA
)";

    CanonicalModel model = MpsReader::readFromString(content, "bound_test");
    XENITH_CHECK_EQ(model.numVariables(), 7);

    // X_LO: 2.0 <= x <= +inf
    auto idx_lo = model.getVariableIndex("X_LO").value();
    XENITH_CHECK_NEAR(model.colLower()[idx_lo], 2.0, 1e-12);
    XENITH_CHECK(isPositiveInfinity(model.colUpper()[idx_lo]));

    // X_UP: 0.0 <= x <= 10.0
    auto idx_up = model.getVariableIndex("X_UP").value();
    XENITH_CHECK_NEAR(model.colLower()[idx_up], 0.0, 1e-12);
    XENITH_CHECK_NEAR(model.colUpper()[idx_up], 10.0, 1e-12);

    // X_FX: 5.0 <= x <= 5.0
    auto idx_fx = model.getVariableIndex("X_FX").value();
    XENITH_CHECK_NEAR(model.colLower()[idx_fx], 5.0, 1e-12);
    XENITH_CHECK_NEAR(model.colUpper()[idx_fx], 5.0, 1e-12);

    // X_FR: -inf <= x <= +inf
    auto idx_fr = model.getVariableIndex("X_FR").value();
    XENITH_CHECK(isNegativeInfinity(model.colLower()[idx_fr]));
    XENITH_CHECK(isPositiveInfinity(model.colUpper()[idx_fr]));

    // X_BV: 0.0 <= x <= 1.0, BINARY
    auto idx_bv = model.getVariableIndex("X_BV").value();
    XENITH_CHECK_NEAR(model.colLower()[idx_bv], 0.0, 1e-12);
    XENITH_CHECK_NEAR(model.colUpper()[idx_bv], 1.0, 1e-12);
    XENITH_CHECK_EQ(static_cast<int>(model.varTypes()[idx_bv]), static_cast<int>(VariableType::BINARY));
}

XENITH_TEST(TestMpsReaderIntegerMarkers) {
    std::string content = R"(NAME          INTEGER_MARKERS
ROWS
 N  COST
 L  CON1
COLUMNS
    X_CONT    COST      1.0       CON1      1.0
    MARK0000  'MARKER'            'INTORG'
    X_INT1    COST      2.0       CON1      3.0
    X_INT2    COST      4.0       CON1      5.0
    MARK0001  'MARKER'            'INTEND'
    X_CONT2   COST      6.0       CON1      7.0
RHS
    RHS1      CON1      50.0
BOUNDS
ENDATA
)";

    CanonicalModel model = MpsReader::readFromString(content, "marker_test");
    XENITH_CHECK_EQ(model.numVariables(), 4);
    XENITH_CHECK(!model.getVariableIndex("MARK0000").has_value());
    XENITH_CHECK(!model.getVariableIndex("MARK0001").has_value());

    auto idx_int1 = model.getVariableIndex("X_INT1").value();
    auto idx_int2 = model.getVariableIndex("X_INT2").value();
    auto idx_cont2 = model.getVariableIndex("X_CONT2").value();

    XENITH_CHECK_EQ(static_cast<int>(model.varTypes()[idx_int1]), static_cast<int>(VariableType::GENERAL_INTEGER));
    XENITH_CHECK_EQ(static_cast<int>(model.varTypes()[idx_int2]), static_cast<int>(VariableType::GENERAL_INTEGER));
    XENITH_CHECK_EQ(static_cast<int>(model.varTypes()[idx_cont2]), static_cast<int>(VariableType::CONTINUOUS));
}

XENITH_TEST(TestMpsReaderDuplicateCoefficients) {
    std::string content = R"(NAME          DUPLICATE_COEFFS
ROWS
 N  COST
 L  ROW1
COLUMNS
    X1        ROW1      2.0
    X1        ROW1      3.0
    X1        COST      1.0
    X1        COST      4.0
RHS
    RHS1      ROW1      10.0
BOUNDS
ENDATA
)";

    CanonicalModel model = MpsReader::readFromString(content, "duplicate_test");
    XENITH_CHECK_EQ(model.numVariables(), 1);
    XENITH_CHECK_EQ(model.numConstraints(), 1);

    XENITH_CHECK_NEAR(model.objective()[0], 5.0, 1e-12);

    std::vector<double> x = {1.0};
    std::vector<double> y(1, 0.0);
    model.matrixA().multiply(x, y);
    XENITH_CHECK_NEAR(y[0], 5.0, 1e-12);
}

XENITH_TEST(TestMpsReaderFileIOFixtures) {
    // 1. Read minimal_lp.mps
    CanonicalModel model1 = MpsReader::readFromFile(resolveFixturePath("minimal_lp.mps"));
    XENITH_CHECK_EQ(model1.name(), "MINIMAL_LP");
    XENITH_CHECK_EQ(model1.numVariables(), 2);
    XENITH_CHECK_EQ(model1.numConstraints(), 2);
    XENITH_CHECK(ModelValidator::validate(model1).isValid);

    // 2. Read equality_row.mps
    CanonicalModel model2 = MpsReader::readFromFile(resolveFixturePath("equality_row.mps"));
    XENITH_CHECK_EQ(model2.numConstraints(), 1);
    XENITH_CHECK_NEAR(model2.rowLower()[0], 10.0, 1e-12);
    XENITH_CHECK_NEAR(model2.rowUpper()[0], 10.0, 1e-12);
    XENITH_CHECK(ModelValidator::validate(model2).isValid);

    // 3. Read greater_than_row.mps
    CanonicalModel model3 = MpsReader::readFromFile(resolveFixturePath("greater_than_row.mps"));
    XENITH_CHECK_EQ(model3.numConstraints(), 1);
    XENITH_CHECK_NEAR(model3.rowLower()[0], 15.0, 1e-12);
    XENITH_CHECK(isPositiveInfinity(model3.rowUpper()[0]));
    XENITH_CHECK(ModelValidator::validate(model3).isValid);

    // 4. Read free_format.mps
    CanonicalModel model4 = MpsReader::readFromFile(resolveFixturePath("free_format.mps"));
    XENITH_CHECK_EQ(model4.name(), "FREE_FORMAT_LP");
    XENITH_CHECK_EQ(model4.numVariables(), 2);
    XENITH_CHECK_EQ(model4.numConstraints(), 2);
    XENITH_CHECK(ModelValidator::validate(model4).isValid);
}

XENITH_TEST(TestMpsReaderMalformedInputs) {
    std::string no_rows = R"(NAME NO_ROWS
COLUMNS
    X1 C1 1.0
ENDATA)";
    XENITH_CHECK_THROW(MpsReader::readFromString(no_rows), MpsParseException);

    std::string invalid_row_type = R"(NAME BAD_ROW
ROWS
 X BAD_TYPE
COLUMNS
ENDATA)";
    XENITH_CHECK_THROW(MpsReader::readFromString(invalid_row_type), MpsParseException);
}

int main() {
    return xenith::test::TestRunner::instance().runAll();
}
