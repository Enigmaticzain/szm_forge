#pragma once

#include "UI/EasyTabBar.hpp"
#include "WorkshopManager.hpp"
#include <string>
#include <vector>

namespace SZM::Workshop3D {

/**
 * @class ComponentWeightTablePanel
 * @brief Displays component weights in a table format for 3D workspace
 */
class ComponentWeightTablePanel : public BeginnerPanel {
public:
    ComponentWeightTablePanel();
    ~ComponentWeightTablePanel() override = default;

private:
    // Tab content functions
    void DrawLibraryTableTab();
    void DrawSceneTableTab();
    void DrawStatisticsTab();
    void DrawExportTab();

    // Helper functions
    void RefreshComponentWeights();
    void RefreshSceneWeights();
    void DrawWeightTable(const std::vector<std::pair<std::string, float>>& weights);
    void ExportWeightsToCSV(const std::string& filepath);

    // State
    std::vector<std::pair<std::string, float>> m_LibraryWeights;
    std::vector<std::pair<std::string, float>> m_SceneWeights;
    
    float m_TotalLibraryWeight = 0.0f;
    float m_TotalSceneWeight = 0.0f;
    float m_AverageComponentWeight = 0.0f;
    
    char m_ExportPath[256] = "";
    bool m_ShowOnlyHeavy = false;
    float m_HeavyThreshold = 10.0f;
    
    int m_SortMode = 0; // 0=name, 1=weight ascending, 2=weight descending
};

} // namespace SZM::Workshop3D
