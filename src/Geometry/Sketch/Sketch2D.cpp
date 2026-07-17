#include "Sketch2D.hpp"
#include "ConstraintSolver2D.hpp"
#include <cmath>

namespace SZM::Geometry {

Sketch2D::Sketch2D(const Vector3& origin, const Vector3& normal)
    : m_PlaneOrigin(origin), m_PlaneNormal(normal.Normalize())
{
    UpdateLocalAxes();
}

void Sketch2D::UpdateLocalAxes() {
    // Arbitrary orthonormal basis
    Vector3 up(0, 1, 0);
    if (std::abs(m_PlaneNormal.y) > 0.99) {
        up = Vector3(1, 0, 0);
    }
    m_PlaneU = up.Cross(m_PlaneNormal).Normalize();
    m_PlaneV = m_PlaneNormal.Cross(m_PlaneU).Normalize();
}

Vector3 Sketch2D::LocalToWorld(double u, double v) const {
    return m_PlaneOrigin + m_PlaneU * u + m_PlaneV * v;
}

uint32_t Sketch2D::AddPoint(double x, double y) {
    auto ent = std::make_unique<SketchEntity>();
    ent->ID = m_NextEntityID++;
    ent->Type = SketchEntityType::Point;
    ent->P1[0] = x; ent->P1[1] = y;
    m_Entities.push_back(std::move(ent));
    return m_NextEntityID - 1;
}

uint32_t Sketch2D::AddLine(double x1, double y1, double x2, double y2) {
    auto ent = std::make_unique<SketchEntity>();
    ent->ID = m_NextEntityID++;
    ent->Type = SketchEntityType::Line;
    ent->P1[0] = x1; ent->P1[1] = y1;
    ent->P2[0] = x2; ent->P2[1] = y2;
    m_Entities.push_back(std::move(ent));
    return m_NextEntityID - 1;
}

uint32_t Sketch2D::AddCircle(double cx, double cy, double r) {
    auto ent = std::make_unique<SketchEntity>();
    ent->ID = m_NextEntityID++;
    ent->Type = SketchEntityType::Circle;
    ent->P1[0] = cx; ent->P1[1] = cy;
    ent->P2[0] = cx + r; ent->P2[1] = cy; // Point on circumference
    m_Entities.push_back(std::move(ent));
    return m_NextEntityID - 1;
}

uint32_t Sketch2D::AddConstraint(const SketchConstraint& constraint) {
    SketchConstraint c = constraint;
    c.ID = m_NextConstraintID++;
    m_Constraints.push_back(c);
    return c.ID;
}

uint32_t Sketch2D::AddDimension(const ParametricDimension& dim) {
    ParametricDimension d = dim;
    d.ID = m_NextDimensionID++;
    m_Dimensions.push_back(d);
    return d.ID;
}

bool Sketch2D::Solve(int maxIterations, double tolerance) {
    return ConstraintSolver2D::Solve(*this, maxIterations, tolerance);
}

SketchProfile Sketch2D::GenerateProfile() const {
    SketchProfile profile;
    profile.PlaneNormal = m_PlaneNormal;
    profile.PlaneOrigin = m_PlaneOrigin;

    // Simple profile extraction: assume lines form a single closed loop in order
    // In a real system, this would do a topological sort of edges to extract loops.
    std::vector<Vector3> loop;
    for (const auto& ent : m_Entities) {
        if (ent->IsConstruction) continue;

        if (ent->Type == SketchEntityType::Line) {
            if (loop.empty()) {
                loop.push_back(LocalToWorld(ent->P1[0], ent->P1[1]));
            }
            // Add end point
            loop.push_back(LocalToWorld(ent->P2[0], ent->P2[1]));
        } else if (ent->Type == SketchEntityType::Circle) {
            // Tessellate circle
            double cx = ent->P1[0];
            double cy = ent->P1[1];
            double dx = ent->P2[0] - cx;
            double dy = ent->P2[1] - cy;
            double r = std::sqrt(dx*dx + dy*dy);

            const int steps = 36;
            for (int i = 0; i < steps; ++i) {
                double theta = 2.0 * 3.14159265359 * i / steps;
                double x = cx + r * std::cos(theta);
                double y = cy + r * std::sin(theta);
                loop.push_back(LocalToWorld(x, y));
            }
        }
    }

    if (!loop.empty()) {
        profile.Loops.push_back(std::move(loop));
    }

    return profile;
}

SketchEntity* Sketch2D::GetEntity(uint32_t id) {
    for (auto& ent : m_Entities) {
        if (ent->ID == id) return ent.get();
    }
    return nullptr;
}

const SketchEntity* Sketch2D::GetEntity(uint32_t id) const {
    for (const auto& ent : m_Entities) {
        if (ent->ID == id) return ent.get();
    }
    return nullptr;
}

} // namespace SZM::Geometry
