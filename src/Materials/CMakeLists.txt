find_package(Threads REQUIRED)

add_library(SZM_Materials_Module STATIC
    MaterialDatabase.cpp
)

target_include_directories(SZM_Materials_Module PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(SZM_Materials_Module PRIVATE Threads::Threads)

target_compile_features(SZM_Materials_Module PUBLIC cxx_std_20)
