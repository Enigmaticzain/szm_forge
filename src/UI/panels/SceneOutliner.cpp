#include "SceneOutliner.hpp"
#include "../../Input/EventBus.hpp"
#include "../../Simulation/SimulationEngine.hpp"
#include <imgui.h>

SceneOutliner::SceneOutliner() {
    // We now fetch data live from the SimulationEngine.
}

void SceneOutliner::RefreshNodes(const std::vector<std::string>& rootNodeNames) {
    m_RootNodes = rootNodeNames;
    if (m_RootNodes.empty()) {
        m_SelectedNode.clear();
    } else if (m_SelectedNode.empty()) {
        m_SelectedNode = m_RootNodes.front();
    }
}

void SceneOutliner::Draw() {
    if (!ImGui::Begin(GetTitle().c_str(), &IsVisible)) {
        ImGui::End();
        return;
    }

    auto* scene = SZM::SimulationEngine::GetInstance().GetScene();
    
    if (!scene) {
        // Fallback to legacy
        const auto& components = SZM::SimulationEngine::GetInstance().GetComponents();
        if (components.empty()) {
            ImGui::TextDisabled("No components loaded.");
            ImGui::End();
            return;
        }

        for (const auto& comp : components) {
            if (!comp) continue;
            std::string label = comp->name + " (ID: " + std::to_string(comp->id) + ")";
            const bool isSelected = (m_SelectedNode == std::to_string(comp->id));
            
            if (ImGui::Selectable(label.c_str(), isSelected)) {
                m_SelectedNode = std::to_string(comp->id);
                SZM::EventBus::GetInstance().Publish("SelectionChanged", m_SelectedNode);
            }
        }
    } else {
        // ECS Path
        auto entities = scene->View<SZM::SceneGraph::TransformComponent>();
        if (entities.empty()) {
            ImGui::TextDisabled("Empty Scene Graph.");
        } else {
            // Find root entities
            std::vector<SZM::SceneGraph::Entity> rootEntities;
            for (auto e : entities) {
                if (scene->GetComponent<SZM::SceneGraph::TransformComponent>(e).parentEntity == SZM::SceneGraph::NullEntity) {
                    rootEntities.push_back(e);
                }
            }

            // Recursive lambda to draw tree
            auto drawEntity = [&](auto& self, SZM::SceneGraph::Entity e) -> void {
                std::string name = "Entity_" + std::to_string(static_cast<uint32_t>(e));
                if (scene->HasComponent<SZM::SceneGraph::TagComponent>(e)) {
                    name = scene->GetComponent<SZM::SceneGraph::TagComponent>(e).name;
                }

                // Check for children
                std::vector<SZM::SceneGraph::Entity> children;
                for (auto potentialChild : entities) {
                    if (scene->GetComponent<SZM::SceneGraph::TransformComponent>(potentialChild).parentEntity == e) {
                        children.push_back(potentialChild);
                    }
                }

                ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
                if (children.empty()) flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
                if (m_SelectedNode == std::to_string(static_cast<uint32_t>(e))) flags |= ImGuiTreeNodeFlags_Selected;

                bool nodeOpen = ImGui::TreeNodeEx((void*)(intptr_t)e, flags, "%s", name.c_str());
                if (ImGui::IsItemClicked()) {
                    m_SelectedNode = std::to_string(static_cast<uint32_t>(e));
                    SZM::EventBus::GetInstance().Publish("SelectionChanged", m_SelectedNode);
                }

                if (nodeOpen && !children.empty()) {
                    for (auto child : children) {
                        self(self, child);
                    }
                    ImGui::TreePop();
                }
            };

            for (auto root : rootEntities) {
                drawEntity(drawEntity, root);
            }
        }
    }

    ImGui::End();
}
