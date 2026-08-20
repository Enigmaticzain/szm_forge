#include "Sketch2D.hpp"
#include "ConstraintSolver2D.hpp"
#include <cmath>
#include <nlohmann/json.hpp>

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

uint32_t Sketch2D::AddArc(double cx, double cy, double r,
                           double startAngleRad, double endAngleRad) {
    auto ent = std::make_unique<SketchEntity>();
    ent->ID   = m_NextEntityID++;
    ent->Type = SketchEntityType::Arc;
    ent->P1[0] = cx; ent->P1[1] = cy;                                    // center
    ent->P2[0] = cx + r * std::cos(startAngleRad);                       // start point
    ent->P2[1] = cy + r * std::sin(startAngleRad);
    ent->P3[0] = cx + r * std::cos(endAngleRad);                         // end point
    ent->P3[1] = cy + r * std::sin(endAngleRad);
    m_Entities.push_back(std::move(ent));
    return m_NextEntityID - 1;
}

int Sketch2D::GetDOFCount() const {
    int dof = 0;
    for (const auto& e : m_Entities) {
        switch (e->Type) {
            case SketchEntityType::Point:  dof += 2; break;
            case SketchEntityType::Line:   dof += 4; break;
            case SketchEntityType::Circle: dof += 4; break;
            case SketchEntityType::Arc:    dof += 6; break;
        }
    }
    // Each constraint removes ~1 DOF on average
    dof -= static_cast<int>(m_Constraints.size());
    return dof;
}

static std::string EntityTypeName(SketchEntityType t) {
    switch (t) {
        case SketchEntityType::Point:  return "point";
        case SketchEntityType::Line:   return "line";
        case SketchEntityType::Circle: return "circle";
        case SketchEntityType::Arc:    return "arc";
    }
    return "unknown";
}

static std::string ConstraintTypeName(ConstraintType t) {
    switch (t) {
        case ConstraintType::Coincident:    return "coincident";
        case ConstraintType::Horizontal:    return "horizontal";
        case ConstraintType::Vertical:      return "vertical";
        case ConstraintType::Tangent:       return "tangent";
        case ConstraintType::Parallel:      return "parallel";
        case ConstraintType::Perpendicular: return "perpendicular";
        case ConstraintType::Concentric:    return "concentric";
        case ConstraintType::EqualLength:   return "equal_length";
        case ConstraintType::EqualRadius:   return "equal_radius";
        case ConstraintType::Distance:      return "distance";
        case ConstraintType::Angle:         return "angle";
        case ConstraintType::Fixed:         return "fixed";
    }
    return "unknown";
}

nlohmann::json Sketch2D::ToJSON() const {
    nlohmann::json j;
    j["plane"] = {
        {"origin", {m_PlaneOrigin.x, m_PlaneOrigin.y, m_PlaneOrigin.z}},
        {"normal", {m_PlaneNormal.x, m_PlaneNormal.y, m_PlaneNormal.z}}
    };
    j["dof"] = GetDOFCount();

    auto entities = nlohmann::json::array();
    for (const auto& e : m_Entities) {
        nlohmann::json ej;
        ej["id"]           = e->ID;
        ej["type"]         = EntityTypeName(e->Type);
        ej["construction"] = e->IsConstruction;
        ej["p1"] = {e->P1[0], e->P1[1]};
        if (e->Type != SketchEntityType::Point)
            ej["p2"] = {e->P2[0], e->P2[1]};
        if (e->Type == SketchEntityType::Arc)
            ej["p3"] = {e->P3[0], e->P3[1]};
        entities.push_back(ej);
    }
    j["entities"] = entities;

    auto constraints = nlohmann::json::array();
    for (const auto& c : m_Constraints) {
        constraints.push_back({
            {"id",          c.ID},
            {"type",        ConstraintTypeName(c.Type)},
            {"entity1",     c.Entity1ID},
            {"entity2",     c.Entity2ID},
            {"sub1",        c.SubEntity1},
            {"sub2",        c.SubEntity2},
            {"dimension_id",c.DimensionID}
        });
    }
    j["constraints"] = constraints;

    auto dims = nlohmann::json::array();
    for (const auto& d : m_Dimensions) {
        dims.push_back({{"id",d.ID},{"name",d.Name},{"value",d.Value},{"expression",d.Expression}});
    }
    j["dimensions"] = dims;
    return j;
}

