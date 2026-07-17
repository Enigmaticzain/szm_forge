cmake_minimum_required(VERSION 3.21)

# Workshop3D Module
# Provides 3D CAD part library, assembly management, and visualization

set(SZM_WORKSHOP3D_SOURCES
    PartLibrary.hpp
    PartLibrary.cpp
    AssemblyManager.hpp
    AssemblyManager.cpp
    CADImporter.hpp
    CADImporter.cpp
    WorkshopManager.hpp
    WorkshopManager.cpp
    WorkshopPanels.hpp
    WorkshopPanels.cpp
    ComponentBrowserPanel.hpp
    ComponentBrowserPanel.cpp
    ComponentWeightTablePanel.hpp
    ComponentWeightTablePanel.cpp
)

# Create Workshop3D module library
add_library(SZM_Workshop3D_Module OBJECT ${SZM_WORKSHOP3D_SOURCES})

target_include_directories(SZM_Workshop3D_Module 
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src
)

# Link dependencies
target_link_libraries(SZM_Workshop3D_Module 
    PUBLIC
        DataCore_SceneGraph
        SZM_FEA_Module
        SZM_Geometry_Module
        SZM_Graphics_Module
        SZM_Simulation_Module
        SZM_UI_Module
)

# Enable C++20
set_target_properties(SZM_Workshop3D_Module PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

# Compiler warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(SZM_Workshop3D_Module PRIVATE -Wall -Wextra -Wpedantic)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    target_compile_options(SZM_Workshop3D_Module PRIVATE /W4)
endif()

message(STATUS "[Workshop3D] Module configured")
