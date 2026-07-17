#pragma once

#include <string>
#include <vector>
#include "../Geometry/MathTypes.hpp"
#include "../Geometry/SolidBody.hpp"

namespace SZM::CAD {

/**
 * @class STEPLoader
 * @brief Base mock parser for STEP (Standard for the Exchange of Product model data) files.
 */
class STEPLoader {
public:
    struct StepMetadata {
        std::string author;
        std::string organization;
        std::string timestamp;
        uint32_t entityCount = 0;
    };

    /**
     * @brief Parses the header of a STEP file to extract basic metadata
     */
    static StepMetadata ParseHeader(const std::string& filePath);

    /**
     * @brief Mocks the translation of STEP geometry into a SolidBody representation
     */
    static std::shared_ptr<Geometry::SolidBody> GenerateMockBody(const std::string& filePath);
};

} // namespace SZM::CAD
