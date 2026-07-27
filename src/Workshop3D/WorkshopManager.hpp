#pragma once

#include "AssemblyManager.hpp"
#include "Assembly/AttachmentManager.hpp"
#include "CADImporter.hpp"
#include "Graphics/Heatmap.hpp"
#include "PartLibrary.hpp"
#include "Simulation/SimulationEngine.hpp"
#include <string>
#include <unordered_map>
#include <vector>

namespace SZM::Workshop3D {

    class WorkshopManager {
    public:
        static WorkshopManager& GetInstance();

        void Init(const std::string& code8Dir = "");
        void Shutdown();
        bool IsInitialized() const { return m_IsInitialized; }

        PartLibrary& GetPartLibrary() { return m_PartLibrary; }
        const PartLibrary& GetPartLibrary() const { return m_PartLibrary; }

        AssemblyManager& GetAssemblyManager() { return m_AssemblyManager; }
        const AssemblyManager& GetAssemblyManager() const { return m_AssemblyManager; }

        Assembly::AttachmentManager& GetAttachmentManager() { return m_AttachmentManager; }
        const Assembly::AttachmentManager& GetAttachmentManager() const { return m_AttachmentManager; }

        std::shared_ptr<Part> ImportPart(const std::string& filepath, const std::string& name = "");
        int ImportCode8Examples(const std::string& code8Dir);

        UUID CreateAssembly(const std::string& name);
        uint32_t AddPartToActiveAssembly(
            const UUID& partId,
            const Geometry::Vector3& position = Geometry::Vector3(0.0, 0.0, 0.0),
            const std::string& instanceName = ""
        );

        std::shared_ptr<Assembly> GetActiveAssembly() { return m_AssemblyManager.GetActiveAssembly(); }
        std::shared_ptr<const Assembly> GetActiveAssembly() const { return m_AssemblyManager.GetActiveAssembly(); }

        void RenderActiveAssembly();
        void UpdateVisualization(const std::string& data, Graphics::Heatmap::Mode mode);
        Graphics::Heatmap::Mode GetVisualizationMode() const { return m_VisualizationMode; }

        int SetupSimulation();
        bool RunSimulation(float duration = 1.0f);
        uint32_t GetSimulationComponentForInstance(uint32_t instanceId) const;
        Geometry::Vector3 SuggestPlacement(const Part& part) const;

        bool SaveWorkshop(const std::string& dirpath);
        bool LoadWorkshop(const std::string& dirpath);

        size_t GetPartCount() const { return m_PartLibrary.GetPartCount(); }
        size_t GetAssemblyCount() const { return m_AssemblyManager.GetAssemblyCount(); }
        size_t GetInstanceCount() const;

        void SetSelectedPart(const UUID& id) { m_SelectedPartId = id; }
        const UUID& GetSelectedPartId() const { return m_SelectedPartId; }
        std::shared_ptr<Part> GetSelectedPart();
        std::shared_ptr<const Part> GetSelectedPart() const;

        void SetSelectedInstance(uint32_t id) { m_SelectedInstanceId = id; }
        uint32_t GetSelectedInstanceId() const { return m_SelectedInstanceId; }
        std::shared_ptr<AssemblyInstance> GetSelectedInstance();
        std::shared_ptr<const AssemblyInstance> GetSelectedInstance() const;

        const std::string& GetCode8Directory() const { return m_Code8Directory; }

    private:
        WorkshopManager() = default;
        ~WorkshopManager() = default;

        WorkshopManager(const WorkshopManager&) = delete;
        WorkshopManager& operator=(const WorkshopManager&) = delete;

        std::string DiscoverCode8Directory() const;
        void EnsureDefaultAssembly();
        void ClearWorkshopSimulationComponents();

        static float MaterialYieldStrength(const std::string& materialName);
        static float MaterialThermalConductivity(const std::string& materialName);

        PartLibrary m_PartLibrary;
        AssemblyManager m_AssemblyManager;
        Assembly::AttachmentManager m_AttachmentManager;
        SimulationEngine* m_Simulation = nullptr;
        bool m_IsInitialized = false;
        std::string m_Code8Directory;
        Graphics::Heatmap::Mode m_VisualizationMode = Graphics::Heatmap::Mode::Stress;

        UUID m_SelectedPartId = UUID::Null();
        uint32_t m_SelectedInstanceId = 0U;

        std::unordered_map<uint32_t, uint32_t> m_InstanceToSimulationComponent;
        std::vector<uint32_t> m_WorkshopComponentIds;
    };

} // namespace SZM::Workshop3D
