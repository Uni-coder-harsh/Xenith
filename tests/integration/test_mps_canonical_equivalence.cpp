#include "tests/test_harness.hpp"
#include "xenith/model/canonical_model.hpp"
#include "xenith/model/model_validator.hpp"
#include "xenith/io/mps/mps_reader.hpp"
#include "xenith/numerics/vector_ops.hpp"
#include <vector>

using namespace xenith;
using namespace xenith::model;
using namespace xenith::numerics;
using namespace xenith::io::mps;

XENITH_TEST(TestMpsDirectModelEquivalence) {
    // -------------------------------------------------------------
    // METHOD A: Construct CanonicalModel directly in C++
    // -------------------------------------------------------------
    CanonicalModel modelA("MINIMAL_LP");
    modelA.setSense(ObjectiveSense::MAXIMIZE);

    Index a_x1 = modelA.addVariable("X1", 0.0, K_INFINITY, 3.0, VariableType::CONTINUOUS);
    Index a_x2 = modelA.addVariable("X2", 0.0, K_INFINITY, 5.0, VariableType::CONTINUOUS);

    Index a_c1 = modelA.addLessOrEqualRow("C1", 8.0);
    Index a_c2 = modelA.addLessOrEqualRow("C2", 12.0);

    std::vector<Triplet> tripletsA = {
        {a_c1, a_x1, 1.0}, {a_c1, a_x2, 2.0},
        {a_c2, a_x1, 3.0}, {a_c2, a_x2, 2.0}
    };
    modelA.setMatrixA(SparseMatrix::fromTriplets(2, 2, tripletsA, SparseStorageFormat::CSC));

    // Verify Method A validation
    ValidationResult valA = ModelValidator::validate(modelA);
    XENITH_CHECK(valA.isValid);

    // -------------------------------------------------------------
    // METHOD B: Read equivalent MPS file
    // -------------------------------------------------------------
    std::string mps_content = R"(NAME          MINIMAL_LP
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

    CanonicalModel modelB = MpsReader::readFromString(mps_content, "equivalence_fixture");

    // Verify Method B validation
    ValidationResult valB = ModelValidator::validate(modelB);
    XENITH_CHECK(valB.isValid);

    // -------------------------------------------------------------
    // EQUIVALENCE COMPARISON
    // -------------------------------------------------------------
    XENITH_CHECK_EQ(modelA.numVariables(), modelB.numVariables());
    XENITH_CHECK_EQ(modelA.numConstraints(), modelB.numConstraints());
    XENITH_CHECK_EQ(static_cast<int>(modelA.sense()), static_cast<int>(modelB.sense()));

    // Variable order & bounds
    for (Index j = 0; j < modelA.numVariables(); ++j) {
        std::string vname = std::string(modelA.varNames()[j]);
        auto b_idx_opt = modelB.getVariableIndex(vname);
        XENITH_CHECK(b_idx_opt.has_value());
        Index b_j = b_idx_opt.value();

        XENITH_CHECK_NEAR(modelA.objective()[j], modelB.objective()[b_j], 1e-12);
        XENITH_CHECK_NEAR(modelA.colLower()[j], modelB.colLower()[b_j], 1e-12);
        XENITH_CHECK_NEAR(modelA.colUpper()[j], modelB.colUpper()[b_j], 1e-12);
        XENITH_CHECK_EQ(static_cast<int>(modelA.varTypes()[j]), static_cast<int>(modelB.varTypes()[b_j]));
    }

    // Constraint row order & bounds
    for (Index i = 0; i < modelA.numConstraints(); ++i) {
        std::string rname = std::string(modelA.rowNames()[i]);
        auto b_idx_opt = modelB.getRowIndex(rname);
        XENITH_CHECK(b_idx_opt.has_value());
        Index b_i = b_idx_opt.value();

        XENITH_CHECK_NEAR(modelA.rowLower()[i], modelB.rowLower()[b_i], 1e-12);
        XENITH_CHECK_NEAR(modelA.rowUpper()[i], modelB.rowUpper()[b_i], 1e-12);
    }

    // Matrix multiplication equivalence
    std::vector<double> x = {2.0, 3.0};
    std::vector<double> yA(2, 0.0);
    std::vector<double> yB(2, 0.0);

    modelA.matrixA().multiply(x, yA);
    modelB.matrixA().multiply(x, yB);

    XENITH_CHECK_NEAR(yA[0], yB[0], 1e-12);
    XENITH_CHECK_NEAR(yA[1], yB[1], 1e-12);
}

int main() {
    return xenith::test::TestRunner::instance().runAll();
}
