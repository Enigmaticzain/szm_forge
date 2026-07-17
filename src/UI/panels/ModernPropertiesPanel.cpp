#include "ModernPropertiesPanel.hpp"
#include <imgui.h>

namespace SZM {

void ModernPropertiesPanel::DrawHeader() {
    ImGui::TextUnformatted("Object Properties");
    ImGui::SameLine(ImGui::GetWindowWidth() - 100);
    UIStyleHelper::StatusBadge("active");
}

void ModernPropertiesPanel::DrawContent() {
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, 6));
    
    DrawTransformSection();
    ImGui::Spacing();
    DrawMaterialSection();
    ImGui::Spacing();
    DrawPhysicsSection();
    ImGui::Spacing();
    DrawAdvancedSection();
    
    ImGui::PopStyleVar();
}

void ModernPropertiesPanel::DrawTransformSection() {
    bool& expanded = m_SectionExpanded["Transform"];
    
    ImGui::PushStyleColor(ImGuiCol_Header, UIStyleHelper::Colors::Violet500);
    if (ImGui::CollapsingHeader("Transform", &expanded, ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::PopStyleColor();
        
        ImGui::Spacing();
        
        // Position
        static float pos[3] = {0.0f, 0.0f, 0.0f};
        ImGui::DragFloat3("Position##transform", pos, 0.1f);
        
        // Rotation
        static float rot[3] = {0.0f, 0.0f, 0.0f};
        ImGui::DragFloat3("Rotation##transform", rot, 1.0f);
        
        // Scale
        static float scale[3] = {1.0f, 1.0f, 1.0f};
        ImGui::DragFloat3("Scale##transform", scale, 0.1f, 0.1f);
        
        ImGui::Spacing();
    } else {
        ImGui::PopStyleColor();
    }
}

void ModernPropertiesPanel::DrawMaterialSection() {
    bool& expanded = m_SectionExpanded["Material"];
    
    ImGui::PushStyleColor(ImGuiCol_Header, UIStyleHelper::Colors::Emerald500);
    if (ImGui::CollapsingHeader("Material", &expanded)) {
        ImGui::PopStyleColor();
        
        ImGui::Spacing();
        
        // Material selector
        static int materialIdx = 0;
        const char* materials[] = {"Steel", "Aluminum", "Titanium", "Composite"};
        ImGui::Combo("Material Type##mat", &materialIdx, materials, IM_ARRAYSIZE(materials));
        
        // Properties
        static float density = 7850.0f;
        ImGui::DragFloat("Density (kg/m³)##mat", &density, 10.0f);
        
        static float youngModulus = 200e9f;
        ImGui::DragFloat("Young's Modulus##mat", &youngModulus, 1e9f);
        
        static float poisson = 0.3f;
        ImGui::SliderFloat("Poisson's Ratio##mat", &poisson, 0.0f, 0.5f);
        
        ImGui::Spacing();
    } else {
        ImGui::PopStyleColor();
    }
}

void ModernPropertiesPanel::DrawPhysicsSection() {
    bool& expanded = m_SectionExpanded["Physics"];
    
    ImGui::PushStyleColor(ImGuiCol_Header, UIStyleHelper::Colors::Blue500);
    if (ImGui::CollapsingHeader("Physics", &expanded)) {
        ImGui::PopStyleColor();
        
        ImGui::Spacing();
        
        static bool isRigid = true;
        ImGui::Checkbox("Rigid Body##phys", &isRigid);
        
        static float mass = 1.0f;
        ImGui::DragFloat("Mass (kg)##phys", &mass, 0.1f, 0.1f);
        
        static bool useGravity = true;
        ImGui::Checkbox("Use Gravity##phys", &useGravity);
        
        static float friction = 0.5f;
        ImGui::SliderFloat("Friction##phys", &friction, 0.0f, 1.0f);
        
        ImGui::Spacing();
    } else {
        ImGui::PopStyleColor();
    }
}

void ModernPropertiesPanel::DrawAdvancedSection() {
    bool& expanded = m_SectionExpanded["Advanced"];
    
    ImGui::PushStyleColor(ImGuiCol_Header, UIStyleHelper::Colors::Amber500);
    if (ImGui::CollapsingHeader("Advanced", &expanded)) {
        ImGui::PopStyleColor();
        
        ImGui::Spacing();
        
        static bool enableFEA = false;
        ImGui::Checkbox("Enable FEA Analysis##adv", &enableFEA);
        
        static bool enableThermal = false;
        ImGui::Checkbox("Enable Thermal Analysis##adv", &enableThermal);
        
        static int meshQuality = 2;
        ImGui::SliderInt("Mesh Quality##adv", &meshQuality, 1, 5);
        
        ImGui::Spacing();
    } else {
        ImGui::PopStyleColor();
    }
}

} // namespace SZM
