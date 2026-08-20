find_package(OpenGL REQUIRED)
find_package(glm QUIET)
# GLFW is built from source in root CMakeLists.txt

# GLAD include directory
set(GLAD_DIR ${CMAKE_SOURCE_DIR}/external/glad)

add_library(SZM_Graphics_Module STATIC
    GraphicsContext.cpp
    Camera.cpp
    MeshGenerator.cpp
    RenderTarget.cpp
    Heatmap.cpp
    ComponentRenderer.cpp
    VisualizationController.cpp
    Advanced/AdvancedMesh.cpp
    Advanced/EnhancedRenderer.cpp
    Advanced/ShaderManager.cpp
    Advanced/SelectionManager.cpp
    Advanced/PickingSystem.cpp
    Advanced/HighlightingSystem.cpp
    Advanced/InteractiveRenderer.cpp
    Advanced/AssemblyVisualizer.cpp
    Advanced/CollisionVisualizer.cpp
    Advanced/DeformationVisualizer.cpp
    Advanced/SimulationVisualizationData.cpp
    Advanced/StressVisualizer.cpp
    Advanced/ThermalVisualizer.cpp
    Advanced/SimulationResultsRenderer.cpp
    Advanced/ViewportCameraController.cpp
    Advanced/ViewportGizmo.cpp
    Advanced/ViewportAnnotationSystem.cpp
)

# Optional Phase-3 renderer sources (require GLM)
find_path(SZM_GLM_INCLUDE_DIR glm/glm.hpp)
if(SZM_GLM_INCLUDE_DIR)
    target_include_directories(SZM_Graphics_Module PUBLIC ${SZM_GLM_INCLUDE_DIR})
    target_sources(SZM_Graphics_Module PRIVATE
        ShaderProgram.cpp
        Mesh.cpp
        Viewport.cpp
        GraphicsManager.cpp
        Phase3Integration.cpp
    )
    target_compile_definitions(SZM_Graphics_Module PUBLIC SZM_HAS_GLM=1)
else()
    message(STATUS "GLM not found. Skipping optional Phase-3 graphics sources.")
endif()

target_include_directories(SZM_Graphics_Module PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${GLAD_DIR}/include
    /usr/include
)

target_link_libraries(SZM_Graphics_Module PUBLIC
    glfw
    glad
    OpenGL::GL
    entt
)

target_compile_features(SZM_Graphics_Module PUBLIC cxx_std_20)
