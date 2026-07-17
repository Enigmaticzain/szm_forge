#include "ConsolePanel.hpp"
#include "../../Simulation/SimulationEngine.hpp"
#include <imgui.h>
#include <cstring>
#include <string>

ConsolePanel::ConsolePanel() {
    AddLog("[System] SZM Forge v0.1.0 — Desktop Engineering Suite.");
    AddLog("[System] Console initialized. Type 'help' for commands.");
}

void ConsolePanel::AddLog(const std::string& message) {
    m_Logs.push_back(message);
    m_ScrollToBottom = true;
}

void ConsolePanel::ClearLogs() {
    m_Logs.clear();
}

void ConsolePanel::Draw() {
    if (!ImGui::Begin(GetTitle().c_str(), &IsVisible)) {
        ImGui::End();
        return;
    }

    if (ImGui::Button("Clear")) {
        ClearLogs();
    }
    ImGui::SameLine();
    ImGui::TextDisabled("Entries: %zu", m_Logs.size());

    ImGui::Separator();

    const float footerHeight =
        ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();

    ImGui::BeginChild("ConsoleScrollRegion", ImVec2(0, -footerHeight), true,
                      ImGuiWindowFlags_HorizontalScrollbar);
    for (const auto& log : m_Logs) {
        ImGui::TextUnformatted(log.c_str());
    }
    if (m_ScrollToBottom) {
        ImGui::SetScrollHereY(1.0f);
        m_ScrollToBottom = false;
    }
    ImGui::EndChild();

    bool reclaimFocus = false;
    if (ImGui::InputText("Command", m_InputBuffer, sizeof(m_InputBuffer),
                         ImGuiInputTextFlags_EnterReturnsTrue)) {
        if (m_InputBuffer[0] != '\0') {
            std::string inputStr(m_InputBuffer);
            AddLog(std::string("[User] ") + inputStr);
            
            if (inputStr == "help") {
                AddLog("[System] Available commands:");
                AddLog("  help   - Show this message");
                AddLog("  clear  - Clear console logs");
                AddLog("  reset  - Reset simulation components");
                AddLog("  status - Show engine status");
            } else if (inputStr == "clear") {
                ClearLogs();
            } else if (inputStr == "reset") {
                SZM::SimulationEngine::GetInstance().ClearAllComponents();
                AddLog("[System] Simulation components cleared.");
            } else if (inputStr == "status") {
                auto& engine = SZM::SimulationEngine::GetInstance();
                AddLog("[System] Simulation Status:");
                AddLog("  Paused: " + std::string(engine.IsPaused() ? "Yes" : "No"));
                AddLog("  Components: " + std::to_string(engine.GetComponents().size()));
            } else {
                AddLog("[System] Unknown command. Type 'help' for available commands.");
            }
        }
        std::memset(m_InputBuffer, 0, sizeof(m_InputBuffer));
        reclaimFocus = true;
    }
    if (reclaimFocus) {
        ImGui::SetKeyboardFocusHere(-1);
    }

    ImGui::End();
}
