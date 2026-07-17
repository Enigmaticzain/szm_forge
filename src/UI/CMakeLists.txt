add_library(SZM_UI_Module STATIC
    UIManager.cpp
    UIPanelRegistry.cpp
    UILayoutManager.cpp
    UIThemeManager.cpp
    ClearUIManager.cpp
    UIStyleHelper.cpp
    UITooltipHelper.cpp
    ModernPanelBase.cpp
    TabManager.cpp
    EasyTabBar.cpp
    panels/ConsolePanel.cpp
    panels/ModernConsolePanel.cpp
    panels/SceneOutliner.cpp
    panels/PropertiesPanel.cpp
    panels/ModernPropertiesPanel.cpp
    panels/SimulationPanel.cpp
    panels/ModernSimulationPanel.cpp
    panels/SimulationDashboardPanel.cpp
    panels/SimpleExamplePanel.cpp
    panels/SimulationVisualizerPanel.cpp
    panels/ComponentBuilderPanel.cpp
    panels/ToolbarPanel.cpp
    panels/DashboardPanel.cpp
    panels/ModernDashboardPanel.cpp
    panels/ViewportPanel.cpp
    panels/InteractiveViewportPanel.cpp
    panels/SelectionPropertiesPanel.cpp
    panels/VisualizationDebugPanel.cpp
    panels/SimulationPlaybackPanel.cpp
    panels/AnalysisResultsPanel.cpp
    panels/HeatmapLegendPanel.cpp
    panels/PerformanceMonitorPanel.cpp
    panels/AdvancedViewportPanel.cpp
    panels/ViewportSettingsPanel.cpp
    panels/UIComponentShowcasePanel.cpp
    panels/TrainingTabUI.cpp
)

target_include_directories(SZM_UI_Module PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/external/imgui
    ${CMAKE_SOURCE_DIR}/external/imgui/backends
)

target_link_libraries(SZM_UI_Module PUBLIC
    imgui
    SZM_Simulation_Module
    SZM_Materials_Module
    SZM_Graphics_Module
    nlohmann_json::nlohmann_json
)

target_compile_features(SZM_UI_Module PUBLIC cxx_std_20)
