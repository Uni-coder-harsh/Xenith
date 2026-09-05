#ifndef XENITH_IO_MPS_MPS_READER_HPP
#define XENITH_IO_MPS_MPS_READER_HPP

#include <string>
#include <istream>
#include "xenith/io/mps/mps_types.hpp"
#include "xenith/model/canonical_model.hpp"

namespace xenith::io::mps {

/**
 * @brief High-performance, section-aware MPS file reader for XENITH.
 * Converts fixed and free format MPS files into validated CanonicalModel instances.
 */
class MpsReader {
public:
    /**
     * @brief Parses an MPS file from disk into a CanonicalModel.
     * @param filepath Path to the .mps file.
     * @param options Reader configuration options.
     * @return Transformed CanonicalModel instance.
     * @throws MpsParseException if syntax or semantic parsing errors occur.
     */
    static model::CanonicalModel readFromFile(const std::string& filepath,
                                              const MpsReaderOptions& options = {});

    /**
     * @brief Parses MPS format data from an input stream into a CanonicalModel.
     * @param stream Input stream containing MPS data.
     * @param stream_name Identifier string for error reporting (e.g. filename).
     * @param options Reader configuration options.
     * @return Transformed CanonicalModel instance.
     * @throws MpsParseException if syntax or semantic parsing errors occur.
     */
    static model::CanonicalModel readFromStream(std::istream& stream,
                                                const std::string& stream_name = "stream",
                                                const MpsReaderOptions& options = {});

    /**
     * @brief Parses MPS format data from a string into a CanonicalModel.
     * @param content In-memory string containing MPS content.
     * @param source_name Identifier string for error reporting.
     * @param options Reader configuration options.
     * @return Transformed CanonicalModel instance.
     * @throws MpsParseException if syntax or semantic parsing errors occur.
     */
    static model::CanonicalModel readFromString(const std::string& content,
                                                const std::string& source_name = "string",
                                                const MpsReaderOptions& options = {});
};

} // namespace xenith::io::mps

#endif // XENITH_IO_MPS_MPS_READER_HPP
