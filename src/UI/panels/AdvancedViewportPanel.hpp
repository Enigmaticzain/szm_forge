#pragma once

#include "InteractiveViewportPanel.hpp"
#include "../Graphics/Advanced/ViewportCameraController.hpp"
#include "../Graphics/Advanced/ViewportGizmo.hpp"
#include "../Graphics/Advanced/ViewportAnnotationSystem.hpp"

namespace SZM::UI {

/**
 * @class AdvancedViewportPanel
 * @brief Full-featured viewport with camera, gizmos, and annotations
 */
class AdvancedViewportPanel : public InteractiveViewportPanel {
public:
    AdvancedViewportPanel();
    ~AdvancedViewportPanel() override;

    void Draw() override;
    const char* GetName() const override { return "Advanced Viewport"; }

private:
    Graphics::ViewportCameraController m_CameraController;
    Graphics::ViewportGizmo m_Gizmo;
    Graphics::ViewportAnnotationSystem& m_AnnotationSystem;

    bool m_ShowGizmo = true;
    bool m_ShowAnnotations = true;
    Graphics::GizmoMode m_CurrentGizmoMode = Graphics::GizmoMode::Translate;

    void DrawCameraControls();
    void DrawGizmoControls();
    void DrawAnnotationControls();
    void HandleViewportInput();
};

} // namespace SZM::UI
