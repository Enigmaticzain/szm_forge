#pragma once

#include "UI/IPanel.hpp"
#include "WorkshopManager.hpp"
#include <cstdint>
#include <imgui.h>
#include <string>

namespace SZM::Workshop3D {

    class PartLibraryPanel : public IPanel {
    public:
        PartLibraryPanel();
        void Draw() override;
        std::string GetTitle() const override { return "Workshop Library"; }

    private:
        void DrawToolbar();
        void DrawPartList();
        void DrawSelectedPartCard();

        char m_SearchFilter[128] = "";
        char m_ImportPath[256] = "";
    };

    class PartEditorPanel : public IPanel {
    public:
        PartEditorPanel();
        void Draw() override;
        std::string GetTitle() const override { return "Part Editor"; }

    private:
        void DrawPrimitiveCreator();
        void DrawSelectedPartDetails();

        char m_PartName[96] = "Workshop Block";
        char m_MaterialName[96] = "Steel";
        char m_Description[192] = "Simple workshop placeholder component";
        float m_Size[3] = { 0.5f, 0.18f, 0.24f };
        bool m_AddToAssemblyAfterCreate = true;
    };

    class AssemblyPanel : public IPanel {
    public:
        AssemblyPanel();
        void Draw() override;
        std::string GetTitle() const override { return "Workshop Assembly"; }

    private:
        void DrawAssemblySelector();
        void DrawPlacementTools();
        void DrawInstanceList();
        void DrawSelectedInstanceEditor();

        char m_AssemblyName[96] = "New Assembly";
        char m_InstanceName[96] = "";
        float m_Placement[3] = { 0.0f, 0.0f, 0.0f };
        float m_Rotation[3] = { 0.0f, 0.0f, 0.0f };
        float m_Scale[3] = { 1.0f, 1.0f, 1.0f };
        bool m_AutoPlacement = true;
    };

    class WorkshopViewportPanel : public IPanel {
    public:
        WorkshopViewportPanel();
        void Draw() override;
        std::string GetTitle() const override { return "Workshop Viewport"; }

    private:
        enum class InteractionMode {
            None,
            OrbitCamera,
            PanCamera,
            DragInstance,
            ResizeInstance
        };

        void DrawViewport();
        void HandleViewportKeyboard(const ImVec2& canvasPos, const ImVec2& canvasSize);

        float m_CameraYaw = 0.75f;
        float m_CameraPitch = 0.45f;
        float m_Zoom = 220.0f;
        Geometry::Vector3 m_CameraTarget = Geometry::Vector3(0.0, 0.0, 0.0);
        bool m_ShowGrid = true;
        bool m_ShowLabels = true;
        bool m_ShowControlsHint = true;
        bool m_AutoRotate = false;
        InteractionMode m_InteractionMode = InteractionMode::None;
        uint32_t m_InteractionInstanceId = 0U;
        uint32_t m_HoveredInstanceId = 0U;
        int m_HoveredHandleIndex = -1;
        ImVec2 m_LastMousePos = ImVec2(0.0f, 0.0f);
        Geometry::Vector3 m_DragStartPosition = Geometry::Vector3(0.0, 0.0, 0.0);
        Geometry::Vector3 m_DragStartScale = Geometry::Vector3(1.0, 1.0, 1.0);
        float m_DragStartHandleDistance = 1.0f;
    };

    class SimulationSetupPanel : public IPanel {
    public:
        SimulationSetupPanel();
        void Draw() override;
        std::string GetTitle() const override { return "Workshop Simulation"; }

    private:
        void DrawSimulationSetup();
        void DrawSelectedInstanceLoads();
        void DrawSimulationResults();

        float m_SimulationDuration = 2.0f;
        float m_TimeStep = 1.0f / 60.0f;
        float m_AppliedForce = 1500.0f;
        float m_HeatInput = 250.0f;
        int m_VisualizationMode = 0;
    };

} // namespace SZM::Workshop3D
