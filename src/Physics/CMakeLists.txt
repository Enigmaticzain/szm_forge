set(SZM_PHYSICS_SOURCES
    RigidBody.cpp
    CollisionDetection.cpp
    PhysicsEngine.cpp
)

if(SZM_ENABLE_BULLET_BACKEND AND TARGET BulletDynamics AND TARGET BulletCollision AND TARGET LinearMath)
    list(APPEND SZM_PHYSICS_SOURCES
        BulletPhysicsAdapter.cpp
    )
endif()

add_library(SZM_Physics_Module STATIC ${SZM_PHYSICS_SOURCES})

target_include_directories(SZM_Physics_Module PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

if(SZM_ENABLE_BULLET_BACKEND AND TARGET BulletDynamics AND TARGET BulletCollision AND TARGET LinearMath)
    if(DEFINED SZM_BULLET_SOURCE_DIR AND EXISTS "${SZM_BULLET_SOURCE_DIR}/src")
        target_include_directories(SZM_Physics_Module PRIVATE
            ${SZM_BULLET_SOURCE_DIR}/src
        )
    endif()
    target_compile_definitions(SZM_Physics_Module PUBLIC SZM_HAS_BULLET=1)
    target_link_libraries(SZM_Physics_Module PUBLIC
        BulletDynamics
        BulletCollision
        LinearMath
    )
endif()

target_compile_features(SZM_Physics_Module PUBLIC cxx_std_20)
