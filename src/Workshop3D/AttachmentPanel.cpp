#include "AttachmentPanel.hpp"
#include "WorkshopManager.hpp"
#include "Assembly/AttachmentManager.hpp"
#include "UI/UITooltipHelper.hpp"
#include <imgui.h>
#include <algorithm>
#include <sstream>

namespace SZM::Workshop3D {

using namespace SZM::Assembly;

// ============================================================================
// CONSTRUCTION
// ============================================================================

AttachmentPanel::AttachmentPanel() {
    // Initialize default thread spec for M10
    m_ThreadSpec.nominalDiameter = 10.0f;
    m_ThreadSpec.pitch = 1.5f;
    m_ThreadSpec.standard = ThreadStandard::Metric;
}

// ============================================================================
// MAIN DRAW
// ============================================================================

void AttachmentPanel::Draw() {
    if (!ImGui::Begin(GetTitle().c_str(), &IsVisible)) {
        ImGui::End();
        return;
    }
    
    // Tab bar
    if (ImGui::BeginTabBar("AttachmentTabs")) {
        if (ImGui::BeginTabItem("Bolts")) {
            m_CurrentTab = Tab::Bolts;
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Hinges")) {
            m_CurrentTab = Tab::Hinges;
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Welds")) {
            m_CurrentTab = Tab::Welds;
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Settings")) {
            m_CurrentTab = Tab::Settings;
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    
    ImGui::Separator();
    
    switch (m_CurrentTab) {
        case Tab::Bolts: DrawBoltTab(); break;
        case Tab::Hinges: DrawHingeTab(); break;
        case Tab::Welds: DrawWeldTab(); break;
        case Tab::Settings: DrawSettingsTab(); break;
    }
    
    ImGui::End();
}

// ============================================================================
// BOLT TAB
// ============================================================================

void AttachmentPanel::DrawBoltTab() {
    auto& workshop = WorkshopManager::GetInstance();
    auto& attachMgr = workshop.GetAttachmentManager();
    
    // Summary stats
    ImGui::Text("Bolts: %zu", attachMgr.GetBoltCount());
    ImGui::SameLine();
    ImGui::TextDisabled("|");
    ImGui::SameLine();
    ImGui::Text("Total Clamp: %.1f kN", attachMgr.CalculateTotalClampForce() / 1000.0f);
    
    ImGui::Separator();
    
    // Two-column layout
    if (ImGui::BeginChild("BoltLeftColumn", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f - 5, 0))) {
        DrawBoltSpecEditor();
        ImGui::Separator();
        DrawBoltPatternEditor();
        ImGui::Separator();
        DrawBoltPlacementSection();
    }
    ImGui::EndChild();
    
    ImGui::SameLine();
    
    if (ImGui::BeginChild("BoltRightColumn", ImVec2(0, 0))) {
        DrawSelectedBoltEditor();
        ImGui::Separator();
        DrawBoltList();
    }
    ImGui::EndChild();
}

void AttachmentPanel::DrawBoltSpecEditor() {
    if (ImGui::CollapsingHeader("Bolt Specification", ImGuiTreeNodeFlags_DefaultOpen)) {
        // Bolt type
        const char* boltTypes[] = {
            "Hex Bolt", "Socket Head Cap Screw", "Machine Screw",
            "Rivet", "Stud", "Anchor Bolt"
        };
        int typeIdx = static_cast<int>(m_BoltType);
        if (ImGui::Combo("Type", &typeIdx, boltTypes, IM_ARRAYSIZE(boltTypes))) {
            m_BoltType = static_cast<FastenerType>(typeIdx);
        }
        
        ImGui::SeparatorText("Thread Specification");
        
        // Thread standard
        const char* standards[] = {"Metric", "Imperial"};
        int stdIdx = m_ThreadSpec.standard == ThreadStandard::Metric ? 0 : 1;
        if (ImGui::Combo("Standard", &stdIdx, standards, IM_ARRAYSIZE(standards))) {
            m_ThreadSpec.standard = stdIdx == 0 ? ThreadStandard::Metric : ThreadStandard::Imperial;
        }
        
        // Diameter with common sizes
        ImGui::SetNextItemWidth(100);
        const char* metricSizes[] = {
            "M3", "M4", "M5", "M6", "M8", "M10", "M12", "M14", "M16",
            "M18", "M20", "M22", "M24", "M27", "M30"
        };
        float diameters[] = {3, 4, 5, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 27, 30};
        
        // Find closest match
        int selectedDiameter = 5; // Default M10
        for (int i = 0; i < IM_ARRAYSIZE(diameters); ++i) {
            if (m_ThreadSpec.nominalDiameter == diameters[i]) {
                selectedDiameter = i;
                break;
            }
        }
        
        if (ImGui::ListBox("Diameter", &selectedDiameter, metricSizes, IM_ARRAYSIZE(metricSizes), 6)) {
            m_ThreadSpec.nominalDiameter = diameters[selectedDiameter];
        }
        ImGui::SameLine();
        ImGui::TextDisabled("(%.1f mm)", m_ThreadSpec.nominalDiameter);
        
        // Pitch
        ImGui::SetNextItemWidth(100);
        const char* pitches[] = {"Coarse", "Fine", "Extra Fine"};
        float pitchValues[] = {1.5f, 1.25f, 1.0f};
        
        int selectedPitch = 0;
        for (int i = 0; i < IM_ARRAYSIZE(pitchValues); ++i) {
            if (std::abs(m_ThreadSpec.pitch - pitchValues[i]) < 0.01f) {
                selectedPitch = i;
                break;
            }
        }
        
        if (ImGui::Combo("Pitch", &selectedPitch, pitches, IM_ARRAYSIZE(pitches))) {
            m_ThreadSpec.pitch = pitchValues[selectedPitch];
        }
        
        ImGui::TextDisabled("Thread: %s", m_ThreadSpec.ToString().c_str());
        
        ImGui::SeparatorText("Dimensions");
        
        // Length
        ImGui::DragFloat("Length (mm)", &m_BoltLength, 1.0f, 5.0f, 500.0f, "%.1f");
        
        // Grade
        ImGui::SeparatorText("Material & Grade");
        const char* grades[] = {
            "8.8 (Standard)", "10.9 (High Strength)", "12.9 (Premium)",
            "Grade 5", "Grade 8", "A325 (Structural)", "A490 (High Strength)"
        };
        int gradeIdx = 0;
        switch (m_BoltGrade) {
            case BoltGrade::Grade8_8: gradeIdx = 0; break;
            case BoltGrade::Grade10_9: gradeIdx = 1; break;
            case BoltGrade::Grade12_9: gradeIdx = 2; break;
            case BoltGrade::Grade5: gradeIdx = 3; break;
            case BoltGrade::Grade8: gradeIdx = 4; break;
            case BoltGrade::A325: gradeIdx = 5; break;
            case BoltGrade::A490: gradeIdx = 6; break;
        }
        if (ImGui::ListBox("Grade", &gradeIdx, grades, IM_ARRAYSIZE(grades), 4)) {
            switch (gradeIdx) {
                case 0: m_BoltGrade = BoltGrade::Grade8_8; break;
                case 1: m_BoltGrade = BoltGrade::Grade10_9; break;
                case 2: m_BoltGrade = BoltGrade::Grade12_9; break;
                case 3: m_BoltGrade = BoltGrade::Grade5; break;
                case 4: m_BoltGrade = BoltGrade::Grade8; break;
                case 5: m_BoltGrade = BoltGrade::A325; break;
                case 6: m_BoltGrade = BoltGrade::A490; break;
            }
        }
        
        // Material
        const char* materials[] = {"Steel", "Stainless", "Brass", "Titanium", "Aluminum"};
        int matIdx = 0;
        for (int i = 0; i < IM_ARRAYSIZE(materials); ++i) {
            if (m_BoltMaterial == materials[i]) {
                matIdx = i;
                break;
            }
        }
        if (ImGui::Combo("Material", &matIdx, materials, IM_ARRAYSIZE(materials))) {
            m_BoltMaterial = materials[matIdx];
        }
        
        // Finish
        const char* finishes[] = {
            "Plain", "Zinc Plated", "Hot Dip Galvanized", "Black Oxide",
            "Phosphate", "Stainless Passivate", "Cadmium", "Geomet"
        };
        int finIdx = 1; // Default Zinc Plated
        for (int i = 0; i < IM_ARRAYSIZE(finishes); ++i) {
            if (m_BoltFinish == finishes[i]) {
                finIdx = i;
                break;
            }
        }
        if (ImGui::Combo("Finish", &finIdx, finishes, IM_ARRAYSIZE(finishes))) {
            m_BoltFinish = finishes[finIdx];
        }
        
        // Calculated values
        BoltSpec spec = BuildCurrentBoltSpec();
        ImGui::SeparatorText("Calculated Values");
        ImGui::TextColored(ImVec4(0.3f, 0.8f, 0.3f, 1.0f),
            "Head: %.1fmm dia, %.1fmm height", spec.headDiameter, spec.headHeight);
        ImGui::TextColored(ImVec4(0.3f, 0.8f, 0.3f, 1.0f),
            "Torque: %.1f Nm", spec.torque);
        ImGui::TextColored(ImVec4(0.3f, 0.8f, 0.3f, 1.0f),
            "Preload: %.0f N (%.1f kN)", spec.preload, spec.preload / 1000.0f);
    }
}

void AttachmentPanel::DrawBoltPatternEditor() {
    if (ImGui::CollapsingHeader("Bolt Pattern", ImGuiTreeNodeFlags_DefaultOpen)) {
        const char* patterns[] = {
            "Single", "Grid (2D)", "Circular", "Bolt Circle", 
            "Along Line", "Custom"
        };
        int patIdx = static_cast<int>(m_PatternType);
        if (ImGui::Combo("Pattern", &patIdx, patterns, IM_ARRAYSIZE(patterns))) {
            m_PatternType = static_cast<PatternType>(patIdx);
        }
        
        switch (m_PatternType) {
            case PatternType::Single:
                ImGui::TextDisabled("Single bolt placement");
                break;
                
            case PatternType::Grid2D: {
                ImGui::DragInt("Rows", &m_PatternRows, 1, 1, 20);
                ImGui::DragInt("Columns", &m_PatternCols, 1, 1, 20);
                ImGui::DragFloat("Spacing X (mm)", &m_PatternSpacingX, 1.0f, 5.0f, 200.0f);
                ImGui::DragFloat("Spacing Y (mm)", &m_PatternSpacingY, 1.0f, 5.0f, 200.0f);
                ImGui::DragFloat("Pattern Angle", &m_PatternAngle, 1.0f, -180.0f, 180.0f);
                
                int totalBolts = m_PatternRows * m_PatternCols;
                ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.3f, 1.0f),
                    "Total bolts: %d", totalBolts);
                break;
            }
            
            case PatternType::Circular:
            case PatternType::BoltCircle: {
                ImGui::DragInt("Bolt Count", &m_BoltCount, 1, 2, 48);
                ImGui::DragFloat("Circle Radius (mm)", &m_CircleRadius, 1.0f, 10.0f, 500.0f);
                ImGui::DragFloat("Start Angle", &m_CircleStartAngle, 1.0f, -180.0f, 180.0f);
                ImGui::DragFloat("End Angle", &m_CircleEndAngle, 1.0f, 0.0f, 360.0f);
                
                ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.3f, 1.0f),
                    "Total bolts: %d", m_BoltCount);
                break;
            }
            
