cmake_minimum_required(VERSION 3.21)
project(SZM_Forge VERSION 0.2.1 LANGUAGES CXX C)
include(CTest)

# ============================================================
#  Global C++20 Standard
# ============================================================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ============================================================
#  Build Mode
# ============================================================
option(
    SZM_BUILD_FULL_DESKTOP
    "Build full desktop stack (GLFW, Eigen, Graphics/UI/Core modules)"
    OFF
)

option(
    SZM_CHAIN_DESKTOP_AFTER_DEMO
    "In lightweight mode, launch desktop Application after terminal demo (requires full GUI deps in code path)"
    OFF
)

option(
    SZM_ENABLE_BULLET_BACKEND
    "Build and link Bullet backend from external_projects/bullet3"
    OFF
)

# ============================================================
#  Integration Options (from codes folder)
# ============================================================
option(
    SZM_ENABLE_THREEJS_INTEGRATION
    "Enable Three.js integration for web-based 3D visualization"
    ON
)

option(
    SZM_ENABLE_ENTT_ECS
    "Use EnTT Entity Component System for architecture"
    ON
)

option(
    SZM_ENABLE_FASTAPI_SERVER
    "Enable FastAPI-based REST simulation server"
    ON
)

option(
    SZM_BUILD_WEBUI
    "Build React-based web dashboard (requires Node.js + npm)"
    ON
)

option(
    SZM_BUILD_AI_SERVICE
    "Build Python AI service with PyTorch backend (requires Python 3.8+)"
    OFF
)

option(
    SZM_USE_REACT_WEBVIEW
    "Launch React dashboard from new ui/ in embedded webview (Theme 1 + Theme 2)"
    ON
)

# ============================================================
#  Platform Definitions
# ============================================================
if(WIN32)
    add_compile_definitions(SZM_PLATFORM_WINDOWS)
elseif(UNIX AND NOT APPLE)
    add_compile_definitions(SZM_PLATFORM_LINUX)
elseif(APPLE)
    add_compile_definitions(SZM_PLATFORM_MACOS)
endif()

# ============================================================
#  Compiler Warnings
# ============================================================
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    add_compile_options(-Wall -Wextra -Wpedantic)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    add_compile_options(/W4)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

# ============================================================
#  Integrated External Libraries (from codes folder)
# ============================================================

# EnTT — Header-only Entity Component System
if(SZM_ENABLE_ENTT_ECS)
    set(ENTT_DIR ${CMAKE_SOURCE_DIR}/external/entt)
    if(EXISTS "${ENTT_DIR}/single_include/entt.hpp")
        message(STATUS "EnTT found at ${ENTT_DIR}")
        add_library(entt INTERFACE)
        target_include_directories(entt INTERFACE ${ENTT_DIR}/single_include)
        add_compile_definitions(SZM_USE_ENTT=1)
    else()
        message(WARNING "EnTT requested but not found at ${ENTT_DIR}")
    endif()
endif()

# Three.js — JavaScript library (included for reference)
if(SZM_ENABLE_THREEJS_INTEGRATION)
    set(THREEJS_DIR ${CMAKE_SOURCE_DIR}/external/three.js)
    if(EXISTS "${THREEJS_DIR}/build/three.min.js")
        message(STATUS "Three.js found at ${THREEJS_DIR}")
        add_compile_definitions(SZM_USE_THREEJS=1)
    else()
        message(WARNING "Three.js requested but not found at ${THREEJS_DIR}")
    endif()
endif()

# FastAPI — Python framework (referenced for Python service)
if(SZM_ENABLE_FASTAPI_SERVER)
    message(STATUS "FastAPI integration enabled for Python backend")
    add_compile_definitions(SZM_USE_FASTAPI=1)
endif()

# nlohmann/json — JSON library (required for AI module)
include(FetchContent)
FetchContent_Declare(
    nlohmann_json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
)
FetchContent_MakeAvailable(nlohmann_json)

# cpp-httplib - Header-only HTTP server for WebAPI/WebViewer
FetchContent_Declare(
    cpp_httplib
    URL https://github.com/yhirose/cpp-httplib/archive/refs/tags/v0.14.3.tar.gz
)
FetchContent_MakeAvailable(cpp_httplib)

