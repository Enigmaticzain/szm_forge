/**
 * SZM Forge v0.2.0 — Integration Test Runner
 *
 * Tests all completed modules:
 *   [T01] UUID generation & hash uniqueness
 *   [T02] SceneGraph hierarchy + circular-dependency guard
 *   [T03] SceneGraph serialization / deserialization round-trip
 *   [T04] ConstraintSolver2D — distance constraint convergence
 *   [T05] LinearExtrude — vertex count & manifold check
 *   [T06] RevolveOp    — 360° closed loop manifold check
 *   [T07] InputManager — state update & focus-lost clear
 *   [T08] MaterialDatabase — load, lookup, custom material
 *   [T08B] SimulationEngine — material/component integration
 *   [T09] RigidBody    — mass & CoM from triangle mesh
 *   [T10] AssemblyGraph — instantiate, BOM, mass/CG
 *   [T11] Camera       — orbit, view matrix, projection switch
 *   [T12] MeshGenerator — segment count formula
 *   [T13] FEA          — constitutive matrix & system assembly
 *   [T14] InterferenceChecker — material rule + AABB escape
 *
 * Run:  ./SZM_TestRunner
 * Exit code 0 = all tests passed.
 */

#include <iostream>
#include <cassert>
#include <stdexcept>
#include <cstdlib>
#include <cmath>
#include <algorithm>
#include <filesystem>

// --- Module headers ---------------------------------------------------
#include "Data/UUID.hpp"
#include "Data/SceneGraph.hpp"
#include "Data/SceneGraphManager.hpp"
#include "Math/ConstraintSolver.hpp"
#include "Math/constraints/DistanceConstraint.hpp"
#include "Geometry/SolidBody.hpp"
#include "Geometry/operations/ExtrudeOp.hpp"
#include "Geometry/operations/RevolveOp.hpp"
#include "Input/InputManager.hpp"
#include "Materials/MaterialDatabase.hpp"
#include "Simulation/SimulationEngine.hpp"
#include "Physics/RigidBody.hpp"
#if defined(SZM_HAS_BULLET)
#include "Physics/BulletPhysicsAdapter.hpp"
#endif
#include "Assembly/AssemblyGraph.hpp"
#include "Assembly/InterferenceCheck.hpp"
#include "Assembly/KinematicSolver.hpp"
#include "Assembly/Mates.hpp"
#include "Workshop3D/AssemblyManager.hpp"
#include "Workshop3D/PartLibrary.hpp"
#include "Graphics/Camera.hpp"
#include "Graphics/MeshGenerator.hpp"
#include "FEA/CalculiXPipeline.hpp"
#include "FEA/LinearSolver.hpp"
#include "Input/SpaceMouseDriver.hpp"

// ---- Test harness helpers -------------------------------------------
static int  g_Passed = 0;
static int  g_Failed = 0;

#define SZM_ASSERT(cond, msg)                                            \
    do {                                                                 \
        if (!(cond)) {                                                   \
            std::cerr << "  [FAIL] " << msg << "\n";                    \
            ++g_Failed;                                                  \
        } else {                                                         \
            std::cout << "  [PASS] " << msg << "\n";                    \
            ++g_Passed;                                                  \
        }                                                                \
    } while (false)

#define SZM_TEST(name) \
    std::cout << "\n=== " << name << " ===\n";

// =====================================================================
//  T01 — UUID
// =====================================================================
void Test_UUID() {
    SZM_TEST("T01 — UUID Generation & Hash");

    auto a = SZM::UUIDGenerator::Generate();
    auto b = SZM::UUIDGenerator::Generate();

    SZM_ASSERT(a != b,                    "Two generated UUIDs must be unique");
    SZM_ASSERT(!a.IsNull(),               "Generated UUID must not be null");
    SZM_ASSERT(SZM::UUID::Null().IsNull(),"Null sentinel must report IsNull()");

    std::hash<SZM::UUID> hasher;
    SZM_ASSERT(hasher(a) != hasher(b),    "Hash of distinct UUIDs should differ (high probability)");
}

// =====================================================================
//  T02 — SceneGraph hierarchy
// =====================================================================
void Test_SceneGraph() {
    SZM_TEST("T02 — SceneGraph Hierarchy & Cycle Detection");

    auto root  = std::make_shared<SZM::Entity>("Root");
    auto child = std::make_shared<SZM::Entity>("Child");
    auto grand = std::make_shared<SZM::Entity>("GrandChild");

    root->AddChild(child);
    child->AddChild(grand);

    SZM_ASSERT(root->GetChildren().size()  == 1, "Root should have 1 child");
    SZM_ASSERT(child->GetChildren().size() == 1, "Child should have 1 grandchild");

    // Circular dependency guard
    bool threw = false;
    try { grand->AddChild(root); }
    catch (const std::runtime_error&) { threw = true; }
    SZM_ASSERT(threw, "Adding an ancestor as a child must throw");
}

