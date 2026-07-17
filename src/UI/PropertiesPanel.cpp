#include "PropertiesPanel.hpp"
#include <imgui.h>

namespace SZM {

    PropertiesPanel::PropertiesPanel() {
        m_SelectedObject = "None";
    }

    void PropertiesPanel::SetSelectedObject(const std::string& objectName) {
        m_SelectedObject = objectName;
    }

    void PropertiesPanel::Draw() {
        ImGui::SetNextWindowSize(ImVec2(300, 400), ImGuiCond_FirstUseEver);
        
        if (!ImGui::Begin(GetTitle().c_str(), &IsVisible)) {
            ImGui::End();
            return;
        }

        ImGui::Text("Selected: %s", m_SelectedObject.c_str());
        ImGui::Separator();

        if (m_SelectedObject != "None") {
            // Transform section
            if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::DragFloat3("Position##pos", m_Position, 0.01f);
                ImGui::DragFloat3("Rotation##rot", m_Rotation, 0.01f);
                ImGui::DragFloat3("Scale##scl", m_Scale, 0.01f);
            }

            // Material section
            if (ImGui::CollapsingHeader("Material", ImGuiTreeNodeFlags_DefaultOpen)) {
                static int material_type = 0;
                ImGui::Combo("Material Type", &material_type, "Steel\0Aluminum\0Titanium\0");
                
                static float roughness = 0.5f;
                static float metallic = 1.0f;
                ImGui::SliderFloat("Roughness", &roughness, 0.0f, 1.0f);
                ImGui::SliderFloat("Metallic", &metallic, 0.0f, 1.0f);
            }

            // Physics section
            if (ImGui::CollapsingHeader("Physics", ImGuiTreeNodeFlags_DefaultOpen)) {
                static float mass = 1.0f;
                ImGui::DragFloat("Mass", &mass, 0.1f, 0.1f, 1000.0f);
                
                static bool is_dynamic = true;
                ImGui::Checkbox("Dynamic", &is_dynamic);
            }
        } else {
            ImGui::Text("Select an object to view properties");
        }

        ImGui::End();
    }

} // namespace SZM
