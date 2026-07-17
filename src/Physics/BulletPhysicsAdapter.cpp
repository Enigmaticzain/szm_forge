#include "BulletPhysicsAdapter.hpp"

#if !defined(SZM_HAS_BULLET)
#error "BulletPhysicsAdapter.cpp requires SZM_HAS_BULLET"
#endif

#include <btBulletDynamicsCommon.h>

#include <algorithm>
#include <utility>

namespace SZM::Physics {

BulletPhysicsAdapter::BulletPhysicsAdapter() = default;

BulletPhysicsAdapter::~BulletPhysicsAdapter() {
    Shutdown();
}

bool BulletPhysicsAdapter::Initialize(const PhysVec3& gravity) {
    Shutdown();

    auto* config = new btDefaultCollisionConfiguration();
    auto* dispatcher = new btCollisionDispatcher(config);
    auto* broadphase = new btDbvtBroadphase();
    auto* solver = new btSequentialImpulseConstraintSolver();
    auto* world = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, config);
    world->setGravity(btVector3(gravity.x, gravity.y, gravity.z));

    auto* groundShape = new btStaticPlaneShape(btVector3(0.0f, 1.0f, 0.0f), 0.0f);
    btTransform groundTransform;
    groundTransform.setIdentity();
    auto* groundMotionState = new btDefaultMotionState(groundTransform);
    btRigidBody::btRigidBodyConstructionInfo groundCi(
        0.0f,
        groundMotionState,
        groundShape,
        btVector3(0.0f, 0.0f, 0.0f)
    );
    auto* groundBody = new btRigidBody(groundCi);
    groundBody->setFriction(0.9f);
    world->addRigidBody(groundBody);

    m_Config = config;
    m_Dispatcher = dispatcher;
    m_Broadphase = broadphase;
    m_Solver = solver;
    m_World = world;
    m_GroundShape = groundShape;
    m_GroundMotionState = groundMotionState;
    m_GroundBody = groundBody;
    m_Initialized = true;
    m_NextHandle = 1;
    return true;
}

void BulletPhysicsAdapter::Shutdown() {
    auto* world = static_cast<btDiscreteDynamicsWorld*>(m_World);
    if (world) {
        for (auto& entry : m_Entries) {
            auto* rigid = static_cast<btRigidBody*>(entry.bulletBody);
            if (rigid) {
                world->removeRigidBody(rigid);
                delete rigid;
            }
            delete static_cast<btMotionState*>(entry.motionState);
            delete static_cast<btCollisionShape*>(entry.collisionShape);
        }
    } else {
        for (auto& entry : m_Entries) {
            delete static_cast<btRigidBody*>(entry.bulletBody);
            delete static_cast<btMotionState*>(entry.motionState);
            delete static_cast<btCollisionShape*>(entry.collisionShape);
        }
    }
    m_Entries.clear();

    auto* groundBody = static_cast<btRigidBody*>(m_GroundBody);
    if (world && groundBody) {
        world->removeRigidBody(groundBody);
    }
    delete groundBody;
    delete static_cast<btMotionState*>(m_GroundMotionState);
    delete static_cast<btCollisionShape*>(m_GroundShape);

    delete static_cast<btDiscreteDynamicsWorld*>(m_World);
    delete static_cast<btSequentialImpulseConstraintSolver*>(m_Solver);
    delete static_cast<btBroadphaseInterface*>(m_Broadphase);
    delete static_cast<btCollisionDispatcher*>(m_Dispatcher);
    delete static_cast<btDefaultCollisionConfiguration*>(m_Config);

    m_World = nullptr;
    m_Solver = nullptr;
    m_Broadphase = nullptr;
    m_Dispatcher = nullptr;
    m_Config = nullptr;
    m_GroundShape = nullptr;
    m_GroundMotionState = nullptr;
    m_GroundBody = nullptr;
    m_Initialized = false;
    m_NextHandle = 1;
}

void BulletPhysicsAdapter::SetGravity(const PhysVec3& gravity) {
    auto* world = static_cast<btDiscreteDynamicsWorld*>(m_World);
    if (!world) {
        return;
    }

    world->setGravity(btVector3(gravity.x, gravity.y, gravity.z));
}

