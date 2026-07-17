#pragma once

#include "../IPanel.hpp"
#include "../../Simulation/SimulationEngine.hpp"
#include "../../Geometry/MathTypes.hpp"
#include <imgui.h>

/**
 * @class SimulationVisualizerPanel
 * @brief Enhanced 3D viewport with interactive camera, advanced heatmaps, and component details.
 *
 * Features:
 *  - Interactive camera (orbit, pan, zoom with mouse)
 *  - Advanced stress/temperature heatmaps with smooth gradients
 *  - 3D component visualization with lighting
 *  - Component filtering and search
 *  - Real-time metrics and danger indicators
 *  - Visualization mode toggles (solid, wireframe, points)
 */
class SimulationVisualizerPanel : public IPanel {
public:
    SimulationVisualizerPanel();
    ~SimulationVisualizerPanel();

    void        Draw()     override;
    std::string GetTitle() const override { return "Simulation Visualizer"; }

private:
    // Rendering
    void RenderViewport();
    void RenderComponents();
    void RenderComponentList();
    void RenderVisualizationControls();
    void OnSimulationTick();
    
    // Color mapping
    SZM::Geometry::Vector3 GetComponentColor(const SZM::SimulationComponent& comp) const;
    SZM::Geometry::Vector3 MapStressToColor(float ratio) const;
    SZM::Geometry::Vector3 MapTemperatureToColor(float ratio) const;
    SZM::Geometry::Vector3 MapDangerToColor(bool isDangerous) const;
    
    // Camera controls
    void UpdateCamera();
    void DrawCameraControls();
    
    // Component rendering
    void DrawComponentCube(const SZM::SimulationComponent& comp, ImDrawList* drawList, 
                          const ImVec2& viewportPos, const ImVec2& viewportSize);
    void DrawComponentDetails(const SZM::SimulationComponent& comp);

private:
    // Camera state
    float m_CameraDistance = 5.0f;
    float m_CameraYaw = 45.0f;
    float m_CameraPitch = 30.0f;
    ImVec2 m_LastMousePos = ImVec2(0, 0);
    bool m_IsMouseDragging = false;
    
    // Visualization state
    bool m_ShowStressHeatmap = false;
    bool m_ShowTemperatureHeatmap = false;
    bool m_ShowWireframe = false;
    bool m_ShowComponentLabels = true;
    bool m_ShowDangerIndicators = true;
    
    // Component filtering
    char m_SearchFilter[64] = "";
    uint32_t m_SelectedComponentId = 0;
    
    // Display options
    float m_ComponentScale = 1.0f;
    bool m_AutoRotate = false;
    float m_AutoRotateSpeed = 15.0f;
};
