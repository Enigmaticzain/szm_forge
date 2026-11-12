#pragma once

struct Vector3 {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    Vector3 operator+(const Vector3& o) const { return {x + o.x, y + o.y, z + o.z}; }
    Vector3 operator-(const Vector3& o) const { return {x - o.x, y - o.y, z - o.z}; }
    Vector3 operator*(float s) const { return {x * s, y * s, z * s}; }
    Vector3 operator/(float s) const {
        if (s == 0.0f) {
            return {0.0f, 0.0f, 0.0f};
        }
        return {x / s, y / s, z / s};
    }
    float dot(const Vector3& o) const { return x * o.x + y * o.y + z * o.z; }
    Vector3 cross(const Vector3& o) const {
        return {y * o.z - z * o.y, z * o.x - x * o.z, x * o.y - y * o.x};
    }
};