            case PatternType::AlongLine: {
                ImGui::TextDisabled("Use placement tool to define line");
                ImGui::DragInt("Count", &m_BoltCount, 1, 2, 20);
                ImGui::TextDisabled("(Line defined by placement)");
                break;
            }
            
            default:
                ImGui::TextDisabled("Custom pattern: define positions in viewport");
                break;
        }
    }
}

void AttachmentPanel::DrawBoltPlacementSection() {
    if (ImGui::CollapsingHeader("Placement", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("Click on a part surface in the viewport");
        ImGui::Text("to place the configured bolt(s).");
        
        ImGui::Separator();
        
        BoltSpec spec = BuildCurrentBoltSpec();
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 1.0f, 1.0f),
            "Ready to place: %s", spec.GetDisplayName().c_str());
        
        BoltPattern pattern = BuildCurrentPattern();
        int boltCount = static_cast<int>(pattern.GetPositions().size());
        ImGui::Text("Pattern will create %d bolt(s)", boltCount);
        
        ImGui::Separator();
        
        if (ImGui::Button("Start Bolt Placement", ImVec2(-1, 0))) {
            auto& workshop = WorkshopManager::GetInstance();
            workshop.GetAttachmentManager().StartBoltPlacement(spec);
        }
        
        ImGui::SameLine();
        
        if (ImGui::Button("Cancel")) {
            auto& workshop = WorkshopManager::GetInstance();
            workshop.GetAttachmentManager().CancelPlacement();
        }
        
        ImGui::Separator();
        
        // Quick patterns section
        ImGui::Text("Quick Patterns:");
        
        ImGui::BeginGroup();
        if (ImGui::SmallButton("M6x16 4x4 Grid")) {
            m_ThreadSpec.nominalDiameter = 6.0f;
            m_ThreadSpec.pitch = 1.0f;
            m_BoltLength = 16.0f;
            m_PatternType = PatternType::Grid2D;
            m_PatternRows = 4;
            m_PatternCols = 4;
            m_PatternSpacingX = 20.0f;
            m_PatternSpacingY = 20.0f;
        }
        ImGui::SameLine();
        if (ImGui::SmallButton("M8x25 6-Bolt")) {
            m_ThreadSpec.nominalDiameter = 8.0f;
            m_ThreadSpec.pitch = 1.25f;
            m_BoltLength = 25.0f;
            m_PatternType = PatternType::Circular;
            m_BoltCount = 6;
            m_CircleRadius = 40.0f;
        }
        ImGui::SameLine();
        if (ImGui::SmallButton("M10 PCD")) {
            m_ThreadSpec.nominalDiameter = 10.0f;
            m_ThreadSpec.pitch = 1.5f;
            m_BoltLength = 30.0f;
            m_PatternType = PatternType::BoltCircle;
            m_BoltCount = 8;
            m_CircleRadius = 60.0f;
        }
        ImGui::EndGroup();
    }
}

