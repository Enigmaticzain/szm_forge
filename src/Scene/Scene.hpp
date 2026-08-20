#pragma once

#include "Entity.hpp"
#include "Components.hpp"
#include <entt/entt.hpp>
#include <string>
#include <shared_mutex>

namespace SZM::SceneGraph {

    /**
     * @class Scene
     * @brief A lightweight ECS Registry acting as our hierarchical scene graph.
     * Maps perfectly to a USD Stage. Now backed by EnTT.
     */
    class Scene {
    public:
        Scene();
        ~Scene() = default;

        Entity CreateEntity(const std::string& name = "Entity");
        void DestroyEntity(Entity entity);

        template<typename T, typename... Args>
        T& AddComponent(Entity entity, Args&&... args) {
            return m_Registry.emplace<T>(entity, std::forward<Args>(args)...);
        }

        template<typename T>
        T& GetComponent(Entity entity) {
            return m_Registry.get<T>(entity);
        }

        template<typename T>
        bool HasComponent(Entity entity) {
            return m_Registry.any_of<T>(entity);
        }

        template<typename... Components>
        auto View() {
            return m_Registry.view<Components...>();
        }

        entt::registry& GetRegistry() { return m_Registry; }

        // Shared mutex for parallel read access during multithreaded tick
        std::shared_mutex& GetMutex() { return m_Mutex; }

    private:
        entt::registry   m_Registry;
        std::shared_mutex m_Mutex;
    };

} // namespace SZM::SceneGraph
