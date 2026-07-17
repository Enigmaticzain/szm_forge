#pragma once

#include "../Data/SceneGraph.hpp"
#include "../Data/UUID.hpp"
#include <unordered_map>
#include <vector>
#include <string>
#include <memory>
#include <stdexcept>

namespace SZM::Assembly {

    // ----------------------------------------------------------------
    //  Material override — applied per-instance (e.g. paint colour)
    // ----------------------------------------------------------------
    struct MaterialOverride {
        bool  IsActive  = false;
        float RGBA[4]   = {1.f, 1.f, 1.f, 1.f};
        float Roughness = 0.5f;
        float Metallic  = 0.0f;
    };

    enum class AssetType {
        Part,
        Assembly
    };

    // ----------------------------------------------------------------
    //  BasePart — heavy CAD data loaded ONCE into the part registry
    // ----------------------------------------------------------------
    struct BasePart {
        UUID        PartID;
        std::string PartName;
        float       Mass     = 0.0f;        ///< [kg]
        float       LocalCG[3] = {0,0,0};   ///< Local centre-of-gravity [m]
        // std::shared_ptr<Geometry::SolidBody> Geometry; // wired in 110-Series
    };

    // ----------------------------------------------------------------
    //  AssemblyTemplate — reusable sub-assembly definition
    // ----------------------------------------------------------------
    struct AssemblyTemplate {
        UUID        TemplateID;
        std::string TemplateName;
        
        struct ComponentRef {
            UUID assetID;
            AssetType type;
            SZM::Transform localTransform;
        };
        std::vector<ComponentRef> Components;
    };

    // ----------------------------------------------------------------
    //  InstanceNode — lightweight reference inserted into SceneGraph
    // ----------------------------------------------------------------
    class InstanceNode : public SZM::Entity {
    public:
        InstanceNode(std::string name, UUID assetRef, AssetType type, const SZM::Transform& tx)
            : SZM::Entity(std::move(name))
            , m_AssetUUID(assetRef)
            , m_AssetType(type)
        {
            SetLocalTransform(tx);
        }

        UUID             GetAssetUUID() const { return m_AssetUUID;  }
        AssetType        GetAssetType() const { return m_AssetType;  }
        MaterialOverride GetMaterial()     const { return m_Material;      }
        void SetMaterialOverride(const MaterialOverride& mat) { m_Material = mat; }

    private:
        UUID             m_AssetUUID;
        AssetType        m_AssetType;
        MaterialOverride m_Material;
    };

    // ----------------------------------------------------------------
    //  Bill of Materials entry
    // ----------------------------------------------------------------
    struct BOMEntry {
        std::string PartName;
        UUID        PartID;
        uint32_t    Quantity  = 0;
        float       TotalMass = 0.0f; ///< [kg]
    };

    // ----------------------------------------------------------------
    //  AssemblyGraphManager
    // ----------------------------------------------------------------
    class AssemblyGraphManager {
    public:
        explicit AssemblyGraphManager(std::shared_ptr<SZM::Entity> assemblyRoot)
            : m_Root(std::move(assemblyRoot)) {}

        /// Register a BasePart into the in-memory part registry (load-once)
        void RegisterBasePart(std::shared_ptr<BasePart> part);

        /// Register an AssemblyTemplate
        void RegisterAssemblyTemplate(std::shared_ptr<AssemblyTemplate> templ);

        /// Instantiate a registered asset (Part or Assembly) under a parent scene-graph node
        std::shared_ptr<InstanceNode> InstantiateAsset(
            std::shared_ptr<SZM::Entity> parent,
            UUID                         assetUUID,
            AssetType                    type,
            const SZM::Transform&        transform,
            const std::string&           instanceName = "Instance"
        );

        /// Traverse the graph and produce a flat Bill of Materials
        std::vector<BOMEntry> GenerateBOM() const;

        /// Compute assembly total mass and centre-of-gravity
        void CalculateMassProperties(float& outTotalMass, float outCG[3]) const;

        /// Collect all world transforms for a given part UUID (for instanced draw calls)
        std::vector<SZM::Transform> GetInstancedDrawData(UUID partUUID) const;

    private:
        std::shared_ptr<SZM::Entity>                           m_Root;
        std::unordered_map<UUID, std::shared_ptr<BasePart>>    m_PartRegistry;
        std::unordered_map<UUID, std::shared_ptr<AssemblyTemplate>> m_TemplateRegistry;

        void TraverseForBOM(
            const std::shared_ptr<SZM::Entity>&      node,
            std::unordered_map<UUID, BOMEntry>&       bomMap) const;

        void TraverseForMassCG(
            const std::shared_ptr<SZM::Entity>&      node,
            float&                                    totalMass,
            float                                     massMoments[3]) const;

        void TraverseForInstances(
            const std::shared_ptr<SZM::Entity>&      node,
            UUID                                      targetPart,
            std::vector<SZM::Transform>&              out) const;
    };

} // namespace SZM::Assembly