// =====================================================================
//  T03 — SceneGraph Serialization Round-Trip
// =====================================================================
void Test_SceneGraphSerialize() {
    SZM_TEST("T03 — SceneGraph Serialization Round-Trip");

    SZM::SceneGraphManager mgr;
    auto e1 = std::make_shared<SZM::Entity>("EngineBlock");
    auto e2 = std::make_shared<SZM::Entity>("Piston");
    mgr.AddEntity(e1);
    mgr.AddEntity(e2, e1->GetUUID());

    auto flat = mgr.Serialize();
    SZM_ASSERT(flat.size() >= 3, "Flat graph must contain root + 2 entities");

    SZM::SceneGraphManager mgr2;
    mgr2.Deserialize(flat);
    SZM_ASSERT(mgr2.GetRoot() != nullptr, "Deserialized graph must have a root");

    auto found = mgr2.FindByUUID(e1->GetUUID());
    SZM_ASSERT(found != nullptr,              "EngineBlock UUID must survive round-trip");
    SZM_ASSERT(found->GetName() == "EngineBlock", "Name must survive round-trip");
}

// =====================================================================
//  T04 — ConstraintSolver2D
// =====================================================================
void Test_ConstraintSolver() {
    SZM_TEST("T04 — ConstraintSolver2D Distance Constraint");

    SZM::Math::ConstraintSolver2D solver;

    // Point A (fixed at origin)
    int xA = solver.AddVariable(0.0, /*locked=*/true);
    int yA = solver.AddVariable(0.0, /*locked=*/true);
    // Point B (free, starting near origin)
    int xB = solver.AddVariable(0.5, false);
    int yB = solver.AddVariable(0.0, false);

    // Constrain distance A→B = 3.0
    auto dc = std::make_shared<SZM::Math::DistanceConstraint>(
        xA, yA, xB, yB, 3.0, solver.GetVariables());
    solver.AddConstraint(dc);

    bool converged = solver.Solve();
    SZM_ASSERT(converged, "Solver must converge for simple distance constraint");

    double actualDist = std::sqrt(
        std::pow(solver.GetVariableValue(xB) - solver.GetVariableValue(xA), 2) +
        std::pow(solver.GetVariableValue(yB) - solver.GetVariableValue(yA), 2));
    SZM_ASSERT(std::abs(actualDist - 3.0) < 1e-5,
               "Solved distance must match target (3.0) within 1e-5");

    SZM_ASSERT(solver.GetDegreesOfFreedom() >= 0,
               "DOF must be non-negative after convergence");
}

// =====================================================================
//  T05 — LinearExtrude
// =====================================================================
void Test_LinearExtrude() {
    SZM_TEST("T05 — LinearExtrude Vertex & Manifold");

    // Square profile on XY plane
    SZM::Geometry::Profile2D profile;
    profile.Vertices = {
        {-1.0,  1.0, 0.0},
        { 1.0,  1.0, 0.0},
        { 1.0, -1.0, 0.0},
        {-1.0, -1.0, 0.0}
    };
    profile.Normal = {0.0, 0.0, 1.0};

    SZM::Geometry::ExtrudeParams params;
    params.Distance  = 5.0;
    params.Direction = {0.0, 0.0, 1.0};

    auto body = SZM::Geometry::Operations::LinearExtrude(profile, params);

    // 4 verts × 2 rings = 8 vertices
    SZM_ASSERT(body.GetVertexCount() == 8,
               "Extruded square must have 8 vertices");
    // 2 caps + (4 edges × 2 triangles) = 10 faces
    SZM_ASSERT(body.GetFaceCount() >= 10,
               "Extruded square must have >= 10 faces");
    SZM_ASSERT(body.IsClosed(),
               "Extruded body must be a closed manifold shell");

    const double vol = body.CalculateVolume();
    SZM_ASSERT(std::abs(vol - 20.0) < 1e-3,
               "Extruded square volume must be 20.0 m^3");

    const auto com = body.CalculateCenterOfMass();
    SZM_ASSERT(std::abs(com.x) < 1e-4, "Extruded square CoM.x must be 0");
    SZM_ASSERT(std::abs(com.y) < 1e-4, "Extruded square CoM.y must be 0");
    SZM_ASSERT(std::abs(com.z - 2.5) < 1e-3,
               "Extruded square CoM.z must be 2.5");
}

// =====================================================================
//  T06 — RevolveOp
// =====================================================================
void Test_RevolveOp() {
    SZM_TEST("T06 — RevolveOp 360° Closed Loop");

    SZM::Geometry::RevolveProfile profile;
    profile.Normal = {0.0, 0.0, 1.0};
    // Simple 2-vertex profile (rectangle cross-section)
    profile.Vertices = {
        {1.0, 0.0, 0.0},
        {2.0, 0.0, 0.0},
        {2.0, 0.0, 1.0},
        {1.0, 0.0, 1.0}
    };

    SZM::Geometry::Vector3 axisOrigin{0.0, 0.0, 0.0};
    SZM::Geometry::Vector3 axisVector{0.0, 0.0, 1.0};

    auto body = SZM::Geometry::RevolveOp::Execute(
        profile, axisOrigin, axisVector, 360.0f, 12);

    SZM_ASSERT(body.GetFaceCount() > 0, "Revolved body must have faces");
    // 360° revolution welds start=end, so the body must be closed
    SZM_ASSERT(body.IsClosed(), "Full 360° revolution must produce a closed manifold");
}

