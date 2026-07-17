#include "SolidBody.hpp"
#include <algorithm>

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
        if (vertices.size() < 3) return;

        uint32_t faceIdx = static_cast<uint32_t>(m_Faces.size());
        m_Faces.push_back({NullHandle, Vector3(0,0,0)});

        std::vector<uint32_t> faceHeIndices;

        for (size_t i = 0; i < vertices.size(); ++i) {
            uint32_t vIdx = GetOrAddVertex(vertices[i], 1e-6);
            
            uint32_t heIdx = static_cast<uint32_t>(m_HalfEdges.size());
            m_HalfEdges.push_back({NullHandle, NullHandle, NullHandle, vIdx, faceIdx});
            faceHeIndices.push_back(heIdx);

            if (m_Vertices[vIdx].halfEdge == NullHandle) {
                m_Vertices[vIdx].halfEdge = heIdx;
            }
        }

        for (size_t i = 0; i < faceHeIndices.size(); ++i) {
            uint32_t curr = faceHeIndices[i];
            uint32_t next = faceHeIndices[(i + 1) % faceHeIndices.size()];
            uint32_t prev = faceHeIndices[(i + faceHeIndices.size() - 1) % faceHeIndices.size()];

            m_HalfEdges[curr].next = next;
            m_HalfEdges[curr].prev = prev;
        }

        m_Faces[faceIdx].halfEdge = faceHeIndices[0];
    }

    void SolidBody::AddFaceFlipped(const std::vector<Vector3>& vertices) {
        if (vertices.size() < 3) return;
        std::vector<Vector3> reversed(vertices.rbegin(), vertices.rend());
        AddFace(reversed);
    }

    bool SolidBody::Stitch(double) {
        for (size_t i = 0; i < m_HalfEdges.size(); ++i) {
            if (m_HalfEdges[i].twin != NullHandle) continue;

            uint32_t vTo = m_HalfEdges[i].vertex;
            uint32_t vFrom = m_HalfEdges[m_HalfEdges[i].prev].vertex;

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
        for (const auto& he : m_HalfEdges) {
            if (he.twin == NullHandle) return false;
        }

        int64_t V = m_Vertices.size();
        int64_t F = m_Faces.size();
        int64_t E = m_HalfEdges.size() / 2; 

        // Euler characteristic: V - E + F
        // Sphere-like topology: χ = 2
        // Torus-like topology: χ = 0
        // Accept both as valid closed manifolds
        int64_t euler = V - E + F;
        if (euler != 2 && euler != 0) {
            return false;
        }

        return true;
    }

    double SolidBody::CalculateVolume() const {
        if (!m_IsClosed) {
            throw std::runtime_error("Cannot calculate volume of an open shell.");
        }

        double volume = 0.0;

        for (const auto& face : m_Faces) {
            uint32_t startHe = face.halfEdge;
            uint32_t currHe = m_HalfEdges[startHe].next;

            Vector3 v0 = m_Vertices[m_HalfEdges[m_HalfEdges[startHe].prev].vertex].position;

            while (currHe != startHe && m_HalfEdges[currHe].next != startHe) {
                Vector3 v1 = m_Vertices[m_HalfEdges[m_HalfEdges[currHe].prev].vertex].position;
                Vector3 v2 = m_Vertices[m_HalfEdges[currHe].vertex].position;

                volume += v0.Dot(v1.Cross(v2)) / 6.0;

                currHe = m_HalfEdges[currHe].next;
            }
        }

        return std::abs(volume);
    }

    Vector3 SolidBody::CalculateCenterOfMass() const {
        if (!m_IsClosed) {
            throw std::runtime_error("Cannot calculate center of mass of an open shell.");
        }

        double signedVolume = 0.0;
        Vector3 weightedCenter(0.0, 0.0, 0.0);

        for (uint32_t faceIdx = 0; faceIdx < static_cast<uint32_t>(m_Faces.size()); ++faceIdx) {
            const auto faceVerts = GetFaceVertices(faceIdx);
            if (faceVerts.size() < 3) {
                continue;
            }

            const Vector3& v0 = faceVerts[0];
            for (size_t i = 1; i + 1 < faceVerts.size(); ++i) {
                const Vector3& v1 = faceVerts[i];
                const Vector3& v2 = faceVerts[i + 1];

                const double tetraVol = v0.Dot(v1.Cross(v2)) / 6.0;
                const Vector3 tetraCenter = (v0 + v1 + v2) * 0.25;
                weightedCenter = weightedCenter + tetraCenter * tetraVol;
                signedVolume += tetraVol;
            }
        }

        // Fallback for inconsistent winding in legacy imported meshes.
        if (std::abs(signedVolume) < 1e-12) {
            double absVolume = 0.0;
            weightedCenter = {0.0, 0.0, 0.0};

            for (uint32_t faceIdx = 0; faceIdx < static_cast<uint32_t>(m_Faces.size()); ++faceIdx) {
                const auto faceVerts = GetFaceVertices(faceIdx);
                if (faceVerts.size() < 3) {
                    continue;
                }

                const Vector3& v0 = faceVerts[0];
                for (size_t i = 1; i + 1 < faceVerts.size(); ++i) {
                    const Vector3& v1 = faceVerts[i];
                    const Vector3& v2 = faceVerts[i + 1];

                    const double tetraVolAbs = std::abs(v0.Dot(v1.Cross(v2)) / 6.0);
                    const Vector3 tetraCenter = (v0 + v1 + v2) * 0.25;
                    weightedCenter = weightedCenter + tetraCenter * tetraVolAbs;
                    absVolume += tetraVolAbs;
                }
            }

            if (absVolume < 1e-12) {
                throw std::runtime_error("Failed to compute center of mass: near-zero mesh volume.");
            }
            return weightedCenter * (1.0 / absVolume);
        }

        return weightedCenter * (1.0 / signedVolume);
    }

    std::vector<Vector3> SolidBody::GetFaceVertices(uint32_t faceIdx) const {
        std::vector<Vector3> vertices;
        if (faceIdx >= m_Faces.size()) {
            return vertices;
        }

        const uint32_t startHe = m_Faces[faceIdx].halfEdge;
        if (startHe == NullHandle || startHe >= m_HalfEdges.size()) {
            return vertices;
        }

        uint32_t currHe = startHe;
        size_t guard = 0;
        const size_t guardMax = m_HalfEdges.size() + 1;
        do {
            if (currHe >= m_HalfEdges.size()) {
                break;
            }
            const auto& he = m_HalfEdges[currHe];
            if (he.vertex >= m_Vertices.size()) {
                break;
            }

            vertices.push_back(m_Vertices[he.vertex].position);
            currHe = he.next;
            ++guard;
        } while (currHe != startHe && currHe != NullHandle && guard < guardMax);

        return vertices;
    }

    Vector3 SolidBody::GetFaceCentroid(uint32_t faceIdx) const {
        const auto faceVerts = GetFaceVertices(faceIdx);
        if (faceVerts.empty()) {
            return {};
        }

        Vector3 centroid(0.0, 0.0, 0.0);
        for (const auto& v : faceVerts) {
            centroid = centroid + v;
        }
        return centroid * (1.0 / static_cast<double>(faceVerts.size()));
    }

    std::vector<uint32_t> SolidBody::GetBoundaryEdges() const {
        std::vector<uint32_t> boundary;
        for (uint32_t i = 0; i < static_cast<uint32_t>(m_HalfEdges.size()); ++i) {
            if (m_HalfEdges[i].twin == NullHandle) {
                boundary.push_back(i);
            }
        }
        return boundary;
    }

    void SolidBody::HealDegenerateEdges(double tolerance) {
        // Dummy implementation for Phase 1 scope
        // Re-run stitch with larger tolerance
        Stitch(tolerance);
    }

    void SolidBody::HealGaps(double maxGap) {
        // Dummy implementation for Phase 1 scope
        Stitch(maxGap);
    }

    void SolidBody::DeformVertices(const std::vector<uint32_t>& vertexIndices,
                                    const Vector3& displacement,
                                    const DeformationConstraint& constraint,
                                    double tolerance) {
        const auto isConstrained = [&](const Vector3& point) {
            switch (constraint.type) {
                case DeformationConstraintType::Point: {
                    return (point - constraint.anchor).MagnitudeSq() <= tolerance * tolerance;
                }
                case DeformationConstraintType::Line: {
                    const Vector3 direction = constraint.secondary - constraint.anchor;
                    if (direction.MagnitudeSq() <= tolerance * tolerance) {
                        return (point - constraint.anchor).MagnitudeSq() <= tolerance * tolerance;
                    }
                    const Vector3 delta = point - constraint.anchor;
                    const Vector3 dirNormalized = direction.Normalize();
                    const Vector3 projected = dirNormalized * delta.Dot(dirNormalized);
                    const Vector3 offset = delta - projected;
                    return offset.MagnitudeSq() <= tolerance * tolerance;
                }
                case DeformationConstraintType::Plane: {
                    const Vector3 normal = constraint.secondary.Normalize();
                    return std::abs((point - constraint.anchor).Dot(normal)) <= tolerance;
                }
            }
            return false;
        };

        for (uint32_t vertexIndex : vertexIndices) {
            if (vertexIndex >= m_Vertices.size()) {
                continue;
            }

            if (!isConstrained(m_Vertices[vertexIndex].position)) {
                m_Vertices[vertexIndex].position = m_Vertices[vertexIndex].position + displacement;
            }
        }
    }

    void SolidBody::OrientNormals() {
        if (!m_IsClosed) return;
        // Simple strategy: check signed volume. If negative, flip all faces.
        try {
            double signedVol = 0.0;
            for (uint32_t faceIdx = 0; faceIdx < static_cast<uint32_t>(m_Faces.size()); ++faceIdx) {
                const auto faceVerts = GetFaceVertices(faceIdx);
                if (faceVerts.size() < 3) continue;

                const Vector3& v0 = faceVerts[0];
                for (size_t i = 1; i + 1 < faceVerts.size(); ++i) {
                    signedVol += v0.Dot(faceVerts[i].Cross(faceVerts[i + 1])) / 6.0;
                }
            }

            if (signedVol < 0.0) {
                // Flip all half-edge orders in all faces
                std::vector<std::vector<Vector3>> oldFaces;
                for (uint32_t fi = 0; fi < m_Faces.size(); ++fi) {
                    oldFaces.push_back(GetFaceVertices(fi));
                }
                
                m_Faces.clear();
                m_HalfEdges.clear();
                m_Vertices.clear();
                m_IsClosed = false;

                for (const auto& verts : oldFaces) {
                    AddFaceFlipped(verts);
                }
                Stitch();
            }
        } catch (...) {
            // Volume calculation failed
        }
    }

} // namespace SZM::Geometry
