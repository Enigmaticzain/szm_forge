#include "AdvancedViewportPanel.hpp"
#include <imgui.h>

namespace SZM::UI {

AdvancedViewportPanel::AdvancedViewportPanel()
    : m_AnnotationSystem(Graphics::ViewportAnnotationSystem::GetInstance()) {
}

AdvancedViewportPanel::~AdvancedViewportPanel() = default;

void AdvancedViewportPanel::Draw() {
    if (!ImGui::Begin(GetName(), &m_IsOpen, ImGuiWindowFlags_NoMove)) {
        ImGui::End();
        return;
    }

    ImVec2 viewportSize = ImGui::GetContentRegionAvail();
    if (viewportSize.x > 0 && viewportSize.y > 0) {
        m_Viewport->Resize(static_cast<uint32_t>(viewportSize.x), static_cast<uint32_t>(viewportSize.y));
        m_CameraController.SetViewportSize(static_cast<uint32_t>(viewportSize.x), static_cast<uint32_t>(viewportSize.y));

        ImGui::Image(
            static_cast<ImTextureID>(m_Viewport->GetTextureID()),
            viewportSize,
            ImVec2(0, 1),
            ImVec2(1, 0)
        );

        m_IsMouseOver = ImGui::IsItemHovered();
        HandleViewportInput();
    }

    ImGui::Separator();

    if (ImGui::BeginTabBar("##ViewportTabs")) {
        if (ImGui::BeginTabItem("Camera")) {
            DrawCameraControls();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Gizmo")) {
            DrawGizmoControls();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Annotations")) {
            DrawAnnotationControls();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Visualization")) {
            DrawVisualizationControls();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}

void AdvancedViewportPanel::DrawCameraControls() {
    ImGui::Text("Camera Controls");
    ImGui::Separator();

    if (ImGui::Button("Reset View")) {
        m_CameraController.ResetView();
    }

    if (ImGui::Button("Frame All")) {
        m_CameraController.FrameAll(glm::vec3(0.0f), 5.0f);
    }

    ImGui::SameLine();
    if (ImGui::Button("Frame Selection")) {
        m_CameraController.FrameSelection(glm::vec3(0.0f), 1.0f);
    }

    float speed = m_CameraController.GetCameraSpeed();
    if (ImGui::SliderFloat("Camera Speed", &speed, 0.001f, 0.1f)) {
        m_CameraController.SetCameraSpeed(speed);
    }

    float zoomSpeed = m_CameraController.GetZoomSpeed();
    if (ImGui::SliderFloat("Zoom Speed", &zoomSpeed, 0.01f, 1.0f)) {
        m_CameraController.SetZoomSpeed(zoomSpeed);
    }

    ImGui::Text("Camera Position: (%.2f, %.2f, %.2f)",
        m_CameraController.GetPosition().x,
        m_CameraController.GetPosition().y,
        m_CameraController.GetPosition().z);
}

void AdvancedViewportPanel::DrawGizmoControls() {
    ImGui::Text("Transform Gizmo");
    ImGui::Separator();

    ImGui::Checkbox("Show Gizmo", &m_ShowGizmo);

    const char* modes[] = {"Translate", "Rotate", "Scale", "None"};
    int currentMode = static_cast<int>(m_CurrentGizmoMode);

    if (ImGui::Combo("Gizmo Mode", &currentMode, modes, IM_ARRAYSIZE(modes))) {
        m_CurrentGizmoMode = static_cast<Graphics::GizmoMode>(currentMode);
        m_Gizmo.SetMode(m_CurrentGizmoMode);
    }

    auto pos = m_Gizmo.GetPosition();
    if (ImGui::InputFloat3("Position##Gizmo", &pos.x)) {
        m_Gizmo.SetPosition(pos);
    }

    auto rot = m_Gizmo.GetRotation();
    if (ImGui::InputFloat3("Rotation##Gizmo", &rot.x)) {
        m_Gizmo.SetRotation(rot);
    }

    auto scale = m_Gizmo.GetScale();
    if (ImGui::InputFloat3("Scale##Gizmo", &scale.x)) {
        m_Gizmo.SetScale(scale);
    }
}

void AdvancedViewportPanel::DrawAnnotationControls() {
    ImGui::Text("Viewport Annotations");
    ImGui::Separator();

    ImGui::Checkbox("Show Annotations", &m_ShowAnnotations);

    if (ImGui::Button("Add Label")) {
        m_AnnotationSystem.AddLabel(glm::vec3(0.0f), "Label");
    }

    ImGui::SameLine();
    if (ImGui::Button("Add Measurement")) {
        m_AnnotationSystem.AddMeasurement(glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), "Distance");
    }

    ImGui::SameLine();
    if (ImGui::Button("Clear All")) {
        m_AnnotationSystem.ClearAnnotations();
    }

    ImGui::Text("Annotations: %zu", m_AnnotationSystem.GetAnnotations().size());
}

void AdvancedViewportPanel::HandleViewportInput() {
    if (!m_IsMouseOver) return;

    ImVec2 mousePos = ImGui::GetMousePos();
    ImVec2 viewportPos = ImGui::GetItemRectMin();

    float screenX = mousePos.x - viewportPos.x;
    float screenY = mousePos.y - viewportPos.y;

    // Handle mouse move
    m_InteractiveRenderer.HandleMouseMove(
        screenX, screenY,
        m_Viewport->GetWidth(),
        m_Viewport->GetHeight(),
        m_Viewport->GetCamera()
    );

    // Handle camera controls
    if (ImGui::IsMouseDragging(ImGuiMouseButton_Middle)) {
        ImVec2 delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Middle);
        m_CameraController.HandleMouseDrag(delta.x, delta.y, Graphics::CameraMode::Orbit);
    }

    if (ImGui::IsMouseDragging(ImGuiMouseButton_Right)) {
        ImVec2 delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right);
        m_CameraController.HandleMouseDrag(delta.x, delta.y, Graphics::CameraMode::Pan);
    }

    // Handle scroll
    if (ImGui::IsItemHovered()) {
        float scroll = ImGui::GetIO().MouseWheel;
        if (scroll != 0.0f) {
            m_CameraController.HandleMouseScroll(scroll);
        }
    }

    // Handle selection
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        bool multiSelect = ImGui::GetIO().KeyCtrl;
        m_InteractiveRenderer.HandleMouseClick(
            screenX, screenY,
            m_Viewport->GetWidth(),
            m_Viewport->GetHeight(),
            m_Viewport->GetCamera(),
            multiSelect
        );
    }
}

} // namespace SZM::UI
