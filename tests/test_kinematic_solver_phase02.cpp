#include <iostream>
#include <cmath>
#include <Eigen/Dense>
#include "Assembly/KinematicSolver.hpp"
#include "Assembly/KinematicTypes.hpp"
#include "Assembly/Mates.hpp"

using namespace SZM::Assembly;

int main() {
    std::cout << "=== Phase 02 — KinematicSolver Verification ===\n";

    KinematicSolver solver;
    auto body = std::make_shared<RigidBodyState>(1, false);
    body->Transform = Eigen::Isometry3d::Identity();
    solver.AddBody(body);

    auto bodyB = std::make_shared<RigidBodyState>(2, true);
    bodyB->Transform = Eigen::Isometry3d::Identity();
    solver.AddBody(bodyB);

    // Add a concrete planar coincident mate (constraint)
    auto mate = std::make_shared<CoincidentPlanarMate>(
        1, 2,
        Eigen::Vector3d::Zero(), Eigen::Vector3d::UnitZ(),
        Eigen::Vector3d::Zero(), Eigen::Vector3d::UnitZ(),
        MateAlignment::Aligned,
        body.get(), bodyB.get());
    solver.AddMate(mate);

    bool converged = solver.Solve(100, 1e-6);

    std::cout << "Converged: " << (converged ? "YES" : "NO") << std::endl;
    std::cout << "Active DOF: " << solver.GetSystemDOF() << std::endl;

    if (!converged) {
        std::cout << "Note: Solver did not converge — unexpected for grounded planar mate.\n";
    }
    std::cout << "=== Phase 02 Verification Complete ===\n";
    return 0;
}
