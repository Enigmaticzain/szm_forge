#pragma once

#include "Camera.hpp"
#include "../Geometry/MathTypes.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace SZM::Graphics {

inline glm::mat4 ToGlmMat4(const Matrix4x4& matrix) {
    return glm::make_mat4(matrix.m);
}

inline glm::vec3 ToGlmVec3(const Geometry::Vector3& vector) {
    return glm::vec3(
        static_cast<float>(vector.x),
        static_cast<float>(vector.y),
        static_cast<float>(vector.z)
    );
}

} // namespace SZM::Graphics
