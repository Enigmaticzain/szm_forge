add_library(DataCore_SceneGraph STATIC
    SceneGraph.cpp
)

target_include_directories(DataCore_SceneGraph PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(DataCore_SceneGraph PUBLIC cxx_std_20)
