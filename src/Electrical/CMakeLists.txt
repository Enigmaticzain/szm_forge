add_library(SZM_Electrical_Module
    ElectricalDataTypes.hpp
    CircuitSolver.hpp
    CircuitSolver.cpp
    MNASolver.hpp
    MNASolver.cpp
)

target_include_directories(SZM_Electrical_Module PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)

target_link_libraries(SZM_Electrical_Module PUBLIC Eigen3::Eigen)
