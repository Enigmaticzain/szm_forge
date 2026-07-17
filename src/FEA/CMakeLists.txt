find_package(Eigen3 3.4 REQUIRED NO_MODULE)
find_package(Threads REQUIRED)

add_library(SZM_FEA_Module STATIC
    CalculiXPipeline.cpp
    LinearSolver.cpp
    VolumetricMesher.cpp
    MeshQuality.cpp
)

target_include_directories(SZM_FEA_Module PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(SZM_FEA_Module PUBLIC
    Eigen3::Eigen
    SZM_Materials_Module
    Threads::Threads
)

target_compile_features(SZM_FEA_Module PUBLIC cxx_std_20)

# Optional: link libgmsh if available
# find_package(gmsh QUIET)
# if(gmsh_FOUND)
#     target_link_libraries(SZM_FEA_Module PRIVATE gmsh)
#     target_compile_definitions(SZM_FEA_Module PRIVATE SZM_HAS_GMSH)
# endif()

# Optional: Intel MKL Eigen backend
option(SZM_USE_MKL "Enable Intel MKL backend for Eigen" OFF)
if(SZM_USE_MKL)
    target_compile_definitions(SZM_FEA_Module PRIVATE EIGEN_USE_MKL_ALL)
endif()