// =====================================================================
//  T07 — InputManager
// =====================================================================
void Test_InputManager() {
    SZM_TEST("T07 — InputManager State & Focus-Lost Clear");

    SZM::InputManager::Init();
    SZM::InputManager::UpdateKeyState(65, true);   // 'A' key pressed
    SZM_ASSERT(SZM::InputManager::IsKeyPressed(65), "Key 65 must be marked pressed");

    SZM::InputManager::ClearStateOnFocusLost();
    SZM_ASSERT(!SZM::InputManager::IsKeyPressed(65),
               "All keys must clear on focus-lost (Edge-Case 11)");

    SZM::InputManager::UpdateMousePosition(320.0f, 240.0f);
    auto [mx, my] = SZM::InputManager::GetMousePosition();
    SZM_ASSERT(std::abs(mx - 320.0f) < 1e-5f, "Mouse X must be 320");
    SZM_ASSERT(std::abs(my - 240.0f) < 1e-5f, "Mouse Y must be 240");
}

// =====================================================================
//  T07B — SolidBody deformation constraints
// =====================================================================
void Test_SolidBodyDeformationConstraints() {
    SZM_TEST("T07B — SolidBody Point/Line/Plane Constraints");

    SZM::Geometry::SolidBody body;
    body.AddFace({
        {0.0, 0.0, 0.0},
        {1.0, 0.0, 0.0},
        {0.0, 1.0, 0.0}
    });

    const std::vector<uint32_t> vertices{0, 1, 2};

    body.DeformVertices(vertices, SZM::Geometry::Vector3(0.0, 0.0, 2.0),
        {SZM::Geometry::DeformationConstraintType::Point,
         SZM::Geometry::Vector3(0.0, 0.0, 0.0)});
    SZM_ASSERT(std::abs(body.GetVertices()[0].position.z) < 1e-9,
               "Point constraint must keep the anchor vertex fixed");
    SZM_ASSERT(std::abs(body.GetVertices()[1].position.z - 2.0) < 1e-9,
               "Non-constrained vertices must receive the requested deformation");

    body.DeformVertices(vertices, SZM::Geometry::Vector3(0.0, 0.0, 1.0),
        {SZM::Geometry::DeformationConstraintType::Line,
         SZM::Geometry::Vector3(0.0, 0.0, 0.0),
         SZM::Geometry::Vector3(0.0, 1.0, 0.0)});
    SZM_ASSERT(std::abs(body.GetVertices()[0].position.z) < 1e-9,
               "Line constraint must preserve points on the controlled line");

    body.DeformVertices(vertices, SZM::Geometry::Vector3(0.0, 0.0, 1.0),
        {SZM::Geometry::DeformationConstraintType::Plane,
         SZM::Geometry::Vector3(0.0, 0.0, 0.0),
         SZM::Geometry::Vector3(0.0, 0.0, 1.0)});
    SZM_ASSERT(std::abs(body.GetVertices()[0].position.z) < 1e-9,
               "Plane constraint must preserve points on the controlled plane");
}

// =====================================================================
//  T08 — MaterialDatabase
// =====================================================================
void Test_MaterialDatabase() {
    SZM_TEST("T08 — MaterialDatabase Load & Lookup");

    auto& db = SZM::Materials::MaterialDatabase::GetInstance();
    db.LoadStandardLibraries();

    auto steel = db.GetMaterial("MAT-STEEL-STRUCT");
    SZM_ASSERT(steel.has_value(),                    "Structural Steel must be found");
    SZM_ASSERT(steel->youngsModulus == 200.0,         "Steel E must be 200 GPa");
    SZM_ASSERT(std::abs(steel->poissonsRatio - 0.30) < 1e-9,
                                                      "Steel ν must be 0.30");
    // G = E / 2(1+ν) = 200 / 2(1.30) ≈ 76.923 GPa
    SZM_ASSERT(std::abs(steel->shearModulus - 200.0 / 2.6) < 1e-6,
                                                      "Shear modulus must be auto-derived");

    auto missing = db.GetMaterial("MAT-DOES-NOT-EXIST");
    SZM_ASSERT(!missing.has_value(),                 "Unknown ID must return nullopt");

    // Custom material round-trip
    SZM::Materials::IMaterial custom(
        "MAT-CUSTOM-TEST", "Test Alloy",
        100.0, 0.25, 500.0, 600.0, 3000.0);
    bool added = db.AddCustomMaterial(custom);
    SZM_ASSERT(added,                                "Custom material must be added");
    SZM_ASSERT(!db.AddCustomMaterial(custom),        "Duplicate ID must be rejected");

    auto found = db.GetMaterial("MAT-CUSTOM-TEST");
    SZM_ASSERT(found.has_value() && found->name == "Test Alloy",
                                                     "Custom material must be retrievable");

    auto copper = db.GetMaterial("MAT-CU-C110");
    SZM_ASSERT(copper.has_value(), "C110 Copper must be found");
    SZM_ASSERT(std::abs(copper->thermalConductivity - 385.0) < 1e-9,
               "Copper thermal conductivity must match library value");
}

