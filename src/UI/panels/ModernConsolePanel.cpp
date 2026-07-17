#include "ModernConsolePanel.hpp"
#include <imgui.h>
#include <ctime>

namespace SZM {

ModernConsolePanel::ModernConsolePanel() {
    AddLog("System initialized", "System");
    AddLog("Console ready for input", "Info");
}

void ModernConsolePanel::AddLog(const std::string& message, const std::string& level) {
    m_Logs.push_back({message, level, static_cast<float>(std::time(nullptr))});
    m_ScrollToBottom = true;
}

void ModernConsolePanel::ClearLogs() {
    m_Logs.clear();
}

void ModernConsolePanel::DrawHeader() {
    ImGui::TextUnformatted("System Console");
    ImGui::SameLine(ImGui::GetWindowWidth() - 100);
    UIStyleHelper::StatusBadge("active");
}

void ModernConsolePanel::DrawContent() {
    DrawLogFilters();
    ImGui::Spacing();
    UIStyleHelper::HorizontalSeparator();
    ImGui::Spacing();
    
    DrawLogEntries();
    
    ImGui::Spacing();
    UIStyleHelper::HorizontalSeparator();
    ImGui::Spacing();
    
    DrawInputField();
}

void ModernConsolePanel::DrawLogFilters() {
    ImGui::Text("Filters:");
    ImGui::SameLine();
    
    ImGui::Checkbox("System##filter", &m_ShowSystem);
    ImGui::SameLine();
    ImGui::Checkbox("Info##filter", &m_ShowInfo);
    ImGui::SameLine();
    ImGui::Checkbox("Warning##filter", &m_ShowWarning);
    ImGui::SameLine();
    ImGui::Checkbox("Error##filter", &m_ShowError);
    
    ImGui::SameLine(ImGui::GetWindowWidth() - 250);
    ImGui::InputTextWithHint("##search", "Search logs...", m_SearchBuffer, sizeof(m_SearchBuffer));
}

void ModernConsolePanel::DrawLogEntries() {
    ImGui::BeginChild("##console_logs", ImVec2(0, -60), true);
    
    for (const auto& entry : m_Logs) {
        bool show = false;
        if (entry.level == "System" && m_ShowSystem) show = true;
        if (entry.level == "Info" && m_ShowInfo) show = true;
        if (entry.level == "Warning" && m_ShowWarning) show = true;
        if (entry.level == "Error" && m_ShowError) show = true;
        
        if (!show) continue;
        
        // Search filter
        if (m_SearchBuffer[0] != '\0' && entry.message.find(m_SearchBuffer) == std::string::npos) {
            continue;
        }
        
        // Color by level
        ImU32 color = UIStyleHelper::GetStatusColor(
            entry.level == "Error" ? "error" :
            entry.level == "Warning" ? "warning" :
            entry.level == "System" ? "idle" : "active"
        );
        
        ImGui::PushStyleColor(ImGuiCol_Text, color);
        ImGui::TextUnformatted(("[" + entry.level + "]").c_str());
        ImGui::PopStyleColor();
        
        ImGui::SameLine();
        ImGui::TextUnformatted(entry.message.c_str());
    }
    
    if (m_ScrollToBottom) {
        ImGui::SetScrollHereY(1.0f);
        m_ScrollToBottom = false;
    }
    
    ImGui::EndChild();
}

void ModernConsolePanel::DrawInputField() {
    ImGui::TextUnformatted("Command:");
    if (ImGui::InputTextWithHint("##console_input", "Enter command...", m_InputBuffer, sizeof(m_InputBuffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
        if (m_InputBuffer[0] != '\0') {
            AddLog(std::string("> ") + m_InputBuffer, "System");
            m_InputBuffer[0] = '\0';
        }
    }
    
    ImGui::SameLine();
    if (UIStyleHelper::ModernButtonSecondary("Clear")) {
        ClearLogs();
    }
}

} // namespace SZM
