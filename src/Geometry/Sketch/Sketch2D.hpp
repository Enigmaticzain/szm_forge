#pragma once

#include "SketchConstraint.hpp"
#include "ParametricDimension.hpp"
#include "SketchProfile.hpp"
#include "../MathTypes.hpp"
#include <vector>
#include <memory>
#include <unordered_map>

namespace SZM::Geometry {

    enum class SketchEntityType {
        Point,
        Line,
        Circle,
        Arc
    };

    struct SketchEntity {
        uint32_t ID;
        SketchEntityType Type;
        bool IsConstruction = false;
        
        // Geometric data (2D coordinates on the sketch plane)
        // For Point: P1 is the position
        // For Line: P1 is start, P2 is end
        // For Circle: P1 is center, P2 is a point on circumference
        // For Arc: P1 is center, P2 is start, P3 is end
        double P1[2] = {0, 0};
        double P2[2] = {0, 0};
        double P3[2] = {0, 0};

        virtual ~SketchEntity() = default;
    };

    // ----------------------------------------------------------------
    //  Sketch2D — A 2D parametric sketch
    // ----------------------------------------------------------------
    class Sketch2D {
    public:
        Sketch2D(const Vector3& origin = {0,0,0}, const Vector3& normal = {0,0,1});

        // Entity creation
        uint32_t AddPoint(double x, double y);
        uint32_t AddLine(double x1, double y1, double x2, double y2);
        uint32_t AddCircle(double cx, double cy, double r);

        // Constraints
        uint32_t AddConstraint(const SketchConstraint& constraint);
        uint32_t AddDimension(const ParametricDimension& dim);

        // Solving
        bool Solve(int maxIterations = 50, double tolerance = 1e-6);

        // Output
        SketchProfile GenerateProfile() const;

        // Accessors
        SketchEntity* GetEntity(uint32_t id);
        const SketchEntity* GetEntity(uint32_t id) const;
        
        const std::vector<std::unique_ptr<SketchEntity>>& GetEntities() const { return m_Entities; }
        const std::vector<SketchConstraint>& GetConstraints() const { return m_Constraints; }
        const std::vector<ParametricDimension>& GetDimensions() const { return m_Dimensions; }

        const Vector3& GetPlaneOrigin() const { return m_PlaneOrigin; }
        const Vector3& GetPlaneNormal() const { return m_PlaneNormal; }

    private:
        Vector3 m_PlaneOrigin;
        Vector3 m_PlaneNormal;
        Vector3 m_PlaneU; // Local X axis
        Vector3 m_PlaneV; // Local Y axis

        std::vector<std::unique_ptr<SketchEntity>> m_Entities;
        std::vector<SketchConstraint>              m_Constraints;
        std::vector<ParametricDimension>           m_Dimensions;

        uint32_t m_NextEntityID    = 1;
        uint32_t m_NextConstraintID = 1;
        uint32_t m_NextDimensionID  = 1;

        void UpdateLocalAxes();
        Vector3 LocalToWorld(double u, double v) const;
    };

} // namespace SZM::Geometry