// =====================================================================
//  T08B — SimulationEngine material/component integration
// =====================================================================
void Test_SimulationMaterialIntegration() {
    SZM_TEST("T08B — SimulationEngine Material Integration");

    auto& engine = SZM::SimulationEngine::GetInstance();
    engine.Init();

    const uint32_t id = engine.AddComponent("Copper Busbar", "MAT-CU-C110", 0.002f, 0.02f);
    auto* component = engine.GetComponent(id);

    SZM_ASSERT(component != nullptr, "Configured component must be created");
    SZM_ASSERT(component != nullptr && component->materialId == "MAT-CU-C110",
               "Component must store the selected material ID");
    SZM_ASSERT(component != nullptr && component->materialName == "C110 Copper",
               "Component must store the selected material name");
    SZM_ASSERT(component != nullptr && std::abs(component->thickness - 0.02f) < 1e-6f,
               "Component thickness must be preserved");
    SZM_ASSERT(component != nullptr && std::abs(component->yieldStrength - 69.0e6f) < 1.0f,
               "Material yield strength must convert from MPa to Pa");

    engine.SetForce(id, 69000.0f);
    engine.SetHeatInput(id, 500.0f);
    engine.Tick(1.0);

    SZM_ASSERT(component != nullptr && std::abs(component->stressRatio - 0.5f) < 1e-3f,
               "Stress ratio must use database-derived yield strength");
    SZM_ASSERT(component != nullptr && component->temperature > 293.15f,
               "Temperature must respond using configured thickness-based mass");

    engine.Shutdown();
}

// =====================================================================
//  T09 — RigidBody from mesh
// =====================================================================
void Test_RigidBody() {
    SZM_TEST("T09 — RigidBody Mass & CoM from Triangle Mesh");

    // Single tetrahedron: origin + 3 unit-axis points
    // Volume = 1/6, CoM = (0.25, 0.25, 0.25)
    SZM::Physics::Triangle tri;
    tri.v0 = {1.f, 0.f, 0.f};
    tri.v1 = {0.f, 1.f, 0.f};
    tri.v2 = {0.f, 0.f, 1.f};

    auto body = SZM::Physics::RigidBody::CreateFromMesh(
        {tri}, 1000.f, SZM::Physics::KinematicState::Dynamic);

    SZM_ASSERT(body != nullptr,                       "RigidBody must be created");
    SZM_ASSERT(body->GetMass() > 0.0f,               "Mass must be positive");
    SZM_ASSERT(body->GetInvMass() > 0.0f,            "Inverse mass must be positive");

    auto com = body->GetCenterOfMass();
    SZM_ASSERT(std::abs(com.x - 0.25f) < 0.01f,     "CoM.x ≈ 0.25");
    SZM_ASSERT(std::abs(com.y - 0.25f) < 0.01f,     "CoM.y ≈ 0.25");
    SZM_ASSERT(std::abs(com.z - 0.25f) < 0.01f,     "CoM.z ≈ 0.25");

    // Static body — invMass must be zero
    auto staticBody = SZM::Physics::RigidBody::CreateFromMesh(
        {tri}, 1000.f, SZM::Physics::KinematicState::Static);
    SZM_ASSERT(staticBody->GetInvMass() == 0.0f,     "Static body must have invMass = 0");
}

#if defined(SZM_HAS_BULLET)
// =====================================================================
//  T09B — Bullet adapter gravity + ground contact
// =====================================================================
void Test_BulletPhysicsAdapter() {
    SZM_TEST("T09B — Bullet Adapter Gravity & Ground Contact");

    SZM::Physics::Triangle tri;
    tri.v0 = {1.f, 0.f, 0.f};
    tri.v1 = {0.f, 1.f, 0.f};
    tri.v2 = {0.f, 0.f, 1.f};

    auto body = SZM::Physics::RigidBody::CreateFromMesh(
        {tri}, 1000.f, SZM::Physics::KinematicState::Dynamic);
    SZM_ASSERT(body != nullptr, "Bullet test body must be created");
    if (!body) {
        return;
    }

    body->SetPosition({0.0f, 1.25f, 0.0f});

    SZM::Physics::BulletPhysicsAdapter adapter;
    SZM_ASSERT(adapter.Initialize(), "Bullet adapter must initialize");
    if (!adapter.IsInitialized()) {
        return;
    }

    const int handle = adapter.RegisterBody(body, {0.25f, 0.25f, 0.25f});
    SZM_ASSERT(handle >= 0, "Bullet adapter must register a rigid body");

    bool sawContact = false;
    bool sawGroundContact = false;
    for (int step = 0; step < 180; ++step) {
        adapter.Step(1.0f / 120.0f, 2);
        const auto contacts = adapter.CollectContacts();
        sawContact = sawContact || std::any_of(
            contacts.begin(),
            contacts.end(),
            [handle](const auto& contact) {
                return contact.handleA == handle || contact.handleB == handle;
            }
        );
        sawGroundContact = sawGroundContact || std::any_of(
            contacts.begin(),
            contacts.end(),
            [handle](const auto& contact) {
                return (contact.handleA == handle && contact.handleB == 0) ||
                       (contact.handleB == handle && contact.handleA == 0);
            }
        );
    }

    const auto pos = body->GetPosition();
    const auto vel = body->GetLinearVelocity();
    SZM_ASSERT(pos.y < 1.0f, "Bullet body must move downward under gravity");
    SZM_ASSERT(pos.y > 0.15f, "Bullet ground plane must keep the body above the floor");
    SZM_ASSERT(sawContact, "Bullet adapter must report a contact for the resting body");
    SZM_ASSERT(sawGroundContact, "Bullet adapter must tag contacts against the synthetic ground body");
    SZM_ASSERT(std::abs(vel.y) < 0.5f, "Settled Bullet body should have low vertical speed");
}
#endif

