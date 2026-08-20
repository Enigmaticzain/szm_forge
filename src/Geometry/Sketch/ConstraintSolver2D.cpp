#include "ConstraintSolver2D.hpp"
#include <cmath>
#include <vector>
#include <unordered_map>
#include <iostream>

namespace SZM::Geometry {

// Each DOF is a coordinate of a point in a sketch entity.
// We flatten all free point coordinates into a state vector X,
// build residuals R and Jacobian J per constraint, then iterate:
//   X -= J^+ * R   (pseudo-inverse via damped least-squares)

static constexpr double LAMBDA = 1e-6; // Levenberg-Marquardt damping

// Map entity ID + sub-index to DOF index in X
// Sub-index: 0=P1x,1=P1y, 2=P2x,3=P2y, 4=P3x,5=P3y
struct DOFMap {
    std::unordered_map<uint32_t, int> entityOffset; // entity ID -> base index in X
    int totalDOF = 0;

    void Build(const std::vector<std::unique_ptr<SketchEntity>>& entities) {
        totalDOF = 0;
        for (const auto& e : entities) {
            entityOffset[e->ID] = totalDOF;
            switch (e->Type) {
                case SketchEntityType::Point:  totalDOF += 2; break;
                case SketchEntityType::Line:   totalDOF += 4; break;
                case SketchEntityType::Circle: totalDOF += 4; break; // center + radius point
                case SketchEntityType::Arc:    totalDOF += 6; break;
            }
        }
    }