void AttachmentPanel::DrawBoltList() {
    if (ImGui::CollapsingHeader("Placed Bolts", ImGuiTreeNodeFlags_DefaultOpen)) {
        auto& workshop = WorkshopManager::GetInstance();
        auto& attachMgr = workshop.GetAttachmentManager();
        const auto& data = attachMgr.GetData();
        
        if (data.bolts.empty()) {
            ImGui::TextDisabled("No bolts placed yet");
            return;
        }
        
        for (const auto& bolt : data.bolts) {
            bool isSelected = data.selectedAttachmentId == bolt.id;
            
            std::ostringstream label;
            label << bolt.spec.GetDisplayName();
            
            if (ImGui::Selectable(label.str().c_str(), isSelected)) {
                attachMgr.SelectAttachment(bolt.id);
            }
            
            ImGui::SameLine();
            ImGui::TextDisabled("%.0f,%.0f,%.0f", 
                bolt.surfaceRef.worldPosition.x,
                bolt.surfaceRef.worldPosition.y,
                bolt.surfaceRef.worldPosition.z);
        }
    }
}

void AttachmentPanel::DrawSelectedBoltEditor() {
    auto& workshop = WorkshopManager::GetInstance();
    auto* bolt = workshop.GetAttachmentManager().GetSelectedBolt();
    
    if (!bolt) {
        ImGui::TextDisabled("Select a bolt to edit");
        return;
    }
    
    if (ImGui::CollapsingHeader("Selected Bolt", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::TextColored(ImVec4(1.0f, 0.9f, 0.3f, 1.0f), "%s", bolt->spec.GetDisplayName().c_str());
        
        ImGui::SeparatorText("Position");
        ImGui::Text("World: (%.1f, %.1f, %.1f)",
            bolt->surfaceRef.worldPosition.x,
            bolt->surfaceRef.worldPosition.y,
            bolt->surfaceRef.worldPosition.z);
        
        ImGui::Text("Offset from surface: %.1f mm", bolt->surfaceRef.offset);
        
        ImGui::SeparatorText("Installation");
        ImGui::Checkbox("Installed", &bolt->isInstalled);
        if (bolt->isInstalled) {
            ImGui::TextColored(ImVec4(0.3f, 0.9f, 0.3f, 1.0f), "Preload: %.0f N", bolt->spec.preload);
            ImGui::TextColored(ImVec4(0.3f, 0.9f, 0.3f, 1.0f), "Torque: %.1f Nm", bolt->spec.torque);
        }
        
        ImGui::Separator();
        
        if (ImGui::Button("Delete Bolt", ImVec2(-1, 0))) {
            workshop.GetAttachmentManager().RemoveBolt(bolt->id);
            workshop.GetAttachmentManager().ClearSelection();
        }
    }
}

// ============================================================================
// HINGE TAB
// ============================================================================

void AttachmentPanel::DrawHingeTab() {
    auto& workshop = WorkshopManager::GetInstance();
    auto& attachMgr = workshop.GetAttachmentManager();
    
    ImGui::Text("Hinges: %zu", attachMgr.GetHingeCount());
    
    ImGui::Separator();
    
    if (ImGui::BeginChild("HingeLeft", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f - 5, 0))) {
        DrawHingeSpecEditor();
        ImGui::Separator();
        DrawHingePlacementSection();
    }
    ImGui::EndChild();
    
    ImGui::SameLine();
    
    if (ImGui::BeginChild("HingeRight", ImVec2(0, 0))) {
        DrawSelectedHingeEditor();
        ImGui::Separator();
        DrawHingeList();
    }
    ImGui::EndChild();
}

