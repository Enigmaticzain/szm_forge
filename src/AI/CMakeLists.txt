add_library(SZM_AI_Module STATIC
    # Core AI Components
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
    
    # Vision System - Screen capture and image analysis
    VisionSystem/VisionSystem.cpp
    
    # Observation System - User action capture and learning
    ObservationSystem/ObservationSystem.cpp
    
    # Learning Integration - Wires all AI systems together
    LearningIntegration/LearningIntegration.cpp
    
    # Machine Learning System - Learn from machines and generate new capabilities
    MachineLearningSystem/MachineLearningSystem.cpp
    
    # Assistant Interface - User-facing AI UI
    AssistantInterface/AssistantInterface.cpp
)

target_include_directories(SZM_AI_Module PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(SZM_AI_Module PUBLIC cxx_std_20)

# Link with threading for agent parallelization
find_package(Threads REQUIRED)

# Find OpenCV for Vision System
find_package(OpenCV REQUIRED)
if(OpenCV_FOUND)
    target_include_directories(SZM_AI_Module PRIVATE ${OpenCV_INCLUDE_DIRS})
    target_link_libraries(SZM_AI_Module PRIVATE ${OpenCV_LIBS})
endif()

target_link_libraries(SZM_AI_Module PUBLIC 
    Threads::Threads 
    nlohmann_json::nlohmann_json
)
