#include "SimulationPlaybackPanel.hpp"
#include <imgui.h>

namespace SZM::UI {

SimulationPlaybackPanel::SimulationPlaybackPanel()
    : m_VisData(Graphics::SimulationVisualizationData::GetInstance()),
      m_ResultsRenderer(Graphics::SimulationResultsRenderer::GetInstance()) {
}

SimulationPlaybackPanel::~SimulationPlaybackPanel() = default;

void SimulationPlaybackPanel::Draw() {
    if (!ImGui::Begin(GetName(), &m_IsOpen)) {
        ImGui::End();
        return;
    }

    DrawPlaybackControls();
    ImGui::Separator();
    DrawFrameSlider();
    ImGui::Separator();
    DrawResultModeSelector();
    ImGui::Separator();
    DrawStatistics();

    ImGui::End();
}

void SimulationPlaybackPanel::DrawPlaybackControls() {
    auto& state = m_VisData.GetPlaybackState();

    if (ImGui::Button("Play", ImVec2(60, 0))) {
        m_VisData.Play();
    }
    ImGui::SameLine();
    if (ImGui::Button("Pause", ImVec2(60, 0))) {
        m_VisData.Pause();
    }
    ImGui::SameLine();
    if (ImGui::Button("Stop", ImVec2(60, 0))) {
        m_VisData.Stop();
    }

    ImGui::SameLine();
    bool looping = state.isLooping;
    if (ImGui::Checkbox("Loop", &looping)) {
        auto newState = state;
        newState.isLooping = looping;
        m_VisData.SetPlaybackState(newState);
    }

    float speed = state.playbackSpeed;
    if (ImGui::SliderFloat("Speed", &speed, 0.1f, 5.0f)) {
        auto newState = state;
        newState.playbackSpeed = speed;
        m_VisData.SetPlaybackState(newState);
    }
}

void SimulationPlaybackPanel::DrawFrameSlider() {
    auto& state = m_VisData.GetPlaybackState();
    int frame = state.currentFrame;
    int maxFrame = state.totalFrames > 0 ? state.totalFrames - 1 : 0;

    if (ImGui::SliderInt("Frame", &frame, 0, maxFrame)) {
        m_VisData.SetCurrentFrame(frame);
    }

    ImGui::Text("Frame %u / %u | Time: %.2f / %.2f s",
        state.currentFrame, state.totalFrames,
        state.currentTime, state.totalTime);
}

void SimulationPlaybackPanel::DrawResultModeSelector() {
    ImGui::Text("Visualization Mode:");
    ImGui::SameLine();

    const char* modes[] = {"Stress", "Temperature", "Deformation", "Combined"};
    int currentMode = static_cast<int>(m_ResultsRenderer.GetResultMode());

    if (ImGui::Combo("##ResultMode", &currentMode, modes, IM_ARRAYSIZE(modes))) {
        m_ResultsRenderer.SetResultMode(static_cast<Graphics::SimulationResultMode>(currentMode));
    }
}

void SimulationPlaybackPanel::DrawStatistics() {
    ImGui::Text("Analysis Results:");
    ImGui::Indent();

    auto& stressViz = m_ResultsRenderer.GetStressVisualizer();
    ImGui::BulletText("Max Stress: %.2f MPa", stressViz.GetMaxStress());
    ImGui::BulletText("Failed Components: %zu", stressViz.GetFailedComponentCount());

    auto& thermalViz = m_ResultsRenderer.GetThermalVisualizer();
    ImGui::BulletText("Max Temperature: %.2f K", thermalViz.GetMaxTemperature());
    ImGui::BulletText("Overheated Components: %zu", thermalViz.GetOverheatedComponentCount());

    ImGui::Unindent();
}

} // namespace SZM::UI
