add_library(SZM_Thermal_Module
    ThermalDataTypes.hpp
    ThermalSolver.hpp
    ThermalSolver.cpp
)

target_include_directories(SZM_Thermal_Module PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(SZM_Thermal_Module PUBLIC SZM_Geometry_Module)
