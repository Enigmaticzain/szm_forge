find_package(Eigen3 3.4 NO_MODULE)

add_library(SZM_Constraint_Solver STATIC
    ConstraintSolver.cpp
)

target_link_libraries(SZM_Constraint_Solver PUBLIC
  $<$<BOOL:${Eigen3_FOUND}>:Eigen3::Eigen>
)

target_include_directories(SZM_Constraint_Solver PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(SZM_Constraint_Solver PUBLIC cxx_std_20)
