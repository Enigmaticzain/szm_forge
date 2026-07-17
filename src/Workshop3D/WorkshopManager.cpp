#include "WorkshopManager.hpp"
#include "FEA/CalculiXPipeline.hpp"
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

namespace {

std::string NormalizeMaterialName(const std::string& materialName) {
    std::string normalized = materialName;
    std::transform(normalized.begin(), normalized.end(), normalized.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return normalized;
}

std::string MaterialIdFor(const std::string& materialName) {
    const std::string material = NormalizeMaterialName(materialName);

    if (material.find("steel") != std::string::npos) return "MAT-STEEL-STRUCT";
    if (material.find("aluminum") != std::string::npos || material.find("aluminium") != std::string::npos) {
        return "MAT-AL-6061-T6";
    }
    if (material.find("titanium") != std::string::npos) return "MAT-TI-6AL4V";
    if (material.find("copper") != std::string::npos) return "MAT-CU-C110";
    return "";
}

float DensityFor(const std::string& materialName) {
    const std::string material = NormalizeMaterialName(materialName);

    if (material.find("steel") != std::string::npos) return 7850.0f;
    if (material.find("aluminum") != std::string::npos || material.find("aluminium") != std::string::npos) return 2700.0f;
    if (material.find("titanium") != std::string::npos) return 4500.0f;
    if (material.find("copper") != std::string::npos) return 8960.0f;
    return 1600.0f;
}

} // namespace

namespace SZM::Workshop3D {

    WorkshopManager& WorkshopManager::GetInstance() {
        static WorkshopManager instance;
        return instance;
    }

    void WorkshopManager::Init(const std::string& code8Dir) {
        if (m_IsInitialized) {
            return;
        }

        m_Simulation = &SimulationEngine::GetInstance();
        EnsureDefaultAssembly();

        m_Code8Directory = code8Dir.empty() ? DiscoverCode8Directory() : code8Dir;
        if (!m_Code8Directory.empty()) {
            const int imported = ImportCode8Examples(m_Code8Directory);
            std::cout << "[Workshop3D] Imported " << imported << " code 8 examples from "
                      << m_Code8Directory << '\n';
        } else {
            std::cout << "[Workshop3D] code 8 directory not found. Workshop started without reference library.\n";
        }

        if (m_SelectedPartId.IsNull() && !m_PartLibrary.GetAllParts().empty()) {
            m_SelectedPartId = m_PartLibrary.GetAllParts().begin()->first;
        }

        m_IsInitialized = true;
    }

    void WorkshopManager::Shutdown() {
        ClearWorkshopSimulationComponents();
        m_PartLibrary.ClearAll();
        m_AssemblyManager = AssemblyManager();
        m_SelectedPartId = UUID::Null();
        m_SelectedInstanceId = 0U;
        m_Code8Directory.clear();
        m_IsInitialized = false;
    }

    std::shared_ptr<Part> WorkshopManager::ImportPart(const std::string& filepath, const std::string& name) {
        auto part = CADImporter::ImportAuto(filepath);
        if (!part) {
            return nullptr;
        }

        if (!name.empty()) {
            auto props = part->GetProperties();
            props.name = name;
            part->SetProperties(props);
        }

        m_PartLibrary.RegisterPart(part);
        if (m_SelectedPartId.IsNull()) {
            m_SelectedPartId = part->GetID();
        }
        return part;
    }

    int WorkshopManager::ImportCode8Examples(const std::string& code8Dir) {
        const int imported = Code8AssetManager::LoadAllExamples(code8Dir, m_PartLibrary);
        if (m_SelectedPartId.IsNull() && !m_PartLibrary.GetAllParts().empty()) {
            m_SelectedPartId = m_PartLibrary.GetAllParts().begin()->first;
        }
        return imported;
    }

    UUID WorkshopManager::CreateAssembly(const std::string& name) {
        auto assembly = m_AssemblyManager.CreateAssembly(name);
        return assembly ? assembly->GetID() : UUID::Null();
    }

    uint32_t WorkshopManager::AddPartToActiveAssembly(
        const UUID& partId,
        const Geometry::Vector3& position,
        const std::string& instanceName
    ) {
        EnsureDefaultAssembly();
        auto assembly = GetActiveAssembly();
        auto part = m_PartLibrary.GetPart(partId);
        if (!assembly || !part) {
            return 0U;
        }

        const uint32_t instanceId = assembly->AddInstance(part, instanceName);
        if (auto instance = assembly->GetInstance(instanceId)) {
            instance->SetTransform(position);
        }
        m_SelectedInstanceId = instanceId;
        return instanceId;
    }

