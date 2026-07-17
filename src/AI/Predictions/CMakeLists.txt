add_library(SZM_AI_Predictions_Module STATIC
    FailurePredictor.cpp
    FailurePredictor.hpp
)

target_include_directories(SZM_AI_Predictions_Module PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
)

target_compile_features(SZM_AI_Predictions_Module PUBLIC cxx_std_20)

# Optional: PyTorch C++ API
# find_package(Torch QUIET)
# if(Torch_FOUND)
#     target_link_libraries(SZM_AI_Predictions_Module PUBLIC ${TORCH_LIBRARIES})
#     target_compile_definitions(SZM_AI_Predictions_Module PUBLIC SZM_HAS_PYTORCH=1)
# endif()
