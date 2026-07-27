#pragma once

#include "UI/IPanel.hpp"
#include "Assembly/AttachmentManager.hpp"
#include "Assembly/FastenerDataTypes.hpp"
#include <cstdint>

namespace SZM::Workshop3D {

/**
 * AttachmentPanel - UI for managing mechanical attachments
 * 
 * Provides controls for:
 * - Placing bolts with precise positioning and patterns
 * - Creating hinges with pivot and axis control
 * - Defining welds along edges and surfaces
 * - Editing attachment specifications
 * - Viewing attachment properties
 */
class AttachmentPanel : public IPanel {
public:
    AttachmentPanel();
    void Draw() override;
    std::string GetTitle() const override { return "Attachments"; }

private:
    // Tab state
    enum class Tab { Bolts, Hinges, Welds, Settings };
    Tab m_CurrentTab = Tab::Bolts;
    
    // =========================================================================
    // BOLT UI
    // =========================================================================
    void DrawBoltTab();
    void DrawBoltPlacementSection();
    void DrawBoltSpecEditor();
    void DrawBoltPatternEditor();
    void DrawBoltList();
    void DrawSelectedBoltEditor();
    
    // Bolt specification state
    Assembly::FastenerType m_BoltType = Assembly::FastenerType::HexBolt;
    Assembly::ThreadSpec m_ThreadSpec;
    Assembly::BoltGrade m_BoltGrade = Assembly::BoltGrade::Grade8_8;
    float m_BoltLength = 30.0f;
    std::string m_BoltMaterial = "Steel";
    std::string m_BoltFinish = "Zinc Plated";
    
    // Pattern state
    Assembly::PatternType m_PatternType = Assembly::PatternType::Single;
    int m_PatternRows = 2;
    int m_PatternCols = 2;
    float m_PatternSpacingX = 25.0f;
    float m_PatternSpacingY = 25.0f;
    float m_PatternAngle = 0.0f;
    int m_BoltCount = 6;
    float m_CircleRadius = 50.0f;
    float m_CircleStartAngle = 0.0f;
    float m_CircleEndAngle = 360.0f;
    
    // =========================================================================
    // HINGE UI
    // =========================================================================
    void DrawHingeTab();
    void DrawHingePlacementSection();
    void DrawHingeSpecEditor();
    void DrawHingeList();
    void DrawSelectedHingeEditor();
    
    // Hinge specification state
    Assembly::HingeType m_HingeType = Assembly::HingeType::Standard;
    float m_HingeLength = 100.0f;
    float m_HingeWidth = 40.0f;
    float m_HingeThickness = 3.0f;
    float m_PinDiameter = 6.0f;
    float m_MinAngle = -180.0f;
    float m_MaxAngle = 180.0f;
    bool m_HasStops = true;
    std::string m_HingeMaterial = "Steel";
    
    // =========================================================================
    // WELD UI
    // =========================================================================
    void DrawWeldTab();
    void DrawWeldPlacementSection();
    void DrawWeldSpecEditor();
    void DrawWeldList();
    void DrawSelectedWeldEditor();
    
    // Weld specification state
    Assembly::WeldType m_WeldType = Assembly::WeldType::Fillet;
    Assembly::WeldProcess m_WeldProcess = Assembly::WeldProcess::GMAW;
    float m_WeldSize = 5.0f;
    float m_WeldThroat = 3.5f;
    std::string m_FillerMaterial = "ER70S-6";
    std::string m_ShieldingGas = "75% Ar / 25% CO2";
    bool m_WeldAllAround = false;
    
    // =========================================================================
    // SETTINGS UI
    // =========================================================================
    void DrawSettingsTab();
    void DrawSnapSettings();
    void DrawPlacementSettings();
    void DrawAnalysisSection();
    
    // Snap settings
    bool m_SnapToGrid = true;
    bool m_SnapToEdge = true;
    bool m_SnapToVertex = true;
    bool m_SnapToCenter = true;
    float m_GridSize = 5.0f;
    float m_SnapRadius = 10.0f;
    
    // Placement settings
    bool m_ShowPreview = true;
    bool m_ShowAttachmentLabels = true;
    float m_PreviewScale = 1.0f;
    
    // =========================================================================
    // HELPERS
    // =========================================================================
    Assembly::BoltSpec BuildCurrentBoltSpec() const;
    Assembly::HingeSpec BuildCurrentHingeSpec() const;
    Assembly::WeldSpec BuildCurrentWeldSpec() const;
    Assembly::BoltPattern BuildCurrentPattern() const;
    
    void UpdateSnapSettings();
    void UpdatePlacementMode();
};

} // namespace SZM::Workshop3D
