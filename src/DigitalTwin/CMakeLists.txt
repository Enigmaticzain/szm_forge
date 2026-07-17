add_library(SZM_DigitalTwin_Module
    DigitalTwinManager.cpp
    SensorTelemetry.cpp
    ToleranceAnalysis.cpp
)

target_include_directories(SZM_DigitalTwin_Module PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)

target_link_libraries(SZM_DigitalTwin_Module PUBLIC
    SZM_Simulation_Module
    SZM_Geometry_Module
)

target_compile_features(SZM_DigitalTwin_Module PUBLIC cxx_std_20)