    void WorkshopManager::RenderActiveAssembly() {
        auto assembly = GetActiveAssembly();
        if (!assembly) {
            return;
        }

        std::cout << "[Workshop3D] Active assembly contains "
                  << assembly->GetInstanceCount() << " instances\n";
    }

    void WorkshopManager::UpdateVisualization(const std::string& data, Graphics::Heatmap::Mode mode) {
        (void)data;
        m_VisualizationMode = mode;
    }

    int WorkshopManager::SetupSimulation() {
        if (!m_Simulation) {
            return 0;
        }

        auto assembly = GetActiveAssembly();
        if (!assembly) {
            return 0;
        }

        ClearWorkshopSimulationComponents();

        for (const auto& [instanceId, instance] : assembly->GetAllInstances()) {
            const auto& props = instance->GetPart()->GetProperties();
            const auto extents = instance->GetScaledSize();
            const float area = static_cast<float>(std::max(0.0005, std::abs(extents.x * extents.z)));
            const float thickness = static_cast<float>(std::max(0.001, std::abs(extents.y)));
            const float density = props.density > 0.0f ? props.density : DensityFor(props.material);
            const std::string materialId = MaterialIdFor(props.material);

            const uint32_t componentId = materialId.empty()
                ? m_Simulation->AddComponent(instance->GetInstanceName())
                : m_Simulation->AddComponent(instance->GetInstanceName(), materialId, area, thickness);
            auto* component = m_Simulation->GetComponent(componentId);
            if (!component) {
                continue;
            }

            component->position = instance->GetPosition();
            component->area = area;
            component->thickness = thickness;
            component->density = density;
            component->yieldStrength = MaterialYieldStrength(props.material);
            component->thermalConductivity = MaterialThermalConductivity(props.material);
            if (!props.material.empty()) {
                component->materialName = props.material;
                if (materialId.empty()) {
                    component->materialId.clear();
                }
            }
            component->massEstimate = static_cast<float>(instance->GetScaledMass());

            m_InstanceToSimulationComponent[instanceId] = component->id;
            m_WorkshopComponentIds.push_back(component->id);
        }

        return static_cast<int>(m_WorkshopComponentIds.size());
    }

    bool WorkshopManager::RunSimulation(float duration) {
        if (!m_Simulation) {
            return false;
        }

        if (m_WorkshopComponentIds.empty()) {
            SetupSimulation();
        }

        const int steps = std::max(1, static_cast<int>(duration * 60.0f));
        const double dt = duration / static_cast<double>(steps);
        for (int step = 0; step < steps; ++step) {
            m_Simulation->Tick(dt);
        }
        return true;
    }

    uint32_t WorkshopManager::GetSimulationComponentForInstance(uint32_t instanceId) const {
        const auto found = m_InstanceToSimulationComponent.find(instanceId);
        return found != m_InstanceToSimulationComponent.end() ? found->second : 0U;
    }

    Geometry::Vector3 WorkshopManager::SuggestPlacement(const Part& part) const {
        auto assembly = GetActiveAssembly();
        const std::size_t index = assembly ? assembly->GetInstanceCount() : 0U;

        const double width = std::max(0.35, std::abs(part.GetBBoxMax().x - part.GetBBoxMin().x));
        const double depth = std::max(0.35, std::abs(part.GetBBoxMax().z - part.GetBBoxMin().z));
        const double spacing = std::max(width, depth) + 0.35;

        const double x = static_cast<double>(index % 4U) * spacing;
        const double z = static_cast<double>(index / 4U) * spacing;
        return Geometry::Vector3(x, 0.0, z);
    }

    bool WorkshopManager::SaveWorkshop(const std::string& dirpath) {
        std::error_code error;
        fs::create_directories(dirpath, error);
        if (error) {
            return false;
        }

        const bool librarySaved = m_PartLibrary.SaveLibrary((fs::path(dirpath) / "parts").string());
        const bool assembliesSaved = m_AssemblyManager.SaveAllAssemblies((fs::path(dirpath) / "assemblies").string(), m_PartLibrary);

        std::ofstream state(fs::path(dirpath) / "workshop.state");
        if (!state.is_open()) {
            return false;
        }

        state << "code8=" << m_Code8Directory << '\n';
        state << "selected_part=" << m_SelectedPartId.high << ',' << m_SelectedPartId.low << '\n';
        state << "selected_instance=" << m_SelectedInstanceId << '\n';
        state << "visualization=" << static_cast<int>(m_VisualizationMode) << '\n';
        return librarySaved && assembliesSaved;
    }

