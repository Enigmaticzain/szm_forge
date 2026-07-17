#include "ClearUIManager.hpp"
#include <imgui_internal.h>
#include <algorithm>

namespace SZM::UI {

// ============================================================
//  UISection Implementation
// ============================================================

UISection::UISection(const std::string& title, const std::string& icon)
    : m_Title(title), m_Icon(icon) {}

void UISection::Draw() {
    if (!m_DrawFunc) return;

    ImGui::PushID(m_Title.c_str());
    
    // Section header with collapse button
    bool open = !m_IsCollapsed;
    if (ImGui::CollapsingHeader(m_Title.c_str(), &open, ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Indent();
        m_DrawFunc();
        ImGui::Unindent();
    }
    m_IsCollapsed = !open;
    
    ImGui::PopID();
}

// ============================================================
//  UITab Implementation
// ============================================================

UITab::UITab(TabType type, const std::string& name, const std::string& icon)
    : m_Type(type), m_Name(name), m_Icon(icon) {}

void UITab::AddSection(std::shared_ptr<UISection> section) {
    if (section) {
        m_Sections.push_back(section);
    }
}

void UITab::Draw() {
    if (!m_IsActive) return;

    ImGui::BeginChild("TabContent", ImVec2(0, 0), false);
    
    for (auto& section : m_Sections) {
        section->Draw();
        ImGui::Spacing();
    }
    
    ImGui::EndChild();
}

// ============================================================
//  ClearUIManager Implementation
// ============================================================

ClearUIManager* ClearUIManager::s_Instance = nullptr;

ClearUIManager& ClearUIManager::GetInstance() {
    if (s_Instance != nullptr) {
        return *s_Instance;
    }
    static ClearUIManager instance;
    s_Instance = &instance;
    return instance;
}

void ClearUIManager::Initialize() {
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 4.0f;
    style.FrameRounding = 3.0f;
    style.GrabRounding = 3.0f;
    style.WindowPadding = ImVec2(12.0f, 12.0f);
    style.FramePadding = ImVec2(8.0f, 6.0f);
    style.ItemSpacing = ImVec2(8.0f, 8.0f);

    SetupDefaultTabs();
}

void ClearUIManager::Shutdown() {
    m_Tabs.clear();
    m_ActiveTab = nullptr;
}

void ClearUIManager::SetupDefaultTabs() {
    CreateTab(TabType::Dashboard, "Dashboard", "📊");
    CreateTab(TabType::Modeling, "Modeling", "🔧");
    CreateTab(TabType::Assembly, "Assembly", "⚙️");
    CreateTab(TabType::Simulation, "Simulation", "▶️");
    CreateTab(TabType::Training, "Training", "🧠");
    CreateTab(TabType::Results, "Results", "📈");
    CreateTab(TabType::Settings, "Settings", "⚙️");

    if (!m_Tabs.empty()) {
        SetActiveTab(TabType::Dashboard);
    }
}

void ClearUIManager::CreateTab(TabType type, const std::string& name, const std::string& icon) {
    auto tab = std::make_shared<UITab>(type, name, icon);
    m_Tabs.push_back(tab);
}

void ClearUIManager::SetActiveTab(TabType type) {
    for (auto& tab : m_Tabs) {
        if (tab->GetType() == type) {
            tab->SetActive(true);
            m_ActiveTab = tab;
        } else {
            tab->SetActive(false);
        }
    }
}

void ClearUIManager::AddSectionToTab(TabType tabType, std::shared_ptr<UISection> section) {
    for (auto& tab : m_Tabs) {
        if (tab->GetType() == tabType) {
            tab->AddSection(section);
            break;
        }
    }
}

void ClearUIManager::RenderUI() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse;
    flags |= ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGui::Begin("SZM_Forge_Main", nullptr, flags);
    ImGui::PopStyleVar(3);

    // Tab bar at top
    ImGui::BeginChild("TabBar", ImVec2(0, 50), true);
    RenderTabBar();
    ImGui::EndChild();

    ImGui::Spacing();

    // Active tab content
    ImGui::BeginChild("TabContent", ImVec2(0, 0), false);
    RenderActiveTab();
    ImGui::EndChild();

    ImGui::End();
}

void ClearUIManager::RenderTabBar() {
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12.0f, 8.0f));

    for (auto& tab : m_Tabs) {
        DrawTabButton(tab);
        ImGui::SameLine();
    }

    ImGui::PopStyleVar(2);
}

void ClearUIManager::DrawTabButton(const std::shared_ptr<UITab>& tab) {
    if (!tab) return;

    ImVec4 bgColor = tab->IsActive() 
        ? ImVec4(0.25f, 0.35f, 0.50f, 1.0f)  // Active: blue
        : ImVec4(0.15f, 0.15f, 0.20f, 1.0f); // Inactive: dark

    ImGui::PushStyleColor(ImGuiCol_Button, bgColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.30f, 0.40f, 0.55f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.25f, 0.35f, 0.50f, 1.0f));

    std::string label = tab->GetIcon() + " " + tab->GetName();
    if (ImGui::Button(label.c_str(), ImVec2(120, 0))) {
        SetActiveTab(tab->GetType());
    }

    ImGui::PopStyleColor(3);
}

void ClearUIManager::RenderActiveTab() {
    if (m_ActiveTab) {
        m_ActiveTab->Draw();
    }
}

} // namespace SZM::UI
