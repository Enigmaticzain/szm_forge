add_library(SZM_Omniverse_Module STATIC
    OmniverseConnector.cpp
    OmniverseConnector.hpp
)

target_include_directories(SZM_Omniverse_Module PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
)

# Dummy linking for now, in a real environment this would link to Omniverse Client Library
# target_link_libraries(SZM_Omniverse_Module PUBLIC omniclient)
