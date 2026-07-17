add_library(SZM_CAD_Module STATIC
    CADImporter.cpp
    CADImporter.hpp
)

target_include_directories(SZM_CAD_Module PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
)

target_compile_features(SZM_CAD_Module PUBLIC cxx_std_20)

# Optional: Mayo CAD kernel
# find_package(Mayo QUIET)
# if(Mayo_FOUND)
#     target_link_libraries(SZM_CAD_Module PUBLIC Mayo::Core)
#     target_compile_definitions(SZM_CAD_Module PUBLIC SZM_HAS_MAYO=1)
# endif()
