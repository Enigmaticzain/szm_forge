add_library(SZM_WebViewer_Module STATIC
    WebViewerServer.hpp
    WebViewerServer.cpp
    ThreeJSRenderer.hpp
    ThreeJSRenderer.cpp
    ReactUIManager.hpp
    ReactUIManager.cpp
    ExportToThreeJS.hpp
    ExportToThreeJS.cpp
)

target_include_directories(SZM_WebViewer_Module PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
)

target_compile_features(SZM_WebViewer_Module PUBLIC cxx_std_20)
target_link_libraries(SZM_WebViewer_Module PUBLIC 
    nlohmann_json::nlohmann_json
    httplib::httplib
    entt
)

# Three.js integration
if(SZM_ENABLE_THREEJS_INTEGRATION)
    target_include_directories(SZM_WebViewer_Module PUBLIC
        ${CMAKE_SOURCE_DIR}/external/three.js
    )
    target_compile_definitions(SZM_WebViewer_Module PUBLIC SZM_HAS_THREEJS=1)
endif()

# React UI integration
set(SZM_REACT_UI_PATH "${CMAKE_SOURCE_DIR}/new ui")
target_compile_definitions(SZM_WebViewer_Module PUBLIC SZM_REACT_UI_PATH="${SZM_REACT_UI_PATH}")

# Webview dependencies (only needed for desktop window)
if(SZM_BUILD_FULL_DESKTOP)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(WEBVIEW_DEPS REQUIRED gtk+-3.0 webkit2gtk-4.1)
    target_include_directories(SZM_WebViewer_Module PUBLIC ${WEBVIEW_DEPS_INCLUDE_DIRS})
    target_link_libraries(SZM_WebViewer_Module PUBLIC ${WEBVIEW_DEPS_LIBRARIES})
endif()

# Optional: WebSocket library (libwebsockets)
# find_package(websockets QUIET)
# if(websockets_FOUND)
#     target_link_libraries(SZM_WebViewer_Module PUBLIC websockets)
#     target_compile_definitions(SZM_WebViewer_Module PUBLIC SZM_HAS_WEBSOCKETS=1)
# endif()
