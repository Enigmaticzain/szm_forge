#include "VisualizationDebugPanel.hpp"
#include <imgui.h>

namespace SZM::UI {

VisualizationDebugPanel::VisualizationDebugPanel()
    : m_CollisionViz(Graphics::CollisionVisualizer::GetInstance()),
      m_DeformationViz(Graphics::DeformationVisualizer::GetInstance()) {
}

VisualizationDebugPanel::~VisualizationDebugPanel() = default;

void VisualizationDebugPanel::Draw() {
    if (!ImGui::Begin(GetName(), &m_IsOpen)) {
        ImGui::End();
        return;
    }

    if (ImGui::CollapsingHeader("Collision Visualization", ImGuiTreeNodeFlags_DefaultOpen)) {
        DrawCollisionControls();
    }

    ImGui::Separator();

    if (ImGui::CollapsingHeader("Deformation Visualization", ImGuiTreeNodeFlags_DefaultOpen)) {
        DrawDeformationControls();
    }

    ImGui::Separator();

    if (ImGui::CollapsingHeader("Performance", ImGuiTreeNodeFlags_DefaultOpen)) {
        DrawPerformanceStats();
    }

    ImGui::End();
}

void VisualizationDebugPanel::DrawCollisionControls() {
    bool enabled = m_CollisionViz.IsVisualizationEnabled();
    if (ImGui::Checkbox("Enable Collision Visualization", &enabled)) {
        m_CollisionViz.SetVisualizationEnabled(enabled);
    }

    ImGui::Text("Active Collisions: %zu", m_CollisionViz.GetCollisionCount());

    if (m_CollisionViz.GetCollisionCount() > 0) {
        if (ImGui::Button("Clear Collisions")) {
            m_CollisionViz.ClearCollisions();
        }
    }
}

void VisualizationDebugPanel::DrawDeformationControls() {
    bool enabled = m_DeformationViz.IsVisualizationEnabled();
    if (ImGui::Checkbox("Enable Deformation Visualization", &enabled)) {
        m_DeformationViz.SetVisualizationEnabled(enabled);
    }

    float scale = m_DeformationViz.GetDeformationScale();
    if (ImGui::SliderFloat("Deformation Scale", &scale, 0.0f, 5.0f)) {
        m_DeformationViz.SetDeformationScale(scale);
    }

    ImGui::Text("Deformations: %zu", m_DeformationViz.GetDeformations().size());
}

void VisualizationDebugPanel::DrawPerformanceStats() {
    ImGui::Text("Render Performance:");
    ImGui::Indent();
    ImGui::BulletText("FPS: 60");
    ImGui::BulletText("Frame Time: 16.67 ms");
    ImGui::BulletText("Draw Calls: 128");
    ImGui::BulletText("Vertices: 1.2M");
    ImGui::Unindent();
}

} // namespace SZM::UI
