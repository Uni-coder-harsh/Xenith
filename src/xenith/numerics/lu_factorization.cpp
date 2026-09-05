#include "xenith/numerics/lu_factorization.hpp"
#include <cmath>
#include <algorithm>
#include <stdexcept>
#include <iostream>

namespace xenith::numerics {

bool LuFactorization::factorize(const SparseMatrix& B,
                                double pivot_tolerance,
                                double threshold_alpha) {
    if (B.rows() != B.cols()) {
        throw std::invalid_argument("LuFactorization requires a square matrix B");
    }

    m_dim = B.rows();
    m_singular = false;

    if (m_dim == 0) {
        return true;
    }

    m_lu.assign(m_dim * m_dim, 0.0);
    m_rowPerm.resize(m_dim);
    m_rowPermInv.resize(m_dim);
    m_colPerm.resize(m_dim);
    m_colPermInv.resize(m_dim);

    for (Index i = 0; i < m_dim; ++i) {
        m_rowPerm[i] = i;
        m_rowPermInv[i] = i;
        m_colPerm[i] = i;
        m_colPermInv[i] = i;
    }

    // Populate dense m_lu from SparseMatrix B (CSC or CSR)
    if (B.format() == SparseStorageFormat::CSC) {
        auto col_ptr = B.outerIndexPtr();
        auto row_idx = B.innerIndices();
        auto vals = B.values();
        for (Index j = 0; j < m_dim; ++j) {
            for (Index p = col_ptr[j]; p < col_ptr[j + 1]; ++p) {
                Index i = row_idx[p];
                m_lu[i * m_dim + j] += vals[p];
            }
        }
    } else { // CSR
        auto row_ptr = B.outerIndexPtr();
        auto col_idx = B.innerIndices();
        auto vals = B.values();
        for (Index i = 0; i < m_dim; ++i) {
            for (Index p = row_ptr[i]; p < row_ptr[i + 1]; ++p) {
                Index j = col_idx[p];
                m_lu[i * m_dim + j] += vals[p];
            }
        }
    }

    // Gaussian Elimination with Markowitz Threshold Pivoting
    for (Index k = 0; k < m_dim; ++k) {
        // Find pivot entry in active submatrix M[k..m_dim-1, k..m_dim-1]
        Index best_r = k;
        Index best_c = k;
        double max_val = 0.0;
        Index min_markowitz = std::numeric_limits<Index>::max();

        // Compute column max values for threshold test
        std::vector<double> col_max(m_dim, 0.0);
        for (Index j = k; j < m_dim; ++j) {
            for (Index i = k; i < m_dim; ++i) {
                col_max[j] = std::max(col_max[j], std::abs(m_lu[i * m_dim + j]));
            }
        }

        // Search for best pivot
        for (Index j = k; j < m_dim; ++j) {
            if (col_max[j] < pivot_tolerance) continue;

            // Row non-zero counts
            for (Index i = k; i < m_dim; ++i) {
                double val_abs = std::abs(m_lu[i * m_dim + j]);
                if (val_abs >= threshold_alpha * col_max[j] && val_abs >= pivot_tolerance) {
                    // Count active nonzeros in row i and col j
                    Index r_count = 0;
                    for (Index c_idx = k; c_idx < m_dim; ++c_idx) {
                        if (std::abs(m_lu[i * m_dim + c_idx]) > 1e-15) r_count++;
                    }
                    Index c_count = 0;
                    for (Index r_idx = k; r_idx < m_dim; ++r_idx) {
                        if (std::abs(m_lu[r_idx * m_dim + j]) > 1e-15) c_count++;
                    }

                    Index markowitz = (r_count > 0 ? r_count - 1 : 0) * (c_count > 0 ? c_count - 1 : 0);
                    if (markowitz < min_markowitz || (markowitz == min_markowitz && val_abs > max_val)) {
                        min_markowitz = markowitz;
                        max_val = val_abs;
                        best_r = i;
                        best_c = j;
                    }
                }
            }
        }

        // If no candidate found above pivot_tolerance, try global max in active submatrix
        if (max_val < pivot_tolerance) {
            for (Index i = k; i < m_dim; ++i) {
                for (Index j = k; j < m_dim; ++j) {
                    double val_abs = std::abs(m_lu[i * m_dim + j]);
                    if (val_abs > max_val) {
                        max_val = val_abs;
                        best_r = i;
                        best_c = j;
                    }
                }
            }
        }

        if (max_val < pivot_tolerance) {
            m_singular = true;
            return false; // Singular basis matrix!
        }

        // Pivot row swap: k <-> best_r
        if (best_r != k) {
            for (Index j = 0; j < m_dim; ++j) {
                std::swap(m_lu[k * m_dim + j], m_lu[best_r * m_dim + j]);
            }
            std::swap(m_rowPerm[k], m_rowPerm[best_r]);
        }

        // Pivot col swap: k <-> best_c
        if (best_c != k) {
            for (Index i = 0; i < m_dim; ++i) {
                std::swap(m_lu[i * m_dim + k], m_lu[i * m_dim + best_c]);
            }
            std::swap(m_colPerm[k], m_colPerm[best_c]);
        }

        double pivot = m_lu[k * m_dim + k];
        if (std::abs(pivot) < pivot_tolerance) {
            m_singular = true;
            return false;
        }

        // Eliminate column entries below pivot
        for (Index i = k + 1; i < m_dim; ++i) {
            double mult = m_lu[i * m_dim + k] / pivot;
            m_lu[i * m_dim + k] = mult; // Store L entry
            for (Index j = k + 1; j < m_dim; ++j) {
                m_lu[i * m_dim + j] -= mult * m_lu[k * m_dim + j];
            }
        }
    }

    // Build inverse permutations
    for (Index i = 0; i < m_dim; ++i) {
        m_rowPermInv[m_rowPerm[i]] = i;
        m_colPermInv[m_colPerm[i]] = i;
    }

    m_singular = false;
    return true;
}

void LuFactorization::solveFtran(std::span<const double> rhs, std::span<double> sol) const {
    if (m_singular) {
        throw std::runtime_error("Cannot solve FTRAN: Basis matrix is singular or uninitialized");
    }
    if (rhs.size() != m_dim || sol.size() != m_dim) {
        throw std::invalid_argument("Vector dimensions in solveFtran do not match matrix dimension");
    }

    // Step 1: Permute rhs z = P * rhs (z[k] = rhs[m_rowPerm[k]])
    std::vector<double> z(m_dim);
    for (Index k = 0; k < m_dim; ++k) {
        z[k] = rhs[m_rowPerm[k]];
    }

    // Step 2: Forward substitution L * w = z (unit lower triangular L)
    std::vector<double> w(m_dim);
    for (Index i = 0; i < m_dim; ++i) {
        double sum = z[i];
        for (Index k = 0; k < i; ++k) {
            sum -= m_lu[i * m_dim + k] * w[k];
        }
        w[i] = sum;
    }

    // Step 3: Back substitution U * v = w (upper triangular U)
    std::vector<double> v(m_dim);
    for (Index i_idx = m_dim; i_idx > 0; --i_idx) {
        Index i = i_idx - 1;
        double sum = w[i];
        for (Index k = i + 1; k < m_dim; ++k) {
            sum -= m_lu[i * m_dim + k] * v[k];
        }
        v[i] = sum / m_lu[i * m_dim + i];
    }

    // Step 4: Permute solution sol = Q * v (sol[j] = v[m_colPermInv[j]])
    for (Index j = 0; j < m_dim; ++j) {
        sol[j] = v[m_colPermInv[j]];
    }
}

void LuFactorization::solveBtran(std::span<const double> rhs, std::span<double> sol) const {
    if (m_singular) {
        throw std::runtime_error("Cannot solve BTRAN: Basis matrix is singular or uninitialized");
    }
    if (rhs.size() != m_dim || sol.size() != m_dim) {
        throw std::invalid_argument("Vector dimensions in solveBtran do not match matrix dimension");
    }

    // B^T = Q * U^T * L^T * P
    // B^T * y = rhs => Q * U^T * L^T * P * y = rhs => U^T * L^T * P * y = Q^T * rhs
    // Step 1: z = Q^T * rhs (z[c] = rhs[m_colPerm[c]])
    std::vector<double> z(m_dim);
    for (Index c = 0; c < m_dim; ++c) {
        z[c] = rhs[m_colPerm[c]];
    }

    // Step 2: Forward substitution U^T * w = z (U^T is lower triangular with U[i,i] on diagonal)
    std::vector<double> w(m_dim);
    for (Index i = 0; i < m_dim; ++i) {
        double sum = z[i];
        for (Index k = 0; k < i; ++k) {
            sum -= m_lu[k * m_dim + i] * w[k];
        }
        w[i] = sum / m_lu[i * m_dim + i];
    }

    // Step 3: Back substitution L^T * v = w (L^T is unit upper triangular)
    std::vector<double> v(m_dim);
    for (Index i_idx = m_dim; i_idx > 0; --i_idx) {
        Index i = i_idx - 1;
        double sum = w[i];
        for (Index k = i + 1; k < m_dim; ++k) {
            sum -= m_lu[k * m_dim + i] * v[k];
        }
        v[i] = sum;
    }

    // Step 4: Permute solution P * y = v => sol[r] = v[m_rowPermInv[r]]
    for (Index r = 0; r < m_dim; ++r) {
        sol[r] = v[m_rowPermInv[r]];
    }
}


} // namespace xenith::numerics
