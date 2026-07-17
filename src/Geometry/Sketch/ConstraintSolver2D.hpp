#pragma once

#include "Sketch2D.hpp"

namespace SZM::Geometry {

    // ----------------------------------------------------------------
    //  ConstraintSolver2D — Newton-Raphson solver for sketch constraints
    // ----------------------------------------------------------------
    class ConstraintSolver2D {
    public:
        /// Attempt to solve the constraints in the sketch.
        /// Modifies the sketch entities in-place if successful.
        /// @return true if converged, false if over-constrained or divergent
        static bool Solve(Sketch2D& sketch, int maxIterations = 50, double tolerance = 1e-6);

    private:
        // Internal helpers for computing Jacobians and residuals
        // would be implemented here in a full algebraic solver.
    };

} // namespace SZM::Geometry