void AttachmentPanel::DrawHingeSpecEditor() {
    if (ImGui::CollapsingHeader("Hinge Specification", ImGuiTreeNodeFlags_DefaultOpen)) {
        const char* types[] = {
            "Standard", "Piano (Continuous)", "Butt", 
            "Barrel", "Spring Return", "Detent"
        };
        int typeIdx = static_cast<int>(m_HingeType);
        if (ImGui::Combo("Type", &typeIdx, types, IM_ARRAYSIZE(types))) {
            m_HingeType = static_cast<HingeType>(typeIdx);
        }
        
        ImGui::SeparatorText("Dimensions");
        ImGui::DragFloat("Length (mm)", &m_HingeLength, 1.0f, 20.0f, 500.0f);
        ImGui::DragFloat("Width (mm)", &m_HingeWidth, 1.0f, 10.0f, 200.0f);
        ImGui::DragFloat("Thickness (mm)", &m_HingeThickness, 0.1f, 1.0f, 20.0f);
        ImGui::DragFloat("Pin Diameter (mm)", &m_PinDiameter, 0.1f, 2.0f, 30.0f);
        
        ImGui::SeparatorText("Motion Limits");
        ImGui::DragFloat("Min Angle (deg)", &m_MinAngle, 1.0f, -180.0f, 180.0f);
        ImGui::DragFloat("Max Angle (deg)", &m_MaxAngle, 1.0f, -180.0f, 180.0f);
        ImGui::Checkbox("Has Physical Stops", &m_HasStops);
        
        ImGui::SeparatorText("Material");
        const char* mats[] = {"Steel", "Stainless", "Aluminum", "Brass", "Nylon"};
        int matIdx = 0;
        for (int i = 0; i < IM_ARRAYSIZE(mats); ++i) {
            if (m_HingeMaterial == mats[i]) {
                matIdx = i;
                break;
            }
        }
        ImGui::Combo("Material", &matIdx, mats, IM_ARRAYSIZE(mats));
        m_HingeMaterial = mats[matIdx];
        
        ImGui::TextColored(ImVec4(0.3f, 0.8f, 0.3f, 1.0f),
            "%s", BuildCurrentHingeSpec().GetDisplayName().c_str());
    }
}

