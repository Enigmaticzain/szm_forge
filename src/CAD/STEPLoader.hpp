#pragma once

#include <string>
#include <vector>
#include <memory>
#include "../Geometry/MathTypes.hpp"
#include "../Geometry/SolidBody.hpp"

namespace SZM::CAD {

/**
 * @struct STEPAssemblyNode
 * @brief One node in the STEP product hierarchy tree.
 */
struct STEPAssemblyNode {
    std::string              id;
    std::string              name;
    bool                     isCyclic = false;
    std::vector<STEPAssemblyNode> children;
};

/**
 * @class STEPLoader
 * @brief STEP file parser — header metadata + assembly hierarchy via Python bridge.
 */
class STEPLoader {
public:
    struct StepMetadata {
        std::string author;
        std::string organization;
        std::string timestamp;
        uint32_t entityCount = 0;
    };

    static StepMetadata ParseHeader(const std::string& filePath);
    static std::shared_ptr<Geometry::SolidBody> GenerateMockBody(const std::string& filePath);

    /**
     * @brief Parse full assembly hierarchy from a STEP file via the Python bridge.
     * @param filePath  Absolute path to the .step / .stp file.
     * @param bridgePort  Port of the external_engine_bridge (default 8003).
     * @return Root assembly node; name=="" on failure.
     */
    static STEPAssemblyNode ParseAssembly(const std::string& filePath, uint16_t bridgePort = 8003);
};

} // namespace SZM::CAD
