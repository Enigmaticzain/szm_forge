add_library(SZM_AI_Module STATIC
    CreatorMode.cpp
    LegendaryCore/LegendaryAIAssistant.cpp
    LegendaryCore/MemoryEngine.cpp
    LegendaryCore/SkillSystem.cpp
    LegendaryCore/AgentFramework.cpp
    LegendaryCore/TrainingPipeline.cpp
    LegendaryCore/ModificationSystem.cpp
    AIOperatingSystem.cpp
    TrainingStudio.cpp
    AgentSystem.cpp
    AIEngine.cpp
)

target_include_directories(SZM_AI_Module PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(SZM_AI_Module PUBLIC cxx_std_20)

# Link with threading for agent parallelization
find_package(Threads REQUIRED)
target_link_libraries(SZM_AI_Module PUBLIC Threads::Threads nlohmann_json::nlohmann_json)
