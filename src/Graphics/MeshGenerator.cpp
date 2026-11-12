#include "MeshGenerator.hpp"
#include "../Geometry/SolidBody.hpp"
#include <cmath>
#include <algorithm>
#include <array>
#include <limits>

namespace SZM::Graphics {

    namespace {
        using SZM::Geometry::Vector3;

        RenderVertex MakeVertex(
            const Vector3& position,
            const Vector3& normal,
            double u,
            double v
        ) {
            return RenderVertex{
                {
                    static_cast<float>(position.x),
                    static_cast<float>(position.y),
                    static_cast<float>(position.z)
                },
                {
                    static_cast<float>(normal.x),
                    static_cast<float>(normal.y),
                    static_cast<float>(normal.z)
                },
                {
                    static_cast<float>(u),
                    static_cast<float>(v)
                }
            };
        }

        Vector3 CalculateNewellNormal(const std::vector<Vector3>& vertices) {
            Vector3 normal{0.0, 0.0, 0.0};
            if (vertices.size() < 3) {
                return {0.0, 1.0, 0.0};
            }

            for (size_t i = 0; i < vertices.size(); ++i) {
                const Vector3& current = vertices[i];
                const Vector3& next = vertices[(i + 1) % vertices.size()];
                normal.x += (current.y - next.y) * (current.z + next.z);
                normal.y += (current.z - next.z) * (current.x + next.x);
                normal.z += (current.x - next.x) * (current.y + next.y);
            }

            if (normal.MagnitudeSq() > 1e-18) {
                return normal.Normalize();
            }

            for (size_t i = 1; i + 1 < vertices.size(); ++i) {
                const Vector3 fallback = (vertices[i] - vertices[0]).Cross(vertices[i + 1] - vertices[0]);
                if (fallback.MagnitudeSq() > 1e-18) {
                    return fallback.Normalize();
                }
            }

            return {0.0, 1.0, 0.0};
        }

        std::array<int, 2> PickProjectionAxes(const Vector3& normal) {
            const double ax = std::abs(normal.x);
            const double ay = std::abs(normal.y);
            const double az = std::abs(normal.z);

            if (ax >= ay && ax >= az) {
                return {1, 2}; // project to YZ
            }
            if (ay >= ax && ay >= az) {
                return {0, 2}; // project to XZ
            }
            return {0, 1}; // project to XY
        }

        double AxisValue(const Vector3& value, int axis) {
            switch (axis) {
                case 0: return value.x;
                case 1: return value.y;
                default: return value.z;
            }
        }
    }

