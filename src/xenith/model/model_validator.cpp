#include "xenith/model/model_validator.hpp"
#include <sstream>
#include <iomanip>

namespace xenith::model {

std::string ValidationDiagnostic::toString() const {
    std::ostringstream oss;
    switch (severity) {
        case DiagnosticSeverity::ERROR: oss << "[ERROR] "; break;
        case DiagnosticSeverity::WARNING: oss << "[WARNING] "; break;
        case DiagnosticSeverity::INFO: oss << "[INFO] "; break;
    }
    oss << "(" << code << ") " << component;
    if (index.has_value()) {
        oss << "[" << index.value() << "]";
    }
    oss << ": " << message;
    return oss.str();
}

std::string ValidationResult::summary() const {
    std::ostringstream oss;
    oss << "Model Validation Status: " << (isValid ? "PASSED" : "FAILED")
        << " (Errors: " << errorCount() << ", Warnings: " << warningCount() << ")\n";
    for (const auto& diag : diagnostics) {
        oss << "  - " << diag.toString() << "\n";
    }
    return oss.str();
}

std::size_t ValidationResult::errorCount() const {
    std::size_t count = 0;
    for (const auto& d : diagnostics) {
        if (d.severity == DiagnosticSeverity::ERROR) count++;
    }
    return count;
}

std::size_t ValidationResult::warningCount() const {
    std::size_t count = 0;
    for (const auto& d : diagnostics) {
        if (d.severity == DiagnosticSeverity::WARNING) count++;
    }
    return count;
}

ValidationResult ModelValidator::validate(const CanonicalModel& model) {
    ValidationResult result;
    result.isValid = true;

    Index n = model.numVariables();
    Index m = model.numConstraints();

    // 1. Dimensional Invariants
    if (model.objective().size() != n) {
        result.isValid = false;
        result.diagnostics.push_back({
            DiagnosticSeverity::ERROR,
            "DIM_OBJ_MISMATCH",
            "Objective size (" + std::to_string(model.objective().size()) + ") != numVariables (" + std::to_string(n) + ")",
            "OBJECTIVE",
            std::nullopt
        });
    }

    if (model.colLower().size() != n) {
        result.isValid = false;
        result.diagnostics.push_back({
            DiagnosticSeverity::ERROR,
            "DIM_LOWER_MISMATCH",
            "Column lower bound size (" + std::to_string(model.colLower().size()) + ") != numVariables (" + std::to_string(n) + ")",
            "VARIABLE",
            std::nullopt
        });
    }

    if (model.colUpper().size() != n) {
        result.isValid = false;
        result.diagnostics.push_back({
            DiagnosticSeverity::ERROR,
            "DIM_UPPER_MISMATCH",
            "Column upper bound size (" + std::to_string(model.colUpper().size()) + ") != numVariables (" + std::to_string(n) + ")",
            "VARIABLE",
            std::nullopt
        });
    }

    if (model.varTypes().size() != n) {
        result.isValid = false;
        result.diagnostics.push_back({
            DiagnosticSeverity::ERROR,
            "DIM_TYPES_MISMATCH",
            "Variable types size (" + std::to_string(model.varTypes().size()) + ") != numVariables (" + std::to_string(n) + ")",
            "VARIABLE",
            std::nullopt
        });
    }

    if (model.rowLower().size() != m) {
        result.isValid = false;
        result.diagnostics.push_back({
            DiagnosticSeverity::ERROR,
            "DIM_ROW_LOWER_MISMATCH",
            "Row lower bound size (" + std::to_string(model.rowLower().size()) + ") != numConstraints (" + std::to_string(m) + ")",
            "CONSTRAINT",
            std::nullopt
        });
    }

    if (model.rowUpper().size() != m) {
        result.isValid = false;
        result.diagnostics.push_back({
            DiagnosticSeverity::ERROR,
            "DIM_ROW_UPPER_MISMATCH",
            "Row upper bound size (" + std::to_string(model.rowUpper().size()) + ") != numConstraints (" + std::to_string(m) + ")",
            "CONSTRAINT",
            std::nullopt
        });
    }

    // Matrix A dimension check
    if (model.matrixA().rows() != m) {
        result.isValid = false;
        result.diagnostics.push_back({
            DiagnosticSeverity::ERROR,
            "DIM_MAT_ROWS_MISMATCH",
            "Matrix A rows (" + std::to_string(model.matrixA().rows()) + ") != numConstraints (" + std::to_string(m) + ")",
            "MATRIX",
            std::nullopt
        });
    }

    if (model.matrixA().cols() != n) {
        result.isValid = false;
        result.diagnostics.push_back({
            DiagnosticSeverity::ERROR,
            "DIM_MAT_COLS_MISMATCH",
            "Matrix A cols (" + std::to_string(model.matrixA().cols()) + ") != numVariables (" + std::to_string(n) + ")",
            "MATRIX",
            std::nullopt
        });
    }

    // 2. Bound Invariants for Variables
    for (Index j = 0; j < n; ++j) {
        double lower = model.colLower()[j];
        double upper = model.colUpper()[j];

        if (isNaN(lower) || isNaN(upper)) {
            result.isValid = false;
            result.diagnostics.push_back({
                DiagnosticSeverity::ERROR,
                "BOUND_NAN",
                "Variable bound contains NaN: lower_bound = " + std::to_string(lower) + ", upper_bound = " + std::to_string(upper),
                "VARIABLE",
                j
            });
            continue;
        }

        if (lower > upper + K_BOUND_TOLERANCE) {
            result.isValid = false;
            std::ostringstream msg;
            msg << "Invalid variable bound: variable = " << j
                << ", lower_bound = " << lower
                << ", upper_bound = " << upper
                << ", expected lower_bound <= upper_bound";
            result.diagnostics.push_back({
                DiagnosticSeverity::ERROR,
                "VAR_BOUND_REVERSED",
                msg.str(),
                "VARIABLE",
                j
            });
        }
    }

    // Bound Invariants for Rows
    for (Index i = 0; i < m; ++i) {
        double lower = model.rowLower()[i];
        double upper = model.rowUpper()[i];

        if (isNaN(lower) || isNaN(upper)) {
            result.isValid = false;
            result.diagnostics.push_back({
                DiagnosticSeverity::ERROR,
                "ROW_BOUND_NAN",
                "Row bound contains NaN: lower_bound = " + std::to_string(lower) + ", upper_bound = " + std::to_string(upper),
                "CONSTRAINT",
                i
            });
            continue;
        }

        if (lower > upper + K_BOUND_TOLERANCE) {
            result.isValid = false;
            std::ostringstream msg;
            msg << "Invalid constraint row bound: row = " << i
                << ", lower_bound = " << lower
                << ", upper_bound = " << upper
                << ", expected lower_bound <= upper_bound";
            result.diagnostics.push_back({
                DiagnosticSeverity::ERROR,
                "ROW_BOUND_REVERSED",
                msg.str(),
                "CONSTRAINT",
                i
            });
        }
    }

    // 3. Numerical Invariants for Objective Vector
    for (Index j = 0; j < n; ++j) {
        double c = model.objective()[j];
        if (!isFinite(c)) {
            result.isValid = false;
            result.diagnostics.push_back({
                DiagnosticSeverity::ERROR,
                "OBJ_NON_FINITE",
                "Non-finite objective coefficient at variable " + std::to_string(j) + ": " + std::to_string(c),
                "OBJECTIVE",
                j
            });
        }
    }

    // 4. Matrix A Validation
    std::string mat_err;
    if (!model.matrixA().validate(&mat_err)) {
        result.isValid = false;
        result.diagnostics.push_back({
            DiagnosticSeverity::ERROR,
            "MAT_A_INVALID",
            "Matrix A internal structural error: " + mat_err,
            "MATRIX",
            std::nullopt
        });
    }

    // Matrix Q Validation (if present)
    if (model.matrixQ().has_value()) {
        const auto& matQ = model.matrixQ().value();
        if (matQ.rows() != n || matQ.cols() != n) {
            result.isValid = false;
            result.diagnostics.push_back({
                DiagnosticSeverity::ERROR,
                "MAT_Q_DIM_MISMATCH",
                "Matrix Q size (" + std::to_string(matQ.rows()) + "x" + std::to_string(matQ.cols()) +
                ") must be " + std::to_string(n) + "x" + std::to_string(n),
                "MATRIX",
                std::nullopt
            });
        }
        if (!matQ.validate(&mat_err)) {
            result.isValid = false;
            result.diagnostics.push_back({
                DiagnosticSeverity::ERROR,
                "MAT_Q_INVALID",
                "Matrix Q internal structural error: " + mat_err,
                "MATRIX",
                std::nullopt
            });
        }
    }

    return result;
}

} // namespace xenith::model
