#include "TableProjectService.hpp"
#include "FastAPIBridge.hpp"
#include "../Simulation/SimulationEngine.hpp"
#include "../Simulation/SimulationComponent.hpp"
#include "../AI/AIEngine.hpp"
#include "../Materials/MaterialDatabase.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>

namespace SZM::WebAPI {

namespace {

constexpr float kG = 9.81f;

std::string materialLabel(const std::string& materialId) {
    const auto mat = Materials::MaterialDatabase::GetInstance().GetMaterial(materialId);
    return mat.has_value() ? mat->name : materialId;
}

} // namespace

TableProjectService& TableProjectService::GetInstance() {
    static TableProjectService instance;
    return instance;
}

namespace {

nlohmann::json componentRow(const SimulationComponent& comp) {
    return {
        {"name", comp.name},
        {"stress_MPa", comp.stress / 1e6f},
        {"yield_MPa", comp.yieldStrength / 1e6f},
        {"temperature_C", comp.temperature - 273.15f},
        {"stressRatio", comp.stressRatio},
        {"tempRatio", comp.tempRatio},
        {"status", comp.stressRatio >= 1.0f ? "FAIL" : (comp.isDangerous ? "WARN" : "SAFE")},
        {"force_N", comp.appliedForce},
        {"material", comp.materialName}
    };
}

} // namespace

void TableProjectService::pushSnapshots(double simTime) {
    auto& engine = SimulationEngine::GetInstance();
    for (const auto& compPtr : engine.GetComponents()) {
        if (!compPtr) {
            continue;
        }
        const auto& c = *compPtr;
        APIManager::GetInstance().PushComponentSnapshot(
            c.name,
            c.stress / 1e6f,
            c.temperature - 273.15f,
            c.stressRatio,
            c.tempRatio,
            simTime);
    }
}

void TableProjectService::applyLoad(float loadKg) {
    m_LoadKg = loadKg;
    if (!m_HasTable) {
        return;
    }

    auto& engine = SimulationEngine::GetInstance();
    const float totalForce = loadKg * kG;

    for (const auto& compPtr : engine.GetComponents()) {
        if (!compPtr) {
            continue;
        }
        const std::string& n = compPtr->name;
        float share = 0.0f;
        if (m_FurnitureType == "ladder") {
            if (n.find("Rail") != std::string::npos) {
                share = 0.35f;
            } else if (n.find("Rung") != std::string::npos) {
                share = 0.05f;
            }
        } else if (m_FurnitureType == "chair") {
            if (n == "Seat") {
                share = 0.55f;
            } else if (n == "Backrest") {
                share = 0.15f;
            } else if (n.find("Leg") != std::string::npos) {
                share = 0.075f;
            }
        } else {
            if (n == "Tabletop") {
                share = 1.0f;
            } else if (n.find("Leg") != std::string::npos) {
                share = 0.25f;
            }
        }
        engine.SetForce(compPtr->id, totalForce * share);
    }
    engine.Tick(0.0);
    pushSnapshots(static_cast<double>(loadKg));
}

nlohmann::json TableProjectService::CreateNewProject(const std::string& name) {
    std::lock_guard<std::mutex> lock(m_Mutex);

    m_ProjectName = name.empty() ? "Table Workshop Project" : name;
    m_HasTable = false;
    m_AIImproved = false;
    m_TopId = 0;
    m_LegIds.clear();
    m_LimitResults.clear();
    m_AIInsights.clear();
    m_LoadKg = 200.0f;
    m_LegArea = 0.05f * 0.05f;

    SimulationEngine::GetInstance().ClearAllComponents();

    std::cout << "[TableProject] New project: " << m_ProjectName << "\n";

    return {
        {"ok", true},
        {"projectName", m_ProjectName},
        {"message", "Empty project created. Pick furniture type and press BUILD."}
    };
}

nlohmann::json TableProjectService::CreateTable(const std::string& materialId, float loadKg) {
    return CreateFurniture("table", materialId, loadKg);
}

nlohmann::json TableProjectService::CreateFurniture(const std::string& type,
                                                     const std::string& materialId,
                                                     float loadKg) {
    std::lock_guard<std::mutex> lock(m_Mutex);

    std::string matId = materialId.empty() ? "MAT-STEEL-STRUCT" : materialId;
    if (!Materials::MaterialDatabase::GetInstance().GetMaterial(matId).has_value()) {
        matId = "MAT-STEEL-STRUCT";
    }
    m_MaterialId = matId;
    m_MaterialLabel = materialLabel(matId);
    m_LoadKg = std::max(loadKg, 1.0f);
    m_FurnitureType = type.empty() ? "table" : type;
    m_LegArea = 0.05f * 0.05f;

    auto& engine = SimulationEngine::GetInstance();
    engine.ClearAllComponents();
    m_LegIds.clear();

    if (m_FurnitureType == "chair") {
        m_TopId = engine.AddComponent("Seat", m_MaterialId, 0.015f, 0.04f);
        engine.AddComponent("Backrest", m_MaterialId, 0.008f, 0.03f);
        m_LegIds = {
            engine.AddComponent("Leg-FL", m_MaterialId, m_LegArea, 0.45f),
            engine.AddComponent("Leg-FR", m_MaterialId, m_LegArea, 0.45f),
            engine.AddComponent("Leg-RL", m_MaterialId, m_LegArea, 0.45f),
            engine.AddComponent("Leg-RR", m_MaterialId, m_LegArea, 0.45f)
        };
    } else if (m_FurnitureType == "ladder") {
        m_TopId = engine.AddComponent("Rail-Left", m_MaterialId, 0.003f, 0.05f);
        m_LegIds.push_back(engine.AddComponent("Rail-Right", m_MaterialId, 0.003f, 0.05f));
        for (int i = 1; i <= 5; ++i) {
            m_LegIds.push_back(engine.AddComponent("Rung-" + std::to_string(i), m_MaterialId, 0.0012f, 0.02f));
        }
    } else {
        constexpr float kTopL = 0.6f;
        constexpr float kTopB = 0.8f;
        constexpr float kTopT = 0.025f;
        const float topEffArea = (4.0f * kTopB * kTopT * kTopT) / (3.0f * kTopL * kTopL);
        m_TopId = engine.AddComponent("Tabletop", m_MaterialId, topEffArea, kTopT);
        m_LegIds = {
            engine.AddComponent("Leg-FL", m_MaterialId, m_LegArea, 0.75f),
            engine.AddComponent("Leg-FR", m_MaterialId, m_LegArea, 0.75f),
            engine.AddComponent("Leg-RL", m_MaterialId, m_LegArea, 0.75f),
            engine.AddComponent("Leg-RR", m_MaterialId, m_LegArea, 0.75f)
        };
        m_FurnitureType = "table";
    }

    m_HasTable = true;
    m_AIImproved = false;
    m_LimitResults.clear();
    m_AIInsights.clear();

    applyLoad(m_LoadKg);

    nlohmann::json parts = nlohmann::json::array();
    for (const auto& compPtr : engine.GetComponents()) {
        if (compPtr) {
            parts.push_back(componentRow(*compPtr));
        }
    }

    std::cout << "[TableProject] " << m_FurnitureType << " created — material=" << m_MaterialLabel
              << " load=" << m_LoadKg << " kg\n";

    return {
        {"ok", true},
        {"furnitureType", m_FurnitureType},
        {"projectName", m_ProjectName},
        {"materialId", m_MaterialId},
        {"material", m_MaterialLabel},
        {"loadKg", m_LoadKg},
        {"parts", parts},
        {"message", m_FurnitureType + " assembled and loaded."}
    };
}

nlohmann::json TableProjectService::SetLoad(float loadKg) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    if (!m_HasTable) {
        return {{"ok", false}, {"error", "Create a table first."}};
    }
    applyLoad(std::max(loadKg, 0.0f));

