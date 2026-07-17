#include "TrainingTabUI.hpp"
#include <imgui.h>

TrainingTabUI::TrainingTabUI() = default;

void TrainingTabUI::Draw() {
    if (!ImGui::Begin(GetTitle().c_str(), &IsVisible, ImGuiWindowFlags_NoMove)) {
        ImGui::End();
        return;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(12.0f, 12.0f));

    // Main sections
    DrawDataImportSection();
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    DrawTrainingControlSection();
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    DrawConfigurationSection();
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    DrawMetricsSection();
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    DrawCheckpointsSection();

    ImGui::PopStyleVar();
    ImGui::End();
}

void TrainingTabUI::DrawDataImportSection() {
    ImGui::Text("📁 Data Import");
    ImGui::Separator();

    ImGui::Columns(2, "ImportColumns", false);
    ImGui::SetColumnWidth(0, 200);

    ImGui::Text("Import Source:");
    ImGui::NextColumn();

    if (ImGui::Button("Browse Files", ImVec2(-1, 0))) {
        m_ShowImportDialog = true;
    }
    ImGui::NextColumn();

    ImGui::Text("Data Type:");
    ImGui::NextColumn();

    static int dataType = 0;
    ImGui::Combo("##DataType", &dataType, "GitHub\0ZIP\0PDF\0Markdown\0Codebase\0");
    ImGui::NextColumn();

    ImGui::Columns(1);

    ImGui::Spacing();
    if (ImGui::Button("Import", ImVec2(100, 0))) {
        // Import action
    }
}

void TrainingTabUI::DrawTrainingControlSection() {
    ImGui::Text("▶️ Training Control");
    ImGui::Separator();

    ImGui::Columns(2, "TrainingColumns", false);
    ImGui::SetColumnWidth(0, 200);

    ImGui::Text("Training Mode:");
    ImGui::NextColumn();

    ImGui::Combo("##TrainingMode", &m_SelectedTrainingMode, 
        "Passive Learning\0Repository Learning\0Architecture Learning\0Code Understanding\0Simulation Learning\0");
    ImGui::NextColumn();

    ImGui::Text("Status:");
    ImGui::NextColumn();

    ImGui::TextColored(
        m_IsTraining ? ImVec4(0.2f, 1.0f, 0.2f, 1.0f) : ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
        m_IsTraining ? "TRAINING" : "IDLE"
    );
    ImGui::NextColumn();

    ImGui::Columns(1);

    ImGui::Spacing();
    DrawProgressBar("Training Progress", m_TrainingProgress);

    ImGui::Spacing();
    ImGui::Columns(3, "ControlButtons", false);

    if (ImGui::Button("Start", ImVec2(-1, 0))) {
        m_IsTraining = true;
        m_TrainingProgress = 0.0f;
    }
    ImGui::NextColumn();

    if (ImGui::Button("Pause", ImVec2(-1, 0))) {
        m_IsTraining = false;
    }
    ImGui::NextColumn();

    if (ImGui::Button("Stop", ImVec2(-1, 0))) {
        m_IsTraining = false;
        m_TrainingProgress = 0.0f;
    }

    ImGui::Columns(1);
}

void TrainingTabUI::DrawConfigurationSection() {
    ImGui::Text("⚙️ Configuration");
    ImGui::Separator();

    ImGui::Columns(2, "ConfigColumns", false);
    ImGui::SetColumnWidth(0, 200);

    static int chunkSize = 512;
    ImGui::Text("Chunk Size:");
    ImGui::NextColumn();
    ImGui::SliderInt("##ChunkSize", &chunkSize, 256, 2048);
    ImGui::NextColumn();

    static int contextSize = 2048;
    ImGui::Text("Context Size:");
    ImGui::NextColumn();
    ImGui::SliderInt("##ContextSize", &contextSize, 512, 8192);
    ImGui::NextColumn();

    static float gpuUsage = 0.8f;
    ImGui::Text("GPU Usage:");
    ImGui::NextColumn();
    ImGui::SliderFloat("##GPUUsage", &gpuUsage, 0.0f, 1.0f, "%.1f");
    ImGui::NextColumn();

    static float cpuUsage = 0.5f;
    ImGui::Text("CPU Usage:");
    ImGui::NextColumn();
    ImGui::SliderFloat("##CPUUsage", &cpuUsage, 0.0f, 1.0f, "%.1f");
    ImGui::NextColumn();

    ImGui::Columns(1);

    ImGui::Spacing();
    if (ImGui::Button("Save Configuration", ImVec2(150, 0))) {
        // Save config
    }
}

void TrainingTabUI::DrawMetricsSection() {
    ImGui::Text("📊 Metrics");
    ImGui::Separator();

    ImGui::Columns(4, "MetricsColumns", false);

    DrawMetricCard("Accuracy", "85.2%");
    ImGui::NextColumn();

    DrawMetricCard("Loss", "0.152");
    ImGui::NextColumn();

    DrawMetricCard("Tokens", "1.2M");
    ImGui::NextColumn();

    DrawMetricCard("Learning Rate", "0.001");

    ImGui::Columns(1);
}

void TrainingTabUI::DrawCheckpointsSection() {
    ImGui::Text("💾 Checkpoints");
    ImGui::Separator();

    if (ImGui::BeginTable("CheckpointsTable", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn("Name");
        ImGui::TableSetupColumn("Accuracy");
        ImGui::TableSetupColumn("Date");
        ImGui::TableSetupColumn("Action");
        ImGui::TableHeadersRow();

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Checkpoint 1");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("85.2%");
        ImGui::TableSetColumnIndex(2);
        ImGui::Text("2024-01-15");
        ImGui::TableSetColumnIndex(3);
        if (ImGui::Button("Load##1")) {}

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Checkpoint 2");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("82.1%");
        ImGui::TableSetColumnIndex(2);
        ImGui::Text("2024-01-14");
        ImGui::TableSetColumnIndex(3);
        if (ImGui::Button("Load##2")) {}

        ImGui::EndTable();
    }

    ImGui::Spacing();
    if (ImGui::Button("Save Checkpoint", ImVec2(150, 0))) {
        // Save checkpoint
    }
}

void TrainingTabUI::DrawProgressBar(const char* label, float progress) {
    ImGui::Text("%s", label);
    ImGui::ProgressBar(progress, ImVec2(-1, 0));
    ImGui::Text("%.1f%%", progress * 100.0f);
}

void TrainingTabUI::DrawMetricCard(const char* label, const std::string& value) {
    ImGui::BeginChild(label, ImVec2(0, 80), true);
    ImGui::Text("%s", label);
    ImGui::Spacing();
    ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "%s", value.c_str());
    ImGui::EndChild();
}

void TrainingTabUI::DrawButton(const char* label, bool& state) {
    if (ImGui::Button(label)) {
        state = !state;
    }
}
