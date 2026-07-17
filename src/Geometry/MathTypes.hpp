#pragma once

#include <cmath>
#include <cstdint>

namespace SZM::Geometry {

    // ----------------------------------------------------------------
    //  Vector3  —  shared across all Geometry sub-modules
    // ----------------------------------------------------------------
    struct Vector3 {
        double x = 0.0, y = 0.0, z = 0.0;

        Vector3() = default;
        Vector3(double x, double y, double z) : x(x), y(y), z(z) {}

        Vector3 operator+(const Vector3& v) const { return {x+v.x, y+v.y, z+v.z}; }
        Vector3 operator-(const Vector3& v) const { return {x-v.x, y-v.y, z-v.z}; }
        Vector3 operator*(double s)         const { return {x*s,   y*s,   z*s  }; }

        double  Dot(const Vector3& v)   const { return x*v.x + y*v.y + z*v.z; }
        Vector3 Cross(const Vector3& v) const {
            return { y*v.z - z*v.y,
                     z*v.x - x*v.z,
                     x*v.y - y*v.x };
        }

        [[nodiscard]] double  Magnitude()  const { return std::sqrt(x*x + y*y + z*z); }
        [[nodiscard]] Vector3 Normalize()  const {
            double m = Magnitude();
            return m > 0.0 ? (*this) * (1.0 / m) : *this;
        }
        [[nodiscard]] double  MagnitudeSq() const { return x*x + y*y + z*z; }
    };

    // ----------------------------------------------------------------
    //  Half-Edge topology handles
    // ----------------------------------------------------------------
    constexpr uint32_t NullHandle = 0xFFFF'FFFFu;

} // namespace SZM::Geometry
