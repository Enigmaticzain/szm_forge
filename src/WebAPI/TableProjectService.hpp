#pragma once

#include <nlohmann/json.hpp>
#include <mutex>
#include <string>
#include <vector>
#include <cstdint>

namespace SZM::WebAPI {

/**
 * @brief Table-with-four-legs workshop project (new project → material → load → limits → AI improve).
 */
class TableProjectService {
public:
    static TableProjectService& GetInstance();

    nlohmann::json CreateNewProject(const std::string& name);
    nlohmann::json CreateTable(const std::string& materialId, float loadKg);
    nlohmann::json CreateFurniture(const std::string& type, const std::string& materialId, float loadKg);
    nlohmann::json SetLoad(float loadKg);
    nlohmann::json RunLimitSweep();
    nlohmann::json RunAIImprove();
    nlohmann::json GetProjectState() const;
    nlohmann::json GetAssemblyTree() const;
    nlohmann::json GetLimits() const;
    nlohmann::json GetAIInsights() const;

private:
    TableProjectService() = default;

    void applyLoad(float loadKg);
    void pushSnapshots(double simTime);

    mutable std::mutex m_Mutex;
    std::string m_ProjectName = "Untitled";
    std::string m_MaterialId = "MAT-STEEL-STRUCT";
    std::string m_MaterialLabel = "Structural Steel";
    float m_LoadKg = 200.0f;
    float m_LegArea = 0.05f * 0.05f;
    bool m_HasTable = false;
    std::string m_FurnitureType = "table";
    bool m_AIImproved = false;
    uint32_t m_TopId = 0;
    std::vector<uint32_t> m_LegIds;
    std::vector<nlohmann::json> m_LimitResults;
    std::vector<nlohmann::json> m_AIInsights;
};

} // namespace SZM::WebAPI
