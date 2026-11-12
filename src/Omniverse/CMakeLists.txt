find_package(Threads REQUIRED)

add_library(SZM_Omniverse_Module STATIC
    OmniverseConnector.cpp
    OmniverseConnector.hpp
    OmniverseLiveSync.cpp
    LiveSyncSession.cpp
    ConflictResolver.cpp
    ChangeBuffer.cpp
    NucleusConnector.cpp
)

target_include_directories(SZM_Omniverse_Module PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(SZM_Omniverse_Module PUBLIC Threads::Threads)
target_compile_features(SZM_Omniverse_Module PUBLIC cxx_std_20)

# Dummy linking for now, in a real environment this would link to Omniverse Client Library
# target_link_libraries(SZM_Omniverse_Module PUBLIC omniclient)