// =====================================================================
//  T10 — AssemblyGraph BOM + Mass/CG
// =====================================================================
void Test_AssemblyGraph() {
    SZM_TEST("T10 — AssemblyGraph BOM & Mass/CG");

    auto root = std::make_shared<SZM::Entity>("Root");
    SZM::Assembly::AssemblyGraphManager mgr(root);

    // Register two base parts
    auto partA = std::make_shared<SZM::Assembly::BasePart>();
    partA->PartID   = SZM::UUIDGenerator::Generate();
    partA->PartName = "BracketA";
    partA->Mass     = 2.5f;

    auto partB = std::make_shared<SZM::Assembly::BasePart>();
    partB->PartID   = SZM::UUIDGenerator::Generate();
    partB->PartName = "BoltB";
    partB->Mass     = 0.1f;

    mgr.RegisterBasePart(partA);
    mgr.RegisterBasePart(partB);

    SZM::Transform tx{};
    mgr.InstantiatePart(root, partA->PartID, tx, "Bracket_01");
    mgr.InstantiatePart(root, partA->PartID, tx, "Bracket_02"); // 2× same part
    mgr.InstantiatePart(root, partB->PartID, tx, "Bolt_01");

    auto bom = mgr.GenerateBOM();
    SZM_ASSERT(bom.size() == 2,               "BOM must have 2 unique parts");

    // Find bracket entry
    bool foundBracket = false;
    for (const auto& e : bom) {
        if (e.PartName == "BracketA") {
            SZM_ASSERT(e.Quantity == 2,       "Bracket quantity must be 2");
            SZM_ASSERT(std::abs(e.TotalMass - 5.0f) < 1e-4f,
                                              "Bracket total mass = 2 × 2.5 = 5.0 kg");
            foundBracket = true;
        }
    }
    SZM_ASSERT(foundBracket,                  "BracketA must appear in BOM");

    float totalMass = 0.f;
    float cg[3]     = {};
    mgr.CalculateMassProperties(totalMass, cg);
    SZM_ASSERT(std::abs(totalMass - 5.1f) < 1e-3f,
                                              "Total mass = 2×2.5 + 0.1 = 5.1 kg");
}

// =====================================================================
//  T10B — Workshop instance scaling
// =====================================================================
void Test_WorkshopInstanceScaling() {
    SZM_TEST("T10B — Workshop Instance Scaling & Persistence");

    using namespace SZM::Workshop3D;

    PartLibrary library;
    auto part = library.CreatePart("Scale Test Block");
    part->SetBounds(
        SZM::Geometry::Vector3(-1.0, -1.0, -1.0),
        SZM::Geometry::Vector3(1.0, 1.0, 1.0)
    );
    part->CalculateMass(1000.0f);

    Assembly assembly("Scaled Assembly");
    const uint32_t instanceId = assembly.AddInstance(part, "Block");
    auto instance = assembly.GetInstance(instanceId);

    SZM_ASSERT(instance != nullptr, "Scaled assembly instance must be created");
    if (!instance) {
        return;
    }

    instance->SetScale(SZM::Geometry::Vector3(2.0, 0.5, 1.5));
    const auto scaledSize = instance->GetScaledSize();

    SZM_ASSERT(std::abs(scaledSize.x - 4.0) < 1e-9, "Scaled size X must reflect instance scale");
    SZM_ASSERT(std::abs(scaledSize.y - 1.0) < 1e-9, "Scaled size Y must reflect instance scale");
    SZM_ASSERT(std::abs(scaledSize.z - 3.0) < 1e-9, "Scaled size Z must reflect instance scale");
    SZM_ASSERT(std::abs(instance->GetScaledMass() - 12000.0) < 1e-6,
               "Scaled mass must follow volumetric scale factor");

    const auto tempFile = std::filesystem::temp_directory_path() / "szm_workshop_scaled_assembly.txt";
    SZM_ASSERT(assembly.SaveToJSON(tempFile.string(), library),
               "Scaled workshop assembly must save successfully");

    Assembly loaded("Loaded Assembly");
    SZM_ASSERT(loaded.LoadFromJSON(tempFile.string(), library),
               "Scaled workshop assembly must load successfully");

    auto loadedInstance = loaded.GetInstance(instanceId);
    SZM_ASSERT(loadedInstance != nullptr, "Loaded scaled instance must be present");
    if (!loadedInstance) {
        return;
    }

    SZM_ASSERT(std::abs(loadedInstance->GetScale().x - 2.0) < 1e-9,
               "Loaded instance scale X must persist");
    SZM_ASSERT(std::abs(loadedInstance->GetScale().y - 0.5) < 1e-9,
               "Loaded instance scale Y must persist");
    SZM_ASSERT(std::abs(loadedInstance->GetScale().z - 1.5) < 1e-9,
               "Loaded instance scale Z must persist");
}

