add_library(SZM_Chemistry_Module
    MolecularDynamics.hpp
    MolecularDynamics.cpp
)

target_include_directories(SZM_Chemistry_Module PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