void AttachmentPanel::DrawHingePlacementSection() {
    if (ImGui::CollapsingHeader("Placement", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("1. Select first part (static)");
        ImGui::Text("2. Select second part (moving)");
        ImGui::Text("3. Click to place hinge pivot");
        
        ImGui::Separator();
        
        if (ImGui::Button("Start Hinge Placement", ImVec2(-1, 0))) {
            auto& workshop = WorkshopManager::GetInstance();
            workshop.GetAttachmentManager().StartHingePlacement(BuildCurrentHingeSpec());
        }
        
        ImGui::SameLine();
        
        if (ImGui::Button("Cancel")) {
            auto& workshop = WorkshopManager::GetInstance();
            workshop.GetAttachmentManager().CancelPlacement();
        }
    }
}

void AttachmentPanel::DrawHingeList() {
    if (ImGui::CollapsingHeader("Placed Hinges", ImGuiTreeNodeFlags_DefaultOpen)) {
        auto& workshop = WorkshopManager::GetInstance();
        auto& attachMgr = workshop.GetAttachmentManager();
        const auto& data = attachMgr.GetData();
        
        if (data.hinges.empty()) {
            ImGui::TextDisabled("No hinges placed yet");
            return;
        }
        
        for (const auto& hinge : data.hinges) {
            bool isSelected = data.selectedAttachmentId == hinge.id;
            
            std::ostringstream label;
            label << hinge.spec.GetDisplayName();
            
            if (ImGui::Selectable(label.str().c_str(), isSelected)) {
                attachMgr.SelectAttachment(hinge.id);
            }
            
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 1.0f, 1.0f), 
                "%.0f°", hinge.currentAngle);
        }
    }
}

void AttachmentPanel::DrawSelectedHingeEditor() {
    auto& workshop = WorkshopManager::GetInstance();
    auto* hinge = workshop.GetAttachmentManager().GetSelectedHinge();
    
    if (!hinge) {
        ImGui::TextDisabled("Select a hinge to edit");
        return;
    }
    
    if (ImGui::CollapsingHeader("Selected Hinge", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::TextColored(ImVec4(1.0f, 0.9f, 0.3f, 1.0f), "%s", hinge->spec.GetDisplayName().c_str());
        
        ImGui::SeparatorText("Pivot Point");
        ImGui::Text("Position: (%.1f, %.1f, %.1f)",
            hinge->pivotPoint.x, hinge->pivotPoint.y, hinge->pivotPoint.z);
        
        ImGui::SeparatorText("Current State");
        ImGui::TextColored(ImVec4(0.3f, 0.8f, 1.0f, 1.0f), "Angle: %.1f°", hinge->currentAngle);
        
        ImGui::SeparatorText("Motion Control");
        if (ImGui::SliderFloat("Adjust Angle", &hinge->currentAngle, 
                                hinge->spec.minAngle, hinge->spec.maxAngle, "%.1f°")) {
            workshop.GetAttachmentManager().UpdateHingeAngle(hinge->id, hinge->currentAngle);
        }
        
        ImGui::Checkbox("Angle Constrained", &hinge->isConstrained);
        
        ImGui::Separator();
        
        if (ImGui::Button("Delete Hinge", ImVec2(-1, 0))) {
            workshop.GetAttachmentManager().RemoveHinge(hinge->id);
            workshop.GetAttachmentManager().ClearSelection();
        }
    }
}

// ============================================================================
// WELD TAB
// ============================================================================

void AttachmentPanel::DrawWeldTab() {
    auto& workshop = WorkshopManager::GetInstance();
    auto& attachMgr = workshop.GetAttachmentManager();
    
    ImGui::Text("Welds: %zu", attachMgr.GetWeldCount());
    ImGui::SameLine();
    ImGui::TextDisabled("|");
    ImGui::SameLine();
    ImGui::Text("Total Strength: %.1f kN", attachMgr.CalculateTotalWeldStrength() / 1000.0f);
    
    ImGui::Separator();
    
    if (ImGui::BeginChild("WeldLeft", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f - 5, 0))) {
        DrawWeldSpecEditor();
        ImGui::Separator();
        DrawWeldPlacementSection();
    }
    ImGui::EndChild();
    
    ImGui::SameLine();
    
    if (ImGui::BeginChild("WeldRight", ImVec2(0, 0))) {
        DrawSelectedWeldEditor();
        ImGui::Separator();
        DrawWeldList();
    }
    ImGui::EndChild();
}