if(SZM_BUILD_FULL_DESKTOP)
    find_package(nlohmann_json QUIET)
    if(NOT nlohmann_json_FOUND)
        include(FetchContent)
        FetchContent_Declare(
            nlohmann_json
            URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
        )
        FetchContent_MakeAvailable(nlohmann_json)
    endif()

    if(SZM_ENABLE_BULLET_BACKEND)
        unset(SZM_BULLET_SOURCE_DIR)
        set(SZM_BULLET_CANDIDATE_DIRS
            ${CMAKE_SOURCE_DIR}/external_projects/bullet3
            ${CMAKE_SOURCE_DIR}/codes/code\ 4
            ${CMAKE_SOURCE_DIR}/code\ 4
        )
        foreach(SZM_BULLET_CANDIDATE ${SZM_BULLET_CANDIDATE_DIRS})
            if(EXISTS "${SZM_BULLET_CANDIDATE}/CMakeLists.txt")
                set(SZM_BULLET_SOURCE_DIR ${SZM_BULLET_CANDIDATE})
                break()
            endif()
        endforeach()

        if(SZM_BULLET_SOURCE_DIR)
            set(BUILD_CPU_DEMOS OFF CACHE BOOL "" FORCE)
            set(BUILD_BULLET2_DEMOS OFF CACHE BOOL "" FORCE)
            set(BUILD_OPENGL3_DEMOS OFF CACHE BOOL "" FORCE)
            set(BUILD_EXTRAS OFF CACHE BOOL "" FORCE)
            set(BUILD_UNIT_TESTS OFF CACHE BOOL "" FORCE)
            set(BUILD_BULLET3 OFF CACHE BOOL "" FORCE)
            set(BUILD_PYBULLET OFF CACHE BOOL "" FORCE)
            set(BUILD_ENET OFF CACHE BOOL "" FORCE)
            set(BUILD_CLSOCKET OFF CACHE BOOL "" FORCE)
            set(USE_GRAPHICAL_BENCHMARK OFF CACHE BOOL "" FORCE)
            set(INSTALL_LIBS OFF CACHE BOOL "" FORCE)
            add_subdirectory(
                ${SZM_BULLET_SOURCE_DIR}
                ${CMAKE_BINARY_DIR}/bullet3
                EXCLUDE_FROM_ALL
            )
            message(STATUS "Bullet backend enabled from ${SZM_BULLET_SOURCE_DIR}")
        else()
            message(WARNING "SZM_ENABLE_BULLET_BACKEND=ON, but no Bullet source bundle was found in external_projects/bullet3, codes/code 4, or code 4.")
        endif()
    endif()

    # ========================================================
    #  External Dependencies (full desktop mode)
    # ========================================================
    # Build GLFW 3.3 from source (avoids system version mismatch)
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
    add_subdirectory(${CMAKE_SOURCE_DIR}/external/glfw EXCLUDE_FROM_ALL)

    # Eigen3 — Linear Algebra (Constraint Solver & Math)
    find_package(Eigen3 3.4 REQUIRED NO_MODULE)

    # OpenGL
    find_package(OpenGL REQUIRED)

    # GLAD — OpenGL function loader
    set(GLAD_DIR ${CMAKE_SOURCE_DIR}/external/glad)
    add_library(glad STATIC ${GLAD_DIR}/src/gl.c)
    target_include_directories(glad PUBLIC ${GLAD_DIR}/include)

    # Dear ImGui — Docking Branch (must be cloned manually)
    # git clone -b docking https://github.com/ocornut/imgui.git external/imgui
    set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/external/imgui)
    add_library(imgui STATIC
        ${IMGUI_DIR}/imgui.cpp
        ${IMGUI_DIR}/imgui_draw.cpp
        ${IMGUI_DIR}/imgui_tables.cpp
        ${IMGUI_DIR}/imgui_widgets.cpp
        ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
        ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
    )
    target_include_directories(imgui PUBLIC
        ${IMGUI_DIR}
        ${IMGUI_DIR}/backends
    )
    target_compile_definitions(imgui PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLAD)
    target_link_libraries(imgui PUBLIC glfw glad OpenGL::GL)

    # ========================================================
    #  Sub-Module Libraries
    # ========================================================
    add_subdirectory(src/Input)
    add_subdirectory(src/Graphics)
    add_subdirectory(src/Data)
    add_subdirectory(src/Geometry)
    add_subdirectory(src/Math)
    add_subdirectory(src/Materials)
    add_subdirectory(src/Simulation)
    add_subdirectory(src/UI)
    add_subdirectory(src/Core)
    add_subdirectory(src/Physics)
    add_subdirectory(src/Fluids)
    add_subdirectory(src/Assembly)
    add_subdirectory(src/FEA)
    add_subdirectory(src/Thermal)
    add_subdirectory(src/Electrical)
    add_subdirectory(src/Chemistry)
    add_subdirectory(src/AI)
    add_subdirectory(src/Workshop3D)
    add_subdirectory(src/WebViewer)
    add_subdirectory(src/CAD)
    add_subdirectory(src/AI/Predictions)
    add_subdirectory(src/WebAPI)
    add_subdirectory(src/DigitalTwin)

    # ========================================================
    #  Main Executable
    # ========================================================
    add_executable(SZM_Forge src/main.cpp)
    target_include_directories(SZM_Forge PRIVATE src)
    target_compile_definitions(SZM_Forge PRIVATE SZM_MODE_DESKTOP=1)
    target_link_libraries(SZM_Forge PRIVATE
        SZM_Core
        SZM_UI_Module
        AppCore_Input_Event_Bus
        SZM_Graphics_Module
        DataCore_SceneGraph
        SZM_Geometry_Module
        SZM_Constraint_Solver
        SZM_Materials_Module
        SZM_Physics_Module
        SZM_Assembly_Module
        SZM_FEA_Module
        SZM_Simulation_Module
        SZM_Thermal_Module
        SZM_Electrical_Module
        SZM_Chemistry_Module
        SZM_AI_Module
        SZM_Workshop3D_Module
        SZM_WebViewer_Module
        SZM_CAD_Module
        SZM_AI_Predictions_Module
        SZM_WebAPI_Module
        SZM_DigitalTwin_Module
        glfw
        glad
        OpenGL::GL
    )
