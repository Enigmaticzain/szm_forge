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
target_link_libraries(SZM_Geometry_Module PUBLIC nlohmann_json::nlohmann_json)

find_package(OpenCASCADE QUIET)
if(OpenCASCADE_FOUND)
    target_sources(SZM_Geometry_Module PRIVATE ShapeLibrary.cpp)
    target_include_directories(SZM_Geometry_Module PUBLIC ${OpenCASCADE_INCLUDE_DIR})
    target_link_libraries(SZM_Geometry_Module PUBLIC ${OpenCASCADE_LIBRARIES})
    target_compile_definitions(SZM_Geometry_Module PUBLIC SZM_HAS_OPENCASCADE=1)
endif()