    nlohmann::json parts = nlohmann::json::array();
    for (const auto& compPtr : SimulationEngine::GetInstance().GetComponents()) {
        if (compPtr) {
            parts.push_back(componentRow(*compPtr));
        }
    }

    return {{"ok", true}, {"loadKg", m_LoadKg}, {"parts", parts}};
}

nlohmann::json TableProjectService::RunLimitSweep() {
    std::lock_guard<std::mutex> lock(m_Mutex);
    if (!m_HasTable) {
        return {{"ok", false}, {"error", "Create a table first."}};
    }

    const std::vector<float> testLoads = m_FurnitureType == "ladder"
        ? std::vector<float>{40.0f, 80.0f, 100.0f, 120.0f, 150.0f, 180.0f}
        : std::vector<float>{50.0f, 100.0f, 200.0f, 350.0f, 500.0f, 700.0f};
    m_LimitResults.clear();

    float maxSafeKg = 0.0f;
    float firstFailKg = -1.0f;
    std::string criticalPart;

    for (float kg : testLoads) {
        applyLoad(kg);

        float worstRatio = 0.0f;
        std::string worstName;
        nlohmann::json parts = nlohmann::json::array();

        for (const auto& compPtr : SimulationEngine::GetInstance().GetComponents()) {
            if (!compPtr) {
                continue;
            }
            parts.push_back(componentRow(*compPtr));
            if (compPtr->stressRatio > worstRatio) {
                worstRatio = compPtr->stressRatio;
                worstName = compPtr->name;
            }
        }

        const bool fail = worstRatio >= 1.0f;
        const bool warn = worstRatio >= 0.8f;
        const std::string status = fail ? "FAIL" : (warn ? "WARN" : "SAFE");

        if (!fail) {
            maxSafeKg = kg;
        } else if (firstFailKg < 0.0f) {
            firstFailKg = kg;
            criticalPart = worstName;
        }

        m_LimitResults.push_back({
            {"loadKg", kg},
            {"worstUtilization", worstRatio * 100.0f},
            {"criticalPart", worstName},
            {"status", status},
            {"parts", parts}
        });
    }

    applyLoad(m_LoadKg);

    return {
        {"ok", true},
        {"maxSafeLoadKg", maxSafeKg},
        {"firstFailureLoadKg", firstFailKg < 0.0f ? nullptr : nlohmann::json(firstFailKg)},
        {"criticalPart", criticalPart},
        {"designLoadKg", m_LoadKg},
        {"results", m_LimitResults}
    };
}

