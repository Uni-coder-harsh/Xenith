#include "tests/test_harness.hpp"
#include "xenith/model/canonical_model.hpp"
#include "xenith/model/model_validator.hpp"
#include "xenith/numerics/vector_ops.hpp"
#include <vector>

using namespace xenith;
using namespace xenith::model;
using namespace xenith::numerics;

XENITH_TEST(TestEndToEndModelNumericsPipeline) {
    // Construct sample canonical LP model:
    // Maximize   3 x1 + 5 x2
    // Subject to x1 + 2 x2 <= 8    (row 0)
    //           3 x1 + 2 x2 <= 12   (row 1)
    //           x1 >= 0, x2 >= 0
    CanonicalModel model("sample_lp");
    model.setSense(ObjectiveSense::MAXIMIZE);

    Index x1 = model.addVariable("x1", 0.0, K_INFINITY, 3.0, VariableType::CONTINUOUS);
    Index x2 = model.addVariable("x2", 0.0, K_INFINITY, 5.0, VariableType::CONTINUOUS);

    Index r1 = model.addLessOrEqualRow("c1", 8.0);
    Index r2 = model.addLessOrEqualRow("c2", 12.0);

    XENITH_CHECK_EQ(model.numVariables(), 2);
    XENITH_CHECK_EQ(model.numConstraints(), 2);

    // Matrix A triplets:
    // Row 0 (c1): x1 + 2*x2
    // Row 1 (c2): 3*x1 + 2*x2
    std::vector<Triplet> triplets = {
        {r1, x1, 1.0}, {r1, x2, 2.0},
        {r2, x1, 3.0}, {r2, x2, 2.0}
    };

    SparseMatrix mat_csc = SparseMatrix::fromTriplets(2, 2, triplets, SparseStorageFormat::CSC);
    model.setMatrixA(mat_csc);

    // 1. Validate Model Invariants
    ValidationResult val_res = ModelValidator::validate(model);
    XENITH_CHECK(val_res.isValid);
    XENITH_CHECK_EQ(val_res.errorCount(), 0);

    // 2. Test SpMV multiplication y = A * x for test vector x = [2.0, 3.0]^T
    // A * [2, 3]^T = [ 1*2 + 2*3, 3*2 + 2*3 ]^T = [ 8.0, 12.0 ]^T
    std::vector<double> x_test = {2.0, 3.0};
    std::vector<double> y_csc(2, 0.0);
    std::vector<double> y_csr(2, 0.0);

    model.matrixA().multiply(x_test, y_csc);

    SparseMatrix mat_csr = model.matrixA().toCSR();
    mat_csr.multiply(x_test, y_csr);

    XENITH_CHECK_NEAR(y_csc[0], 8.0, 1e-12);
    XENITH_CHECK_NEAR(y_csc[1], 12.0, 1e-12);

    // Cross-representation equality verification
    XENITH_CHECK_NEAR(y_csr[0], y_csc[0], 1e-12);
    XENITH_CHECK_NEAR(y_csr[1], y_csc[1], 1e-12);

    // 3. Test Transpose SpMV y = A^T * dual for dual vector y_dual = [1.0, 1.0]^T
    // A^T * [1, 1]^T = [ 1*1 + 3*1, 2*1 + 2*1 ]^T = [ 4.0, 4.0 ]^T
    std::vector<double> y_dual = {1.0, 1.0};
    std::vector<double> ATy_csc(2, 0.0);
    std::vector<double> ATy_csr(2, 0.0);

    model.matrixA().multiplyTranspose(y_dual, ATy_csc);
    mat_csr.multiplyTranspose(y_dual, ATy_csr);

    XENITH_CHECK_NEAR(ATy_csc[0], 4.0, 1e-12);
    XENITH_CHECK_NEAR(ATy_csc[1], 4.0, 1e-12);

    XENITH_CHECK_NEAR(ATy_csr[0], ATy_csc[0], 1e-12);
    XENITH_CHECK_NEAR(ATy_csr[1], ATy_csc[1], 1e-12);

    // 4. Objective Dot Product Verification c^T x = 3*2 + 5*3 = 21.0
    double obj_val = dot(model.objective(), x_test);
    XENITH_CHECK_NEAR(obj_val, 21.0, 1e-12);
}

int main() {
    return xenith::test::TestRunner::instance().runAll();
}