int BulletPhysicsAdapter::RegisterBody(
    std::shared_ptr<RigidBody> body,
    const PhysVec3& halfExtents
) {
    if (!m_Initialized || !body) {
        return -1;
    }

    for (const auto& entry : m_Entries) {
        if (entry.body == body) {
            return entry.handle;
        }
    }

    auto* world = static_cast<btDiscreteDynamicsWorld*>(m_World);
    if (!world) {
        return -1;
    }

    const btVector3 bulletHalfExtents(
        std::max(0.01f, halfExtents.x),
        std::max(0.01f, halfExtents.y),
        std::max(0.01f, halfExtents.z)
    );
    auto* shape = new btBoxShape(bulletHalfExtents);

    btTransform start;
    start.setIdentity();
    const PhysVec3 pos = body->GetPosition();
    start.setOrigin(btVector3(pos.x, pos.y, pos.z));

    auto* motionState = new btDefaultMotionState(start);

    float mass = 0.0f;
    if (body->GetState() == KinematicState::Dynamic) {
        mass = std::max(0.0f, body->GetMass());
    }
    btVector3 localInertia(0.0f, 0.0f, 0.0f);
    if (mass > 0.0f) {
        shape->calculateLocalInertia(mass, localInertia);
    }

    btRigidBody::btRigidBodyConstructionInfo ci(mass, motionState, shape, localInertia);
    auto* bulletBody = new btRigidBody(ci);
    const PhysVec3 vel = body->GetLinearVelocity();
    bulletBody->setLinearVelocity(btVector3(vel.x, vel.y, vel.z));
    const PhysVec3 angVel = body->GetAngularVelocity();
    bulletBody->setAngularVelocity(btVector3(angVel.x, angVel.y, angVel.z));
    bulletBody->setDamping(0.04f, 0.1f);
    bulletBody->setFriction(0.85f);
    bulletBody->setRestitution(0.08f);
    bulletBody->setSleepingThresholds(0.05f, 0.05f);

    if (mass > 0.0f) {
        const float minHalfExtent = std::min({halfExtents.x, halfExtents.y, halfExtents.z});
        bulletBody->setCcdMotionThreshold(std::max(0.02f, minHalfExtent * 0.5f));
        bulletBody->setCcdSweptSphereRadius(std::max(0.01f, minHalfExtent * 0.4f));
        bulletBody->setContactProcessingThreshold(std::max(0.1f, minHalfExtent * 6.0f));
    }

    if (body->GetState() == KinematicState::Kinematic) {
        bulletBody->setCollisionFlags(
            bulletBody->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT
        );
        bulletBody->setActivationState(DISABLE_DEACTIVATION);
    }

    world->addRigidBody(bulletBody);
    const int handle = m_NextHandle++;
    m_Entries.push_back(Entry{handle, std::move(body), halfExtents, shape, motionState, bulletBody});
    return handle;
}

bool BulletPhysicsAdapter::UnregisterBody(int handle) {
    auto* entry = FindEntry(handle);
    auto* world = static_cast<btDiscreteDynamicsWorld*>(m_World);
    if (!entry) {
        return false;
    }

    if (world) {
        if (auto* rigid = static_cast<btRigidBody*>(entry->bulletBody)) {
            world->removeRigidBody(rigid);
        }
    }

    delete static_cast<btRigidBody*>(entry->bulletBody);
    delete static_cast<btMotionState*>(entry->motionState);
    delete static_cast<btCollisionShape*>(entry->collisionShape);

    m_Entries.erase(
        std::remove_if(
            m_Entries.begin(),
            m_Entries.end(),
            [handle](const Entry& candidate) { return candidate.handle == handle; }
        ),
        m_Entries.end()
    );
    return true;
}

bool BulletPhysicsAdapter::ApplyCentralForce(int handle, const PhysVec3& force) {
    auto* entry = FindEntry(handle);
    if (!entry || !entry->bulletBody) {
        return false;
    }

    auto* rigid = static_cast<btRigidBody*>(entry->bulletBody);
    rigid->activate(true);
    rigid->applyCentralForce(btVector3(force.x, force.y, force.z));
    return true;
}

