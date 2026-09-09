#include "xenith/solution/lp_solution_validator.hpp"
#include <cmath>
#include <sstream>

namespace xenith::solution {

model::ValidationResult LpSolutionValidator::validate(const model::CanonicalModel& model,
                                                       const solver::SolveResult& result,
                                                       double tolerance) {
    model::ValidationResult val_res;

    if (result.status != ModelStatus::OPTIMAL) {
        // Validation only applies to primal optimal solutions
        val_res.isValid = true;
        return val_res;
    }

    Index n = model.numVariables();
    Index m = model.numConstraints();

    if (result.primal_solution.size() != n) {
        val_res.isValid = false;
        model::ValidationDiagnostic diag;
        diag.severity = model::DiagnosticSeverity::ERROR;
        diag.code = "SOLUTION_DIMENSION_MISMATCH";
        diag.component = "VARIABLE";
        diag.message = "Primal solution size != model variable count";
        val_res.diagnostics.push_back(diag);
        return val_res;
    }

    // 1. Validate Variable Bounds
    for (Index j = 0; j < n; ++j) {
        double val = result.primal_solution[j];
        double l = model.colLower()[j];
        double u = model.colUpper()[j];

        if (!isNegativeInfinity(l) && val < l - tolerance) {
            val_res.isValid = false;
            model::ValidationDiagnostic diag;
            diag.severity = model::DiagnosticSeverity::ERROR;
            diag.code = "VAR_LOWER_BOUND_VIOLATION";
            diag.component = "VARIABLE";
            diag.index = j;
            std::ostringstream oss;
            oss << "Variable " << model.varNames()[j] << " (index " << j << ") value " << val << " < lower bound " << l;
            diag.message = oss.str();
            val_res.diagnostics.push_back(diag);
        }

        if (!isPositiveInfinity(u) && val > u + tolerance) {
            val_res.isValid = false;
            model::ValidationDiagnostic diag;
            diag.severity = model::DiagnosticSeverity::ERROR;
            diag.code = "VAR_UPPER_BOUND_VIOLATION";
            diag.component = "VARIABLE";
            diag.index = j;
            std::ostringstream oss;
            oss << "Variable " << model.varNames()[j] << " (index " << j << ") value " << val << " > upper bound " << u;
            diag.message = oss.str();
            val_res.diagnostics.push_back(diag);
        }
    }

    // 2. Validate Constraint Bounds: Ax
    if (m > 0) {
        std::vector<double> Ax(m, 0.0);
        model.matrixA().multiply(std::span<const double>(result.primal_solution.data(), n), std::span<double>(Ax.data(), m));

        for (Index i = 0; i < m; ++i) {
            double l_r = model.rowLower()[i];
            double u_r = model.rowUpper()[i];

            if (!isNegativeInfinity(l_r) && Ax[i] < l_r - tolerance) {
                val_res.isValid = false;
                model::ValidationDiagnostic diag;
                diag.severity = model::DiagnosticSeverity::ERROR;
                diag.code = "ROW_LOWER_BOUND_VIOLATION";
                diag.component = "CONSTRAINT";
                diag.index = i;
                std::ostringstream oss;
                oss << "Row " << model.rowNames()[i] << " (index " << i << ") activity " << Ax[i] << " < lower bound " << l_r;
                diag.message = oss.str();
                val_res.diagnostics.push_back(diag);
            }

            if (!isPositiveInfinity(u_r) && Ax[i] > u_r + tolerance) {
                val_res.isValid = false;
                model::ValidationDiagnostic diag;
                diag.severity = model::DiagnosticSeverity::ERROR;
                diag.code = "ROW_UPPER_BOUND_VIOLATION";
                diag.component = "CONSTRAINT";
                diag.index = i;
                std::ostringstream oss;
                oss << "Row " << model.rowNames()[i] << " (index " << i << ") activity " << Ax[i] << " > upper bound " << u_r;
                diag.message = oss.str();
                val_res.diagnostics.push_back(diag);
            }
        }
    }

    // 3. Independently Recompute Objective Value: c^T x + offset
    double recomputed_obj = 0.0;
    for (Index j = 0; j < n; ++j) {
        recomputed_obj += model.objective()[j] * result.primal_solution[j];
    }
    recomputed_obj += model.objectiveOffset();

    double obj_diff = std::abs(recomputed_obj - result.objective_value);
    if (obj_diff > tolerance) {
        val_res.isValid = false;
        model::ValidationDiagnostic diag;
        diag.severity = model::DiagnosticSeverity::ERROR;
        diag.code = "OBJECTIVE_MISMATCH";
        diag.component = "OBJECTIVE";
        std::ostringstream oss;
        oss << "Recomputed objective " << recomputed_obj << " differs from solver objective " << result.objective_value << " by " << obj_diff;
        diag.message = oss.str();
        val_res.diagnostics.push_back(diag);
    }

    return val_res;
}

model::ValidationResult LpSolutionValidator::validate(const model::CanonicalModel& model,
                                                       const solver::PdlpResult& result,
                                                       double tolerance) {
    solver::SolveResult conv;
    conv.status = result.status;
    conv.primal_solution = result.primal_solution;
    conv.objective_value = result.objective_value;
    conv.iterations = result.iterations;
    conv.primal_residual = result.primal_residual;
    conv.dual_residual = result.dual_residual;
    conv.message = result.message;
    return validate(model, conv, tolerance);
}

} // namespace xenith::solution
