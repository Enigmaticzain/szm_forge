#include "UIManager.hpp"
#include "UIStyleHelper.hpp"
#include "UITooltipHelper.hpp"
#include "WorkshopTooltips.hpp"
#include "../Simulation/SimulationEngine.hpp"
#include <imgui.h>
#include <imgui_internal.h>
#include <array>
#include <algorithm>
#include <cstdio>
#include <GLFW/glfw3.h>

namespace {

constexpr std::array<const char*, 6> kOverviewPanels = {
    "Toolbar",
    "Dashboard",
    "Scene Outliner",
    "Properties Inspector",
    "Simulation Visualizer",
    "SZM-CEA Console"
};

constexpr std::array<const char*, 5> kCorePanels = {
    "Toolbar",
    "Dashboard",
    "Scene Outliner",
    "Properties Inspector",
    "SZM-CEA Console"
};

constexpr std::array<const char*, 7> kBuildPanels = {
    "Toolbar",
    "Component Browser",
    "Workshop Library",
    "Workshop Assembly",
    "Part Editor",
    "Workshop Viewport",
    "Properties Inspector"
};

constexpr std::array<const char*, 5> kWorkshopPanels = {
    "Component Browser",
    "Workshop Library",
    "Workshop Assembly",
    "Part Editor",
    "Workshop Viewport"
};

constexpr std::array<const char*, 7> kAnalyzePanels = {
    "Toolbar",
    "Simulation Control",
    "Workshop Simulation",
    "Simulation Visualizer",
    "Component Weight Table",
    "Properties Inspector",
    "SZM-CEA Console"
};

constexpr std::array<const char*, 5> kAnalysisPanels = {
    "Simulation Control",
    "Workshop Simulation",
    "Simulation Visualizer",
    "Component Weight Table",
    "Component Builder"
};

size_t CountDangerousComponents(
    const std::vector<std::unique_ptr<SZM::SimulationComponent>>& components
) {
    return static_cast<size_t>(std::count_if(
        components.begin(),
        components.end(),
        [](const auto& component) {
            return component && component->isDangerous;
        }
    ));
}

float AverageTemperatureRatio(
    const std::vector<std::unique_ptr<SZM::SimulationComponent>>& components
) {
    if (components.empty()) {
        return 0.0f;
    }

    float total = 0.0f;
    for (const auto& component : components) {
        if (component) {
            total += component->tempRatio;
        }
    }

    return total / static_cast<float>(components.size());
}

void RenderSummaryMetric(const char* label, const char* value) {
    ImGui::BeginGroup();
    ImGui::TextDisabled("%s", label);
    ImGui::Text("%s", value);
    ImGui::EndGroup();
}

template <size_t N>
bool TitleInList(const std::string& title, const std::array<const char*, N>& candidates) {
    return std::find_if(
        candidates.begin(),
        candidates.end(),
        [&title](const char* candidate) {
            return title == candidate;
        }
    ) != candidates.end();
}

const char* WorkspaceLabel(SZM::WorkspacePreset preset) {
    switch (preset) {
        case SZM::WorkspacePreset::Overview:
            return "Overview";
        case SZM::WorkspacePreset::Build:
            return "Build";
        case SZM::WorkspacePreset::Analyze:
            return "Analyze";
        case SZM::WorkspacePreset::Full:
            return "Full Desktop";
        case SZM::WorkspacePreset::Custom:
            return "Custom";
    }

    return "Build";
}

} // namespace

namespace SZM {

    UIManager* UIManager::s_Instance = nullptr;

    UIManager& UIManager::GetInstance() {
        if (s_Instance != nullptr) {
            return *s_Instance;
        }

        static UIManager instance;
        return instance;
    }

    UIManager::UIManager() {
        s_Instance = this;
    }

    UIManager::~UIManager() {
        if (s_Instance == this) {
            s_Instance = nullptr;
        }
    }

