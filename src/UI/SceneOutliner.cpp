#include "SceneOutliner.hpp"
#include <imgui.h>

namespace SZM {

    SceneOutliner::SceneOutliner() {
        m_RootNodes = {"Engine_Assembly_01", "Chassis_Frame", "Sensor_Array"};
    }

    void SceneOutliner::AddNode(const std::string& nodeName) {
        m_RootNodes.push_back(nodeName);
    }

    void SceneOutliner::Clear() {
        m_RootNodes.clear();
    }

    void SceneOutliner::Draw() {
        ImGui::SetNextWindowSize(ImVec2(300, 400), ImGuiCond_FirstUseEver);
        
        if (!ImGui::Begin(GetTitle().c_str(), &IsVisible)) {
            ImGui::End();
            return;
        }

        ImGui::BeginChild("SceneTree", ImVec2(0, 0), false);

        for (const auto& node : m_RootNodes) {
            ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
            
            if (ImGui::TreeNodeEx(node.c_str(), flags)) {
                // Render child nodes
                ImGui::Indent();
                ImGui::Text("Child Node 1");
                ImGui::Text("Child Node 2");
                ImGui::Unindent();
                
                ImGui::TreePop();
            }
        }

        ImGui::EndChild();
        ImGui::End();
    }

} // namespace SZM