    RenderMesh MeshGenerator::TessellateSolid(
        const SZM::Geometry::SolidBody& body,
        const TessellationConfig& config
    ) {
        RenderMesh mesh;

        const uint32_t faceCount = static_cast<uint32_t>(body.GetFaceCount());
        if (faceCount == 0U || config.MaxTriangles == 0U) {
            return mesh;
        }

        mesh.Vertices.reserve(body.GetVertexCount() * 3U);
        mesh.Indices.reserve(std::min<uint32_t>(config.MaxTriangles, faceCount * 2U) * 3U);

        uint32_t emittedTriangles = 0U;
        for (uint32_t faceIndex = 0; faceIndex < faceCount; ++faceIndex) {
            const std::vector<Vector3> faceVertices = body.GetFaceVertices(faceIndex);
            if (faceVertices.size() < 3) {
                continue;
            }

            const uint32_t trianglesForFace = static_cast<uint32_t>(faceVertices.size() - 2);
            if (emittedTriangles + trianglesForFace > config.MaxTriangles) {
                break;
            }

            const Vector3 normal = CalculateNewellNormal(faceVertices);
            const auto projectionAxes = PickProjectionAxes(normal);

            double minU = std::numeric_limits<double>::max();
            double maxU = std::numeric_limits<double>::lowest();
            double minV = std::numeric_limits<double>::max();
            double maxV = std::numeric_limits<double>::lowest();

            for (const Vector3& vertex : faceVertices) {
                const double u = AxisValue(vertex, projectionAxes[0]);
                const double v = AxisValue(vertex, projectionAxes[1]);
                minU = std::min(minU, u);
                maxU = std::max(maxU, u);
                minV = std::min(minV, v);
                maxV = std::max(maxV, v);
            }

            const double rangeU = std::max(1e-9, maxU - minU);
            const double rangeV = std::max(1e-9, maxV - minV);
            const uint32_t vertexOffset = static_cast<uint32_t>(mesh.Vertices.size());

            for (const Vector3& vertex : faceVertices) {
                const double projectedU = AxisValue(vertex, projectionAxes[0]);
                const double projectedV = AxisValue(vertex, projectionAxes[1]);
                mesh.Vertices.push_back(MakeVertex(
                    vertex,
                    normal,
                    (projectedU - minU) / rangeU,
                    (projectedV - minV) / rangeV
                ));
            }

            const size_t nVerts = faceVertices.size();
            if (nVerts == 3) {
                mesh.Indices.push_back(vertexOffset);
                mesh.Indices.push_back(vertexOffset + 1);
                mesh.Indices.push_back(vertexOffset + 2);
                ++emittedTriangles;
            } else if (nVerts > 3) {
                // Ear clipping in projected 2D coordinates
                struct P2D { double u, v; uint32_t idx; };
                std::vector<P2D> poly(nVerts);
                for (size_t i = 0; i < nVerts; ++i) {
                    poly[i] = {
                        AxisValue(faceVertices[i], projectionAxes[0]),
                        AxisValue(faceVertices[i], projectionAxes[1]),
                        static_cast<uint32_t>(vertexOffset + i)
                    };
                }

                // Compute signed area
                double signedArea = 0.0;
                for (size_t i = 0; i < nVerts; ++i) {
                    size_t j = (i + 1) % nVerts;
                    signedArea += (poly[i].u * poly[j].v - poly[j].u * poly[i].v);
                }

                auto isConvex = [signedArea](const P2D& a, const P2D& b, const P2D& c) {
                    double cross = (b.u - a.u) * (c.v - a.v) - (b.v - a.v) * (c.u - a.u);
                    return signedArea >= 0.0 ? (cross > 1e-9) : (cross < -1e-9);
                };

                auto pointInTriangle = [](const P2D& p, const P2D& a, const P2D& b, const P2D& c) {
                    double cp1 = (b.u - a.u) * (p.v - a.v) - (b.v - a.v) * (p.u - a.u);
                    double cp2 = (c.u - b.u) * (p.v - b.v) - (c.v - b.v) * (p.u - b.u);
                    double cp3 = (a.u - c.u) * (p.v - c.v) - (a.v - c.v) * (p.u - c.u);
                    return (cp1 >= -1e-9 && cp2 >= -1e-9 && cp3 >= -1e-9) ||
                           (cp1 <= 1e-9 && cp2 <= 1e-9 && cp3 <= 1e-9);
                };

                while (poly.size() > 3) {
                    bool earFound = false;
                    for (size_t i = 0; i < poly.size(); ++i) {
                        size_t prev = (i == 0) ? (poly.size() - 1) : (i - 1);
                        size_t next = (i + 1) % poly.size();

                        if (!isConvex(poly[prev], poly[i], poly[next])) continue;

                        bool hasInnerPoint = false;
                        for (size_t j = 0; j < poly.size(); ++j) {
                            if (j == prev || j == i || j == next) continue;
                            if (pointInTriangle(poly[j], poly[prev], poly[i], poly[next])) {
                                hasInnerPoint = true;
                                break;
                            }
                        }

                        if (!hasInnerPoint) {
                            mesh.Indices.push_back(poly[prev].idx);
                            mesh.Indices.push_back(poly[i].idx);
                            mesh.Indices.push_back(poly[next].idx);
                            ++emittedTriangles;
                            poly.erase(poly.begin() + i);
                            earFound = true;
                            break;
                        }
                    }
                    if (!earFound) {
                        // Fallback fan for remaining degenerate vertices
                        for (size_t k = 1; k + 1 < poly.size(); ++k) {
                            mesh.Indices.push_back(poly[0].idx);
                            mesh.Indices.push_back(poly[k].idx);
                            mesh.Indices.push_back(poly[k + 1].idx);
                            ++emittedTriangles;
                        }
                        break;
                    }
                }
                if (poly.size() == 3) {
                    mesh.Indices.push_back(poly[0].idx);
                    mesh.Indices.push_back(poly[1].idx);
                    mesh.Indices.push_back(poly[2].idx);
                    ++emittedTriangles;
                }
            }
        }

        return mesh;
    }

    uint32_t MeshGenerator::CalculateSegmentCount(double radius, double chordalError) {
        if (radius <= 0.0 || chordalError <= 0.0) {
            return 3;
        }

        // Degenerate case: error >= radius
        if (chordalError >= radius) {
            return 3;
        }

        // Full-circle chordal error bound:
        //   sagitta = R * (1 - cos(π / N)) <= ε
        //   N >= π / arccos(1 - ε / R)
        
        double ratio = 1.0 - (chordalError / radius);
        double angle = std::acos(std::max(-1.0, std::min(1.0, ratio)));
        
        if (angle <= 0.0) {
            return 3;
        }

        uint32_t segments = static_cast<uint32_t>(std::ceil(3.14159265359 / angle));
        return std::max(3u, std::min(segments, 1024u));
    }

    double MeshGenerator::ResolveChordalError(const TessellationConfig& config) {
        const double baseError = std::max(1e-6, config.MaxChordalError);
        if (!config.EnableDynamicLOD) {
            return baseError;
        }

        const double distanceScale = std::clamp(config.CameraDistance, 0.25, 12.0);
        return baseError * distanceScale;
    }

} // namespace SZM::Graphics