    void UIManager::Initialize() {
        // ImGui context setup is assumed to be handled in Module 010.1
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;   // Enable Docking
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Monitor Tear-off

        // Apply modern theme
        UIStyleHelper::ApplyTheme(UIStyleHelper::ThemeMode::Light);
    }

    void UIManager::RegisterPanel(std::shared_ptr<IPanel> panel) {
        m_Panels.push_back(panel);
    }

    void UIManager::RenderUI() {
        // 1. Create the Central Dockspace spanning the OS window
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

        ImGui::Begin("SZM_Forge_DockSpace", nullptr, window_flags);
        ImGui::PopStyleVar(3);

        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        bool resetLayoutRequested = false;
        bool showAllPanelsRequested = false;

        // 3. Menu Bar
        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Export Scene to USD (Omniverse)")) {
                    WebAPI::APIManager::GetInstance().GetExportAPI().ExportSceneUSD();
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Exit")) {
                    if (GLFWwindow* window = glfwGetCurrentContext()) {
                        glfwSetWindowShouldClose(window, GLFW_TRUE);
                    }
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Layout")) {
                if (ImGui::MenuItem("Reset Layout")) {
                    resetLayoutRequested = true;
                }
                if (ImGui::MenuItem("Show All Panels")) {
                    showAllPanelsRequested = true;
                    m_ActiveWorkspace = WorkspacePreset::Full;
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Panels")) {
                for (auto& panel : m_Panels) {
                    ImGui::MenuItem(panel->GetTitle().c_str(), nullptr, &panel->IsVisible);
                }
                ImGui::EndMenu();
            }

            const auto& components = SimulationEngine::GetInstance().GetComponents();
            const size_t componentCount = components.size();
            const size_t dangerCount = CountDangerousComponents(components);

            char componentBuffer[64];
            char dangerBuffer[64];
            std::snprintf(componentBuffer, sizeof(componentBuffer), "Components %zu", componentCount);
            std::snprintf(dangerBuffer, sizeof(dangerBuffer), "At risk %zu", dangerCount);

            const float rightAnchor = ImGui::GetWindowWidth() - 300.0f;
            if (rightAnchor > ImGui::GetCursorPosX()) {
                ImGui::SetCursorPosX(rightAnchor);
            }
            ImGui::TextDisabled("Forge Desktop");
            ImGui::SameLine();
            UIStyleHelper::StatusBadge(dangerCount > 0 ? "warning" : "online");
            ImGui::SameLine();
            ImGui::TextDisabled("%s", componentBuffer);
            ImGui::SameLine();
            ImGui::TextDisabled("%s", dangerBuffer);
            ImGui::EndMenuBar();
        }

        RenderCommandDeck(resetLayoutRequested, showAllPanelsRequested);
        ImGui::Spacing();

