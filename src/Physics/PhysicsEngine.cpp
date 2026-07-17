#include "PhysicsEngine.hpp"
#include <algorithm>
#include <iostream>

namespace SZM::Physics {

void PhysicsEngine::Init() {
    m_Bodies.clear();
    std::cout << "[SZM Physics] 200-Series Physics Engine initialised.\n";
}

void PhysicsEngine::Tick(double deltaTime) {
    float dt = static_cast<float>(deltaTime);

    for (auto& body : m_Bodies) {
        if (body->GetState() == KinematicState::Dynamic) {
            // Apply continuous forces like gravity
            body->ApplyCentralForce(m_Gravity * body->GetMass());
            
            // Advance position and velocity
            body->Integrate(dt);
        }
    }
}

void PhysicsEngine::AddBody(std::shared_ptr<RigidBody> body) {
    if (body) m_Bodies.push_back(body);
}

void PhysicsEngine::RemoveBody(std::shared_ptr<RigidBody> body) {
    m_Bodies.erase(std::remove(m_Bodies.begin(), m_Bodies.end(), body), m_Bodies.end());
}

} // namespace SZM::Physics