#pragma once

#include "MathTypes.hpp"
#include <vector>
#include <cstdint>
#include <cmath>
#include <stdexcept>
#include <iostream>

namespace SZM::Geometry {

    struct Vertex {
        Vector3 position;
        uint32_t halfEdge = NullHandle;
    };

    struct Face {
        uint32_t halfEdge = NullHandle;
        Vector3 normal;
    };

    struct HalfEdge {
        uint32_t twin = NullHandle;
        uint32_t next = NullHandle;
        uint32_t prev = NullHandle;
        uint32_t vertex = NullHandle;
        uint32_t face = NullHandle;
    };

    enum class DeformationConstraintType {
        Point,
        Line,
        Plane
    };

    struct DeformationConstraint {
        DeformationConstraintType type = DeformationConstraintType::Point;
        Vector3 anchor{0.0, 0.0, 0.0};
        Vector3 secondary{0.0, 0.0, 0.0};
    };

    class SolidBody {
    public:
        SolidBody() = default;

        void AddFace(const std::vector<Vector3>& vertices);
        void AddFaceFlipped(const std::vector<Vector3>& vertices);
        bool Stitch(double tolerance = 1e-6);
        
        bool VerifyManifold() const;
        double CalculateVolume() const;
        Vector3 CalculateCenterOfMass() const;
        std::vector<Vector3> GetFaceVertices(uint32_t faceIdx) const;
        Vector3 GetFaceCentroid(uint32_t faceIdx) const;

        size_t GetVertexCount() const { return m_Vertices.size(); }
        size_t GetFaceCount() const { return m_Faces.size(); }
        size_t GetEdgeCount() const { return m_HalfEdges.size() / 2; }
        bool IsClosed() const { return m_IsClosed; }

        const std::vector<Vertex>& GetVertices() const { return m_Vertices; }
        std::vector<Vertex>& GetVerticesMutable() { return m_Vertices; }
        const std::vector<Face>& GetFaces() const { return m_Faces; }
        const std::vector<HalfEdge>& GetHalfEdges() const { return m_HalfEdges; }

        // Added functionality
        void HealDegenerateEdges(double tolerance = 1e-6);
        void HealGaps(double maxGap = 1e-4);
        std::vector<uint32_t> GetBoundaryEdges() const;
        void OrientNormals();
        void DeformVertices(const std::vector<uint32_t>& vertexIndices,
                            const Vector3& displacement,
                            const DeformationConstraint& constraint,
                            double tolerance = 1e-9);

    private:
        std::vector<Vertex> m_Vertices;
        std::vector<Face> m_Faces;
        std::vector<HalfEdge> m_HalfEdges;

        bool m_IsClosed = false;

        uint32_t GetOrAddVertex(const Vector3& pos, double tolerance);
    };

} // namespace SZM::Geometry
