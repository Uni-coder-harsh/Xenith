#ifndef XENITH_NUMERICS_LU_FACTORIZATION_HPP
#define XENITH_NUMERICS_LU_FACTORIZATION_HPP

#include <span>
#include <vector>
#include <string>
#include <optional>
#include "xenith/common/types.hpp"
#include "xenith/common/constants.hpp"
#include "xenith/numerics/sparse_matrix.hpp"

namespace xenith::numerics {

/**
 * @brief Sparse LU Factorization Engine for Basis Matrices with Markowitz threshold pivoting.
 * Computes P * B * Q = L * U and provides FTRAN (B y = a) and BTRAN (B^T y = a) solves.
 */
class LuFactorization {
public:
    LuFactorization() = default;

    /**
     * @brief Computes LU factorization of an m x m matrix B.
     * @param B Input matrix (must be square m x m).
     * @param pivot_tolerance Minimum absolute magnitude for non-singular pivot (default 1e-10).
     * @param threshold_alpha Threshold factor for Markowitz candidate eligibility in [0, 1] (default 0.1).
     * @return true if non-singular and factorized successfully, false if singular.
     */
    bool factorize(const SparseMatrix& B,
                   double pivot_tolerance = 1e-10,
                   double threshold_alpha = 0.1);

    /// Checks if the last factorization was singular or uninitialized.
    bool isSingular() const { return m_singular; }

    /// Dimension m of the factorized matrix.
    Index dim() const { return m_dim; }

    /**
     * @brief FTRAN: Solves B * y = rhs.
     * @param rhs Input right-hand side vector (size m).
     * @param sol Output solution vector (size m).
     * @throws std::runtime_error if factorize failed or singular.
     */
    void solveFtran(std::span<const double> rhs, std::span<double> sol) const;

    /**
     * @brief BTRAN: Solves B^T * y = rhs.
     * @param rhs Input right-hand side vector (size m).
     * @param sol Output solution vector (size m).
     * @throws std::runtime_error if factorize failed or singular.
     */
    void solveBtran(std::span<const double> rhs, std::span<double> sol) const;

private:
    Index m_dim{0};
    bool m_singular{true};

    // Dense elimination matrix M storing L (strictly lower) and U (upper)
    std::vector<double> m_lu; // Size m * m

    // Permutation vectors
    std::vector<Index> m_rowPerm;     // Size m: original_row -> permuted_row (P)
    std::vector<Index> m_rowPermInv;  // Size m: permuted_row -> original_row (P^T)
    std::vector<Index> m_colPerm;     // Size m: original_col -> permuted_col (Q)
    std::vector<Index> m_colPermInv;  // Size m: permuted_col -> original_col (Q^T)
};

} // namespace xenith::numerics

#endif // XENITH_NUMERICS_LU_FACTORIZATION_HPP
