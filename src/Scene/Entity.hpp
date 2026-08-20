#pragma once

#include <entt/entt.hpp>
#include <string>

namespace SZM::SceneGraph {

    // Define Entity as an alias to entt::entity
    using Entity = entt::entity;

    constexpr Entity NullEntity = entt::null;

} // namespace SZM::SceneGraph
