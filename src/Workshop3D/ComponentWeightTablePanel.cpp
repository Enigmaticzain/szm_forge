#include "ComponentWeightTablePanel.hpp"
#include "UI/UIStyleHelper.hpp"
#include <imgui.h>
#include <algorithm>
#include <fstream>

namespace SZM::Workshop3D {

ComponentWeightTablePanel::ComponentWeightTablePanel()
    : BeginnerPanel("Component Weight Table") {
    
    // Add tabs
    AddTab("library", "Library", "📚", [this]() { DrawLibraryTableTab(); });
    AddTab("scene", "Scene", "🎬", [this]() { DrawSceneTableTab(); });
    AddTab("stats", "Statistics", "📊", [this]() { DrawStatisticsTab(); });
    AddTab("export", "Export", "💾", [this]() { DrawExportTab(); });
    
    RefreshComponentWeights();
    RefreshSceneWeights();
}

void ComponentWeightTablePanel::DrawLibraryTableTab() {
    ImGui::TextUnformatted("Component Library Weights");
    ImGui::Spacing();
    
    // Refresh button
    if (UIStyleHelper::ModernButtonPrimary("Refresh")) {
        RefreshComponentWeights();
    }
    ImGui::SameLine();
    
    // Sort options
    ImGui::TextUnformatted("Sort by:");
    ImGui::SameLine();
    ImGui::Combo("##sort", &m_SortMode, "Name\0Weight (Low→High)\0Weight (High→Low)\0");
    
    ImGui::Spacing();
    
    // Filter heavy components
    ImGui::Checkbox("Show Only Heavy Components", &m_ShowOnlyHeavy);
    if (m_ShowOnlyHeavy) {
        ImGui::SameLine();
        ImGui::DragFloat("Threshold (kg)##library", &m_HeavyThreshold, 0.1f, 0.1f, 1000.0f);
    }
    
    ImGui::Spacing();
    UIStyleHelper::HorizontalSeparator();
    ImGui::Spacing();
    
    // Total weight
    ImGui::TextColored(ImVec4(0.55f, 0.36f, 0.96f, 1.0f),
        "Total Library Weight: %.2f kg", m_TotalLibraryWeight);
    ImGui::TextColored(ImVec4(0.23f, 0.51f, 0.96f, 1.0f),
        "Average Component Weight: %.2f kg", m_AverageComponentWeight);
    
    ImGui::Spacing();
    UIStyleHelper::HorizontalSeparator();
    ImGui::Spacing();
    
    // Weight table
    ImGui::TextUnformatted("Component Weights:");
    ImGui::Spacing();
    
    // Table header
    ImGui::Columns(3, "##library_table", true);
    ImGui::TextColored(ImVec4(0.55f, 0.36f, 0.96f, 1.0f), "Component Name");
    ImGui::NextColumn();
    ImGui::TextColored(ImVec4(0.55f, 0.36f, 0.96f, 1.0f), "Weight (kg)");
    ImGui::NextColumn();
    ImGui::TextColored(ImVec4(0.55f, 0.36f, 0.96f, 1.0f), "Percentage");
    ImGui::NextColumn();
    ImGui::Separator();
    
    // Table rows
    for (const auto& [name, weight] : m_LibraryWeights) {
        if (m_ShowOnlyHeavy && weight < m_HeavyThreshold) {
            continue;
        }
        
        float percentage = (m_TotalLibraryWeight > 0) ? (weight / m_TotalLibraryWeight) * 100.0f : 0.0f;
        
        ImGui::TextUnformatted(name.c_str());
        ImGui::NextColumn();
        ImGui::TextColored(ImVec4(0.06f, 0.73f, 0.51f, 1.0f), "%.2f", weight);
        ImGui::NextColumn();
        ImGui::TextColored(ImVec4(0.96f, 0.62f, 0.04f, 1.0f), "%.1f%%", percentage);
        ImGui::NextColumn();
    }
    
    ImGui::Columns(1);
}

void ComponentWeightTablePanel::DrawSceneTableTab() {
    ImGui::TextUnformatted("Scene Component Weights");
    ImGui::Spacing();
    
    // Refresh button
    if (UIStyleHelper::ModernButtonPrimary("Refresh")) {
        RefreshSceneWeights();
    }
    ImGui::SameLine();
    
    // Sort options
    ImGui::TextUnformatted("Sort by:");
    ImGui::SameLine();
    ImGui::Combo("##sort_scene", &m_SortMode, "Name\0Weight (Low→High)\0Weight (High→Low)\0");
    
    ImGui::Spacing();
    UIStyleHelper::HorizontalSeparator();
    ImGui::Spacing();
    
    // Total weight
    ImGui::TextColored(ImVec4(0.55f, 0.36f, 0.96f, 1.0f),
        "Total Scene Weight: %.2f kg", m_TotalSceneWeight);
    
    ImGui::Spacing();
    UIStyleHelper::HorizontalSeparator();
    ImGui::Spacing();
    
    // Weight table
    ImGui::TextUnformatted("Scene Component Weights:");
    ImGui::Spacing();
    
    if (m_SceneWeights.empty()) {
        ImGui::TextDisabled("No components in scene. Add components from Library tab.");
    } else {
        // Table header
        ImGui::Columns(3, "##scene_table", true);
        ImGui::TextColored(ImVec4(0.55f, 0.36f, 0.96f, 1.0f), "Component Name");
        ImGui::NextColumn();
        ImGui::TextColored(ImVec4(0.55f, 0.36f, 0.96f, 1.0f), "Weight (kg)");
        ImGui::NextColumn();
        ImGui::TextColored(ImVec4(0.55f, 0.36f, 0.96f, 1.0f), "Percentage");
        ImGui::NextColumn();
        ImGui::Separator();
        
        // Table rows
        for (const auto& [name, weight] : m_SceneWeights) {
            float percentage = (m_TotalSceneWeight > 0) ? (weight / m_TotalSceneWeight) * 100.0f : 0.0f;
            
            ImGui::TextUnformatted(name.c_str());
            ImGui::NextColumn();
            ImGui::TextColored(ImVec4(0.06f, 0.73f, 0.51f, 1.0f), "%.2f", weight);
            ImGui::NextColumn();
            ImGui::TextColored(ImVec4(0.96f, 0.62f, 0.04f, 1.0f), "%.1f%%", percentage);
            ImGui::NextColumn();
        }
        
        ImGui::Columns(1);
    }
}

void ComponentWeightTablePanel::DrawStatisticsTab() {
    ImGui::TextUnformatted("Weight Statistics");
    ImGui::Spacing();
    UIStyleHelper::HorizontalSeparator();
    ImGui::Spacing();
    
    // Library statistics
    ImGui::TextUnformatted("Library Statistics:");
    ImGui::Spacing();
    
    ImGui::TextColored(ImVec4(0.55f, 0.36f, 0.96f, 1.0f), "Total Components: %zu", m_LibraryWeights.size());
    ImGui::TextColored(ImVec4(0.23f, 0.51f, 0.96f, 1.0f), "Total Weight: %.2f kg", m_TotalLibraryWeight);
    ImGui::TextColored(ImVec4(0.06f, 0.73f, 0.51f, 1.0f), "Average Weight: %.2f kg", m_AverageComponentWeight);
    
    // Find heaviest and lightest
    if (!m_LibraryWeights.empty()) {
        auto heaviest = std::max_element(m_LibraryWeights.begin(), m_LibraryWeights.end(),
            [](const auto& a, const auto& b) { return a.second < b.second; });
        auto lightest = std::min_element(m_LibraryWeights.begin(), m_LibraryWeights.end(),
            [](const auto& a, const auto& b) { return a.second < b.second; });
        
        ImGui::TextColored(ImVec4(0.96f, 0.62f, 0.04f, 1.0f), "Heaviest: %s (%.2f kg)", 
            heaviest->first.c_str(), heaviest->second);
        ImGui::TextColored(ImVec4(0.96f, 0.62f, 0.04f, 1.0f), "Lightest: %s (%.2f kg)", 
            lightest->first.c_str(), lightest->second);
    }
    
    ImGui::Spacing();
    UIStyleHelper::HorizontalSeparator();
    ImGui::Spacing();
    
    // Scene statistics
    ImGui::TextUnformatted("Scene Statistics:");
    ImGui::Spacing();
    
    ImGui::TextColored(ImVec4(0.55f, 0.36f, 0.96f, 1.0f), "Components in Scene: %zu", m_SceneWeights.size());
    ImGui::TextColored(ImVec4(0.23f, 0.51f, 0.96f, 1.0f), "Total Scene Weight: %.2f kg", m_TotalSceneWeight);
    
    if (m_SceneWeights.size() > 0) {
        float avgScene = m_TotalSceneWeight / m_SceneWeights.size();
        ImGui::TextColored(ImVec4(0.06f, 0.73f, 0.51f, 1.0f), "Average Scene Component: %.2f kg", avgScene);
    }
    
    ImGui::Spacing();
    UIStyleHelper::HorizontalSeparator();
    ImGui::Spacing();
    
    // Weight distribution
    ImGui::TextUnformatted("Weight Distribution:");
    ImGui::Spacing();
    
    if (m_TotalLibraryWeight > 0) {
        float scenePercentage = (m_TotalSceneWeight / m_TotalLibraryWeight) * 100.0f;
        ImGui::TextColored(ImVec4(0.96f, 0.62f, 0.04f, 1.0f), "Scene vs Library: %.1f%%", scenePercentage);
    }
}

void ComponentWeightTablePanel::DrawExportTab() {
    ImGui::TextUnformatted("Export Weight Data");
    ImGui::Spacing();
    UIStyleHelper::HorizontalSeparator();
    ImGui::Spacing();
    
    ImGui::TextUnformatted("Export to CSV File:");
    ImGui::InputText("File Path##export", m_ExportPath, sizeof(m_ExportPath));
    ImGui::SameLine();
    
    if (UIStyleHelper::ModernButtonPrimary("Browse")) {
        // File browser would go here
    }
    
    ImGui::Spacing();
    
    // Export buttons
    if (UIStyleHelper::ModernButtonPrimary("Export Library Weights")) {
        if (m_ExportPath[0]) {
            ExportWeightsToCSV(m_ExportPath);
        }
    }
    ImGui::SameLine();
    
    if (UIStyleHelper::ModernButtonPrimary("Export Scene Weights")) {
        if (m_ExportPath[0]) {
            ExportWeightsToCSV(m_ExportPath);
        }
    }
    
    ImGui::Spacing();
    UIStyleHelper::HorizontalSeparator();
    ImGui::Spacing();
    
    ImGui::TextUnformatted("Export Format:");
    ImGui::TextDisabled("CSV (Comma-Separated Values)");
    ImGui::TextDisabled("Columns: Component Name, Weight (kg), Percentage");
    ImGui::Spacing();
    
    ImGui::TextUnformatted("Usage:");
    ImGui::TextDisabled("1. Enter file path (e.g., /path/to/weights.csv)");
    ImGui::TextDisabled("2. Click Export button");
    ImGui::TextDisabled("3. Open CSV in Excel or spreadsheet app");
}

void ComponentWeightTablePanel::RefreshComponentWeights() {
    m_LibraryWeights.clear();
    m_TotalLibraryWeight = 0.0f;
    
    auto& library = WorkshopManager::GetInstance().GetPartLibrary();
    
    for (const auto& [id, part] : library.GetAllParts()) {
        if (part) {
            float weight = part->GetMass();
            m_LibraryWeights.push_back({part->GetProperties().name, weight});
            m_TotalLibraryWeight += weight;
        }
    }
    
    // Sort based on mode
    if (m_SortMode == 1) {
        std::sort(m_LibraryWeights.begin(), m_LibraryWeights.end(),
            [](const auto& a, const auto& b) { return a.second < b.second; });
    } else if (m_SortMode == 2) {
        std::sort(m_LibraryWeights.begin(), m_LibraryWeights.end(),
            [](const auto& a, const auto& b) { return a.second > b.second; });
    } else {
        std::sort(m_LibraryWeights.begin(), m_LibraryWeights.end(),
            [](const auto& a, const auto& b) { return a.first < b.first; });
    }
    
    if (!m_LibraryWeights.empty()) {
        m_AverageComponentWeight = m_TotalLibraryWeight / m_LibraryWeights.size();
    }
}

void ComponentWeightTablePanel::RefreshSceneWeights() {
    m_SceneWeights.clear();
    m_TotalSceneWeight = 0.0f;
    
    auto assembly = WorkshopManager::GetInstance().GetActiveAssembly();
    if (!assembly) return;
    
    // Get instances from assembly
    // Note: This is a simplified version - actual implementation depends on Assembly structure
    // For now, we'll populate from library as placeholder
    RefreshComponentWeights();
    m_SceneWeights = m_LibraryWeights;
    m_TotalSceneWeight = m_TotalLibraryWeight;
}

void ComponentWeightTablePanel::DrawWeightTable(const std::vector<std::pair<std::string, float>>& weights) {
    ImGui::Columns(3, "##weight_table", true);
    ImGui::TextColored(ImVec4(0.55f, 0.36f, 0.96f, 1.0f), "Component");
    ImGui::NextColumn();
    ImGui::TextColored(ImVec4(0.55f, 0.36f, 0.96f, 1.0f), "Weight (kg)");
    ImGui::NextColumn();
    ImGui::TextColored(ImVec4(0.55f, 0.36f, 0.96f, 1.0f), "Percentage");
    ImGui::NextColumn();
    ImGui::Separator();
    
    for (const auto& [name, weight] : weights) {
        ImGui::TextUnformatted(name.c_str());
        ImGui::NextColumn();
        ImGui::Text("%.2f", weight);
        ImGui::NextColumn();
        ImGui::Text("%.1f%%", (weight / m_TotalLibraryWeight) * 100.0f);
        ImGui::NextColumn();
    }
    
    ImGui::Columns(1);
}

void ComponentWeightTablePanel::ExportWeightsToCSV(const std::string& filepath) {
    std::ofstream file(filepath);
    if (!file.is_open()) return;
    
    // Write header
    file << "Component Name,Weight (kg),Percentage\n";
    
    // Write library weights
    for (const auto& [name, weight] : m_LibraryWeights) {
        float percentage = (m_TotalLibraryWeight > 0) ? (weight / m_TotalLibraryWeight) * 100.0f : 0.0f;
        file << name << "," << weight << "," << percentage << "\n";
    }
    
    // Write total
    file << "TOTAL," << m_TotalLibraryWeight << ",100.0\n";
    
    file.close();
}

} // namespace SZM::Workshop3D
