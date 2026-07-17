cmake_minimum_required(VERSION 3.20)

# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
# LEGENDARY CORE MODULE
# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

set(LEGENDARY_CORE_SOURCES
    MemoryEngine.cpp
    SkillSystem.cpp
    AgentFramework.cpp
    TrainingPipeline.cpp
    ModificationSystem.cpp
    LegendaryAIAssistant.cpp
)

set(LEGENDARY_CORE_HEADERS
    MemoryEngine.hpp
    SkillSystem.hpp
    AgentFramework.hpp
    TrainingPipeline.hpp
    ModificationSystem.hpp
    LegendaryAIAssistant.hpp
)

add_library(SZM_LegendaryCore_Module STATIC
    ${LEGENDARY_CORE_SOURCES}
    ${LEGENDARY_CORE_HEADERS}
)

target_include_directories(SZM_LegendaryCore_Module PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
)

# Dependencies
target_link_libraries(SZM_LegendaryCore_Module PUBLIC
    nlohmann_json::nlohmann_json
)

# C++20 Standard
target_compile_features(SZM_LegendaryCore_Module PUBLIC cxx_std_20)

# Compiler Flags
if(MSVC)
    target_compile_options(SZM_LegendaryCore_Module PRIVATE /W4)
else()
    target_compile_options(SZM_LegendaryCore_Module PRIVATE -Wall -Wextra -Wpedantic)
endif()

message(STATUS "✓ SZM_LegendaryCore_Module configured")
