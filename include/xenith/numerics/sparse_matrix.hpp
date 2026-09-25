#ifndef XENITH_NUMERICS_SPARSE_MATRIX_HPP
#define XENITH_NUMERICS_SPARSE_MATRIX_HPP

#include <span>
#include <vector>
#include <string>
#include <optional>
#include <stdexcept>
#include "xenith/common/types.hpp"
#include "xenith/common/constants.hpp"

namespace xenith::numerics {

/// Sparse matrix storage format: CSC (Column-compressed) or CSR (Row-compressed).
enum class SparseStorageFormat {
    CSC,
    CSR
};

/// Coordinate triplet entry for sparse matrix assembly.
struct Triplet {
    Index row{0};
    Index col{0};
    double value{0.0};
};

/**
 * @brief Reusable Sparse Matrix Abstraction supporting CSC and CSR compressed storage.
 */
class SparseMatrix {
public:
    /// Default constructor creates a valid empty 0x0 CSC matrix.
    SparseMatrix() : m_rows(0), m_cols(0), m_outerPtr({0}), m_innerIndices({}), m_values({}), m_format(SparseStorageFormat::CSC) {}

    /**
     * @brief Constructs a SparseMatrix from explicit compressed arrays.
     * @param rows Total row dimension (m).
     * @param cols Total column dimension (n).
     * @param outer_ptr Outer vector offsets (size n+1 for CSC, m+1 for CSR).
     * @param inner_indices Inner dimension indices (size nnz; row indices for CSC, col indices for CSR).
     * @param values Non-zero entry values (size nnz).
     * @param format Storage format (CSC or CSR).
     */
    SparseMatrix(Index rows,
                 Index cols,
                 std::vector<Index> outer_ptr,
                 std::vector<Index> inner_indices,
                 std::vector<double> values,
                 SparseStorageFormat format = SparseStorageFormat::CSC);

    /**
     * @brief Constructs a SparseMatrix from a list of coordinate triplets.
     * @param rows Total row dimension (m).
     * @param cols Total column dimension (n).
     * @param triplets List of non-zero triplet entries (duplicates are accumulated).
     * @param target_format Target format (CSC or CSR).
     */
    static SparseMatrix fromTriplets(Index rows,
                                     Index cols,
                                     const std::vector<Triplet>& triplets,
                                     SparseStorageFormat target_format = SparseStorageFormat::CSC);

    // Dimension and statistics accessors
    Index rows() const { return m_rows; }
    Index cols() const { return m_cols; }
    Index nonZeros() const { return m_values.size(); }
    SparseStorageFormat format() const { return m_format; }
    bool empty() const { return m_rows == 0 && m_cols == 0; }

    // Direct access to raw compressed arrays
    std::span<const Index> outerIndexPtr() const { return m_outerPtr; }
    std::span<const Index> innerIndices() const { return m_innerIndices; }
    std::span<const double> values() const { return m_values; }

    /**
     * @brief Converts the current matrix to CSC format.
     * Returns a copy if already in CSC.
     */
    SparseMatrix toCSC() const;

    /**
     * @brief Converts the current matrix to CSR format.
     * Returns a copy if already in CSR.
     */
    SparseMatrix toCSR() const;

    /**
     * @brief Validates internal structural invariants.
     * @param out_error Optional pointer to receive a human-readable diagnostic if invalid.
     * @return true if valid, false otherwise.
     */
    bool validate(std::string* out_error = nullptr) const;

    /**
     * @brief Matrix-vector multiplication: y = A * x.
     * @param x Input vector of dimension cols().
     * @param y Output vector of dimension rows().
     * @throws std::invalid_argument if x.size() != cols() or y.size() != rows().
     */
    void multiply(std::span<const double> x, std::span<double> y) const;

    /**
     * @brief Matrix-transpose-vector multiplication: y = A^T * x.
     * @param x Input vector of dimension rows().
     * @param y Output vector of dimension cols().
     * @throws std::invalid_argument if x.size() != rows() or y.size() != cols().
     */
    void multiplyTranspose(std::span<const double> x, std::span<double> y) const;

    /**
     * @brief Compute the infinity norm of each row.
     * @param out Output vector of size rows().
     */
    void rowInfinityNorms(std::span<double> out) const;

    /**
     * @brief Compute the infinity norm of each column.
     * @param out Output vector of size cols().
     */
    void colInfinityNorms(std::span<double> out) const;

    /**
     * @brief Compute the l1 norm of each row.
     * @param out Output vector of size rows().
     */
    void rowL1Norms(std::span<double> out) const;

    /**
     * @brief Compute the l1 norm of each column.
     * @param out Output vector of size cols().
     */
    void colL1Norms(std::span<double> out) const;

    /**
     * @brief Scale each row by the corresponding element in diag.
     * @param diag Input vector of size rows().
     */
    void scaleRows(std::span<const double> diag);

    /**
     * @brief Scale each column by the corresponding element in diag.
     * @param diag Input vector of size cols().
     */
    void scaleCols(std::span<const double> diag);

    /**
     * @brief Estimate spectral norm using power iteration.
     * @param num_iters Number of iterations to run.
     */
    double spectralNormEstimate(Index num_iters = 50) const;

private:
    Index m_rows{0};
    Index m_cols{0};
    std::vector<Index> m_outerPtr{0};
    std::vector<Index> m_innerIndices;
    std::vector<double> m_values;
    SparseStorageFormat m_format{SparseStorageFormat::CSC};
};

} // namespace xenith::numerics

#endif // XENITH_NUMERICS_SPARSE_MATRIX_HPP
