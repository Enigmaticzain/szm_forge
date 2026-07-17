#include "Phase5PhysicsIntegration.hpp"

#include "../Input/EventBus.hpp"
#include "../Physics/PhysicsEngine.hpp"
#include "../Simulation/SimulationEngine.hpp"

#include <algorithm>
#include <cmath>
#include <vector>

namespace SZM {

namespace {

constexpr float kProxyThicknessM = 0.01f;
constexpr float kMinVolumeM3 = 1e-6f;
constexpr float kAppliedForceScale = 5e-4f;
constexpr float kMinHalfExtentM = 0.02f;
constexpr const char* kBulletBackendName = "Bullet (codes/code 4)";
constexpr const char* kFallbackBackendName = "Built-in Proxy";

Physics::PhysVec3 ComputeComponentHalfExtents(const SimulationComponent& comp) {
    const float footprintArea = std::max(comp.area, 1.0e-6f);
    const float footprintSide = std::sqrt(footprintArea);
    const float halfSide = std::max(footprintSide * 0.5f, kMinHalfExtentM);
    const float halfThickness = std::max(comp.thickness * 0.5f, kMinHalfExtentM * 0.5f);

    return Physics::PhysVec3{halfSide, halfThickness, halfSide};
}

std::vector<Physics::Triangle> BuildBoxProxyMesh(const Physics::PhysVec3& halfExtents) {
    const Physics::PhysVec3 p000{-halfExtents.x, -halfExtents.y, -halfExtents.z};
    const Physics::PhysVec3 p100{ halfExtents.x, -halfExtents.y, -halfExtents.z};
    const Physics::PhysVec3 p110{ halfExtents.x,  halfExtents.y, -halfExtents.z};
    const Physics::PhysVec3 p010{-halfExtents.x,  halfExtents.y, -halfExtents.z};
    const Physics::PhysVec3 p001{-halfExtents.x, -halfExtents.y,  halfExtents.z};
    const Physics::PhysVec3 p101{ halfExtents.x, -halfExtents.y,  halfExtents.z};
    const Physics::PhysVec3 p111{ halfExtents.x,  halfExtents.y,  halfExtents.z};
    const Physics::PhysVec3 p011{-halfExtents.x,  halfExtents.y,  halfExtents.z};

    return {
        {p001, p101, p111}, {p001, p111, p011}, // front
        {p000, p110, p100}, {p000, p010, p110}, // back
        {p000, p001, p011}, {p000, p011, p010}, // left
        {p100, p110, p111}, {p100, p111, p101}, // right
        {p010, p011, p111}, {p010, p111, p110}, // top
        {p000, p100, p101}, {p000, p101, p001}, // bottom
    };
}

float ComputeKineticEnergyJ(const Physics::RigidBody& body) {
    const auto velocity = body.GetLinearVelocity();
    const float speedSq =
        velocity.x * velocity.x +
        velocity.y * velocity.y +
        velocity.z * velocity.z;
    return 0.5f * body.GetMass() * speedSq;
}

} // namespace

void Phase5PhysicsIntegration::Initialize() {
    m_Bodies.clear();
    m_LastCollisionCount = 0;
    m_LastDeltaTime = 1.0 / 60.0;
    m_UseBullet = false;
#if defined(SZM_HAS_BULLET)
    m_UseBullet = m_BulletAdapter.Initialize(Physics::PhysicsEngine::GetInstance().GetGravity());
#endif
    m_Initialized = true;
}

void Phase5PhysicsIntegration::Shutdown() {
    auto& physics = Physics::PhysicsEngine::GetInstance();
    for (auto& [componentId, tracked] : m_Bodies) {
        (void)componentId;
        if (tracked.bulletHandle >= 0) {
#if defined(SZM_HAS_BULLET)
            m_BulletAdapter.UnregisterBody(tracked.bulletHandle);
#endif
        } else if (tracked.body) {
            physics.RemoveBody(tracked.body);
        }
    }
    m_Bodies.clear();
#if defined(SZM_HAS_BULLET)
    m_BulletAdapter.Shutdown();
#endif
    m_UseBullet = false;
    m_Initialized = false;
}

float Phase5PhysicsIntegration::EstimateComponentVolumeM3(float areaM2) {
    return std::max(areaM2 * kProxyThicknessM, kMinVolumeM3);
}

Physics::PhysVec3 Phase5PhysicsIntegration::ComputeSpawnPosition(uint32_t componentId) {
    const int index = static_cast<int>(componentId > 0 ? componentId - 1 : 0);
    const int col = index % 5;
    const int row = index / 5;
    return Physics::PhysVec3{
        static_cast<float>(col) * 0.6f - 1.2f,
        0.75f + static_cast<float>(row) * 0.35f,
        static_cast<float>(row % 3) * 0.4f
    };
}

std::shared_ptr<Physics::RigidBody>
Phase5PhysicsIntegration::CreateBodyFromComponent(
    uint32_t componentId,
    Physics::PhysVec3* outHalfExtents
) {
    auto* comp = SimulationEngine::GetInstance().GetComponent(componentId);
    if (!comp) {
        return nullptr;
    }

    const Physics::PhysVec3 halfExtents = ComputeComponentHalfExtents(*comp);
    if (outHalfExtents) {
        *outHalfExtents = halfExtents;
    }

    const auto proxyMesh = BuildBoxProxyMesh(halfExtents);

    auto body = Physics::RigidBody::CreateFromMesh(
        proxyMesh,
        std::max(comp->density, 1.0f),
        Physics::KinematicState::Dynamic
    );
    if (!body) {
        return nullptr;
    }

    const bool hasExplicitPosition =
        (std::abs(comp->position.x) > 1e-6) ||
        (std::abs(comp->position.y) > 1e-6) ||
        (std::abs(comp->position.z) > 1e-6);
    const Physics::PhysVec3 spawnPos = hasExplicitPosition
        ? Physics::PhysVec3{
            static_cast<float>(comp->position.x),
            static_cast<float>(comp->position.y),
            static_cast<float>(comp->position.z)}
        : ComputeSpawnPosition(componentId);

    body->SetPosition(spawnPos);
    comp->position = Geometry::Vector3(spawnPos.x, spawnPos.y, spawnPos.z);
    comp->massEstimate = body->GetMass();
    return body;
}

void Phase5PhysicsIntegration::SyncBodySetWithSimulation() {
    auto& sim = SimulationEngine::GetInstance();
    auto& physics = Physics::PhysicsEngine::GetInstance();
    const auto& components = sim.GetComponents();

    std::vector<uint32_t> activeIds;
    activeIds.reserve(components.size());

    for (const auto& comp : components) {
        if (!comp) {
            continue;
        }
        activeIds.push_back(comp->id);

        if (m_Bodies.find(comp->id) != m_Bodies.end()) {
            continue;
        }

        Physics::PhysVec3 halfExtents{0.5f, 0.5f, 0.5f};
        auto body = CreateBodyFromComponent(comp->id, &halfExtents);
        if (!body) {
            continue;
        }

        int bulletHandle = -1;
#if defined(SZM_HAS_BULLET)
        if (m_UseBullet) {
            bulletHandle = m_BulletAdapter.RegisterBody(body, halfExtents);
        }
#endif
        if (bulletHandle < 0) {
            physics.AddBody(body);
        }

        m_Bodies.emplace(comp->id, TrackedBody{comp->id, body, halfExtents, bulletHandle});
    }

    std::vector<uint32_t> staleIds;
    staleIds.reserve(m_Bodies.size());
    for (const auto& [componentId, tracked] : m_Bodies) {
        if (std::find(activeIds.begin(), activeIds.end(), componentId) == activeIds.end()) {
            if (tracked.bulletHandle >= 0) {
#if defined(SZM_HAS_BULLET)
                m_BulletAdapter.UnregisterBody(tracked.bulletHandle);
#endif
            } else if (tracked.body) {
                physics.RemoveBody(tracked.body);
            }
            staleIds.push_back(componentId);
        }
    }
    for (const uint32_t staleId : staleIds) {
        m_Bodies.erase(staleId);
    }
}

void Phase5PhysicsIntegration::PrePhysicsStep(double deltaTime) {
    if (!m_Initialized) {
        return;
    }

    m_LastDeltaTime = std::max(deltaTime, 1.0 / 240.0);
    SyncBodySetWithSimulation();

    auto& sim = SimulationEngine::GetInstance();
    for (auto& [componentId, tracked] : m_Bodies) {
        auto* comp = sim.GetComponent(componentId);
        if (!comp || !tracked.body) {
            continue;
        }

        // Keep engineering force values from destabilizing visualization dynamics.
        const float fx = comp->appliedForce * kAppliedForceScale;
        if (tracked.bulletHandle >= 0) {
#if defined(SZM_HAS_BULLET)
            m_BulletAdapter.ApplyCentralForce(tracked.bulletHandle, {fx, 0.0f, 0.0f});
#endif
        } else {
            tracked.body->ApplyCentralForce({fx, 0.0f, 0.0f});
        }
    }
}

void Phase5PhysicsIntegration::PostPhysicsStep() {
    if (!m_Initialized) {
        return;
    }

#if defined(SZM_HAS_BULLET)
    using CollisionContact = Physics::BulletPhysicsAdapter::CollisionContact;
    std::vector<CollisionContact> contacts;
#endif

#if defined(SZM_HAS_BULLET)
    if (m_UseBullet) {
        m_BulletAdapter.Step(static_cast<float>(m_LastDeltaTime), 4);
        contacts = m_BulletAdapter.CollectContacts();
        m_LastCollisionCount = contacts.size();
    } else {
        m_LastCollisionCount = 0;
    }
#else
    m_LastCollisionCount = 0;
#endif

    auto& sim = SimulationEngine::GetInstance();
    for (auto& [componentId, tracked] : m_Bodies) {
        auto* comp = sim.GetComponent(componentId);
        if (!comp || !tracked.body) {
            continue;
        }

        const auto pos = tracked.body->GetPosition();
        const auto vel = tracked.body->GetLinearVelocity();
        comp->position = Geometry::Vector3(pos.x, pos.y, pos.z);
        comp->linearVelocity = Geometry::Vector3(vel.x, vel.y, vel.z);
        comp->massEstimate = tracked.body->GetMass();
        comp->physicsBackend = m_UseBullet ? kBulletBackendName : kFallbackBackendName;
        comp->activeContactCount = 0;
        comp->maxContactPenetration = 0.0f;
        comp->kineticEnergy = ComputeKineticEnergyJ(*tracked.body);
        comp->isGrounded = false;
    }

#if defined(SZM_HAS_BULLET)
    if (m_UseBullet && !contacts.empty()) {
        std::unordered_map<int, uint32_t> handleToComponentId;
        handleToComponentId.reserve(m_Bodies.size());
        for (const auto& [componentId, tracked] : m_Bodies) {
            if (tracked.bulletHandle >= 0) {
                handleToComponentId.emplace(tracked.bulletHandle, componentId);
            }
        }

        for (const auto& contact : contacts) {
            const float penetration = std::max(0.0f, -contact.separation);
            const auto applyContact = [&](int handle, bool touchesGround) {
                if (handle <= 0) {
                    return;
                }

                const auto it = handleToComponentId.find(handle);
                if (it == handleToComponentId.end()) {
                    return;
                }

                if (auto* comp = sim.GetComponent(it->second)) {
                    ++comp->activeContactCount;
                    comp->maxContactPenetration =
                        std::max(comp->maxContactPenetration, penetration);
                    comp->isGrounded = comp->isGrounded || touchesGround;
                }
            };

            applyContact(contact.handleA, contact.handleB == 0);
            applyContact(contact.handleB, contact.handleA == 0);
        }
    }
#endif

    if (m_LastCollisionCount > 0) {
        EventBus::GetInstance().Publish(
            "Phase5PhysicsCollision",
            std::any(static_cast<uint32_t>(m_LastCollisionCount))
        );
    }
    EventBus::GetInstance().Publish("Phase5PhysicsSync", std::any());
}

} // namespace SZM