nlohmann::json TableProjectService::RunAIImprove() {
    std::lock_guard<std::mutex> lock(m_Mutex);
    if (!m_HasTable) {
        return {{"ok", false}, {"error", "Create a table first."}};
    }

    AI::AIEngine::GetInstance().Init();

    float worstRatio = 0.0f;
    uint32_t worstId = m_TopId;
    for (const auto& compPtr : SimulationEngine::GetInstance().GetComponents()) {
        if (compPtr && compPtr->stressRatio > worstRatio) {
            worstRatio = compPtr->stressRatio;
            worstId = compPtr->id;
        }
    }

    AI::DesignMetrics metrics;
    metrics.stress = worstRatio;
    metrics.weight = m_LoadKg;
    metrics.cost = 50.0f;

    const auto recommendations = AI::AIEngine::GetInstance().AnalyzeDesign(metrics);
    const auto failure = AI::AIEngine::GetInstance().PredictFailure(worstId, metrics);

    auto& engine = SimulationEngine::GetInstance();

    // AI improvement: thicken legs (+40% area) and optionally upgrade material if still critical
    m_LegArea *= 1.4f;
    for (uint32_t legId : m_LegIds) {
        engine.SetComponentGeometry(legId, m_LegArea, 0.75f);
    }

    if (worstRatio >= 0.85f && m_MaterialId == "MAT-AL-6061-T6") {
        m_MaterialId = "MAT-STEEL-STRUCT";
        m_MaterialLabel = materialLabel(m_MaterialId);
        for (uint32_t id : m_LegIds) {
            engine.SetComponentMaterial(id, m_MaterialId);
        }
        engine.SetComponentMaterial(m_TopId, m_MaterialId);
    } else if (worstRatio >= 0.85f) {
        m_MaterialId = "MAT-TI-6AL4V";
        m_MaterialLabel = materialLabel(m_MaterialId);
        for (uint32_t id : m_LegIds) {
            engine.SetComponentMaterial(id, m_MaterialId);
        }
        engine.SetComponentMaterial(m_TopId, m_MaterialId);
    }

    m_AIImproved = true;
    applyLoad(m_LoadKg);

    m_AIInsights.clear();
    for (const auto& rec : recommendations) {
        m_AIInsights.push_back({
            {"title", rec.title},
            {"description", rec.description},
            {"confidence", static_cast<int>(rec.confidence * 100.0f)},
            {"actions", rec.actions},
            {"type", "recommendation"}
        });
    }

    m_AIInsights.push_back({
        {"title", "Applied: Leg cross-section increase"},
        {"description", "AI increased leg/rail area by 40% to reduce stress under design load."},
        {"confidence", 92},
        {"actions", nlohmann::json::array({"Leg area x1.4", "Re-run limit sweep"})},
        {"type", "optimization"}
    });

    if (!failure.failureMode.empty()) {
        m_AIInsights.push_back({
            {"title", "Failure risk: " + failure.failureMode},
            {"description", "Predicted before improvement. Mitigations: " +
             (failure.preventiveMeasures.empty() ? "reduce load" : failure.preventiveMeasures[0])},
            {"confidence", static_cast<int>(failure.probability * 100.0f)},
            {"type", "warning"}
        });
    }

    nlohmann::json partsAfter = nlohmann::json::array();
    for (const auto& compPtr : engine.GetComponents()) {
        if (compPtr) {
            partsAfter.push_back(componentRow(*compPtr));
        }
    }

    std::cout << "[TableProject] AI improvement applied — legs thickened, material=" << m_MaterialLabel << "\n";

    return {
        {"ok", true},
        {"improved", true},
        {"materialId", m_MaterialId},
        {"material", m_MaterialLabel},
        {"legArea_m2", m_LegArea},
        {"parts", partsAfter},
        {"insights", m_AIInsights}
    };
}

