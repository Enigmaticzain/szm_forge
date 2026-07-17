#include "ComponentBrowserPanel.hpp"
#include "UI/UIStyleHelper.hpp"
#include "UI/UITooltipHelper.hpp"
#include "UI/WorkshopTooltips.hpp"
#include <imgui.h>
#include <filesystem>

using SZM::UITooltip::Hover;
using SZM::UITooltip::HelpMarker;
using SZM::UITooltip::SectionHint;
namespace WT = SZM::WorkshopTooltips;

namespace SZM::Workshop3D {

ComponentBrowserPanel::ComponentBrowserPanel() 
    : BeginnerPanel("Component Browser") {
    
    // Add tabs
    AddTab("library", "Library", "📚", [this]() { DrawLibraryTab(); }, WT::TabLibrary);
    AddTab("import", "Import", "📥", [this]() { DrawImportTab(); }, WT::TabImport);
    AddTab("placement", "Placement", "📍", [this]() { DrawPlacementTab(); }, WT::TabPlacement);
    AddTab("visualization", "Visualization", "🎨", [this]() { DrawVisualizationTab(); }, WT::TabVisualization);
    AddTab("settings", "Settings", "⚙️", [this]() { DrawSettingsTab(); }, WT::TabSettings);
    
    RefreshComponentList();
}

void ComponentBrowserPanel::DrawLibraryTab() {
    SectionHint("Browse parts imported from CAD or the codes/ example bundles. Hover controls for details.");
    ImGui::TextUnformatted("Component Library");
    HelpMarker(WT::TabLibrary);
    ImGui::Spacing();

    ImGui::TextUnformatted("Search:");
    HelpMarker(WT::BrowserSearch);
    ImGui::SameLine();
    ImGui::InputText("##search", m_SearchFilter, sizeof(m_SearchFilter));
    Hover(WT::BrowserSearch);
    
    ImGui::Spacing();
    UIStyleHelper::HorizontalSeparator();
    ImGui::Spacing();
    
    // Component count
    auto& library = WorkshopManager::GetInstance().GetPartLibrary();
    ImGui::TextColored(ImVec4(0.55f, 0.36f, 0.96f, 1.0f), 
        "Total Components: %zu", library.GetPartCount());
    ImGui::Spacing();
    
    // Component list
    ImGui::TextUnformatted("Available Components:");
    ImGui::Spacing();
    
    if (library.GetPartCount() == 0) {
        ImGui::TextDisabled("No components in library. Use Import tab to add components.");
    } else {
        for (const auto& [id, part] : library.GetAllParts()) {
            if (!part) continue;
            
            // Filter by search
            std::string partName = part->GetProperties().name;
            if (!m_SearchFilter[0] || partName.find(m_SearchFilter) != std::string::npos) {
                DisplayComponentCard(part);
            }
        }
    }
}

void ComponentBrowserPanel::DrawImportTab() {
    SectionHint("STEP/GEO import uses the geometry pipeline; primitives are created in-memory.");
    ImGui::TextUnformatted("Import Components");
    HelpMarker(WT::CodesFEA);
    ImGui::Spacing();

    ImGui::TextUnformatted("Import from File:");
    HelpMarker(WT::BrowserImportPath);
    ImGui::InputText("File Path##import", m_ImportPath, sizeof(m_ImportPath));
    Hover(WT::BrowserImportPath);
    ImGui::SameLine();

    if (UIStyleHelper::ModernButtonPrimary("Browse", "Open file picker when a native dialog is wired.")) {
        // File browser would go here
        ImGui::OpenPopup("file_browser");
    }
    
    ImGui::Spacing();
    
    // Import buttons
    if (UIStyleHelper::ModernButtonPrimary("Import STEP", WT::BrowserImportStep)) {
        if (m_ImportPath[0]) {
            ImportComponentFromFile(m_ImportPath);
        }
    }
    ImGui::SameLine();

    if (UIStyleHelper::ModernButtonPrimary("Import GEO", WT::BrowserImportGeo)) {
        if (m_ImportPath[0]) {
            ImportComponentFromFile(m_ImportPath);
        }
    }
    
    ImGui::Spacing();
    UIStyleHelper::HorizontalSeparator();
    ImGui::Spacing();
    
    // Create primitive
    ImGui::TextUnformatted("Or Create Primitive Component:");
    ImGui::Spacing();
    
    ImGui::InputText("Component Name##create", m_ComponentName, sizeof(m_ComponentName));
    Hover(WT::EditorPartName);

    static float primitiveSize[3] = {0.5f, 0.5f, 0.5f};
    ImGui::DragFloat3("Size##primitive", primitiveSize, 0.01f, 0.01f, 10.0f);
    Hover(WT::EditorSize);

    static int primitiveType = 0;
    ImGui::Combo("Primitive Type##create", &primitiveType, "Box\0Cylinder\0Sphere\0Cone\0");
    Hover(WT::BrowserPrimitiveType);

    ImGui::Spacing();
    if (UIStyleHelper::ModernButtonPrimary("Create Component", WT::BrowserCreatePrimitive)) {
        auto& manager = WorkshopManager::GetInstance();
        auto part = manager.GetPartLibrary().CreatePart(m_ComponentName);
        if (part) {
            Part::Properties props = part->GetProperties();
            props.category = "Primitive";
            props.material = "Steel";
            part->SetProperties(props);
            RefreshComponentList();
        }
    }
}

void ComponentBrowserPanel::DrawPlacementTab() {
    ImGui::TextUnformatted("Component Placement");
    ImGui::Spacing();
    
    if (!m_SelectedComponent) {
        ImGui::TextDisabled("Select a component from Library tab first");
        return;
    }
    
    ImGui::TextUnformatted("Selected Component:");
    ImGui::TextColored(ImVec4(0.55f, 0.36f, 0.96f, 1.0f), 
        "%s", m_SelectedComponent->GetProperties().name.c_str());
    ImGui::Spacing();
    
    // Position
    ImGui::TextUnformatted("Position:");
    ImGui::DragFloat3("Position##placement", m_PlacementPos, 0.01f);
    Hover(WT::BrowserPlacementPos);

    ImGui::TextUnformatted("Rotation (degrees):");
    ImGui::DragFloat3("Rotation##placement", m_PlacementRot, 1.0f);
    Hover(WT::BrowserPlacementRot);

    ImGui::TextUnformatted("Scale:");
    ImGui::DragFloat("Scale##placement", &m_ComponentScale, 0.01f, 0.1f, 10.0f);
    Hover(WT::BrowserPlacementScale);
    
    ImGui::Spacing();
    UIStyleHelper::HorizontalSeparator();
    ImGui::Spacing();
    
    // Auto placement
    ImGui::Checkbox("Auto Placement", &m_AutoPlacement);
    Hover(WT::BrowserAutoPlacement);
    ImGui::TextDisabled("Automatically position components to avoid overlap");

    ImGui::Spacing();

    if (UIStyleHelper::ModernButtonPrimary("Add to Scene", WT::BrowserAddToScene)) {
        AddComponentToScene(m_SelectedComponent);
    }
    
    ImGui::Spacing();
    ImGui::TextDisabled("Component will be added to active assembly");
}

void ComponentBrowserPanel::DrawVisualizationTab() {
    ImGui::TextUnformatted("Visualization Settings");
    ImGui::Spacing();
    
    // Display options
    ImGui::Checkbox("Show Grid", &m_ShowGrid);
    Hover(WT::BrowserShowGrid);
    ImGui::Checkbox("Show Labels", &m_ShowLabels);
    Hover(WT::BrowserShowLabels);
    ImGui::Checkbox("Show Bounds", &m_ShowBounds);
    Hover(WT::BrowserShowBounds);
    
    ImGui::Spacing();
    UIStyleHelper::HorizontalSeparator();
    ImGui::Spacing();
    
    // Visualization mode
    ImGui::TextUnformatted("Visualization Mode:");
    ImGui::Combo("Mode##viz", &m_VisualizationMode,
        "Solid\0Wireframe\0Stress\0Thermal\0Displacement\0");
    Hover(WT::BrowserVizMode);
    
    ImGui::Spacing();
    
    // Heatmap range
    if (m_VisualizationMode > 1) {
        ImGui::TextUnformatted("Heatmap Range:");
        ImGui::DragFloat("Min##heatmap", &m_HeatmapMin, 1.0f);
        Hover(WT::BrowserHeatmapRange);
        ImGui::DragFloat("Max##heatmap", &m_HeatmapMax, 1.0f);
        Hover(WT::BrowserHeatmapRange);
    }
    
    ImGui::Spacing();
    UIStyleHelper::HorizontalSeparator();
    ImGui::Spacing();
    
    // Statistics
    ImGui::TextUnformatted("Scene Statistics:");
    auto& manager = WorkshopManager::GetInstance();
    ImGui::TextColored(ImVec4(0.06f, 0.73f, 0.51f, 1.0f),
        "Components in Library: %zu", manager.GetPartCount());
    ImGui::TextColored(ImVec4(0.23f, 0.51f, 0.96f, 1.0f),
        "Assemblies: %zu", manager.GetAssemblyCount());
    ImGui::TextColored(ImVec4(0.96f, 0.62f, 0.04f, 1.0f),
        "Instances in Scene: %zu", manager.GetInstanceCount());
}

void ComponentBrowserPanel::RefreshComponentList() {
    auto& library = WorkshopManager::GetInstance().GetPartLibrary();
    m_FilteredComponents.clear();
    
    for (const auto& [id, part] : library.GetAllParts()) {
        if (part) {
            m_FilteredComponents.push_back(part);
        }
    }
}

void ComponentBrowserPanel::DisplayComponentCard(const std::shared_ptr<Part>& part) {
    if (!part) return;
    
    const auto& props = part->GetProperties();
    
    ImGui::PushID(static_cast<int>(part->GetID().high ^ part->GetID().low));
    
    // Card background
    ImGui::PushStyleColor(ImGuiCol_ChildBg, UIStyleHelper::GetSurfaceColor(0));
    ImGui::BeginChild("##card", ImVec2(0, 100), true);
    
    // Component name
    ImGui::TextColored(ImVec4(0.55f, 0.36f, 0.96f, 1.0f), "%s", props.name.c_str());
    
    // Component info
    ImGui::TextDisabled("Material: %s", props.material.c_str());
    ImGui::TextDisabled("Category: %s", props.category.c_str());
    ImGui::TextDisabled("Mass: %.2f kg", props.mass);
    
    ImGui::Spacing();
    
    // Select button
    if (ImGui::Button("Select##component", ImVec2(80, 0))) {
        m_SelectedComponent = part;
    }
    Hover("Select this part for placement and property editing.");
    
    ImGui::EndChild();
    ImGui::PopStyleColor();
    ImGui::PopID();
    
    ImGui::Spacing();
}

void ComponentBrowserPanel::ImportComponentFromFile(const std::string& filepath) {
    auto& manager = WorkshopManager::GetInstance();
    
    // Try to import based on file extension
    std::string ext = filepath.substr(filepath.find_last_of(".") + 1);
    
    std::shared_ptr<Part> part = nullptr;
    if (ext == "step" || ext == "stp") {
        part = manager.GetPartLibrary().ImportFromSTEP(filepath);
    } else if (ext == "geo") {
        part = manager.GetPartLibrary().ImportFromGEO(filepath);
    }
    
    if (part) {
        RefreshComponentList();
        m_SelectedComponent = part;
    }
}

void ComponentBrowserPanel::AddComponentToScene(const std::shared_ptr<Part>& part) {
    if (!part) return;
    
    auto& manager = WorkshopManager::GetInstance();
    Geometry::Vector3 pos(m_PlacementPos[0], m_PlacementPos[1], m_PlacementPos[2]);
    
    if (m_AutoPlacement) {
        pos = manager.SuggestPlacement(*part);
    }
    
    manager.AddPartToActiveAssembly(part->GetID(), pos, m_ComponentName);
}

void ComponentBrowserPanel::DrawSettingsTab() {
    SectionHint("Preferences for import, grid snap, and codes/ physics preview hooks.");
    ImGui::TextUnformatted("Component Browser Settings");
    HelpMarker(WT::TabSettings);
    ImGui::Spacing();
    UIStyleHelper::HorizontalSeparator();
    ImGui::Spacing();

    ImGui::TextUnformatted("Auto-Save Options:");
    ImGui::Checkbox("Enable Auto-Save", &m_EnableAutoSave);
    Hover("Save imported parts to the workshop library folder automatically.");
    ImGui::TextDisabled("Automatically save components when imported");
    ImGui::Spacing();
    
    // Grid settings
    ImGui::TextUnformatted("Grid Settings:");
    ImGui::Checkbox("Enable Snap to Grid", &m_EnableSnapToGrid);
    Hover("Round placement coordinates to the grid step (machine layout aid).");
    ImGui::DragFloat("Grid Size##settings", &m_GridSize, 0.01f, 0.01f, 1.0f);
    Hover("Grid cell size in meters for snap placement.");
    ImGui::TextDisabled("Snap components to grid when placing");
    ImGui::Spacing();
    
    UIStyleHelper::HorizontalSeparator();
    ImGui::Spacing();
    
    // Default material
    ImGui::TextUnformatted("Default Material:");
    ImGui::Combo("Material##settings", &m_DefaultMaterial,
        "Steel\0Aluminum\0Titanium\0Composite\0");
    Hover(WT::EditorMaterial);
    ImGui::TextDisabled("Material used for new primitive components");
    ImGui::Spacing();
    
    // Display settings
    ImGui::TextUnformatted("Display Settings:");
    ImGui::Checkbox("Show Component Info", &m_ShowComponentInfo);
    ImGui::TextDisabled("Display material and mass in library");
    ImGui::Spacing();
    
    // Physics preview
    ImGui::Checkbox("Enable Physics Preview", &m_EnablePhysicsPreview);
    Hover(WT::CodesBullet);
    ImGui::TextDisabled("Show physics simulation preview in viewport");
    ImGui::Spacing();
    
    UIStyleHelper::HorizontalSeparator();
    ImGui::Spacing();
    
    // Reset settings
    ImGui::TextUnformatted("Reset:");
    if (UIStyleHelper::ModernButtonSecondary("Reset to Defaults")) {
        m_EnableAutoSave = true;
        m_EnableSnapToGrid = false;
        m_GridSize = 0.1f;
        m_DefaultMaterial = 0;
        m_ShowComponentInfo = true;
        m_EnablePhysicsPreview = false;
    }
    ImGui::TextDisabled("Restore all settings to default values");
    ImGui::Spacing();
    
    UIStyleHelper::HorizontalSeparator();
    ImGui::Spacing();
    
    // Settings info
    ImGui::TextUnformatted("Settings Information:");
    ImGui::TextDisabled("Auto-Save: Saves components automatically");
    ImGui::TextDisabled("Snap to Grid: Aligns components to grid");
    ImGui::TextDisabled("Physics Preview: Shows real-time physics");
}

} // namespace SZM::Workshop3D
