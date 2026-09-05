#include "xenith/numerics/sparse_matrix.hpp"
#include <algorithm>
#include <cmath>
#include <map>

namespace xenith::numerics {

SparseMatrix::SparseMatrix(Index rows,
                           Index cols,
                           std::vector<Index> outer_ptr,
                           std::vector<Index> inner_indices,
                           std::vector<double> values,
                           SparseStorageFormat format)
    : m_rows(rows),
      m_cols(cols),
      m_outerPtr(std::move(outer_ptr)),
      m_innerIndices(std::move(inner_indices)),
      m_values(std::move(values)),
      m_format(format) {
    std::string err;
    if (!validate(&err)) {
        throw std::invalid_argument("Malformed SparseMatrix construction: " + err);
    }
}

SparseMatrix SparseMatrix::fromTriplets(Index rows,
                                       Index cols,
                                       const std::vector<Triplet>& triplets,
                                       SparseStorageFormat target_format) {
    if (target_format == SparseStorageFormat::CSC) {
        // Accumulate duplicate entries using map keyed by (col, row)
        std::map<std::pair<Index, Index>, double> entry_map;
        for (const auto& t : triplets) {
            if (t.row >= rows || t.col >= cols) {
                throw std::out_of_range("Triplet indices out of matrix bounds: (" +
                                        std::to_string(t.row) + ", " + std::to_string(t.col) +
                                        ") for matrix size " + std::to_string(rows) + "x" +
                                        std::to_string(cols));
            }
            if (std::isnan(t.value)) {
                throw std::invalid_argument("Triplet contains NaN value at (" +
                                            std::to_string(t.row) + ", " + std::to_string(t.col) + ")");
            }
            entry_map[{t.col, t.row}] += t.value;
        }

        std::vector<Index> col_ptr(cols + 1, 0);
        for (const auto& [key, val] : entry_map) {
            if (std::abs(val) > 0.0) { // Keep non-zeros
                col_ptr[key.first + 1]++;
            }
        }

        for (Index j = 0; j < cols; ++j) {
            col_ptr[j + 1] += col_ptr[j];
        }

        Index total_nnz = col_ptr[cols];
        std::vector<Index> row_ind(total_nnz);
        std::vector<double> values(total_nnz);

        std::vector<Index> current_pos = col_ptr;
        for (const auto& [key, val] : entry_map) {
            if (std::abs(val) > 0.0) {
                Index col = key.first;
                Index row = key.second;
                Index pos = current_pos[col]++;
                row_ind[pos] = row;
                values[pos] = val;
            }
        }

        return SparseMatrix(rows, cols, std::move(col_ptr), std::move(row_ind), std::move(values), SparseStorageFormat::CSC);
    } else {
        // CSR format
        std::map<std::pair<Index, Index>, double> entry_map;
        for (const auto& t : triplets) {
            if (t.row >= rows || t.col >= cols) {
                throw std::out_of_range("Triplet indices out of matrix bounds: (" +
                                        std::to_string(t.row) + ", " + std::to_string(t.col) +
                                        ") for matrix size " + std::to_string(rows) + "x" +
                                        std::to_string(cols));
            }
            if (std::isnan(t.value)) {
                throw std::invalid_argument("Triplet contains NaN value at (" +
                                            std::to_string(t.row) + ", " + std::to_string(t.col) + ")");
            }
            entry_map[{t.row, t.col}] += t.value;
        }

        std::vector<Index> row_ptr(rows + 1, 0);
        for (const auto& [key, val] : entry_map) {
            if (std::abs(val) > 0.0) {
                row_ptr[key.first + 1]++;
            }
        }

        for (Index i = 0; i < rows; ++i) {
            row_ptr[i + 1] += row_ptr[i];
        }

        Index total_nnz = row_ptr[rows];
        std::vector<Index> col_ind(total_nnz);
        std::vector<double> values(total_nnz);

        std::vector<Index> current_pos = row_ptr;
        for (const auto& [key, val] : entry_map) {
            if (std::abs(val) > 0.0) {
                Index row = key.first;
                Index col = key.second;
                Index pos = current_pos[row]++;
                col_ind[pos] = col;
                values[pos] = val;
            }
        }

        return SparseMatrix(rows, cols, std::move(row_ptr), std::move(col_ind), std::move(values), SparseStorageFormat::CSR);
    }
}

SparseMatrix SparseMatrix::toCSC() const {
    if (m_format == SparseStorageFormat::CSC) {
        return *this;
    }
    // Convert CSR -> CSC
    std::vector<Triplet> triplets;
    triplets.reserve(nonZeros());
    for (Index i = 0; i < m_rows; ++i) {
        for (Index p = m_outerPtr[i]; p < m_outerPtr[i + 1]; ++p) {
            triplets.push_back({i, m_innerIndices[p], m_values[p]});
        }
    }
    return fromTriplets(m_rows, m_cols, triplets, SparseStorageFormat::CSC);
}

SparseMatrix SparseMatrix::toCSR() const {
    if (m_format == SparseStorageFormat::CSR) {
        return *this;
    }
    // Convert CSC -> CSR
    std::vector<Triplet> triplets;
    triplets.reserve(nonZeros());
    for (Index j = 0; j < m_cols; ++j) {
        for (Index p = m_outerPtr[j]; p < m_outerPtr[j + 1]; ++p) {
            triplets.push_back({m_innerIndices[p], j, m_values[p]});
        }
    }
    return fromTriplets(m_rows, m_cols, triplets, SparseStorageFormat::CSR);
}

bool SparseMatrix::validate(std::string* out_error) const {
    Index outer_dim = (m_format == SparseStorageFormat::CSC) ? m_cols : m_rows;
    Index inner_max = (m_format == SparseStorageFormat::CSC) ? m_rows : m_cols;

    if (m_outerPtr.size() != outer_dim + 1) {
        if (out_error) {
            *out_error = "Outer pointer size mismatch: expected " +
                         std::to_string(outer_dim + 1) + ", got " +
                         std::to_string(m_outerPtr.size());
        }
        return false;
    }

    if (m_innerIndices.size() != m_values.size()) {
        if (out_error) {
            *out_error = "Inner indices size (" + std::to_string(m_innerIndices.size()) +
                         ") does not match values size (" + std::to_string(m_values.size()) + ")";
        }
        return false;
    }

    if (m_outerPtr[0] != 0) {
        if (out_error) {
            *out_error = "Outer pointer start index must be 0, got " + std::to_string(m_outerPtr[0]);
        }
        return false;
    }

    if (m_outerPtr[outer_dim] != m_values.size()) {
        if (out_error) {
            *out_error = "Outer pointer total count (" + std::to_string(m_outerPtr[outer_dim]) +
                         ") does not match non-zeros count (" + std::to_string(m_values.size()) + ")";
        }
        return false;
    }

    for (Index k = 0; k < outer_dim; ++k) {
        if (m_outerPtr[k] > m_outerPtr[k + 1]) {
            if (out_error) {
                *out_error = "Non-monotonic outer pointer at index " + std::to_string(k) +
                             ": " + std::to_string(m_outerPtr[k]) + " > " + std::to_string(m_outerPtr[k + 1]);
            }
            return false;
        }

        // Validate inner indices monotonic and bounded
        Index prev_inner = 0;
        for (Index p = m_outerPtr[k]; p < m_outerPtr[k + 1]; ++p) {
            Index inner_idx = m_innerIndices[p];
            if (inner_idx >= inner_max) {
                if (out_error) {
                    *out_error = "Inner index out of range at pos " + std::to_string(p) +
                                 ": " + std::to_string(inner_idx) + " >= " + std::to_string(inner_max);
                }
                return false;
            }
            if (p > m_outerPtr[k] && inner_idx <= prev_inner) {
                if (out_error) {
                    *out_error = "Inner indices not strictly monotonic in slice " + std::to_string(k) +
                                 ": " + std::to_string(inner_idx) + " <= " + std::to_string(prev_inner);
                }
                return false;
            }
            if (std::isnan(m_values[p])) {
                if (out_error) {
                    *out_error = "NaN matrix element at entry position " + std::to_string(p);
                }
                return false;
            }
            prev_inner = inner_idx;
        }
    }

    return true;
}

void SparseMatrix::multiply(std::span<const double> x, std::span<double> y) const {
    if (x.size() != m_cols) {
        throw std::invalid_argument("Input vector x size (" + std::to_string(x.size()) +
                                    ") does not match matrix columns (" + std::to_string(m_cols) + ")");
    }
    if (y.size() != m_rows) {
        throw std::invalid_argument("Output vector y size (" + std::to_string(y.size()) +
                                    ") does not match matrix rows (" + std::to_string(m_rows) + ")");
    }

    std::fill(y.begin(), y.end(), 0.0);

    if (m_format == SparseStorageFormat::CSC) {
        for (Index j = 0; j < m_cols; ++j) {
            double xj = x[j];
            if (xj == 0.0) continue;
            for (Index p = m_outerPtr[j]; p < m_outerPtr[j + 1]; ++p) {
                y[m_innerIndices[p]] += m_values[p] * xj;
            }
        }
    } else { // CSR
        for (Index i = 0; i < m_rows; ++i) {
            double sum = 0.0;
            for (Index p = m_outerPtr[i]; p < m_outerPtr[i + 1]; ++p) {
                sum += m_values[p] * x[m_innerIndices[p]];
            }
            y[i] = sum;
        }
    }
}

void SparseMatrix::multiplyTranspose(std::span<const double> x, std::span<double> y) const {
    if (x.size() != m_rows) {
        throw std::invalid_argument("Input vector x size (" + std::to_string(x.size()) +
                                    ") does not match matrix rows (" + std::to_string(m_rows) + ")");
    }
    if (y.size() != m_cols) {
        throw std::invalid_argument("Output vector y size (" + std::to_string(y.size()) +
                                    ") does not match matrix columns (" + std::to_string(m_cols) + ")");
    }

    if (m_format == SparseStorageFormat::CSC) {
        for (Index j = 0; j < m_cols; ++j) {
            double sum = 0.0;
            for (Index p = m_outerPtr[j]; p < m_outerPtr[j + 1]; ++p) {
                sum += m_values[p] * x[m_innerIndices[p]];
            }
            y[j] = sum;
        }
    } else { // CSR
        std::fill(y.begin(), y.end(), 0.0);
        for (Index i = 0; i < m_rows; ++i) {
            double xi = x[i];
            if (xi == 0.0) continue;
            for (Index p = m_outerPtr[i]; p < m_outerPtr[i + 1]; ++p) {
                y[m_innerIndices[p]] += m_values[p] * xi;
            }
        }
    }
}

} // namespace xenith::numerics
