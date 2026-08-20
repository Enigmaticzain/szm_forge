add_library(SZM_WebAPI_Module STATIC
    FastAPIBridge.hpp
    FastAPIBridge.cpp
    TableProjectService.hpp
    TableProjectService.cpp
    StructuralSolveService.hpp
    StructuralSolveService.cpp
    SimulationServer.hpp
    SimulationServer.cpp
)

target_include_directories(SZM_WebAPI_Module PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
)

target_compile_features(SZM_WebAPI_Module PUBLIC cxx_std_20)

find_package(Threads REQUIRED)
target_link_libraries(SZM_WebAPI_Module PUBLIC
    Threads::Threads
    nlohmann_json::nlohmann_json
    httplib::httplib
    SZM_WebViewer_Module
    SZM_Simulation_Module
    SZM_AI_Module
    SZM_FEA_Module
    SZM_Materials_Module
    SZM_Knowledge_Module
)

# FastAPI integration
if(SZM_ENABLE_FASTAPI_SERVER)
    target_compile_definitions(SZM_WebAPI_Module PUBLIC SZM_HAS_FASTAPI=1)
endif()

# Optional: FastAPI C++ bridge (via pybind11)
# find_package(pybind11 QUIET)
# if(pybind11_FOUND)
#     target_link_libraries(SZM_WebAPI_Module PUBLIC pybind11::pybind11)
#     target_compile_definitions(SZM_WebAPI_Module PUBLIC SZM_HAS_FASTAPI_BRIDGE=1)
# endif()
