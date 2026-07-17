struct Vector3 {
    float x, y, z;
    Vector3 operator+(const Vector3& o) const { return {x + o.x, y + o.y, z + o.z}; }
    Vector3 operator*(float s) const { return {x * s, y * s, z * s}; }
    Vector3 operator/(float s) const { return {x / s, y / s, z / s}; }
};

// Represents a Material Override for an instance (e.g., Paint Color)