    // Get pointer into X for entity e, sub-index s (0=P1x,1=P1y,2=P2x,...)
    int Idx(uint32_t entityID, int sub) const {
        auto it = entityOffset.find(entityID);
        if (it == entityOffset.end()) return -1;
        return it->second + sub;
    }
};

// Pack entity coordinates into flat vector
static std::vector<double> Pack(const std::vector<std::unique_ptr<SketchEntity>>& entities) {
    std::vector<double> X;
    for (const auto& e : entities) {
        X.push_back(e->P1[0]); X.push_back(e->P1[1]);
        if (e->Type != SketchEntityType::Point) {
            X.push_back(e->P2[0]); X.push_back(e->P2[1]);
        }
        if (e->Type == SketchEntityType::Arc) {
            X.push_back(e->P3[0]); X.push_back(e->P3[1]);
        }
    }
    return X;
}

// Unpack flat vector back into entities
static void Unpack(const std::vector<double>& X,
                   std::vector<std::unique_ptr<SketchEntity>>& entities) {
    int i = 0;
    for (auto& e : entities) {
        e->P1[0] = X[i++]; e->P1[1] = X[i++];
        if (e->Type != SketchEntityType::Point) {
            e->P2[0] = X[i++]; e->P2[1] = X[i++];
        }
        if (e->Type == SketchEntityType::Arc) {
            e->P3[0] = X[i++]; e->P3[1] = X[i++];
        }
    }
}

bool ConstraintSolver2D::Solve(Sketch2D& sketch, int maxIterations, double tolerance) {
    auto& entities    = const_cast<std::vector<std::unique_ptr<SketchEntity>>&>(sketch.GetEntities());
    auto& constraints = sketch.GetConstraints();
    auto& dimensions  = sketch.GetDimensions();

    if (constraints.empty()) return true;

    DOFMap dofMap;
    dofMap.Build(entities);
    const int N = dofMap.totalDOF;
    if (N == 0) return true;

    std::vector<double> X = Pack(entities);

    for (int iter = 0; iter < maxIterations; ++iter) {
        // Build residual R and Jacobian J (M constraints x N DOFs)
        std::vector<double> R;
        // J stored row-major: J[row * N + col]
        std::vector<double> J;

        auto addRow = [&](double residual, std::vector<std::pair<int,double>> partials) {
            R.push_back(residual);
            std::vector<double> row(N, 0.0);
            for (auto [idx, val] : partials)
                if (idx >= 0 && idx < N) row[idx] = val;
            J.insert(J.end(), row.begin(), row.end());
        };

        for (const auto& c : constraints) {
            auto* e1 = sketch.GetEntity(c.Entity1ID);
            auto* e2 = c.Entity2ID ? sketch.GetEntity(c.Entity2ID) : nullptr;

            // Resolve sub-entity point indices
            auto ptIdx = [&](const SketchEntity* e, int sub) -> std::pair<int,int> {
                if (!e) return {-1,-1};
                int base = dofMap.entityOffset.at(e->ID);
                // sub: 0=P1, 1=P2, 2=P3
                return {base + sub*2, base + sub*2 + 1};
            };

            int s1 = std::max(0, c.SubEntity1);
            int s2 = std::max(0, c.SubEntity2);

            switch (c.Type) {
                case ConstraintType::Fixed: {
                    if (!e1) break;
                    // Fix P1 to its current value (zero residual always — skip)
                    break;
                }
                case ConstraintType::Coincident: {
                    if (!e1 || !e2) break;
                    auto [ax, ay] = ptIdx(e1, s1);
                    auto [bx, by] = ptIdx(e2, s2);
                    // Rx = X[ax] - X[bx] = 0
                    addRow(X[ax] - X[bx], {{ax, 1.0}, {bx, -1.0}});
                    addRow(X[ay] - X[by], {{ay, 1.0}, {by, -1.0}});
                    break;
                }
                case ConstraintType::Horizontal: {
                    if (!e1) break;
                    // Line: P1y == P2y
                    auto [p1x, p1y] = ptIdx(e1, 0);
                    auto [p2x, p2y] = ptIdx(e1, 1);
                    addRow(X[p1y] - X[p2y], {{p1y, 1.0}, {p2y, -1.0}});
                    break;
                }
                case ConstraintType::Vertical: {
                    if (!e1) break;
                    auto [p1x, p1y] = ptIdx(e1, 0);
                    auto [p2x, p2y] = ptIdx(e1, 1);
                    addRow(X[p1x] - X[p2x], {{p1x, 1.0}, {p2x, -1.0}});
                    break;
                }
                case ConstraintType::Distance: {
                    if (!e1 || !e2) break;
                    // Find driving dimension value
                    double target = 0.0;
                    for (const auto& d : dimensions)
                        if (d.ID == c.DimensionID) { target = d.Value; break; }
                    auto [ax, ay] = ptIdx(e1, s1);
                    auto [bx, by] = ptIdx(e2, s2);
                    double dx = X[ax] - X[bx];
                    double dy = X[ay] - X[by];
                    double dist = std::sqrt(dx*dx + dy*dy);
                    if (dist < 1e-12) break;
                    double res = dist - target;
                    addRow(res, {
                        {ax,  dx/dist}, {ay,  dy/dist},
                        {bx, -dx/dist}, {by, -dy/dist}
                    });
                    break;
                }
                case ConstraintType::EqualLength: {
                    if (!e1 || !e2) break;
                    auto [a1x,a1y] = ptIdx(e1, 0); auto [a2x,a2y] = ptIdx(e1, 1);
                    auto [b1x,b1y] = ptIdx(e2, 0); auto [b2x,b2y] = ptIdx(e2, 1);
                    double dax=X[a2x]-X[a1x], day=X[a2y]-X[a1y];
                    double dbx=X[b2x]-X[b1x], dby=X[b2y]-X[b1y];
                    double la=std::sqrt(dax*dax+day*day);
                    double lb=std::sqrt(dbx*dbx+dby*dby);
                    if (la<1e-12||lb<1e-12) break;
                    addRow(la - lb, {
                        {a1x,-dax/la},{a1y,-day/la},{a2x,dax/la},{a2y,day/la},
                        {b1x, dbx/lb},{b1y, dby/lb},{b2x,-dbx/lb},{b2y,-dby/lb}
                    });
                    break;
                }
                default: break;
            }
        }

        if (R.empty()) break;

        const int M = static_cast<int>(R.size());

        // Compute residual norm
        double rNorm = 0.0;
        for (double r : R) rNorm += r * r;
        rNorm = std::sqrt(rNorm);
        if (rNorm < tolerance) break;

        // Damped least-squares: dX = -(J^T J + lambda I)^-1 J^T R
        // Simple Gauss-Seidel approximation for small systems:
        // Use J^T * (J * dX + R) = 0  =>  (J^T J + lI) dX = -J^T R
        std::vector<double> JtR(N, 0.0);
        std::vector<double> JtJ(N * N, 0.0);
        for (int r = 0; r < M; ++r) {
            for (int c = 0; c < N; ++c) {
                JtR[c] += J[r*N+c] * R[r];
                for (int k = 0; k < N; ++k)
                    JtJ[c*N+k] += J[r*N+c] * J[r*N+k];
            }
        }
        // Add damping
        for (int i = 0; i < N; ++i) JtJ[i*N+i] += LAMBDA;

        // Solve (JtJ) dX = -JtR via Gauss elimination (small N)
        std::vector<double> dX(N, 0.0);
        // Augmented matrix [JtJ | -JtR]
        std::vector<std::vector<double>> aug(N, std::vector<double>(N+1));
        for (int i = 0; i < N; ++i) {
            for (int j = 0; j < N; ++j) aug[i][j] = JtJ[i*N+j];
            aug[i][N] = -JtR[i];
        }
        // Forward elimination
        for (int col = 0; col < N; ++col) {
            int pivot = col;
            for (int row = col+1; row < N; ++row)
                if (std::abs(aug[row][col]) > std::abs(aug[pivot][col])) pivot = row;
            std::swap(aug[col], aug[pivot]);
            if (std::abs(aug[col][col]) < 1e-14) continue;
            double inv = 1.0 / aug[col][col];
            for (int row = col+1; row < N; ++row) {
                double f = aug[row][col] * inv;
                for (int k = col; k <= N; ++k) aug[row][k] -= f * aug[col][k];
            }
        }
        // Back substitution
        for (int i = N-1; i >= 0; --i) {
            if (std::abs(aug[i][i]) < 1e-14) continue;
            dX[i] = aug[i][N];
            for (int j = i+1; j < N; ++j) dX[i] -= aug[i][j] * dX[j];
            dX[i] /= aug[i][i];
        }

        for (int i = 0; i < N; ++i) X[i] += dX[i];
    }

    Unpack(X, entities);
    std::cout << "[ConstraintSolver2D] Solved " << constraints.size() << " constraints, "
              << N << " DOFs\n";
    return true;
}

} // namespace SZM::Geometry
