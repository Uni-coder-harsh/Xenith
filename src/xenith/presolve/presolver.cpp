#include "xenith/presolve/presolver.hpp"
#include <cmath>

namespace xenith::presolve {

Presolver::Presolver() : m_options() {}
Presolver::Presolver(Options options) : m_options(options) {}

PresolveResult Presolver::presolve(const model::CanonicalModel& model) {
    PresolveResult result;
    result.original_vars = model.numVariables();
    result.original_constraints = model.numConstraints();
    
    std::vector<bool> keep_row(model.numConstraints(), true);
    std::vector<bool> keep_col(model.numVariables(), true);
    
    std::vector<double> l_x(model.colLower().begin(), model.colLower().end());
    std::vector<double> u_x(model.colUpper().begin(), model.colUpper().end());
    std::vector<double> l_r(model.rowLower().begin(), model.rowLower().end());
    std::vector<double> u_r(model.rowUpper().begin(), model.rowUpper().end());
    
    // Check variable bound feasibility
    for (Index j = 0; j < model.numVariables(); ++j) {
        if (l_x[j] > u_x[j] + m_options.feasibility_tolerance) {
            result.was_infeasible = true;
            result.message = "Infeasible variable bounds: lower > upper for variable " + std::to_string(j);
            return result;
        }
    }

    auto A = model.matrixA().toCSC();
    
    std::vector<Index> row_nnz(model.numConstraints(), 0);
    std::vector<Index> col_nnz(model.numVariables(), 0);
    std::vector<Index> row_single_col(model.numConstraints(), 0);
    std::vector<double> row_single_val(model.numConstraints(), 0.0);
    
    if (!A.empty() && A.cols() > 0) {
        Index active_cols = std::min(model.numVariables(), A.cols());
        for (Index j = 0; j < active_cols; ++j) {
            if (j + 1 < A.outerIndexPtr().size()) {
                Index start = A.outerIndexPtr()[j];
                Index end = A.outerIndexPtr()[j+1];
                col_nnz[j] = end - start;
                for (Index p = start; p < end; ++p) {
                    Index i = A.innerIndices()[p];
                    double val = A.values()[p];
                    if (i < model.numConstraints()) {
                        row_nnz[i]++;
                        row_single_col[i] = j;
                        row_single_val[i] = val;
                    }
                }
            }
        }
    }
    
    if (m_options.fix_variables) {
        for (Index j = 0; j < model.numVariables(); ++j) {
            if (u_x[j] - l_x[j] <= m_options.feasibility_tolerance) {
                keep_col[j] = false;
                result.records.push_back({PresolveAction::FIX_VARIABLE, j, l_x[j], 0.0});
            }
        }
    }
    
    if (m_options.remove_empty_rows) {
        for (Index i = 0; i < model.numConstraints(); ++i) {
            if (row_nnz[i] == 0) {
                if ((isFinite(l_r[i]) && l_r[i] > m_options.feasibility_tolerance) || 
                    (isFinite(u_r[i]) && u_r[i] < -m_options.feasibility_tolerance)) {
                    result.was_infeasible = true;
                    result.message = "Infeasible empty row";
                    return result;
                }
                keep_row[i] = false;
                result.records.push_back({PresolveAction::REMOVE_EMPTY_ROW, i, 0.0, 0.0});
            }
        }
    }
    
    if (m_options.remove_singletons) {
        for (Index i = 0; i < model.numConstraints(); ++i) {
            if (keep_row[i] && row_nnz[i] == 1) {
                Index j = row_single_col[i];
                double val = row_single_val[i];
                
                double bnd1 = l_r[i] / val;
                double bnd2 = u_r[i] / val;
                double new_l = val > 0 ? bnd1 : bnd2;
                double new_u = val > 0 ? bnd2 : bnd1;
                
                if (isNegativeInfinity(l_r[i])) {
                    if (val > 0) new_l = -K_INFINITY;
                    else new_u = K_INFINITY;
                }
                if (isPositiveInfinity(u_r[i])) {
                    if (val > 0) new_u = K_INFINITY;
                    else new_l = -K_INFINITY;
                }
                
                if (new_l > l_x[j]) l_x[j] = new_l;
                if (new_u < u_x[j]) u_x[j] = new_u;
                
                if (l_x[j] > u_x[j] + m_options.feasibility_tolerance) {
                    result.was_infeasible = true;
                    result.message = "Infeasible singleton row bound tightening";
                    return result;
                }
                
                keep_row[i] = false;
                result.records.push_back({PresolveAction::REMOVE_SINGLETON_ROW, i, 0.0, 0.0});
            }
        }
    }
    
    if (m_options.remove_empty_cols) {
        for (Index j = 0; j < model.numVariables(); ++j) {
            if (keep_col[j] && col_nnz[j] == 0) {
                double obj = model.objective()[j];
                double fixed_val = 0.0;
                if (obj > m_options.feasibility_tolerance) {
                    if (isNegativeInfinity(l_x[j])) {
                        result.was_infeasible = true;
                        result.message = "Unbounded empty column";
                        return result;
                    }
                    fixed_val = l_x[j];
                } else if (obj < -m_options.feasibility_tolerance) {
                    if (isPositiveInfinity(u_x[j])) {
                        result.was_infeasible = true;
                        result.message = "Unbounded empty column";
                        return result;
                    }
                    fixed_val = u_x[j];
                } else {
                    fixed_val = isFinite(l_x[j]) ? l_x[j] : 0.0;
                    if (fixed_val < l_x[j]) fixed_val = l_x[j];
                    if (fixed_val > u_x[j]) fixed_val = u_x[j];
                }
                
                keep_col[j] = false;
                result.records.push_back({PresolveAction::REMOVE_EMPTY_COLUMN, j, fixed_val, 0.0});
            }
        }
    }
    
    model::CanonicalModel reduced(model.name() + "_presolved");
    reduced.setSense(model.sense());
    reduced.setObjectiveOffset(model.objectiveOffset());
    
    std::vector<Index> col_map(model.numVariables(), -1);
    Index num_kept_cols = 0;
    for (Index j = 0; j < model.numVariables(); ++j) {
        if (keep_col[j]) {
            reduced.addVariable(model.varNames()[j], l_x[j], u_x[j], model.objective()[j], model.varTypes()[j]);
            col_map[j] = num_kept_cols++;
        } else {
            result.vars_removed++;
        }
    }
    
    std::vector<Index> row_map(model.numConstraints(), -1);
    Index num_kept_rows = 0;
    for (Index i = 0; i < model.numConstraints(); ++i) {
        if (keep_row[i]) {
            reduced.addRow(model.rowNames()[i], l_r[i], u_r[i]);
            row_map[i] = num_kept_rows++;
        } else {
            result.constraints_removed++;
        }
    }
    
    std::vector<numerics::Triplet> triplets;
    for (Index j = 0; j < model.numVariables(); ++j) {
        if (!keep_col[j]) continue;
        for (Index p = A.outerIndexPtr()[j]; p < A.outerIndexPtr()[j+1]; ++p) {
            Index i = A.innerIndices()[p];
            if (keep_row[i]) {
                triplets.push_back({row_map[i], col_map[j], A.values()[p]});
            }
        }
    }
    
    reduced.setMatrixA(numerics::SparseMatrix::fromTriplets(num_kept_rows, num_kept_cols, triplets, numerics::SparseStorageFormat::CSC));
    
    if (model.matrixQ().has_value()) {
        auto Q = model.matrixQ()->toCSC();
        std::vector<numerics::Triplet> q_triplets;
        for (Index j = 0; j < model.numVariables(); ++j) {
            if (!keep_col[j]) continue;
            for (Index p = Q.outerIndexPtr()[j]; p < Q.outerIndexPtr()[j+1]; ++p) {
                Index i = Q.innerIndices()[p];
                if (keep_col[i]) {
                    q_triplets.push_back({col_map[i], col_map[j], Q.values()[p]});
                }
            }
        }
        reduced.setMatrixQ(numerics::SparseMatrix::fromTriplets(num_kept_cols, num_kept_cols, q_triplets, numerics::SparseStorageFormat::CSC));
    }
    
    result.reduced_model = reduced;
    return result;
}

std::vector<double> Presolver::postsolve(
    const std::vector<double>& reduced_solution,
    const PresolveResult& presolve_result) {
    
    std::vector<double> full_solution(presolve_result.original_vars, 0.0);
    
    std::vector<bool> keep_col(presolve_result.original_vars, true);
    for (const auto& rec : presolve_result.records) {
        if (rec.action == PresolveAction::FIX_VARIABLE || rec.action == PresolveAction::REMOVE_EMPTY_COLUMN) {
            keep_col[rec.original_index] = false;
        }
    }
    
    Index reduced_idx = 0;
    for (Index j = 0; j < presolve_result.original_vars; ++j) {
        if (keep_col[j]) {
            full_solution[j] = reduced_solution[reduced_idx++];
        }
    }
    
    for (auto it = presolve_result.records.rbegin(); it != presolve_result.records.rend(); ++it) {
        const auto& rec = *it;
        if (rec.action == PresolveAction::FIX_VARIABLE || rec.action == PresolveAction::REMOVE_EMPTY_COLUMN) {
            full_solution[rec.original_index] = rec.value;
        }
    }
    
    return full_solution;
}

} // namespace xenith::presolve
