#include "RigidBody.hpp"
#include "../Core/ErrorHandling.hpp"
#include "../Core/ConfigConstants.hpp"
#include <cmath>
#include <algorithm>

namespace SZM::Physics {

// Use configuration constants for all tunable parameters
namespace {
    namespace Config = SZM::Config::Physics;
}


// ---- PhysMat3 helpers -----------------------------------------------
PhysMat3 PhysMat3::operator*(const PhysMat3& o) const {
    PhysMat3 r; r.m[0][0] = r.m[1][1] = r.m[2][2] = 0.0f;
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            for (int k = 0; k < 3; ++k)
                r.m[i][j] += m[i][k] * o.m[k][j];
    return r;
}
PhysVec3 PhysMat3::operator*(const PhysVec3& v) const {
    return PhysVec3{
        m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z,
        m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z,
        m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z
    };
}
PhysMat3 PhysMat3::Transpose() const {
    PhysMat3 result;
    result.m[0][0] = m[0][0]; result.m[0][1] = m[1][0]; result.m[0][2] = m[2][0];
    result.m[1][0] = m[0][1]; result.m[1][1] = m[1][1]; result.m[1][2] = m[2][1];
    result.m[2][0] = m[0][2]; result.m[2][1] = m[1][2]; result.m[2][2] = m[2][2];
    return result;
}
PhysMat3 PhysMat3::Inverse() const {
    float det = m[0][0]*(m[1][1]*m[2][2]-m[2][1]*m[1][2])
              - m[0][1]*(m[1][0]*m[2][2]-m[1][2]*m[2][0])
              + m[0][2]*(m[1][0]*m[2][1]-m[1][1]*m[2][0]);
    
    if (std::abs(det) < Config::MatrixSingularityThreshold) {
        // Log warning but continue with zero matrix
        // TODO: Implement logging system for warnings
        return PhysMat3{{{0,0,0},{0,0,0},{0,0,0}}};
    }
    
    float id = 1.0f / det;
    PhysMat3 r;
    r.m[0][0] = (m[1][1]*m[2][2]-m[2][1]*m[1][2])*id;
    r.m[0][1] = (m[0][2]*m[2][1]-m[0][1]*m[2][2])*id;
    r.m[0][2] = (m[0][1]*m[1][2]-m[0][2]*m[1][1])*id;
    r.m[1][0] = (m[1][2]*m[2][0]-m[1][0]*m[2][2])*id;
    r.m[1][1] = (m[0][0]*m[2][2]-m[0][2]*m[2][0])*id;
    r.m[1][2] = (m[1][0]*m[0][2]-m[0][0]*m[1][2])*id;
    r.m[2][0] = (m[1][0]*m[2][1]-m[2][0]*m[1][1])*id;
    r.m[2][1] = (m[2][0]*m[0][1]-m[0][0]*m[2][1])*id;
    r.m[2][2] = (m[0][0]*m[1][1]-m[1][0]*m[0][1])*id;
    return r;
}

// ---- Constructor ----------------------------------------------------
RigidBody::RigidBody(KinematicState state)
    : m_State(state) {}

// ---- Tetrahedron integral helper ------------------------------------
static void TetraIntegrals(
    const PhysVec3& p1, const PhysVec3& p2, const PhysVec3& p3,
    float& vol,  PhysVec3& cg,
    float& ixx,  float& iyy, float& izz,
    float& ixy,  float& iyz, float& izx)
{
    float det = p1.dot(p2.cross(p3));
    vol = det / 6.0f;
    cg  = (p1 + p2 + p3) / 4.0f;

    auto sq = [](float a, float b, float c) {
        return a*a + a*b + b*b + a*c + b*c + c*c;
    };
    auto pr = [](float ax, float bx, float cx,
                  float ay, float by, float cy) {
        return 2*ax*ay + bx*ay + cx*ay + ax*by + 2*bx*by + cx*by
             + ax*cy  + bx*cy  + 2*cx*cy;
    };

    float intX2 = (det/60.f) * sq(p1.x, p2.x, p3.x);
    float intY2 = (det/60.f) * sq(p1.y, p2.y, p3.y);
    float intZ2 = (det/60.f) * sq(p1.z, p2.z, p3.z);
    float intXY = (det/120.f)* pr(p1.x,p2.x,p3.x, p1.y,p2.y,p3.y);
    float intYZ = (det/120.f)* pr(p1.y,p2.y,p3.y, p1.z,p2.z,p3.z);
    float intZX = (det/120.f)* pr(p1.z,p2.z,p3.z, p1.x,p2.x,p3.x);

    ixx = intY2 + intZ2;  iyy = intX2 + intZ2;  izz = intX2 + intY2;
    ixy = -intXY;         iyz = -intYZ;          izx = -intZX;
}

// ---- CreateFromMesh -------------------------------------------------
std::shared_ptr<RigidBody> RigidBody::CreateFromMesh(
    const std::vector<Triangle>& mesh, float density, KinematicState state)
{
    // Validate input parameters
    SZM_CHECK(!mesh.empty(), 
        "Cannot create rigid body from empty mesh", "Physics");
    SZM_CHECK(density > 0.0f, 
        "Material density must be positive (provided: " + std::to_string(density) + ")", "Physics");

    auto body = std::make_shared<RigidBody>(state);

    float    totalVol = 0.f;
    PhysVec3 totalCG  = {};
    float Ixx=0,Iyy=0,Izz=0, Ixy=0,Iyz=0,Izx=0;

    // Integrate volume, center of mass, and inertia over all tetrahedra
    for (const auto& tri : mesh) {
        float vol; PhysVec3 cg;
        float ixx,iyy,izz,ixy,iyz,izx;
        TetraIntegrals(tri.v0, tri.v1, tri.v2,
                       vol, cg, ixx, iyy, izz, ixy, iyz, izx);
        totalVol += vol;
        totalCG   = totalCG + cg * vol;
        Ixx += ixx; Iyy += iyy; Izz += izz;
        Ixy += ixy; Iyz += iyz; Izx += izx;
    }

    // Check for degenerate geometry
    if (totalVol <= Config::MinVolumeThreshold) {
        // Warn but create static body to allow continued operation
        // TODO: Implement warning logger
        body->m_State   = KinematicState::Static;
        body->m_Mass    = 0.0f;
        body->m_InvMass = 0.0f;
        return body;
    }

    body->m_CenterOfMassLocal = totalCG / totalVol;
    body->m_Mass              = totalVol * density;

    // For non-dynamic bodies, only compute mass (no inertia needed)
    if (state != KinematicState::Dynamic) {
        body->m_InvMass = 0.0f;
        return body;
    }
    
    // For dynamic bodies, compute inverse mass
    body->m_InvMass = 1.0f / body->m_Mass;

    // Scale inertia components by density
    Ixx*=density; Iyy*=density; Izz*=density;
    Ixy*=density; Iyz*=density; Izx*=density;

    // Apply parallel axis theorem: shift inertia from origin to center of mass
    float m  = body->m_Mass;
    float cx = body->m_CenterOfMassLocal.x;
    float cy = body->m_CenterOfMassLocal.y;
    float cz = body->m_CenterOfMassLocal.z;
    Ixx -= m*(cy*cy + cz*cz);
    Iyy -= m*(cx*cx + cz*cz);
    Izz -= m*(cx*cx + cy*cy);
    Ixy += m*cx*cy;  Iyz += m*cy*cz;  Izx += m*cx*cz;

    // Store inertia tensor in local (body) frame
    auto& I = body->m_InertiaTensorLocal;
    I.m[0][0]=Ixx; I.m[0][1]=Ixy; I.m[0][2]=Izx;
    I.m[1][0]=Ixy; I.m[1][1]=Iyy; I.m[1][2]=Iyz;
    I.m[2][0]=Izx; I.m[2][1]=Iyz; I.m[2][2]=Izz;
    
    // Precompute inverse for efficiency
    body->m_InvInertiaTensorLocal = I.Inverse();
    body->m_InvInertiaTensorWorld = body->m_InvInertiaTensorLocal;

    return body;
}

// ---- UpdateWorldInertia ---------------------------------------------
void RigidBody::UpdateWorldInertia(const PhysMat3& R) {
    if (m_State != KinematicState::Dynamic) return;
    PhysMat3 Rt   = R.Transpose();
    PhysMat3 temp = R * m_InvInertiaTensorLocal;
    m_InvInertiaTensorWorld = temp * Rt;
}

} // namespace SZM::Physics
