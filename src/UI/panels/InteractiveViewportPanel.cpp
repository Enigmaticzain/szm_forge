#include "InteractiveViewportPanel.hpp"
#include <imgui.h>

namespace SZM::UI {

InteractiveViewportPanel::InteractiveViewportPanel()
    : m_InteractiveRenderer(Graphics::InteractiveRenderer::GetInstance()) {
    m_InteractiveRenderer.Init();
}

InteractiveViewportPanel::~InteractiveViewportPanel() {
    m_InteractiveRenderer.Shutdown();
}

void InteractiveViewportPanel::Draw() {
    if (!ImGui::Begin(GetName(), &m_IsOpen, ImGuiWindowFlags_NoMove)) {
        ImGui::End();
        return;
    }

    ImVec2 viewportSize = ImGui::GetContentRegionAvail();
    if (viewportSize.x > 0 && viewportSize.y > 0) {
        m_Viewport->Resize(static_cast<uint32_t>(viewportSize.x), static_cast<uint32_t>(viewportSize.y));
        ImGui::Image(
            static_cast<ImTextureID>(m_Viewport->GetTextureID()),
            viewportSize,
            ImVec2(0, 1),
            ImVec2(1, 0)
        );

        m_IsMouseOver = ImGui::IsItemHovered();
        HandleMouseInput();
    }

    ImGui::Separator();
    DrawSelectionInfo();
    ImGui::Separator();
    DrawVisualizationControls();

    ImGui::End();
}

void InteractiveViewportPanel::HandleMouseInput() {
    if (!m_IsMouseOver) return;

    ImVec2 mousePos = ImGui::GetMousePos();
    ImVec2 viewportPos = ImGui::GetItemRectMin();

    float screenX = mousePos.x - viewportPos.x;
    float screenY = mousePos.y - viewportPos.y;

    // Handle mouse move for hover detection
    m_InteractiveRenderer.HandleMouseMove(
        screenX, screenY,
        m_Viewport->GetWidth(),
        m_Viewport->GetHeight(),
        m_Viewport->GetCamera()
    );

    // Handle mouse click
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

    m_LastMouseX = screenX;
    m_LastMouseY = screenY;
}

void InteractiveViewportPanel::DrawSelectionInfo() {
    auto& selectionMgr = m_InteractiveRenderer.GetSelectionManager();
    size_t selectionCount = selectionMgr.GetSelectionCount();

    ImGui::Text("Selection: %zu component(s) selected", selectionCount);

    if (selectionCount > 0) {
        ImGui::SameLine();
        if (ImGui::Button("Clear Selection")) {
            selectionMgr.ClearSelection();
        }

        ImGui::Text("Selected IDs:");
        ImGui::Indent();
        for (uint32_t id : selectionMgr.GetSelectedIds()) {
            ImGui::BulletText("Component %u", id);
        }
        ImGui::Unindent();
    }

    auto hoveredId = m_InteractiveRenderer.GetHighlightingSystem().GetHovered();
    if (hoveredId > 0) {
        ImGui::Text("Hovered: Component %u", hoveredId);
    }
}

} // namespace SZM::UI
