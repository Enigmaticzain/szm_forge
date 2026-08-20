#include "Scene.hpp"

namespace SZM::SceneGraph {

    Scene::Scene() {
    }

    Entity Scene::CreateEntity(const std::string& name) {
        Entity e = m_Registry.create();
        
        // Every entity automatically gets a Tag and a Transform
        TagComponent tag;
        tag.name = name;
        tag.usdPath = "/Root/" + name + "_" + std::to_string(static_cast<uint32_t>(e));
        AddComponent<TagComponent>(e, tag);
        
        TransformComponent transform;
        AddComponent<TransformComponent>(e, transform);

        return e;
    }

    void Scene::DestroyEntity(Entity entity) {
        m_Registry.destroy(entity);
    }

} // namespace SZM::SceneGraph
