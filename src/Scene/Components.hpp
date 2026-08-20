#pragma once

#include "../Geometry/MathTypes.hpp"
#include "Entity.hpp"
#include <string>

namespace SZM::SceneGraph {

    /**
     * @brief Hierarchical transform component (USD compatible mapping)
     */
    struct TransformComponent {
        Geometry::Vector3 position{0.0f, 0.0f, 0.0f};
        Geometry::Vector3 rotation{0.0f, 0.0f, 0.0f}; // Euler angles for now
        Geometry::Vector3 scale{1.0f, 1.0f, 1.0f};

        // Hierarchy
        Entity parentEntity = NullEntity; // NullEntity means no parent (root)
    };

    /**
     * @brief Identifier and metadata
     */
    struct TagComponent {
        std::string name = "Entity";
        std::string usdPath = "/Root/Entity"; // Mapping for Omniverse/USD
    };

    /**
     * @brief Geometry/Mesh reference
     */
    struct MeshComponent {
        std::string meshId; // Reference to a loaded asset
        float area = 0.0f;
        float thickness = 0.0f;
    };

    /**
     * @brief Material mapping (MaterialX / USD Preview Surface)
     */
    struct MaterialComponent {
        std::string materialId = "MAT-DEFAULT";
        float density = 1000.0f;
        float yieldStrength = 250e6f;
        float thermalConductivity = 50.0f;
    };

    /**
     * @brief Runtime simulation state
     */
    struct PhysicsStateComponent {
        Geometry::Vector3 linearVelocity{0.0f, 0.0f, 0.0f};
        float appliedForce = 0.0f;
        float stress = 0.0f;
        float temperature = 293.15f; // Ambient K
        float heatInput = 0.0f;
        float yieldStrength = 250e6f; // Default 250 MPa
        float stressRatio = 0.0f;
        float tempRatio = 0.0f;
        
        bool isDangerous = false;
    };

} // namespace SZM::SceneGraph