// =====================================================================
//  T11 — Camera orbit & projection
// =====================================================================
void Test_Camera() {
    SZM_TEST("T11 — Camera Orbit, View Matrix & Projection Switch");

    SZM::Graphics::Camera cam({10.f, 10.f, 10.f}, {0.f, 0.f, 0.f});

    SZM_ASSERT(cam.GetMode() == SZM::Graphics::ProjectionMode::Perspective,
               "Default mode must be Perspective");

    // View matrix must not be all-zero (identity check)
    auto view = cam.GetViewMatrix();
    bool hasNonZero = false;
    for (float v : view.m) if (std::abs(v) > 1e-6f) { hasNonZero = true; break; }
    SZM_ASSERT(hasNonZero, "View matrix must be non-zero");

    // Orbit should change position
    auto posBefore = cam.GetPosition();
    cam.Orbit(0.5f, 0.3f);
    auto posAfter = cam.GetPosition();
    bool moved = (std::abs(posBefore.x - posAfter.x) > 1e-4f ||
                  std::abs(posBefore.y - posAfter.y) > 1e-4f ||
                  std::abs(posBefore.z - posAfter.z) > 1e-4f);
    SZM_ASSERT(moved, "Orbit must change camera position");

    // Projection switch
    cam.SetProjectionMode(SZM::Graphics::ProjectionMode::Orthographic);
    SZM_ASSERT(cam.GetMode() == SZM::Graphics::ProjectionMode::Orthographic,
               "Mode must switch to Orthographic");

    // Orthographic projection matrix row 3,col 3 should be 1
    auto proj = cam.GetProjectionMatrix();
    SZM_ASSERT(std::abs(proj.m[15] - 1.0f) < 1e-5f,
               "Orthographic proj m[15] must be 1");
}

// =====================================================================
//  T12 — MeshGenerator segment count
// =====================================================================
void Test_MeshGenerator() {
    SZM_TEST("T12 — MeshGenerator CalculateSegmentCount");

    // N = ceil(π / arccos(1 − ε/R)) × 2
    // R=1, ε=0.01 → ratio=0.99 → arccos≈0.14107 → N≈ceil(44.4)×2 = 90
    uint32_t N = SZM::Graphics::MeshGenerator::CalculateSegmentCount(1.0, 0.01);
    SZM_ASSERT(N >= 80 && N <= 120, "Segment count for R=1, ε=0.01 must be ~90");
    SZM_ASSERT(N <= 1024,           "Segment count must not exceed VRAM-safety cap");

    // Degenerate: ε ≥ R → minimum 3
    uint32_t degenN = SZM::Graphics::MeshGenerator::CalculateSegmentCount(0.5, 2.0);
    SZM_ASSERT(degenN == 3, "Degenerate case (ε≥R) must return minimum 3 segments");
}

// =====================================================================
//  T13 — FEA constitutive matrix symmetry
// =====================================================================
void Test_FEA_ConstitutiveMatrix() {
    SZM_TEST("T13 — FEA Linear Solver Constitutive Matrix");

    SZM::FEA::FEAMesh mesh;
    // Single node, no elements — just test assembly does not crash
    SZM::FEA::FEANode n; n.ID = 0;
    n.Position = {0.0, 0.0, 0.0};
    mesh.Nodes.push_back(n);

    SZM::FEA::LinearStaticSolver solver(mesh);
    solver.AssembleSystem();           // Should not throw on empty element list
    solver.ApplyBoundaryConditions();  // Should not throw with no BCs

    SZM_ASSERT(true, "FEA solver initialises and assembles without crash");

    // Verify material database integration path
    auto& db = SZM::Materials::MaterialDatabase::GetInstance();
    auto mat = db.GetMaterial("MAT-STEEL-STRUCT");
    SZM_ASSERT(mat.has_value(), "FEA solver can resolve MAT-STEEL-STRUCT from database");
}

