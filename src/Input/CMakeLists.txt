add_library(AppCore_Input_Event_Bus STATIC
    InputManager.cpp
    SpaceMouseDriver.cpp
)

target_include_directories(AppCore_Input_Event_Bus PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(AppCore_Input_Event_Bus PUBLIC cxx_std_20)
