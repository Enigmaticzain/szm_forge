#pragma once

#include <string>
#include <vector>
#include "../Geometry/MathTypes.hpp"
#include "../Geometry/SolidBody.hpp"

namespace SZM::CAD {

/**
 * @class IGESLoader
 * @brief Base mock parser for IGES (Initial Graphics Exchange Specification) files.
 */
class IGESLoader {
public:
    struct IgesMetadata {
        std::string systemID;
        std::string date;
        std::string author;
        uint32_t directoryEntryCount = 0;
    };

    /**
     * @brief Parses the Start and Global sections of an IGES file to extract basic metadata
     */
    static IgesMetadata ParseHeader(const std::string& filePath);

    /**
     * @brief Mocks the translation of IGES geometry into a SolidBody representation
     */
    static std::shared_ptr<Geometry::SolidBody> GenerateMockBody(const std::string& filePath);
};

} // namespace SZM::CAD