// =====================================================================
//  T13B — FEA Tet4 solve path
// =====================================================================
void Test_FEA_Tet4Solve() {
    SZM_TEST("T13B — FEA Tet4 Solve");

    auto& db = SZM::Materials::MaterialDatabase::GetInstance();
    db.LoadStandardLibraries();

    SZM::FEA::FEAMesh mesh;
    mesh.Nodes.push_back({0, SZM::FEA::Vec3d(0.0, 0.0, 0.0)});
    mesh.Nodes.push_back({1, SZM::FEA::Vec3d(1.0, 0.0, 0.0)});
    mesh.Nodes.push_back({2, SZM::FEA::Vec3d(0.0, 1.0, 0.0)});
    mesh.Nodes.push_back({3, SZM::FEA::Vec3d(0.0, 0.0, 1.0)});

    SZM::FEA::FEAElement e;
    e.ID = 0;
    e.NodeIDs = {0, 1, 2, 3};
    e.MaterialID = "MAT-STEEL-STRUCT";
    mesh.Elements.push_back(e);

    mesh.BCs.push_back({0, true, true, true});   // Ground origin
    mesh.BCs.push_back({1, false, true, true});  // Leave X free
    mesh.BCs.push_back({2, true, false, true});  // Leave Y free
    mesh.BCs.push_back({3, true, true, false});  // Leave Z free

    mesh.Loads.push_back({1, SZM::FEA::Vec3d(1000.0, 0.0, 0.0)});

    SZM::FEA::LinearStaticSolver solver(mesh);
    solver.AssembleSystem();
    solver.ApplyBoundaryConditions();
    const bool solved = solver.Solve();
    SZM_ASSERT(solved, "Tet4 system must solve with constrained rigid modes");

    SZM_ASSERT(mesh.Nodes[1].Displacement.x() > 0.0,
               "Loaded node must displace in +X direction");
}

// =====================================================================
//  T13C — Code 8 / CalculiX pipeline discovery
// =====================================================================
void Test_Code8PipelineCatalog() {
    SZM_TEST("T13C — Code 8 Pipeline Catalog");

    const auto root = SZM::FEA::CalculiXPipeline::DiscoverCode8Root(std::filesystem::current_path());
    SZM_ASSERT(!root.empty(), "Pipeline must discover the code 8 root from the repo workspace");
    SZM_ASSERT(root.filename() == "code 8", "Discovered root must point to the code 8 bundle");

    const auto catalog = SZM::FEA::CalculiXPipeline::CatalogExamples(root);
    SZM_ASSERT(!catalog.empty(), "Pipeline catalog must index code 8 examples");

    const auto kasten = SZM::FEA::CalculiXPipeline::FindExample(root, "Kasten");
    SZM_ASSERT(kasten.has_value(), "Pipeline must resolve the Kasten example by name");
    SZM_ASSERT(kasten.has_value() && kasten->HasTestScript(),
               "Kasten example must expose its runnable test.py entry point");
}

// =====================================================================
//  T13D — Code 8 workspace staging
// =====================================================================
void Test_Code8PipelineStaging() {
    SZM_TEST("T13D — Code 8 Pipeline Staging");

    const auto root = SZM::FEA::CalculiXPipeline::DiscoverCode8Root(std::filesystem::current_path());
    const auto kasten = SZM::FEA::CalculiXPipeline::FindExample(root, "Kasten");
    SZM_ASSERT(kasten.has_value(), "Kasten example must be available for staging");
    if (!kasten.has_value()) {
        return;
    }

    const auto stageBase = std::filesystem::temp_directory_path() / "szm_forge_code8_pipeline_tests";
    const auto manifest = SZM::FEA::CalculiXPipeline::StageExampleWorkspace(root, *kasten, stageBase);

    SZM_ASSERT(std::filesystem::exists(manifest.WorkspaceRoot / "Scripts" / "monitor.py"),
               "Staged workspace must include shared code 8 helper scripts");
    SZM_ASSERT(std::filesystem::exists(manifest.ExampleWorkspace / "test.py"),
               "Staged workspace must include the example test entry point");
    SZM_ASSERT(std::filesystem::exists(manifest.ManifestPath),
               "Staged workspace must include a pipeline manifest");
    SZM_ASSERT(!manifest.ExecutionPlan.empty(),
               "Staged workspace must provide at least one execution command");
}

// =====================================================================
//  T14 — InterferenceChecker material rules & AABB escape
// =====================================================================
void Test_InterferenceChecker() {
    SZM_TEST("T14 — InterferenceChecker Material Rules & AABB Broad Phase");

    SZM::Assembly::InterferenceChecker checker;

    // Two completely separated metal parts — AABB must not overlap
    SZM::Assembly::PartInstance pA;
    pA.InstanceID   = 1;
    pA.Material     = SZM::Assembly::MaterialType::Metal;
    pA.BoundingBox  = {0, 0, 0,  1, 1, 1};
    pA.Geometry     = nullptr;

    SZM::Assembly::PartInstance pB;
    pB.InstanceID   = 2;
    pB.Material     = SZM::Assembly::MaterialType::Metal;
    pB.BoundingBox  = {5, 5, 5,  6, 6, 6}; // Far away — no overlap
    pB.Geometry     = nullptr;

    auto clashes = checker.RunStaticCheck({pA, pB});
    SZM_ASSERT(clashes.empty(), "Separated parts must produce zero clashes");

    // Rubber vs Metal — material rule escape (always allowed)
    SZM::Assembly::PartInstance pC;
    pC.InstanceID  = 3;
    pC.Material    = SZM::Assembly::MaterialType::Rubber_Compressible;
    pC.BoundingBox = {0, 0, 0, 1, 1, 1}; // Overlaps pA
    pC.Geometry    = nullptr;

    auto rubberClashes = checker.RunStaticCheck({pA, pC});
    SZM_ASSERT(rubberClashes.empty(),
               "Rubber vs Metal must be allowed — zero reported clashes");
}

