find_package(Eigen3 3.4 NO_MODULE)
find_package(Threads REQUIRED)

add_library(SZM_Assembly_Module STATIC
    AssemblyGraph.cpp
    KinematicSolver.cpp
    InterferenceCheck.cpp
)

target_include_directories(SZM_Assembly_Module PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(SZM_Assembly_Module PUBLIC
    DataCore_SceneGraph
    SZM_Geometry_Module
    Threads::Threads
)
if(Eigen3_FOUND)
    target_link_libraries(SZM_Assembly_Module PUBLIC Eigen3::Eigen)
endif()


target_compile_features(SZM_Assembly_Module PUBLIC cxx_std_20)
