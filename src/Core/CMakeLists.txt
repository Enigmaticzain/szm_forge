add_library(SZM_Core STATIC
    Application.cpp
    ApplicationModule.cpp
    Phase5PhysicsIntegration.cpp
)

target_include_directories(SZM_Core PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/external/imgui
    ${CMAKE_SOURCE_DIR}/external/imgui/backends
    ${CMAKE_SOURCE_DIR}/external/glad/include
)

target_link_libraries(SZM_Core PUBLIC
    AppCore_Input_Event_Bus
    glfw
    glad
    SZM_WebViewer_Module
    SZM_WebAPI_Module
    SZM_Simulation_Module
    SZM_AI_Module
)

find_package(PkgConfig REQUIRED)
pkg_check_modules(WEBVIEW_DEPS REQUIRED gtk+-3.0 webkit2gtk-4.1)
target_include_directories(SZM_Core PUBLIC ${WEBVIEW_DEPS_INCLUDE_DIRS})
target_link_libraries(SZM_Core PUBLIC ${WEBVIEW_DEPS_LIBRARIES})

if(SZM_ENABLE_BULLET_BACKEND AND TARGET BulletDynamics AND TARGET BulletCollision AND TARGET LinearMath)
    target_compile_definitions(SZM_Core PRIVATE SZM_HAS_BULLET=1)
endif()

if(SZM_USE_REACT_WEBVIEW)
    target_compile_definitions(SZM_Core PRIVATE SZM_USE_REACT_WEBVIEW=1)
endif()

target_compile_features(SZM_Core PUBLIC cxx_std_20)
