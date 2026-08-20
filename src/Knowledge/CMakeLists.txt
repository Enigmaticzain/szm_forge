add_library(SZM_Knowledge_Module STATIC
    KnowledgeBaseService.cpp
)

target_include_directories(SZM_Knowledge_Module PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(SZM_Knowledge_Module PUBLIC
    nlohmann_json::nlohmann_json
)

target_compile_features(SZM_Knowledge_Module PUBLIC cxx_std_20)