        const float sidebarWidth = std::clamp(ImGui::GetContentRegionAvail().x * 0.22f, 240.0f, 320.0f);

        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::ColorConvertU32ToFloat4(UIStyleHelper::GetSurfaceColor(0)));
        ImGui::PushStyleColor(ImGuiCol_Border, ImGui::ColorConvertU32ToFloat4(UIStyleHelper::GetBorderColor()));
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 14.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(14.0f, 14.0f));
        ImGui::BeginChild("ForgeSidebar", ImVec2(sidebarWidth, 0.0f), true);
        RenderSidebar(resetLayoutRequested, showAllPanelsRequested);
        ImGui::EndChild();
        ImGui::PopStyleVar(3);
        ImGui::PopStyleColor(2);

        ImGui::SameLine(0.0f, 12.0f);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::BeginChild("ForgeDockHost", ImVec2(0.0f, 0.0f), false);
        ImGui::PopStyleVar();

        if (showAllPanelsRequested) {
            for (auto& panel : m_Panels) {
                panel->IsVisible = true;
            }
        }

        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

        // 2. Initial Layout Construction
        if (m_FirstTimeDocking || resetLayoutRequested) {
            m_FirstTimeDocking = false;
            BuildDefaultDockspace(dockspace_id);
        }

        // 4. Render all registered panels
        for (auto& panel : m_Panels) {
            if (!panel || !panel->IsVisible) {
                continue;
            }

            if (!m_PendingFocusPanelTitle.empty() && panel->GetTitle() == m_PendingFocusPanelTitle) {
                ImGui::SetNextWindowFocus();
            }

            panel->Draw();

            if (!m_PendingFocusPanelTitle.empty() && panel->GetTitle() == m_PendingFocusPanelTitle) {
                m_PendingFocusPanelTitle.clear();
            }
        }

        ImGui::EndChild();

        ImGui::End(); // End SZM_Forge_DockSpace
    }

    void UIManager::RenderCommandDeck(bool& resetLayoutRequested, bool& showAllPanelsRequested) {
        const auto& components = SimulationEngine::GetInstance().GetComponents();
        const size_t componentCount = components.size();
        const size_t dangerCount = CountDangerousComponents(components);
        const float averageTemp = AverageTemperatureRatio(components);

        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::ColorConvertU32ToFloat4(UIStyleHelper::GetSurfaceColor(1)));
        ImGui::PushStyleColor(ImGuiCol_Border, ImGui::ColorConvertU32ToFloat4(UIStyleHelper::GetBorderColor()));
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 12.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(18.0f, 14.0f));

        ImGui::BeginChild("ForgeCommandDeck", ImVec2(0.0f, 168.0f), true, ImGuiWindowFlags_NoScrollbar);
        ImGui::Text("SZM Forge");
        ImGui::TextDisabled("Workspace presets, panel tabs, and quick layout tools.");

        const float summaryAnchor = ImGui::GetWindowWidth() - 340.0f;
        if (summaryAnchor > 260.0f) {
            ImGui::SameLine(summaryAnchor);
        } else {
            ImGui::NewLine();
        }

        char countValue[32];
        char riskValue[32];
        char thermalValue[32];
        std::snprintf(countValue, sizeof(countValue), "%zu", componentCount);
        std::snprintf(riskValue, sizeof(riskValue), "%zu", dangerCount);
        std::snprintf(thermalValue, sizeof(thermalValue), "%.0f%%", averageTemp * 100.0f);

        RenderSummaryMetric("Components", countValue);
        ImGui::SameLine();
        RenderSummaryMetric("At risk", riskValue);
        ImGui::SameLine();
        RenderSummaryMetric("Thermal", thermalValue);
        ImGui::SameLine();
        UIStyleHelper::StatusBadge(dangerCount > 0 ? "warning" : "online");
        ImGui::SameLine();
        ImGui::TextDisabled("%s", WorkspaceLabel(m_ActiveWorkspace));

        UIStyleHelper::Spacing();

        if (ImGui::BeginTabBar(
                "##ForgeCommandDeckTabs",
                ImGuiTabBarFlags_FittingPolicyResizeDown | ImGuiTabBarFlags_NoCloseWithMiddleMouseButton
            )) {
            if (ImGui::BeginTabItem("Workspaces")) {
                RenderWorkspaceTab();
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Panels")) {
                RenderPanelsTab();
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Layout")) {
                RenderLayoutTab(resetLayoutRequested, showAllPanelsRequested);
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        ImGui::EndChild();
        ImGui::PopStyleVar(3);
        ImGui::PopStyleColor(2);
    }

    void UIManager::RenderSidebar(bool& resetLayoutRequested, bool& showAllPanelsRequested) {
        const size_t visiblePanelCount = static_cast<size_t>(std::count_if(
            m_Panels.begin(),
            m_Panels.end(),
            [](const std::shared_ptr<IPanel>& panel) {
                return panel && panel->IsVisible;
            }
        ));

        ImGui::Text("All Tabs");
        ImGui::TextDisabled("%zu of %zu panels visible", visiblePanelCount, m_Panels.size());
        UIStyleHelper::Spacing();

        ImGui::TextDisabled("Workspaces");
        RenderWorkspaceTab();
        UIStyleHelper::Spacing();
        UIStyleHelper::HorizontalSeparator();
        UIStyleHelper::Spacing();

        ImGui::TextDisabled("Appearance");
        const UIStyleHelper::ThemeMode currentTheme = UIStyleHelper::GetThemeMode();
        const float availableWidth = std::max(ImGui::GetContentRegionAvail().x, 180.0f);
        const float themeButtonWidth = (availableWidth - 8.0f) * 0.5f;

        if (currentTheme == UIStyleHelper::ThemeMode::Light
                ? UIStyleHelper::ModernButtonPrimary("Light", ImVec2(themeButtonWidth, 0.0f))
                : UIStyleHelper::ModernButtonSecondary("Light", ImVec2(themeButtonWidth, 0.0f))) {
            UIStyleHelper::ApplyTheme(UIStyleHelper::ThemeMode::Light);
        }

        ImGui::SameLine();
        if (currentTheme == UIStyleHelper::ThemeMode::Dark
                ? UIStyleHelper::ModernButtonPrimary("Dark", ImVec2(themeButtonWidth, 0.0f))
                : UIStyleHelper::ModernButtonSecondary("Dark", ImVec2(themeButtonWidth, 0.0f))) {
            UIStyleHelper::ApplyTheme(UIStyleHelper::ThemeMode::Dark);
        }

        UIStyleHelper::Spacing();
        UIStyleHelper::HorizontalSeparator();
        UIStyleHelper::Spacing();

        const auto findPanel = [this](const char* title) -> std::shared_ptr<IPanel> {
            const auto it = std::find_if(
                m_Panels.begin(),
                m_Panels.end(),
                [title](const std::shared_ptr<IPanel>& panel) {
                    return panel && panel->GetTitle() == title;
                }
            );

            return it != m_Panels.end() ? *it : nullptr;
        };

        const auto drawGroup = [this, &findPanel](const char* heading, const auto& titles) {
            ImGui::TextDisabled("%s", heading);
            for (const char* title : titles) {
                if (const auto panel = findPanel(title)) {
                    DrawSidebarPanelEntry(panel);
                }
            }
            ImGui::Spacing();
        };

        drawGroup("Core", kCorePanels);
        drawGroup("Workshop", kWorkshopPanels);
        drawGroup("Analysis", kAnalysisPanels);

        if (ImGui::CollapsingHeader("codes/ integrations", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::TextUnformatted("Bullet physics");
            UITooltip::HelpMarker(WorkshopTooltips::CodesBullet);
            ImGui::TextUnformatted("Three.js web viewer");
            UITooltip::HelpMarker(WorkshopTooltips::CodesThreeJs);
            ImGui::TextUnformatted("EnTT entity system");
            UITooltip::HelpMarker(WorkshopTooltips::CodesEntt);
            ImGui::TextUnformatted("FastAPI simulation API");
            UITooltip::HelpMarker(WorkshopTooltips::CodesFastApi);
            ImGui::TextUnformatted("FEA / code8 meshes");
            UITooltip::HelpMarker(WorkshopTooltips::CodesFEA);
            ImGui::Spacing();
        }

        UIStyleHelper::HorizontalSeparator();
        UIStyleHelper::Spacing();

        const float fullWidth = std::max(ImGui::GetContentRegionAvail().x, 120.0f);
        if (UIStyleHelper::ModernButtonSecondary("Show All Panels", ImVec2(fullWidth, 0.0f))) {
            showAllPanelsRequested = true;
            m_ActiveWorkspace = WorkspacePreset::Full;
        }

        if (UIStyleHelper::ModernButtonSecondary("Reset Layout", ImVec2(fullWidth, 0.0f))) {
            resetLayoutRequested = true;
        }

        ImGui::TextDisabled("Click a tab name to open or focus that view.");
    }

    void UIManager::RenderWorkspaceTab() {
        struct WorkspaceOption {
            WorkspacePreset preset;
            const char* label;
            const char* description;
        };

        const std::array<WorkspaceOption, 4> options = {{
            {WorkspacePreset::Overview, "Overview", WorkshopTooltips::WorkspaceOverview},
            {WorkspacePreset::Build, "Build", WorkshopTooltips::WorkspaceBuild},
            {WorkspacePreset::Analyze, "Analyze", WorkshopTooltips::WorkspaceAnalyze},
            {WorkspacePreset::Full, "Full Desktop", WorkshopTooltips::WorkspaceFull}
        }};

        const float availableWidth = ImGui::GetContentRegionAvail().x;
        const float buttonWidth = std::clamp((availableWidth - 18.0f) * 0.25f, 116.0f, 168.0f);
        const float spacing = ImGui::GetStyle().ItemSpacing.x;
        float rowWidth = 0.0f;

        for (size_t index = 0; index < options.size(); ++index) {
            const WorkspaceOption& option = options[index];
            const bool isActive = m_ActiveWorkspace == option.preset;

            if (rowWidth > 0.0f && rowWidth + buttonWidth > availableWidth) {
                ImGui::NewLine();
                rowWidth = 0.0f;
            } else if (rowWidth > 0.0f) {
                ImGui::SameLine();
            }

            if (isActive
                    ? UIStyleHelper::ModernButtonPrimary(option.label, ImVec2(buttonWidth, 0.0f))
                    : UIStyleHelper::ModernButtonSecondary(option.label, ImVec2(buttonWidth, 0.0f))) {
                ApplyWorkspacePreset(option.preset);
            }

            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("%s", option.description);
            }
            rowWidth += buttonWidth + spacing;
        }

        ImGui::Spacing();
        ImGui::TextDisabled("Presets change panel visibility without throwing away your dock arrangement.");
    }

    void UIManager::RenderPanelsTab() {
        const auto findPanel = [this](const char* title) -> std::shared_ptr<IPanel> {
            const auto it = std::find_if(
                m_Panels.begin(),
                m_Panels.end(),
                [title](const std::shared_ptr<IPanel>& panel) {
                    return panel && panel->GetTitle() == title;
                }
            );

            return it != m_Panels.end() ? *it : nullptr;
        };

        const float availableWidth = ImGui::GetContentRegionAvail().x;
        const int columns = availableWidth > 720.0f ? 3 : 2;
        const float buttonWidth = std::max(150.0f, (availableWidth - (columns - 1) * 8.0f) / columns);

        const auto drawGroup = [this, &findPanel, columns, buttonWidth](const char* heading, const auto& titles) {
            ImGui::TextDisabled("%s", heading);

            int renderedCount = 0;
            for (const char* title : titles) {
                if (const auto panel = findPanel(title)) {
                    if (renderedCount > 0 && renderedCount % columns != 0) {
                        ImGui::SameLine();
                    }
                    DrawPanelVisibilityButton(panel, buttonWidth);
                    ++renderedCount;
                }
            }

            ImGui::Spacing();
        };

        drawGroup("Core", kCorePanels);
        drawGroup("Workshop", kWorkshopPanels);
        drawGroup("Analysis", kAnalysisPanels);
    }

    void UIManager::RenderLayoutTab(bool& resetLayoutRequested, bool& showAllPanelsRequested) {
        const size_t visiblePanelCount = static_cast<size_t>(std::count_if(
            m_Panels.begin(),
            m_Panels.end(),
            [](const std::shared_ptr<IPanel>& panel) {
                return panel && panel->IsVisible;
            }
        ));

        if (UIStyleHelper::ModernButtonSecondary("Reset Layout", ImVec2(140.0f, 0.0f))) {
            resetLayoutRequested = true;
        }

        ImGui::SameLine();

        if (UIStyleHelper::ModernButtonSecondary("Show All Panels", ImVec2(160.0f, 0.0f))) {
            showAllPanelsRequested = true;
            m_ActiveWorkspace = WorkspacePreset::Full;
        }

        ImGui::Spacing();
        ImGui::Text("Visible panels: %zu / %zu", visiblePanelCount, m_Panels.size());
        ImGui::TextDisabled("Workspace presets handle visibility; the dockspace still remembers where you place tabs.");
        ImGui::TextDisabled("Use the Panels tab for quick toggles when you want to keep the current preset mostly intact.");
    }

    void UIManager::DrawPanelVisibilityButton(const std::shared_ptr<IPanel>& panel, float width) {
        if (!panel) {
            return;
        }

        const std::string title = panel->GetTitle();
        if (panel->IsVisible
                ? UIStyleHelper::ModernButtonPrimary(title.c_str(), ImVec2(width, 0.0f))
                : UIStyleHelper::ModernButtonSecondary(title.c_str(), ImVec2(width, 0.0f))) {
            if (panel->IsVisible) {
                panel->IsVisible = false;
                if (m_SelectedPanelTitle == title) {
                    m_SelectedPanelTitle.clear();
                }
                if (m_PendingFocusPanelTitle == title) {
                    m_PendingFocusPanelTitle.clear();
                }
                m_ActiveWorkspace = WorkspacePreset::Custom;
            } else {
                RevealAndFocusPanel(panel);
            }
        }

        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("%s", panel->IsVisible ? "Visible" : "Hidden");
        }
    }

    void UIManager::DrawSidebarPanelEntry(const std::shared_ptr<IPanel>& panel) {
        if (!panel) {
            return;
        }

        const std::string title = panel->GetTitle();
        const bool isSelected = m_SelectedPanelTitle == title;
        const float toggleWidth = 58.0f;
        const float navWidth = std::max(120.0f, ImGui::GetContentRegionAvail().x - toggleWidth - 8.0f);

        ImGui::PushID(title.c_str());
        if (isSelected
                ? UIStyleHelper::ModernButtonPrimary(title.c_str(), ImVec2(navWidth, 0.0f))
                : UIStyleHelper::ModernButtonSecondary(title.c_str(), ImVec2(navWidth, 0.0f))) {
            RevealAndFocusPanel(panel);
        }

        ImGui::SameLine();
        if (panel->IsVisible) {
            if (UIStyleHelper::ModernButtonSecondary("Hide", ImVec2(toggleWidth, 0.0f))) {
                panel->IsVisible = false;
                if (m_SelectedPanelTitle == title) {
                    m_SelectedPanelTitle.clear();
                }
                if (m_PendingFocusPanelTitle == title) {
                    m_PendingFocusPanelTitle.clear();
                }
                m_ActiveWorkspace = WorkspacePreset::Custom;
            }
        } else {
            if (UIStyleHelper::ModernButton("Open", ImVec2(toggleWidth, 0.0f))) {
                RevealAndFocusPanel(panel);
            }
        }

        ImGui::PopID();
    }

    void UIManager::RevealAndFocusPanel(const std::shared_ptr<IPanel>& panel) {
        if (!panel) {
            return;
        }

        const bool wasVisible = panel->IsVisible;
        panel->IsVisible = true;

        const std::string title = panel->GetTitle();
        m_SelectedPanelTitle = title;
        m_PendingFocusPanelTitle = title;

        if (!wasVisible && m_ActiveWorkspace != WorkspacePreset::Full) {
            m_ActiveWorkspace = WorkspacePreset::Custom;
        }
    }

    void UIManager::ApplyWorkspacePreset(WorkspacePreset preset) {
        m_ActiveWorkspace = preset;

        for (auto& panel : m_Panels) {
            if (!panel) {
                continue;
            }

            if (preset == WorkspacePreset::Full) {
                panel->IsVisible = true;
                continue;
            }

            const std::string title = panel->GetTitle();
            if (preset == WorkspacePreset::Overview) {
                panel->IsVisible = TitleInList(title, kOverviewPanels);
                continue;
            }

            if (preset == WorkspacePreset::Build) {
                panel->IsVisible = TitleInList(title, kBuildPanels);
                continue;
            }

            panel->IsVisible = TitleInList(title, kAnalyzePanels);
        }

        const char* preferredTitle = nullptr;
        switch (preset) {
            case WorkspacePreset::Overview:
                preferredTitle = "Dashboard";
                break;
            case WorkspacePreset::Build:
                preferredTitle = "Workshop Viewport";
                break;
            case WorkspacePreset::Analyze:
                preferredTitle = "Simulation Visualizer";
                break;
            case WorkspacePreset::Full:
                preferredTitle = "Workshop Viewport";
                break;
            case WorkspacePreset::Custom:
                preferredTitle = nullptr;
                break;
        }

        m_SelectedPanelTitle.clear();
        if (preferredTitle != nullptr) {
            const auto it = std::find_if(
                m_Panels.begin(),
                m_Panels.end(),
                [preferredTitle](const std::shared_ptr<IPanel>& panel) {
                    return panel && panel->IsVisible && panel->GetTitle() == preferredTitle;
                }
            );
            if (it != m_Panels.end()) {
                m_SelectedPanelTitle = (*it)->GetTitle();
            }
        }

        if (m_SelectedPanelTitle.empty()) {
            const auto it = std::find_if(
                m_Panels.begin(),
                m_Panels.end(),
                [](const std::shared_ptr<IPanel>& panel) {
                    return panel && panel->IsVisible;
                }
            );
            if (it != m_Panels.end()) {
                m_SelectedPanelTitle = (*it)->GetTitle();
            }
        }

        m_PendingFocusPanelTitle = m_SelectedPanelTitle;
    }

    void UIManager::BuildDefaultDockspace(unsigned int dockspace_id) {
        ImGui::DockBuilderRemoveNode(dockspace_id); // Clear out existing layout
        ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace); // Add empty node
        ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetWindowSize());

        // Split the dockspace
        ImGuiID dock_main_id = dockspace_id;
        ImGuiID dock_id_top = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Up, 0.10f, nullptr, &dock_main_id);
        ImGuiID dock_id_left = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.22f, nullptr, &dock_main_id);
        ImGuiID dock_id_right = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.28f, nullptr, &dock_main_id);
        ImGuiID dock_id_bottom = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Down, 0.28f, nullptr, &dock_main_id);
        ImGuiID dock_id_bottom_right = ImGui::DockBuilderSplitNode(dock_id_bottom, ImGuiDir_Right, 0.40f, nullptr, &dock_id_bottom);
        ImGuiID dock_id_right_top = ImGui::DockBuilderSplitNode(dock_id_right, ImGuiDir_Up, 0.42f, nullptr, &dock_id_right);

        // Route specific panels to specific docking nodes
        ImGui::DockBuilderDockWindow("Toolbar", dock_id_top);
        ImGui::DockBuilderDockWindow("Scene Outliner", dock_id_left);
        ImGui::DockBuilderDockWindow("Component Browser", dock_id_left);
        ImGui::DockBuilderDockWindow("Component Builder", dock_id_left);
        ImGui::DockBuilderDockWindow("Workshop Library", dock_id_left);
        ImGui::DockBuilderDockWindow("Workshop Assembly", dock_id_left);
        ImGui::DockBuilderDockWindow("Dashboard", dock_id_right_top);
        ImGui::DockBuilderDockWindow("Properties Inspector", dock_id_right);
        ImGui::DockBuilderDockWindow("Part Editor", dock_id_right);
        ImGui::DockBuilderDockWindow("Workshop Simulation", dock_id_right);
        ImGui::DockBuilderDockWindow("SZM-CEA Console", dock_id_bottom);
        ImGui::DockBuilderDockWindow("Simulation Control", dock_id_bottom_right);
        ImGui::DockBuilderDockWindow("Component Weight Table", dock_id_bottom_right);
        ImGui::DockBuilderDockWindow("Simulation Visualizer", dock_main_id);
        ImGui::DockBuilderDockWindow("Workshop Viewport", dock_main_id);

        ImGui::DockBuilderFinish(dockspace_id);
    }

} // namespace SZM