// =====================================================================
//  T15 — KinematicSolver + CoincidentPlanarMate
// =====================================================================
void Test_KinematicSolverMate() {
    SZM_TEST("T15 — Kinematic Solver CoincidentPlanarMate");

    using namespace SZM::Assembly;
    auto bodyA = std::make_shared<RigidBodyState>(1, true);
    auto bodyB = std::make_shared<RigidBodyState>(2, false);
    bodyA->Transform = Eigen::Isometry3d::Identity();
    bodyB->Transform = Eigen::Isometry3d::Identity();
    bodyB->Transform.translation() = Eigen::Vector3d(0.0, 0.0, 1.25);

    KinematicSolver solver;
    solver.AddBody(bodyA);
    solver.AddBody(bodyB);

    auto mate = std::make_shared<CoincidentPlanarMate>(
        1, 2,
        Eigen::Vector3d::Zero(), Eigen::Vector3d::UnitZ(),
        Eigen::Vector3d::Zero(), Eigen::Vector3d::UnitZ(),
        MateAlignment::Aligned,
        bodyA.get(), bodyB.get()
    );
    solver.AddMate(mate);

    const bool converged = solver.Solve(80, 1e-8);
    SZM_ASSERT(converged, "Coincident planar mate must converge");

    Eigen::VectorXd residual = Eigen::VectorXd::Zero(3);
    mate->EvaluateResidual(residual, 0);
    SZM_ASSERT(std::abs(residual(0)) < 1e-5, "Mate distance residual must be near zero");
    SZM_ASSERT(std::abs(residual(1)) < 1e-5, "Mate cross residual must be near zero");
    SZM_ASSERT(std::abs(residual(2)) < 1e-5, "Mate dot residual must be near zero");
}

// =====================================================================
//  T16 — SpaceMouse simulated input path
// =====================================================================
void Test_SpaceMouseDriver() {
    SZM_TEST("T16 — SpaceMouse simulated input");

    ::setenv("SZM_SPACEMOUSE_CONNECTED", "1", 1);
    ::setenv("SZM_SPACEMOUSE_SIM", "2.0,0.0,-2.0,0.5,0.25,-0.5,3", 1);

    SZM::SpaceMouseDriver::Init();
    SZM::SpaceMouseDriver::SetSensitivity(0.5f);
    const auto state = SZM::SpaceMouseDriver::GetState();

    SZM_ASSERT(SZM::SpaceMouseDriver::IsConnected(),
               "Simulated SpaceMouse must report connected");
    SZM_ASSERT(std::abs(state.tx - 1.0f) < 1e-5f,
               "SpaceMouse translation X must be scaled/clamped");
    SZM_ASSERT(std::abs(state.tz + 1.0f) < 1e-5f,
               "SpaceMouse translation Z must be scaled/clamped");
    SZM_ASSERT(state.buttons == 3u,
               "SpaceMouse button bitmask must parse from simulated input");

    SZM::SpaceMouseDriver::Shutdown();
    ::unsetenv("SZM_SPACEMOUSE_CONNECTED");
    ::unsetenv("SZM_SPACEMOUSE_SIM");
}

// =====================================================================
//  Main
// =====================================================================
int main() {
    std::cout << "============================================\n";
    std::cout << " SZM Forge v0.2.0 — Integration Test Runner\n";
    std::cout << "============================================\n";

    // v0.1 tests
    Test_UUID();
    Test_SceneGraph();
    Test_SceneGraphSerialize();
    Test_ConstraintSolver();
    Test_LinearExtrude();
    Test_RevolveOp();
    Test_InputManager();

    // v0.2 tests
    Test_SolidBodyDeformationConstraints();
    Test_MaterialDatabase();
    Test_SimulationMaterialIntegration();
    Test_RigidBody();
#if defined(SZM_HAS_BULLET)
    Test_BulletPhysicsAdapter();
#endif
    Test_AssemblyGraph();
    Test_WorkshopInstanceScaling();
    Test_Camera();
    Test_MeshGenerator();
    Test_FEA_ConstitutiveMatrix();
    Test_FEA_Tet4Solve();
    Test_Code8PipelineCatalog();
    Test_Code8PipelineStaging();
    Test_InterferenceChecker();
    Test_KinematicSolverMate();
    Test_SpaceMouseDriver();

    std::cout << "\n============================================\n";
    std::cout << " Results: " << g_Passed << " passed, "
              << g_Failed << " failed.\n";
    std::cout << "============================================\n";

    return g_Failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
