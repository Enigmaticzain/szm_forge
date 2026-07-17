add_library(SZM_Geometry_Module STATIC
    ExtrudeOp.cpp
    SolidBody.cpp
    TopologyValidator.cpp
    BRepQueries.cpp
    AABB.cpp
    operations/BooleanOp.cpp
    operations/RevolveOp.cpp
    operations/SweepOp.cpp
    operations/LoftOp.cpp
    Sketch/Sketch2D.cpp
    Sketch/ConstraintSolver2D.cpp
)

target_include_directories(SZM_Geometry_Module PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/operations
    ${CMAKE_CURRENT_SOURCE_DIR}/Sketch
)

target_compile_features(SZM_Geometry_Module PUBLIC cxx_std_20)
