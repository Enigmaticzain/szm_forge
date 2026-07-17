#pragma once

#include "UI/EasyTabBar.hpp"
#include "WorkshopManager.hpp"
#include <string>
#include <vector>

namespace SZM::Workshop3D {

/**
 * @class ComponentBrowserPanel
 * @brief Easy-to-use component browser with import and visibility
 */
class ComponentBrowserPanel : public BeginnerPanel {
public:
    ComponentBrowserPanel();
    ~ComponentBrowserPanel() override = default;

private:
    // Tab content functions
    void DrawLibraryTab();
    void DrawImportTab();
    void DrawPlacementTab();
    void DrawVisualizationTab();
    void DrawSettingsTab();

    // Helper functions
    void RefreshComponentList();
    void DisplayComponentCard(const std::shared_ptr<Part>& part);
    void ImportComponentFromFile(const std::string& filepath);
    void AddComponentToScene(const std::shared_ptr<Part>& part);

    // State
    char m_SearchFilter[128] = "";
    char m_ImportPath[256] = "";
    char m_ComponentName[96] = "New Component";
    
    float m_PlacementPos[3] = {0.0f, 0.0f, 0.0f};
    float m_PlacementRot[3] = {0.0f, 0.0f, 0.0f};
    float m_ComponentScale = 1.0f;
    
    bool m_ShowGrid = true;
    bool m_ShowLabels = true;
    bool m_ShowBounds = true;
    bool m_AutoPlacement = true;
    
    std::vector<std::shared_ptr<Part>> m_FilteredComponents;
    std::shared_ptr<Part> m_SelectedComponent = nullptr;
    
    int m_VisualizationMode = 0;
    float m_HeatmapMin = 0.0f;
    float m_HeatmapMax = 100.0f;
    bool m_EnableAutoSave = true;
    bool m_EnableSnapToGrid = false;
    float m_GridSize = 0.1f;
    int m_DefaultMaterial = 0;
    bool m_ShowComponentInfo = true;
    bool m_EnablePhysicsPreview = false;
};

} // namespace SZM::Workshop3D