Sketch2D Sketch2D::FromJSON(const nlohmann::json& j) {
    Vector3 origin{0,0,0}, normal{0,0,1};
    if (j.contains("plane")) {
        auto& p = j["plane"];
        if (p.contains("origin")) { origin.x=p["origin"][0]; origin.y=p["origin"][1]; origin.z=p["origin"][2]; }
        if (p.contains("normal")) { normal.x=p["normal"][0]; normal.y=p["normal"][1]; normal.z=p["normal"][2]; }
    }
    Sketch2D sk(origin, normal);

    static const std::unordered_map<std::string, SketchEntityType> typeMap = {
        {"point",SketchEntityType::Point},{"line",SketchEntityType::Line},
        {"circle",SketchEntityType::Circle},{"arc",SketchEntityType::Arc}
    };
    static const std::unordered_map<std::string, ConstraintType> cTypeMap = {
        {"coincident",ConstraintType::Coincident},{"horizontal",ConstraintType::Horizontal},
        {"vertical",ConstraintType::Vertical},{"tangent",ConstraintType::Tangent},
        {"parallel",ConstraintType::Parallel},{"perpendicular",ConstraintType::Perpendicular},
        {"concentric",ConstraintType::Concentric},{"equal_length",ConstraintType::EqualLength},
        {"equal_radius",ConstraintType::EqualRadius},{"distance",ConstraintType::Distance},
        {"angle",ConstraintType::Angle},{"fixed",ConstraintType::Fixed}
    };

    for (const auto& ej : j.value("entities", nlohmann::json::array())) {
        std::string t = ej.value("type", "point");
        auto it = typeMap.find(t);
        if (it == typeMap.end()) continue;
        auto ent = std::make_unique<SketchEntity>();
        ent->ID           = ej.value("id", sk.m_NextEntityID);
        ent->Type         = it->second;
        ent->IsConstruction = ej.value("construction", false);
        if (ej.contains("p1")) { ent->P1[0]=ej["p1"][0]; ent->P1[1]=ej["p1"][1]; }
        if (ej.contains("p2")) { ent->P2[0]=ej["p2"][0]; ent->P2[1]=ej["p2"][1]; }
        if (ej.contains("p3")) { ent->P3[0]=ej["p3"][0]; ent->P3[1]=ej["p3"][1]; }
        sk.m_NextEntityID = std::max(sk.m_NextEntityID, ent->ID + 1);
        sk.m_Entities.push_back(std::move(ent));
    }
    for (const auto& cj : j.value("constraints", nlohmann::json::array())) {
        SketchConstraint c{};
        c.ID         = cj.value("id", sk.m_NextConstraintID);
        auto ct = cTypeMap.find(cj.value("type", ""));
        if (ct == cTypeMap.end()) continue;
        c.Type       = ct->second;
        c.Entity1ID  = cj.value("entity1", 0u);
        c.Entity2ID  = cj.value("entity2", 0u);
        c.SubEntity1 = cj.value("sub1", -1);
        c.SubEntity2 = cj.value("sub2", -1);
        c.DimensionID= cj.value("dimension_id", 0u);
        sk.m_NextConstraintID = std::max(sk.m_NextConstraintID, c.ID + 1);
        sk.m_Constraints.push_back(c);
    }
    for (const auto& dj : j.value("dimensions", nlohmann::json::array())) {
        ParametricDimension d{};
        d.ID         = dj.value("id", sk.m_NextDimensionID);
        d.Name       = dj.value("name", "");
        d.Value      = dj.value("value", 0.0);
        d.Expression = dj.value("expression", "");
        sk.m_NextDimensionID = std::max(sk.m_NextDimensionID, d.ID + 1);
        sk.m_Dimensions.push_back(d);
    }
    return sk;
}

} // namespace SZM::Geometry