void AttachmentPanel::DrawWeldSpecEditor() {
    if (ImGui::CollapsingHeader("Weld Specification", ImGuiTreeNodeFlags_DefaultOpen)) {
        const char* types[] = {
            "Fillet", "Butt", "Plug", "Slot", 
            "Spot", "Seam", "Plug & Slot", "Fillet All-Around"
        };
        int typeIdx = static_cast<int>(m_WeldType);
        if (ImGui::Combo("Weld Type", &typeIdx, types, IM_ARRAYSIZE(types))) {
            m_WeldType = static_cast<WeldType>(typeIdx);
        }
        
        ImGui::SeparatorText("Process");
        const char* processes[] = {
            "SMAW (Stick)", "GMAW (MIG)", "FCAW (Flux-Core)",
            "GTAW (TIG)", "SAW (Submerged Arc)", "EBW", "LBW (Laser)"
        };
        int procIdx = static_cast<int>(m_WeldProcess);
        if (ImGui::Combo("Process", &procIdx, processes, IM_ARRAYSIZE(processes))) {
            m_WeldProcess = static_cast<WeldProcess>(procIdx);
        }
        
        ImGui::SeparatorText("Dimensions");
        ImGui::DragFloat("Leg Size (mm)", &m_WeldSize, 0.5f, 2.0f, 25.0f);
        ImGui::DragFloat("Throat (mm)", &m_WeldThroat, 0.5f, 1.0f, 20.0f);
        ImGui::Checkbox("Weld All-Around", &m_WeldAllAround);
        
        ImGui::SeparatorText("Filler Material");
        const char* fillers[] = {
            "ER70S-6 (MIG Mild Steel)", "ER308L (Stainless)",
            "ER4043 (Aluminum)", "E7018 (Stick)"
        };
        int fillIdx = 0;
        for (int i = 0; i < IM_ARRAYSIZE(fillers); ++i) {
            if (m_FillerMaterial.find(fillers[i]) != std::string::npos) {
                fillIdx = i;
                break;
            }
        }
        if (ImGui::Combo("Filler", &fillIdx, fillers, IM_ARRAYSIZE(fillers))) {
            m_FillerMaterial = fillers[fillIdx];
        }
        
        ImGui::SeparatorText("Calculated Strength");
        WeldSpec spec = BuildCurrentWeldSpec();
        spec.CalculateStrength();
        ImGui::TextColored(ImVec4(0.3f, 0.9f, 0.3f, 1.0f),
            "Leg Force: %.1f N/mm", spec.legForce);
        
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 1.0f, 1.0f),
            "%s", spec.GetDisplayName().c_str());
    }
}