nlohmann::json TableProjectService::GetProjectState() const {
    std::lock_guard<std::mutex> lock(m_Mutex);
    return {
        {"projectName", m_ProjectName},
        {"hasTable", m_HasTable},
        {"hasFurniture", m_HasTable},
        {"furnitureType", m_FurnitureType},
        {"materialId", m_MaterialId},
        {"material", m_MaterialLabel},
        {"loadKg", m_LoadKg},
        {"aiImproved", m_AIImproved}
    };
}

nlohmann::json TableProjectService::GetAssemblyTree() const {
    std::lock_guard<std::mutex> lock(m_Mutex);
    if (!m_HasTable) {
        return {{"name", m_ProjectName}, {"children", nlohmann::json::array()}};
    }

    const auto& engine = SimulationEngine::GetInstance();
    nlohmann::json legs = nlohmann::json::array();
    for (uint32_t legId : m_LegIds) {
        if (const auto* leg = engine.GetComponent(legId)) {
            legs.push_back({
                {"name", leg->name},
                {"type", "Part"},
                {"material", leg->materialName},
                {"stressRatio", leg->stressRatio}
            });
        }
    }

    nlohmann::json topNode = {{"name", "Tabletop"}, {"type", "Part"}};
    if (const auto* top = engine.GetComponent(m_TopId)) {
        topNode["material"] = top->materialName;
        topNode["stressRatio"] = top->stressRatio;
    }

    return {
        {"name", m_ProjectName},
        {"type", "Assembly"},
        {"loadKg", m_LoadKg},
        {"material", m_MaterialLabel},
        {"children", nlohmann::json::array({
            topNode,
            {{"name", "Legs (x4)"}, {"type", "Group"}, {"children", legs}}
        })}
    };
}

nlohmann::json TableProjectService::GetLimits() const {
    std::lock_guard<std::mutex> lock(m_Mutex);
    return {{"results", m_LimitResults}};
}

nlohmann::json TableProjectService::GetAIInsights() const {
    std::lock_guard<std::mutex> lock(m_Mutex);
    return {{"insights", m_AIInsights}};
}

} // namespace SZM::WebAPI
