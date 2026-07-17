#include "CollisionDetection.hpp"
#include <cfloat>
#include <cmath>

namespace SZM::Physics {

// ---- Support functions ----------------------------------------------
PhysVec3 CollisionDetection::GetFurthestPoint(const RigidBody& body, PhysVec3 dir) {
    // Temporary convex proxy: centered box (1 m edge length) around body position.
    // This keeps GJK/EPA active until full hull integration is wired.
    const PhysVec3 p = body.GetPosition();
    const float hx = 0.5f;
    const float hy = 0.5f;
    const float hz = 0.5f;

    return {
        p.x + (dir.x >= 0.0f ? hx : -hx),
        p.y + (dir.y >= 0.0f ? hy : -hy),
        p.z + (dir.z >= 0.0f ? hz : -hz)
    };
}
PhysVec3 CollisionDetection::Support(const RigidBody& a, const RigidBody& b, PhysVec3 dir) {
    return GetFurthestPoint(a, dir) -
           GetFurthestPoint(b, {-dir.x, -dir.y, -dir.z});
}

// ---- GJK ------------------------------------------------------------
bool CollisionDetection::ExecuteGJK(const RigidBody& a, const RigidBody& b, Simplex& out) {
    PhysVec3 searchDir = {1.f, 0.f, 0.f};
    PhysVec3 sup       = Support(a, b, searchDir);
    out.PushFront(sup);
    searchDir = {-sup.x, -sup.y, -sup.z};

    for (int i = 0; i < 64; ++i) {
        sup = Support(a, b, searchDir);
        if (sup.dot(searchDir) < 0.f) return false;
        out.PushFront(sup);
        if (HandleSimplex(out, searchDir)) return true;
    }
    return false;
}

bool CollisionDetection::HandleSimplex(Simplex& s, PhysVec3& dir) {
    if (s.Size == 2) {
        PhysVec3 ab = s.Points[1] - s.Points[0];
        PhysVec3 ao = {-s.Points[0].x, -s.Points[0].y, -s.Points[0].z};
        if (ab.dot(ao) > 0)
            dir = ab.cross(ao).cross(ab);
        else {
            s.Size = 1;
            dir = ao;
        }
    } else if (s.Size == 3) {
        PhysVec3 ab  = s.Points[1] - s.Points[0];
        PhysVec3 ac  = s.Points[2] - s.Points[0];
        PhysVec3 ao  = {-s.Points[0].x, -s.Points[0].y, -s.Points[0].z};
        PhysVec3 abc = ab.cross(ac);

        if (abc.cross(ac).dot(ao) > 0) {
            if (ac.dot(ao) > 0) { s.Points[1] = s.Points[2]; s.Size = 2;
                                  dir = ac.cross(ao).cross(ac); }
            else { s.Size = 2; return HandleSimplex(s, dir); }
        } else if (ab.cross(abc).dot(ao) > 0) {
            s.Size = 2; return HandleSimplex(s, dir);
        } else {
            dir = (abc.dot(ao) > 0) ? abc : PhysVec3{-abc.x,-abc.y,-abc.z};
        }
    } else if (s.Size == 4) {
        return true; // Origin enclosed in tetrahedron
    }
    return false;
}

// ---- EPA ------------------------------------------------------------
ContactPoint CollisionDetection::ExecuteEPA(
    const RigidBody& a, const RigidBody& b, const Simplex& /*gjk*/)
{
    // Placeholder penetration estimate from proxy body centers.
    const PhysVec3 pa = a.GetPosition();
    const PhysVec3 pb = b.GetPosition();
    PhysVec3 d = { pa.x - pb.x, pa.y - pb.y, pa.z - pb.z };
    const float distSq = d.dot(d);
    const float dist = std::sqrt(std::max(0.0f, distSq));
    const float proxyRadius = 0.8660254f; // sqrt(3) * 0.5 (half-diagonal of unit cube)

    ContactPoint cp;
    if (dist > 1e-6f) {
        cp.normal = { d.x / dist, d.y / dist, d.z / dist };
    } else {
        cp.normal = {0.f, 1.f, 0.f};
    }
    cp.penetration = std::max(0.0f, (2.0f * proxyRadius) - dist);
    cp.worldPoint  = { (pa.x + pb.x) * 0.5f, (pa.y + pb.y) * 0.5f, (pa.z + pb.z) * 0.5f };
    return cp;
}

// ---- GenerateManifold -----------------------------------------------
bool CollisionDetection::GenerateManifold(
    std::shared_ptr<RigidBody> bodyA,
    std::shared_ptr<RigidBody> bodyB,
    ContactManifold& out)
{
    Simplex simplex;
    if (!ExecuteGJK(*bodyA, *bodyB, simplex)) return false;

    ContactPoint cp = ExecuteEPA(*bodyA, *bodyB, simplex);
    out.BodyA = bodyA;
    out.BodyB = bodyB;
    out.Contacts.push_back(cp);
    // Full face-contact manifold generation (Sutherland-Hodgman) goes here
    return true;
}

// ---- MergeManifolds (warm-starting) ---------------------------------
void CollisionDetection::MergeManifolds(
    std::vector<ContactManifold>& cache, ContactManifold& newM)
{
    for (auto& old : cache) {
        bool sameBodyPair =
            (old.BodyA == newM.BodyA && old.BodyB == newM.BodyB) ||
            (old.BodyA == newM.BodyB && old.BodyB == newM.BodyA);
        if (!sameBodyPair) continue;

        for (auto& nc : newM.Contacts) {
            for (const auto& oc : old.Contacts) {
                if (nc.Matches(oc, 0.02f)) {
                    nc.normalImpulseAccum      = oc.normalImpulseAccum;
                    nc.tangentImpulseAccum[0]  = oc.tangentImpulseAccum[0];
                    nc.tangentImpulseAccum[1]  = oc.tangentImpulseAccum[1];
                    break;
                }
            }
        }
    }
}

// ---- SweepTestCCD ---------------------------------------------------
bool CollisionDetection::SweepTestCCD(
    std::shared_ptr<RigidBody> a, std::shared_ptr<RigidBody> b,
    PhysVec3 velA, PhysVec3 velB, float dt,
    float& outTOI, PhysVec3& outHitNormal)
{
    if (!a || !b || dt <= 0.0f) {
        return false;
    }

    // Approximate CCD with swept spheres around proxy collision shapes.
    const float radiusA = 0.8660254f; // sqrt(3) * 0.5
    const float radiusB = 0.8660254f;
    const float radiusSum = radiusA + radiusB;

    const PhysVec3 p0 = a->GetPosition();
    const PhysVec3 q0 = b->GetPosition();
    const PhysVec3 r0 = { p0.x - q0.x, p0.y - q0.y, p0.z - q0.z };
    const PhysVec3 v = { velA.x - velB.x, velA.y - velB.y, velA.z - velB.z };

    const float aCoeff = v.dot(v);
    const float bCoeff = 2.0f * r0.dot(v);
    const float cCoeff = r0.dot(r0) - radiusSum * radiusSum;

    // Already overlapping now.
    if (cCoeff <= 0.0f) {
        outTOI = 0.0f;
        const float len = std::sqrt(std::max(1e-12f, r0.dot(r0)));
        outHitNormal = (len > 1e-6f)
            ? PhysVec3{ r0.x / len, r0.y / len, r0.z / len }
            : PhysVec3{ 0.0f, 1.0f, 0.0f };
        return true;
    }

    // No relative motion.
    if (aCoeff < 1e-8f) {
        return false;
    }

    const float discriminant = bCoeff * bCoeff - 4.0f * aCoeff * cCoeff;
    if (discriminant < 0.0f) {
        return false;
    }

    const float sqrtDisc = std::sqrt(discriminant);
    const float t0 = (-bCoeff - sqrtDisc) / (2.0f * aCoeff);
    const float t1 = (-bCoeff + sqrtDisc) / (2.0f * aCoeff);

    float tImpact = -1.0f;
    if (t0 >= 0.0f && t0 <= dt) {
        tImpact = t0;
    } else if (t1 >= 0.0f && t1 <= dt) {
        tImpact = t1;
    }

    if (tImpact < 0.0f) {
        return false;
    }

    outTOI = tImpact;
    const PhysVec3 rImpact = {
        r0.x + v.x * tImpact,
        r0.y + v.y * tImpact,
        r0.z + v.z * tImpact
    };
    const float impactLen = std::sqrt(std::max(1e-12f, rImpact.dot(rImpact)));
    outHitNormal = (impactLen > 1e-6f)
        ? PhysVec3{ rImpact.x / impactLen, rImpact.y / impactLen, rImpact.z / impactLen }
        : PhysVec3{ 0.0f, 1.0f, 0.0f };

    return true;
}

} // namespace SZM::Physics