else()
    message(STATUS "Configuring lightweight mechanical demo build.")

    # ========================================================
    #  Lightweight Build (simulation-only, no GUI dependencies)
    # ========================================================
    add_subdirectory(src/Input)
    add_subdirectory(src/Data)
    add_subdirectory(src/Geometry)
    add_subdirectory(src/Assembly)
    add_subdirectory(src/Materials)
    add_subdirectory(src/FEA)
    add_subdirectory(src/Simulation)
    add_subdirectory(src/Fluids)
    add_subdirectory(src/CLI)
    add_subdirectory(src/AI)
    add_subdirectory(src/AI/Predictions)
    add_subdirectory(src/DigitalTwin)
    add_subdirectory(src/WebAPI)
    add_subdirectory(src/WebViewer)
    add_subdirectory(src/Chemistry)

    # ========================================================
    #  React UI Build (if enabled)
    # ========================================================
    if(SZM_BUILD_WEBUI)
        message(STATUS "Building React UI...")
        
        # Check if Node.js is available
        find_program(NPM_COMMAND npm)
        if(NPM_COMMAND)
            message(STATUS "Found npm: ${NPM_COMMAND}")
            
            # Build React UI
            set(REACT_UI_PATH "${CMAKE_SOURCE_DIR}/new ui")
            
            # Run npm install if node_modules doesn't exist
            if(NOT EXISTS "${REACT_UI_PATH}/node_modules")
                message(STATUS "Installing React dependencies...")
                execute_process(
                    COMMAND ${NPM_COMMAND} install
                    WORKING_DIRECTORY ${REACT_UI_PATH}
                    RESULT_VARIABLE NPM_INSTALL_RESULT
                )
                if(NOT NPM_INSTALL_RESULT EQUAL 0)
                    message(WARNING "Failed to install React dependencies")
                else()
                    message(STATUS "React dependencies installed successfully")
                endif()
            endif()
            
            # Build React UI
            message(STATUS "Building React UI...")
            execute_process(
                COMMAND ${NPM_COMMAND} run build
                WORKING_DIRECTORY ${REACT_UI_PATH}
                RESULT_VARIABLE NPM_BUILD_RESULT
            )
            if(NOT NPM_BUILD_RESULT EQUAL 0)
                message(WARNING "Failed to build React UI")
            else()
                message(STATUS "React UI built successfully")
            endif()
        else()
            message(WARNING "npm not found - React UI will not be built")
        endif()
    endif()

    # ========================================================
    #  Main Executable
    # ========================================================
    add_executable(SZM_Forge src/main.cpp)
    target_include_directories(SZM_Forge PRIVATE src)
    target_link_libraries(SZM_Forge PRIVATE
        DataCore_SceneGraph
        SZM_Geometry_Module
        SZM_Assembly_Module
        SZM_Simulation_Module
        SZMFluids
        SZM_CLI_Module
        SZM_AI_Module
        SZM_AI_Predictions_Module
        SZM_DigitalTwin_Module
        SZM_WebAPI_Module
        SZM_WebViewer_Module
        SZM_Chemistry_Module
    )
endif()
