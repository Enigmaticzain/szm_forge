#pragma once

#include "Data/UUID.hpp"
#include "Graphics/Camera.hpp"
#include "PartLibrary.hpp"
#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

namespace SZM::Workshop3D {

    class AssemblyInstance {
    public:
        explicit AssemblyInstance(std::shared_ptr<Part> part);
        ~AssemblyInstance() = default;

        std::shared_ptr<Part> GetPart() const { return m_Part; }

        void SetTransform(
            const Geometry::Vector3& position,
            const Geometry::Vector3& rotation = Geometry::Vector3(0.0, 0.0, 0.0)
        );
        void SetScale(const Geometry::Vector3& scale);
        const Geometry::Vector3& GetPosition() const { return m_Position; }
        const Geometry::Vector3& GetRotation() const { return m_Rotation; }
        const Geometry::Vector3& GetScale() const { return m_Scale; }
        Graphics::Matrix4x4 GetTransformMatrix() const;
        Geometry::Vector3 GetScaledBBoxMin() const;
        Geometry::Vector3 GetScaledBBoxMax() const;
        Geometry::Vector3 GetScaledSize() const;
        double GetScaledMass() const;
        double GetScaledRadius() const;

        void SetInstanceName(const std::string& name) { m_InstanceName = name; }
        const std::string& GetInstanceName() const { return m_InstanceName; }
        void SetVisible(bool visible) { m_IsVisible = visible; }
        bool IsVisible() const { return m_IsVisible; }

        void AddConstraint(uint32_t targetInstanceId, const std::string& constraintType);
        const std::vector<std::pair<uint32_t, std::string>>& GetConstraints() const { return m_Constraints; }

    private:
        std::shared_ptr<Part> m_Part;
        std::string m_InstanceName;
        Geometry::Vector3 m_Position;
        Geometry::Vector3 m_Rotation;
        Geometry::Vector3 m_Scale = Geometry::Vector3(1.0, 1.0, 1.0);
        bool m_IsVisible = true;
        std::vector<std::pair<uint32_t, std::string>> m_Constraints;
    };

    class Assembly {
    public:
        explicit Assembly(const std::string& name);
        ~Assembly() = default;

        const std::string& GetName() const { return m_Name; }
        void SetName(const std::string& name) { m_Name = name; }
        const UUID& GetID() const { return m_ID; }

        uint32_t AddInstance(std::shared_ptr<Part> part, const std::string& instanceName = "");
        std::shared_ptr<AssemblyInstance> GetInstance(uint32_t instanceId);
        std::shared_ptr<const AssemblyInstance> GetInstance(uint32_t instanceId) const;
        bool RemoveInstance(uint32_t instanceId);
        void ClearInstances();

        size_t GetInstanceCount() const { return m_Instances.size(); }
        const std::map<uint32_t, std::shared_ptr<AssemblyInstance>>& GetAllInstances() const { return m_Instances; }
        std::vector<uint32_t> GetInstancesByPart(const UUID& partId) const;

        Geometry::Vector3 CalculateCenterOfMass() const;
        float CalculateTotalMass() const;
        void CalculateBounds(Geometry::Vector3& minBounds, Geometry::Vector3& maxBounds) const;

        void AddConstraint(uint32_t instance1, uint32_t instance2, const std::string& type);
        const std::vector<std::tuple<uint32_t, uint32_t, std::string>>& GetConstraints() const { return m_Constraints; }

        bool SaveToJSON(const std::string& filepath, const PartLibrary& library) const;
        bool LoadFromJSON(const std::string& filepath, const PartLibrary& library);

    private:
        UUID m_ID;
        std::string m_Name;
        std::map<uint32_t, std::shared_ptr<AssemblyInstance>> m_Instances;
        std::vector<std::tuple<uint32_t, uint32_t, std::string>> m_Constraints;
        uint32_t m_NextInstanceId = 1U;
    };

    class AssemblyManager {
    public:
        AssemblyManager() = default;
        ~AssemblyManager() = default;

        std::shared_ptr<Assembly> CreateAssembly(const std::string& name);
        std::shared_ptr<Assembly> GetAssembly(const UUID& id);
        std::shared_ptr<const Assembly> GetAssembly(const UUID& id) const;
        std::shared_ptr<Assembly> GetAssemblyByName(const std::string& name);
        bool RemoveAssembly(const UUID& id);

        size_t GetAssemblyCount() const { return m_Assemblies.size(); }
        const std::map<UUID, std::shared_ptr<Assembly>>& GetAllAssemblies() const { return m_Assemblies; }

        void SetActiveAssembly(const UUID& id) { m_ActiveAssemblyId = id; }
        std::shared_ptr<Assembly> GetActiveAssembly();
        std::shared_ptr<const Assembly> GetActiveAssembly() const;
        const UUID& GetActiveAssemblyId() const { return m_ActiveAssemblyId; }

        bool SaveAllAssemblies(const std::string& dirpath, const PartLibrary& library) const;
        bool LoadAllAssemblies(const std::string& dirpath, const PartLibrary& library);

    private:
        std::map<UUID, std::shared_ptr<Assembly>> m_Assemblies;
        std::map<std::string, UUID> m_NameIndex;
        UUID m_ActiveAssemblyId = UUID::Null();
    };

} // namespace SZM::Workshop3D
