add_library(SZM_CLI_Module STATIC
    InteractiveMenu.cpp
    InteractiveMenu.hpp
)

target_include_directories(SZM_CLI_Module PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(SZM_CLI_Module PUBLIC
    SZM_Simulation_Module
)

target_compile_features(SZM_CLI_Module PRIVATE cxx_std_20)