void BulletPhysicsAdapter::Step(float dtSeconds, int maxSubSteps) {
    if (!m_Initialized) {
        return;
    }
    auto* world = static_cast<btDiscreteDynamicsWorld*>(m_World);
    if (!world) {
        return;
    }

    const float dt = std::max(1e-4f, dtSeconds);
    world->stepSimulation(dt, std::max(1, maxSubSteps), 1.0f / 120.0f);
    SyncFromBullet();
}

void BulletPhysicsAdapter::SyncFromBullet() {
    for (auto& entry : m_Entries) {
        if (!entry.body || !entry.bulletBody) {
            continue;
        }

        auto* rigid = static_cast<btRigidBody*>(entry.bulletBody);
        btTransform t;
        if (rigid->getMotionState()) {
            rigid->getMotionState()->getWorldTransform(t);
        } else {
            t = rigid->getWorldTransform();
        }

        const btVector3 origin = t.getOrigin();
        entry.body->SetPosition({origin.x(), origin.y(), origin.z()});

        const btVector3 linVel = rigid->getLinearVelocity();
        entry.body->SetLinearVelocity({linVel.x(), linVel.y(), linVel.z()});

        const btVector3 angVel = rigid->getAngularVelocity();
        entry.body->SetAngularVelocity({angVel.x(), angVel.y(), angVel.z()});
    }
}

std::vector<BulletPhysicsAdapter::CollisionContact> BulletPhysicsAdapter::CollectContacts() const {
    std::vector<CollisionContact> contacts;
    auto* world = static_cast<btDiscreteDynamicsWorld*>(m_World);
    if (!world) {
        return contacts;
    }

    auto* dispatcher = world->getDispatcher();
    if (!dispatcher) {
        return contacts;
    }

    const int manifoldCount = dispatcher->getNumManifolds();
    contacts.reserve(static_cast<std::size_t>(manifoldCount));

    for (int manifoldIndex = 0; manifoldIndex < manifoldCount; ++manifoldIndex) {
        const auto* manifold = dispatcher->getManifoldByIndexInternal(manifoldIndex);
        if (!manifold || manifold->getNumContacts() <= 0) {
            continue;
        }

        const int handleA = ResolveHandle(manifold->getBody0());
        const int handleB = ResolveHandle(manifold->getBody1());
        if (handleA < 0 && handleB < 0) {
            continue;
        }

        int selectedIndex = 0;
        btScalar bestDistance = manifold->getContactPoint(0).getDistance();
        for (int contactIndex = 1; contactIndex < manifold->getNumContacts(); ++contactIndex) {
            const auto& candidate = manifold->getContactPoint(contactIndex);
            if (candidate.getDistance() < bestDistance) {
                bestDistance = candidate.getDistance();
                selectedIndex = contactIndex;
            }
        }

        const auto& pt = manifold->getContactPoint(selectedIndex);
        contacts.push_back(CollisionContact{
            handleA,
            handleB,
            {pt.getPositionWorldOnB().x(), pt.getPositionWorldOnB().y(), pt.getPositionWorldOnB().z()},
            {pt.m_normalWorldOnB.x(), pt.m_normalWorldOnB.y(), pt.m_normalWorldOnB.z()},
            pt.getDistance()
        });
    }

    return contacts;
}

BulletPhysicsAdapter::Entry* BulletPhysicsAdapter::FindEntry(int handle) {
    auto it = std::find_if(
        m_Entries.begin(),
        m_Entries.end(),
        [handle](const Entry& entry) { return entry.handle == handle; }
    );
    return it != m_Entries.end() ? &(*it) : nullptr;
}

const BulletPhysicsAdapter::Entry* BulletPhysicsAdapter::FindEntry(int handle) const {
    auto it = std::find_if(
        m_Entries.begin(),
        m_Entries.end(),
        [handle](const Entry& entry) { return entry.handle == handle; }
    );
    return it != m_Entries.end() ? &(*it) : nullptr;
}

int BulletPhysicsAdapter::ResolveHandle(const void* bulletBody) const {
    if (bulletBody == m_GroundBody) {
        return kGroundHandle;
    }

    const auto it = std::find_if(
        m_Entries.begin(),
        m_Entries.end(),
        [bulletBody](const Entry& entry) { return entry.bulletBody == bulletBody; }
    );
    return it != m_Entries.end() ? it->handle : -1;
}

} // namespace SZM::Physics
