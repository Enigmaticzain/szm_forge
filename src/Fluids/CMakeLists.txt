cmake_minimum_required(VERSION 3.10)

# Fluids Module
add_library(SZMFluids STATIC
    FluidProperties.cpp
    FluidProperties.hpp
    FluidStaticsEngine.cpp
    FluidStaticsEngine.hpp
    FluidDynamicsEngine.cpp
    FluidDynamicsEngine.hpp
    BoundaryConditions.cpp
    BoundaryConditions.hpp
)

find_path(SZM_GLM_INCLUDE_DIR glm/glm.hpp)
target_include_directories(SZMFluids PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    $<$<BOOL:${SZM_GLM_INCLUDE_DIR}>:${SZM_GLM_INCLUDE_DIR}>
    /usr/include
)

# GLM is header-only, no link target needed

set_target_properties(SZMFluids PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# Add to parent export
set(SZM_MODULES ${SZM_MODULES} SZMFluids PARENT_SCOPE)
