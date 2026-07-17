#include "SolidBody.hpp"


// ===== NEW ADDITIONS FROM CODE BUNDLES =====

namespace SZM::Geometry {

    uint32_t SolidBody::GetOrAddVertex(const Vector3& pos, double tolerance) {
        for (size_t i = 0; i < m_Vertices.size(); ++i) {
            Vector3 diff = m_Vertices[i].position - pos;
            if ((diff.x*diff.x + diff.y*diff.y + diff.z*diff.z) <= (tolerance * tolerance)) {
                return static_cast<uint32_t>(i);
            }
        }
        m_Vertices.push_back({pos, NullHandle});
        return static_cast<uint32_t>(m_Vertices.size() - 1);
    }

    void SolidBody::AddFace(const std::vector<Vector3>& vertices) {
        if (vertices.size() < 3) return; // Degenerate face

        uint32_t faceIdx = static_cast<uint32_t>(m_Faces.size());
        m_Faces.push_back({NullHandle, Vector3(0,0,0)}); // Normal calculated later

        std::vector<uint32_t> faceHeIndices;

        // Create half-edges for this face
        for (size_t i = 0; i < vertices.size(); ++i) {
            uint32_t vIdx = GetOrAddVertex(vertices[i], 1e-6);
            
            uint32_t heIdx = static_cast<uint32_t>(m_HalfEdges.size());
            m_HalfEdges.push_back({NullHandle, NullHandle, NullHandle, vIdx, faceIdx});
            faceHeIndices.push_back(heIdx);

            if (m_Vertices[vIdx].halfEdge == NullHandle) {
                m_Vertices[vIdx].halfEdge = heIdx;
            }
        }

        // Link next/prev for the newly created half-edges
        for (size_t i = 0; i < faceHeIndices.size(); ++i) {
            uint32_t curr = faceHeIndices[i];
            uint32_t next = faceHeIndices[(i + 1) % faceHeIndices.size()];
            uint32_t prev = faceHeIndices[(i + faceHeIndices.size() - 1) % faceHeIndices.size()];

            m_HalfEdges[curr].next = next;
            m_HalfEdges[curr].prev = prev;
        }

        m_Faces[faceIdx].halfEdge = faceHeIndices[0];
    }

    bool SolidBody::Stitch(double tolerance) {
        // Find twins for all half-edges
        // A twin goes from Vertex B to Vertex A, while the half-edge goes from A to B
        for (size_t i = 0; i < m_HalfEdges.size(); ++i) {
            if (m_HalfEdges[i].twin != NullHandle) continue;

            uint32_t vTo = m_HalfEdges[i].vertex;
            uint32_t vFrom = m_HalfEdges[m_HalfEdges[i].prev].vertex;

            // Search for the matching twin
            for (size_t j = i + 1; j < m_HalfEdges.size(); ++j) {
                if (m_HalfEdges[j].twin != NullHandle) continue;

                uint32_t twinVTo = m_HalfEdges[j].vertex;
                uint32_t twinVFrom = m_HalfEdges[m_HalfEdges[j].prev].vertex;

                if (vTo == twinVFrom && vFrom == twinVTo) {
                    m_HalfEdges[i].twin = static_cast<uint32_t>(j);
                    m_HalfEdges[j].twin = static_cast<uint32_t>(i);
                    break;
                }
            }
        }

        m_IsClosed = VerifyManifold();
        return m_IsClosed;
    }

    bool SolidBody::VerifyManifold() const {
        // 1. Water-tight check: Every half-edge must have a twin
        for (const auto& he : m_HalfEdges) {
            if (he.twin == NullHandle) return false;
        }

        // 2. Euler-Poincaré Characteristic: V - E + F = 2 (Assuming Genus 0)
        int64_t V = m_Vertices.size();
        int64_t F = m_Faces.size();
        int64_t E = m_HalfEdges.size() / 2; 

        if (V - E + F != 2) {
            return false;
        }

        return true;
    }

    double SolidBody::CalculateVolume() const {
        if (!m_IsClosed) {
            throw std::runtime_error("Cannot calculate volume of an open shell (non-manifold).");
        }

        double volume = 0.0;

        // Applying the discrete Divergence Theorem
        for (const auto& face : m_Faces) {
            uint32_t startHe = face.halfEdge;
            uint32_t currHe = m_HalfEdges[startHe].next;

            // Triangulate face using a triangle fan from the first vertex
            Vector3 v0 = m_Vertices[m_HalfEdges[m_HalfEdges[startHe].prev].vertex].position;

            while (currHe != startHe && m_HalfEdges[currHe].next != startHe) {
                Vector3 v1 = m_Vertices[m_HalfEdges[m_HalfEdges[currHe].prev].vertex].position;
                Vector3 v2 = m_Vertices[m_HalfEdges[currHe].vertex].position;

                // Signed volume of tetrahedron
                volume += v0.Dot(v1.Cross(v2)) / 6.0;

                currHe = m_HalfEdges[currHe].next;
            }
        }

        return std::abs(volume);
    }

    Vector3 SolidBody::CalculateCenterOfMass() const {
        // Implementation of volume-weighted centroid summation goes here.
        // Left as an extension following a similar surface integral pattern.
        return Vector3(0, 0, 0); 
    }

} // namespace SZM::Geometry
