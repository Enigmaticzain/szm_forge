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
    const std::vector<double> X0 = X;

    auto buildResidualSystem = [&](const std::vector<double>& state,
                                   std::vector<double>& R,
                                   std::vector<double>* J) {
        R.clear();
        if (J) J->clear();

        auto addRow = [&](double residual, std::vector<std::pair<int,double>> partials) {
            R.push_back(residual);
            if (!J) return;
            std::vector<double> row(N, 0.0);
            for (auto [idx, val] : partials)
                if (idx >= 0 && idx < N) row[idx] = val;
            J->insert(J->end(), row.begin(), row.end());
        };

        for (const auto& c : constraints) {
            auto* e1 = sketch.GetEntity(c.Entity1ID);
            auto* e2 = c.Entity2ID ? sketch.GetEntity(c.Entity2ID) : nullptr;

            auto ptIdx = [&](const SketchEntity* e, int sub) -> std::pair<int,int> {
                if (!e) return {-1,-1};
                auto it = dofMap.entityOffset.find(e->ID);
                if (it == dofMap.entityOffset.end()) return {-1,-1};
                return {it->second + sub*2, it->second + sub*2 + 1};
            };

            int s1 = std::max(0, c.SubEntity1);
            int s2 = std::max(0, c.SubEntity2);

            switch (c.Type) {
                case ConstraintType::Fixed: {
                    if (!e1) break;
                    auto [ax, ay] = ptIdx(e1, s1);
                    if (ax >= 0) addRow(state[ax] - X0[ax], {{ax, 1.0}});
                    if (ay >= 0) addRow(state[ay] - X0[ay], {{ay, 1.0}});
                    break;
                }
                case ConstraintType::Coincident:
                case ConstraintType::Concentric: {
                    if (!e1 || !e2) break;
                    auto [ax, ay] = ptIdx(e1, (c.Type == ConstraintType::Concentric) ? 0 : s1);
                    auto [bx, by] = ptIdx(e2, (c.Type == ConstraintType::Concentric) ? 0 : s2);
                    if (ax >= 0 && bx >= 0) {
                        addRow(state[ax] - state[bx], {{ax, 1.0}, {bx, -1.0}});
                        addRow(state[ay] - state[by], {{ay, 1.0}, {by, -1.0}});
                    }
                    break;
                }
                case ConstraintType::Horizontal: {
                    if (!e1 || e1->Type == SketchEntityType::Point) break;
                    auto [p1x, p1y] = ptIdx(e1, 0);
                    auto [p2x, p2y] = ptIdx(e1, 1);
                    if (p1y >= 0 && p2y >= 0)
                        addRow(state[p1y] - state[p2y], {{p1y, 1.0}, {p2y, -1.0}});
                    break;
                }
                case ConstraintType::Vertical: {
                    if (!e1 || e1->Type == SketchEntityType::Point) break;
                    auto [p1x, p1y] = ptIdx(e1, 0);
                    auto [p2x, p2y] = ptIdx(e1, 1);
                    if (p1x >= 0 && p2x >= 0)
                        addRow(state[p1x] - state[p2x], {{p1x, 1.0}, {p2x, -1.0}});
                    break;
                }
                case ConstraintType::Parallel:
                case ConstraintType::Perpendicular: {
                    if (!e1 || !e2 || e1->Type == SketchEntityType::Point || e2->Type == SketchEntityType::Point) break;
                    auto [a1x, a1y] = ptIdx(e1, 0); auto [a2x, a2y] = ptIdx(e1, 1);
                    auto [b1x, b1y] = ptIdx(e2, 0); auto [b2x, b2y] = ptIdx(e2, 1);
                    double dax = state[a2x] - state[a1x], day = state[a2y] - state[a1y];
                    double dbx = state[b2x] - state[b1x], dby = state[b2y] - state[b1y];
                    if (c.Type == ConstraintType::Parallel) {
                        addRow(dax * dby - day * dbx, {
                            {a1x, -dby}, {a2x, dby}, {a1y, dbx}, {a2y, -dbx},
                            {b1x, day},  {b2x, -day},{b1y, -dax},{b2y, dax}
                        });
                    } else {
                        addRow(dax * dbx + day * dby, {
                            {a1x, -dbx}, {a2x, dbx}, {a1y, -dby}, {a2y, dby},
                            {b1x, -dax}, {b2x, dax}, {b1y, -day}, {b2y, day}
                        });
                    }
                    break;
                }
                case ConstraintType::Distance: {
                    if (!e1 || !e2) break;
                    double target = 0.0;
                    for (const auto& d : dimensions)
                        if (d.ID == c.DimensionID) { target = d.Value; break; }
                    auto [ax, ay] = ptIdx(e1, s1);
                    auto [bx, by] = ptIdx(e2, s2);
                    double dx = state[ax] - state[bx];
                    double dy = state[ay] - state[by];
                    double dist = std::sqrt(dx*dx + dy*dy);
                    if (dist < 1e-12) break;
                    addRow(dist - target, {
                        {ax,  dx/dist}, {ay,  dy/dist},
                        {bx, -dx/dist}, {by, -dy/dist}
                    });
                    break;
                }
                case ConstraintType::EqualLength:
                case ConstraintType::EqualRadius: {
                    if (!e1 || !e2 || e1->Type == SketchEntityType::Point || e2->Type == SketchEntityType::Point) break;
                    auto [a1x,a1y] = ptIdx(e1, 0); auto [a2x,a2y] = ptIdx(e1, 1);
                    auto [b1x,b1y] = ptIdx(e2, 0); auto [b2x,b2y] = ptIdx(e2, 1);
                    double dax = state[a2x] - state[a1x], day = state[a2y] - state[a1y];
                    double dbx = state[b2x] - state[b1x], dby = state[b2y] - state[b1y];
                    double la = std::sqrt(dax*dax + day*day);
                    double lb = std::sqrt(dbx*dbx + dby*dby);
                    if (la < 1e-12 || lb < 1e-12) break;
                    addRow(la - lb, {
                        {a1x,-dax/la},{a1y,-day/la},{a2x,dax/la},{a2y,day/la},
                        {b1x, dbx/lb},{b1y, dby/lb},{b2x,-dbx/lb},{b2y,-dby/lb}
                    });
                    break;
                }
                default: break;
            }
        }
    };

    auto residualNorm = [](const std::vector<double>& R) {
        double sumSq = 0.0;
        for (double r : R) sumSq += r * r;
        return std::sqrt(sumSq);
    };

    bool converged = false;
    for (int iter = 0; iter < maxIterations; ++iter) {
        std::vector<double> R;
        std::vector<double> J;
        buildResidualSystem(X, R, &J);

        if (R.empty()) {
            converged = true;
            break;
        }

        const int M = static_cast<int>(R.size());
        if (residualNorm(R) < tolerance) {
            converged = true;
            break;
        }

        std::vector<double> JtR(N, 0.0);
        std::vector<double> JtJ(N * N, 0.0);
        for (int r = 0; r < M; ++r) {
            for (int c = 0; c < N; ++c) {
                JtR[c] += J[r*N+c] * R[r];
                for (int k = 0; k < N; ++k)
                    JtJ[c*N+k] += J[r*N+c] * J[r*N+k];
            }
        }
        for (int i = 0; i < N; ++i) JtJ[i*N+i] += LAMBDA;

        std::vector<double> dX(N, 0.0);
        std::vector<std::vector<double>> aug(N, std::vector<double>(N+1));
        for (int i = 0; i < N; ++i) {
            for (int j = 0; j < N; ++j) aug[i][j] = JtJ[i*N+j];
            aug[i][N] = -JtR[i];
        }
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
        for (int i = N-1; i >= 0; --i) {
            if (std::abs(aug[i][i]) < 1e-14) continue;
            dX[i] = aug[i][N];
            for (int j = i+1; j < N; ++j) dX[i] -= aug[i][j] * dX[j];
            dX[i] /= aug[i][i];
        }

        for (int i = 0; i < N; ++i) X[i] += dX[i];
    }

    if (!converged) {
        std::vector<double> finalR;
        buildResidualSystem(X, finalR, nullptr);
        converged = finalR.empty() || (residualNorm(finalR) < tolerance);
    }

    if (converged) {
        Unpack(X, entities);
        std::cout << "[ConstraintSolver2D] Solved " << constraints.size() << " constraints, "
                  << N << " DOFs\n";
    } else {
        std::cerr << "[ConstraintSolver2D] Failed to converge within "
                  << maxIterations << " iterations\n";
    }
    return converged;
}

} // namespace SZM::Geometry