void AttachmentPanel::DrawWeldPlacementSection() {
    if (ImGui::CollapsingHeader("Placement", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("Select weld type above, then:");
        
        ImGui::BulletText("Fillet: Click on edge between parts");
        ImGui::BulletText("Butt: Select edge, define gap");
        ImGui::BulletText("Spot: Click points on surface");
        ImGui::BulletText("All-Around: Select part to weld perimeter");
        
        ImGui::Separator();
        
        if (ImGui::Button("Start Weld Placement", ImVec2(-1, 0))) {
            auto& workshop = WorkshopManager::GetInstance();
            workshop.GetAttachmentManager().StartWeldPlacement(BuildCurrentWeldSpec());
        }
        
        ImGui::SameLine();
        
        if (ImGui::Button("Cancel")) {
            auto& workshop = WorkshopManager::GetInstance();
            workshop.GetAttachmentManager().CancelPlacement();
        }
        
        ImGui::Separator();
        
        ImGui::Text("Quick Welds:");
        if (ImGui::SmallButton("6mm Fillet")) {
            m_WeldType = WeldType::Fillet;
            m_WeldSize = 6.0f;
            m_WeldThroat = 4.0f;
        }
        ImGui::SameLine();
        if (ImGui::SmallButton("8mm Fillet")) {
            m_WeldType = WeldType::Fillet;
            m_WeldSize = 8.0f;
            m_WeldThroat = 5.5f;
        }
        ImGui::SameLine();
        if (ImGui::SmallButton("MIG Spot")) {
            m_WeldType = WeldType::Spot;
            m_WeldProcess = WeldProcess::GMAW;
        }
    }
}

void AttachmentPanel::DrawWeldList() {
    if (ImGui::CollapsingHeader("Placed Welds", ImGuiTreeNodeFlags_DefaultOpen)) {
        auto& workshop = WorkshopManager::GetInstance();
        auto& attachMgr = workshop.GetAttachmentManager();
        const auto& data = attachMgr.GetData();
        
        if (data.welds.empty()) {
            ImGui::TextDisabled("No welds placed yet");
            return;
        }
        
        for (const auto& weld : data.welds) {
            bool isSelected = data.selectedAttachmentId == weld.id;
            
            std::ostringstream label;
            label << weld.spec.GetDisplayName();
            
            if (ImGui::Selectable(label.str().c_str(), isSelected)) {
                attachMgr.SelectAttachment(weld.id);
            }
            
            ImGui::SameLine();
            ImGui::TextDisabled("%.0fmm", weld.totalLength);
        }
    }
}

void AttachmentPanel::DrawSelectedWeldEditor() {
    auto& workshop = WorkshopManager::GetInstance();
    auto* weld = workshop.GetAttachmentManager().GetSelectedWeld();
    
    if (!weld) {
        ImGui::TextDisabled("Select a weld to edit");
        return;
    }
    
    if (ImGui::CollapsingHeader("Selected Weld", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::TextColored(ImVec4(1.0f, 0.9f, 0.3f, 1.0f), "%s", weld->spec.GetDisplayName().c_str());
        
        ImGui::SeparatorText("Geometry");
        ImGui::Text("Total Length: %.1f mm", weld->totalLength);
        ImGui::Text("Leg Size: %.1f mm", weld->spec.size);
        
        ImGui::SeparatorText("Strength");
        ImGui::TextColored(ImVec4(0.3f, 0.9f, 0.3f, 1.0f), "Total Strength: %.1f kN", 
            weld->totalStrength / 1000.0f);
        ImGui::Text("Quality Factor: %.0f%%", weld->qualityFactor * 100.0f);
        
        ImGui::SeparatorText("Visual");
        ImGui::Checkbox("Visible", &weld->isVisible);
        
        ImGui::Separator();
        
        if (ImGui::Button("Delete Weld", ImVec2(-1, 0))) {
            workshop.GetAttachmentManager().RemoveWeld(weld->id);
            workshop.GetAttachmentManager().ClearSelection();
        }
    }
}

// ============================================================================
// SETTINGS TAB
// ============================================================================

void AttachmentPanel::DrawSettingsTab() {
    DrawSnapSettings();
    ImGui::Separator();
    DrawPlacementSettings();
    ImGui::Separator();
    DrawAnalysisSection();
}

void AttachmentPanel::DrawSnapSettings() {
    if (ImGui::CollapsingHeader("Snap Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Checkbox("Snap to Grid", &m_SnapToGrid);
        ImGui::SameLine();
        ImGui::Checkbox("Grid labels", nullptr);
        
        if (m_SnapToGrid) {
            ImGui::SetNextItemWidth(100);
            ImGui::DragFloat("Grid Size", &m_GridSize, 1.0f, 1.0f, 50.0f, "%.1f mm");
        }
        
        ImGui::Checkbox("Snap to Edges", &m_SnapToEdge);
        ImGui::Checkbox("Snap to Vertices", &m_SnapToVertex);
        ImGui::Checkbox("Snap to Centers", &m_SnapToCenter);
        
        ImGui::SetNextItemWidth(100);
        ImGui::DragFloat("Snap Radius", &m_SnapRadius, 1.0f, 1.0f, 50.0f, "%.1f mm");
        
        ImGui::Separator();
        
        if (ImGui::Button("Update Snap Settings")) {
            UpdateSnapSettings();
        }
    }
}

void AttachmentPanel::DrawPlacementSettings() {
    if (ImGui::CollapsingHeader("Placement Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Checkbox("Show Preview", &m_ShowPreview);
        ImGui::Checkbox("Show Labels", &m_ShowAttachmentLabels);
        
        ImGui::DragFloat("Preview Scale", &m_PreviewScale, 0.1f, 0.1f, 5.0f, "%.1fx");
        
        ImGui::Separator();
        
        if (ImGui::Button("Cancel Placement Mode")) {
            auto& workshop = WorkshopManager::GetInstance();
            workshop.GetAttachmentManager().CancelPlacement();
        }
    }
}

void AttachmentPanel::DrawAnalysisSection() {
    if (ImGui::CollapsingHeader("Assembly Analysis", ImGuiTreeNodeFlags_DefaultOpen)) {
        auto& workshop = WorkshopManager::GetInstance();
        auto& attachMgr = workshop.GetAttachmentManager();
        
        ImGui::Text("Attachment Summary:");
        ImGui::BulletText("Bolts: %zu", attachMgr.GetBoltCount());
        ImGui::BulletText("Hinges: %zu", attachMgr.GetHingeCount());
        ImGui::BulletText("Welds: %zu", attachMgr.GetWeldCount());
        
        ImGui::Separator();
        
        ImGui::Text("Load Capacity:");
        ImGui::TextColored(ImVec4(0.3f, 0.8f, 0.3f, 1.0f), 
            "Total Clamp Force: %.1f kN", attachMgr.CalculateTotalClampForce() / 1000.0f);
        ImGui::TextColored(ImVec4(0.3f, 0.8f, 0.3f, 1.0f), 
            "Total Weld Strength: %.1f kN", attachMgr.CalculateTotalWeldStrength() / 1000.0f);
        
        ImGui::Separator();
        
        if (ImGui::Button("Generate Report")) {
            std::string report = attachMgr.GenerateAttachmentReport();
            ImGui::SetClipboardText(report.c_str());
            ImGui::OpenPopup("Report Copied!");
        }
        
        if (ImGui::BeginPopup("Report Copied!")) {
            ImGui::Text("Report copied to clipboard!");
            ImGui::EndPopup();
        }
        
        ImGui::Separator();
        
        if (attachMgr.IsAdequatelyFastened()) {
            ImGui::TextColored(ImVec4(0.3f, 0.9f, 0.3f, 1.0f), "Assembly Status: FASTENED");
        } else {
            ImGui::TextColored(ImVec4(0.9f, 0.3f, 0.3f, 1.0f), "Assembly Status: UNFASTENED");
        }
    }
}

// ============================================================================
// HELPERS
// ============================================================================

BoltSpec AttachmentPanel::BuildCurrentBoltSpec() const {
    BoltSpec spec;
    spec.type = m_BoltType;
    spec.thread = m_ThreadSpec;
    spec.grade = m_BoltGrade;
    spec.length = m_BoltLength;
    spec.material = m_BoltMaterial;
    spec.finish = m_BoltFinish;
    spec.CalculateFromThread();
    return spec;
}

HingeSpec AttachmentPanel::BuildCurrentHingeSpec() const {
    HingeSpec spec;
    spec.type = m_HingeType;
    spec.length = m_HingeLength;
    spec.width = m_HingeWidth;
    spec.thickness = m_HingeThickness;
    spec.pinDiameter = m_PinDiameter;
    spec.material = m_HingeMaterial;
    spec.minAngle = m_MinAngle;
    spec.maxAngle = m_MaxAngle;
    spec.hasStops = m_HasStops;
    return spec;
}

WeldSpec AttachmentPanel::BuildCurrentWeldSpec() const {
    WeldSpec spec;
    spec.type = m_WeldType;
    spec.process = m_WeldProcess;
    spec.size = m_WeldSize;
    spec.throat = m_WeldThroat;
    spec.fillerMaterial = m_FillerMaterial;
    spec.weldAllAround = m_WeldAllAround;
    spec.CalculateStrength();
    return spec;
}

BoltPattern AttachmentPanel::BuildCurrentPattern() const {
    BoltPattern pattern;
    pattern.type = m_PatternType;
    pattern.rows = m_PatternRows;
    pattern.columns = m_PatternCols;
    pattern.spacingX = m_PatternSpacingX;
    pattern.spacingY = m_PatternSpacingY;
    pattern.patternAngle = m_PatternAngle;
    pattern.boltCount = m_BoltCount;
    pattern.circleRadius = m_CircleRadius;
    pattern.startAngle = m_CircleStartAngle;
    pattern.endAngle = m_CircleEndAngle;
    return pattern;
}

void AttachmentPanel::UpdateSnapSettings() {
    auto& workshop = WorkshopManager::GetInstance();
    workshop.GetAttachmentManager().SetSnapSettings(
        m_SnapToGrid, m_SnapToEdge, m_SnapToVertex, m_SnapToCenter,
        m_GridSize, m_SnapRadius
    );
}

void AttachmentPanel::UpdatePlacementMode() {
    // Called when placement mode changes to update UI state
}

} // namespace SZM::Workshop3D