    bool WorkshopManager::LoadWorkshop(const std::string& dirpath) {
        const bool libraryLoaded = m_PartLibrary.LoadLibrary((fs::path(dirpath) / "parts").string());
        const bool assembliesLoaded = m_AssemblyManager.LoadAllAssemblies((fs::path(dirpath) / "assemblies").string(), m_PartLibrary);

        std::ifstream state(fs::path(dirpath) / "workshop.state");
        if (state.is_open()) {
            std::string line;
            while (std::getline(state, line)) {
                if (line.rfind("code8=", 0U) == 0U) {
                    m_Code8Directory = line.substr(6U);
                } else if (line.rfind("selected_instance=", 0U) == 0U) {
                    m_SelectedInstanceId = static_cast<uint32_t>(std::stoul(line.substr(18U)));
                } else if (line.rfind("visualization=", 0U) == 0U) {
                    m_VisualizationMode = static_cast<Graphics::Heatmap::Mode>(std::stoi(line.substr(14U)));
                }
            }
        }

        EnsureDefaultAssembly();
        return libraryLoaded && assembliesLoaded;
    }

    size_t WorkshopManager::GetInstanceCount() const {
        size_t count = 0U;
        for (const auto& [id, assembly] : m_AssemblyManager.GetAllAssemblies()) {
            (void)id;
            count += assembly->GetInstanceCount();
        }
        return count;
    }

    std::shared_ptr<Part> WorkshopManager::GetSelectedPart() {
        if (m_SelectedPartId.IsNull()) {
            return nullptr;
        }
        return m_PartLibrary.GetPart(m_SelectedPartId);
    }

    std::shared_ptr<const Part> WorkshopManager::GetSelectedPart() const {
        if (m_SelectedPartId.IsNull()) {
            return nullptr;
        }
        return m_PartLibrary.GetPart(m_SelectedPartId);
    }

    std::shared_ptr<AssemblyInstance> WorkshopManager::GetSelectedInstance() {
        auto assembly = GetActiveAssembly();
        return assembly ? assembly->GetInstance(m_SelectedInstanceId) : nullptr;
    }

    std::shared_ptr<const AssemblyInstance> WorkshopManager::GetSelectedInstance() const {
        auto assembly = GetActiveAssembly();
        return assembly ? assembly->GetInstance(m_SelectedInstanceId) : nullptr;
    }

    std::string WorkshopManager::DiscoverCode8Directory() const {
        const fs::path discovered = SZM::FEA::CalculiXPipeline::DiscoverCode8Root(fs::current_path());
        return discovered.empty() ? std::string() : discovered.string();
    }

    void WorkshopManager::EnsureDefaultAssembly() {
        if (!m_AssemblyManager.GetActiveAssembly()) {
            CreateAssembly("Workshop Assembly");
        }
    }

    void WorkshopManager::ClearWorkshopSimulationComponents() {
        if (!m_Simulation) {
            return;
        }

        for (const uint32_t componentId : m_WorkshopComponentIds) {
            m_Simulation->RemoveComponent(componentId);
        }
        m_WorkshopComponentIds.clear();
        m_InstanceToSimulationComponent.clear();
    }

    float WorkshopManager::MaterialYieldStrength(const std::string& materialName) {
        const std::string material = [&materialName]() {
            std::string normalized = materialName;
            std::transform(normalized.begin(), normalized.end(), normalized.begin(), [](unsigned char c) {
                return static_cast<char>(std::tolower(c));
            });
            return normalized;
        }();

        if (material.find("steel") != std::string::npos) return 250e6f;
        if (material.find("aluminum") != std::string::npos || material.find("aluminium") != std::string::npos) return 95e6f;
        if (material.find("titanium") != std::string::npos) return 880e6f;
        if (material.find("copper") != std::string::npos) return 220e6f;
        return 120e6f;
    }

    float WorkshopManager::MaterialThermalConductivity(const std::string& materialName) {
        const std::string material = [&materialName]() {
            std::string normalized = materialName;
            std::transform(normalized.begin(), normalized.end(), normalized.begin(), [](unsigned char c) {
                return static_cast<char>(std::tolower(c));
            });
            return normalized;
        }();

        if (material.find("steel") != std::string::npos) return 50.0f;
        if (material.find("aluminum") != std::string::npos || material.find("aluminium") != std::string::npos) return 160.0f;
        if (material.find("titanium") != std::string::npos) return 7.4f;
        if (material.find("copper") != std::string::npos) return 385.0f;
        return 18.0f;
    }

} // namespace SZM::Workshop3D
