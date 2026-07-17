#pragma once

#include "../IPanel.hpp"
#include "../../Simulation/SimulationEngine.hpp"
#include <string>
#include <cstdint>

/**
 * @class ComponentBuilderPanel
 * @brief Interactive component builder with material presets.
 *
 * Features:
 *  - Material presets (Steel, Aluminum, Titanium, Copper)
 *  - Geometry input (area, thickness)
 *  - Quick-add buttons for common component types
 *  - Component template library
 */
class ComponentBuilderPanel : public IPanel {
public:
    ComponentBuilderPanel();

    void        Draw()     override;
    std::string GetTitle() const override { return "Component Builder"; }

private:
    void DrawMaterialPresets();
    void DrawGeometryInput();
    void DrawComponentTemplates();
    void ApplyMaterialPreset(const std::string& materialId);
    void CreateTemplateComponent(const char* name,
                                 const std::string& materialId,
                                 float area,
                                 float thickness);

private:
    char     m_ComponentName[64] = "Beam";
    float    m_Area = 0.01f;           // m²
    float    m_Thickness = 0.01f;      // m
    std::string m_SelectedMaterialId = "MAT-STEEL-STRUCT";
    
    // Material properties
    float    m_Density = 7850.0f;
    float    m_YieldStrength = 250e6f;
    float    m_ThermalConductivity = 50.0f;
};
