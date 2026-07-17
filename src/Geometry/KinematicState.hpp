struct Matrix3x3 {
    float m[3][3] = {{1,0,0}, {0,1,0}, {0,0,1}};

    Matrix3x3 operator*(const Matrix3x3& o) const {
        Matrix3x3 res = {0};
        for(int i=0; i<3; ++i)
            for(int j=0; j<3; ++j)
                for(int k=0; k<3; ++k)
                    res.m[i][j] += m[i][k] * o.m[k][j];
        return res;
    }

    Matrix3x3 transpose() const {
        return {{ {m[0][0], m[1][0], m[2][0]}, 
                  {m[0][1], m[1][1], m[2][1]}, 
                  {m[0][2], m[1][2], m[2][2]} }};
    }

    // Fast 3x3 Inverse using Cofactors
    Matrix3x3 inverse() const {
        float det = m[0][0] * (m[1][1] * m[2][2] - m[2][1] * m[1][2]) -
                    m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0]) +
                    m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);

        if (std::abs(det) < 1e-6f) return Matrix3x3{0}; // Degenerate case handled by caller

        float invDet = 1.0f / det;
        Matrix3x3 res;
        res.m[0][0] = (m[1][1] * m[2][2] - m[2][1] * m[1][2]) * invDet;
        res.m[0][1] = (m[0][2] * m[2][1] - m[0][1] * m[2][2]) * invDet;
        res.m[0][2] = (m[0][1] * m[1][2] - m[0][2] * m[1][1]) * invDet;
        res.m[1][0] = (m[1][2] * m[2][0] - m[1][0] * m[2][2]) * invDet;
        res.m[1][1] = (m[0][0] * m[2][2] - m[0][2] * m[2][0]) * invDet;
        res.m[1][2] = (m[1][0] * m[0][2] - m[0][0] * m[1][2]) * invDet;
        res.m[2][0] = (m[1][0] * m[2][1] - m[2][0] * m[1][1]) * invDet;
        res.m[2][1] = (m[2][0] * m[0][1] - m[0][0] * m[2][1]) * invDet;
        res.m[2][2] = (m[0][0] * m[1][1] - m[1][0] * m[0][1]) * invDet;
        return res;
    }
};

enum class KinematicState {
    Dynamic,
    Kinematic, // Motor-driven, infinite mass logic to other objects
    Static     // Grounded, infinite mass
};

// Abstracted SolidBody/Triangle structure from Geometry Module