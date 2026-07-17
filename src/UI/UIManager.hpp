#pragma once

#include <vector>
#include <string>
#include <memory>
#include "IPanel.hpp"

namespace SZM {

    enum class WorkspacePreset {
        Overview,
        Build,
        Analyze,
        Full,
        Custom
    };

    class UIManager {
    public:
        static UIManager& GetInstance();

        UIManager();
        ~UIManager();

        void Initialize();
        void RegisterPanel(std::shared_ptr<IPanel> panel);

        // Main render loop call
        void RenderUI();

        // Forces a specific layout if no save file is found
        void BuildDefaultDockspace(unsigned int dockspaceId);

        void ApplyWorkspacePreset(WorkspacePreset preset);

    private:
        void RenderCommandDeck(bool& resetLayoutRequested, bool& showAllPanelsRequested);
        void RenderSidebar(bool& resetLayoutRequested, bool& showAllPanelsRequested);
        void RenderWorkspaceTab();
        void RenderPanelsTab();
        void RenderLayoutTab(bool& resetLayoutRequested, bool& showAllPanelsRequested);
        void DrawPanelVisibilityButton(const std::shared_ptr<IPanel>& panel, float width);
        void DrawSidebarPanelEntry(const std::shared_ptr<IPanel>& panel);
        void RevealAndFocusPanel(const std::shared_ptr<IPanel>& panel);

        static UIManager* s_Instance;
        std::vector<std::shared_ptr<IPanel>> m_Panels;
        bool m_FirstTimeDocking = true;
        WorkspacePreset m_ActiveWorkspace = WorkspacePreset::Build;
        std::string m_SelectedPanelTitle;
        std::string m_PendingFocusPanelTitle;
    };

} // namespace SZM
