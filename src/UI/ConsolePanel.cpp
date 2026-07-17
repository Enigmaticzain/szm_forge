#include "ConsolePanel.hpp"
#include <imgui.h>

namespace SZM {

    ConsolePanel::ConsolePanel() {
        AddLog("[System] SZM-CEA AI Assistant Initialized.");
    }

    void ConsolePanel::AddLog(const std::string& log) {
        m_Logs.push_back(log);
    }

    void ConsolePanel::Clear() {
        m_Logs.clear();
    }

    void ConsolePanel::Draw() {
        ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
        
        if (!ImGui::Begin(GetTitle().c_str(), &IsVisible)) {
            ImGui::End();
            return;
        }

        // Display logs
        ImGui::BeginChild("ScrollingRegion", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), false, ImGuiWindowFlags_HorizontalScrollbar);
        
        for (const auto& log : m_Logs) {
            ImGui::TextUnformatted(log.c_str());
        }

        if (m_AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
            ImGui::SetScrollHereY(1.0f);
        }

        ImGui::EndChild();

        // Input area
        ImGui::Separator();
        if (ImGui::InputText("##CommandInput", m_InputBuffer, sizeof(m_InputBuffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
            if (m_InputBuffer[0] != '\0') {
                AddLog(std::string(">> ") + m_InputBuffer);
                m_InputBuffer[0] = '\0';
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("Clear")) {
            Clear();
        }

        ImGui::End();
    }

} // namespace SZM
