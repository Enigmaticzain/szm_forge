#include "InterferenceCheck.hpp"
#include "../Geometry/operations/BooleanOp.hpp"
#include <thread>
#include <cmath>
#include <algorithm>
#include <iostream>

namespace SZM::Assembly {

// ------------------------------------------------- IsOverlapAllowed --
bool InterferenceChecker::IsOverlapAllowed(MaterialType a, MaterialType b) const {
    // Rubber gaskets are allowed to compress into adjacent metal faces
    if ((a == MaterialType::Rubber_Compressible && b == MaterialType::Metal) ||
        (b == MaterialType::Rubber_Compressible && a == MaterialType::Metal))
        return true;

    // Fluid volumes conform to their containers — overlap is by design
    if (a == MaterialType::Fluid_Volume || b == MaterialType::Fluid_Volume)
        return true;

    return false;
}

// ----------------------------------------------- NarrowPhaseBoolean --
std::optional<ClashVolume> InterferenceChecker::NarrowPhaseBoolean(
    const PartInstance& pA, const PartInstance& pB)
{
    if (!pA.Geometry || !pB.Geometry) {
        return std::nullopt;
    }

    try {
        auto intersection = SZM::Geometry::BooleanCSG::Execute(
            pA.Geometry,
            pB.Geometry,
            SZM::Geometry::BooleanOpType::Intersect
        );

        if (!intersection || !intersection->IsClosed()) {
            return std::nullopt;
        }

        const double volume = intersection->CalculateVolume();
        if (volume <= 0.0) {
            return std::nullopt;
        }

        return ClashVolume{
            pA.InstanceID,
            pB.InstanceID,
            volume,
            intersection
        };
    } catch (const std::exception&) {
        // Degenerate or non-manifold inputs are treated as no clash in this pass.
        return std::nullopt;
    }
}

// -------------------------------------------------- RunStaticCheck ---
std::vector<ClashVolume> InterferenceChecker::RunStaticCheck(
    const std::vector<PartInstance>& nodes, double tol)
{
    std::vector<ClashVolume> globalClashes;
    std::mutex               mutex;

    const size_t N = nodes.size();
    if (N < 2) return globalClashes;

    unsigned int hwThreads  = std::thread::hardware_concurrency();
    unsigned int numThreads = (hwThreads == 0) ? 4 : hwThreads;

    // Divide the outer loop across threads
    auto workerTask = [&](size_t start, size_t end) {
        std::vector<ClashVolume> local;
        for (size_t i = start; i < end; ++i) {
            for (size_t j = i + 1; j < N; ++j) {
                const auto& pA = nodes[i];
                const auto& pB = nodes[j];

                // 1. Material rule (fastest escape)
                if (IsOverlapAllowed(pA.Material, pB.Material)) continue;

                // 2. AABB broad phase
                if (!pA.BoundingBox.Intersects(pB.BoundingBox, tol)) continue;

                // 3. Narrow phase Boolean
                auto clash = NarrowPhaseBoolean(pA, pB);
                if (clash.has_value())
                    local.push_back(std::move(clash.value()));
            }
        }
        if (!local.empty()) {
            std::scoped_lock lock(mutex);
            globalClashes.insert(globalClashes.end(),
                std::make_move_iterator(local.begin()),
                std::make_move_iterator(local.end()));
        }
    };

    size_t chunk = static_cast<size_t>(
        std::ceil(static_cast<double>(N) / numThreads));

    std::vector<std::jthread> workers;
    workers.reserve(numThreads);
    for (unsigned int t = 0; t < numThreads; ++t) {
        size_t s = t * chunk;
        size_t e = std::min(s + chunk, N);
        if (s >= N) break;
        workers.emplace_back(workerTask, s, e);
    }
    // std::jthread joins automatically on destruction

    std::cout << "[SZM Interference] Check complete: "
              << globalClashes.size() << " clash(es) detected.\n";
    return globalClashes;
}

} // namespace SZM::Assembly
