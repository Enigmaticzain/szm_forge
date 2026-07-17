add_library(SZM_Simulation_Module
    SimulationEngine.cpp
    SimulationEngine.hpp
    SimulationComponent.hpp
    Scheduler.cpp
    CouplingManager.cpp
)

target_include_directories(SZM_Simulation_Module PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)

target_link_libraries(SZM_Simulation_Module PUBLIC
    AppCore_Input_Event_Bus
    SZM_Materials_Module
)

target_compile_features(SZM_Simulation_Module PUBLIC cxx_std_20)
