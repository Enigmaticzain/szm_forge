#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

// Camera UBO — shared with fragment shader, binding = 0
layout (std140, binding = 0) uniform CameraMatrices {
    mat4 view;
    mat4 projection;
    vec3 camPos;
};

uniform mat4 model;

out vec3 WorldPos;
out vec3 Normal;
out vec2 TexCoords;

void main() {
    vec4 worldPosition = model * vec4(aPos, 1.0);
    WorldPos  = worldPosition.xyz;

    // Normal matrix handles non-uniform scaling correctly
    Normal    = mat3(transpose(inverse(model))) * aNormal;
    TexCoords = aTexCoords;

    gl_Position = projection * view * worldPosition;
}
