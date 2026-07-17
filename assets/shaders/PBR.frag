#version 460 core

out vec4 FragColor;

in vec3 WorldPos;
in vec3 Normal;
in vec2 TexCoords;

// --- Material uniforms (set per draw call) ---
uniform vec3  albedo;
uniform float metallic;
uniform float roughness;
uniform float ao;

// --- Lighting context (up to 4 point lights) ---
uniform vec3 lightPositions[4];
uniform vec3 lightColors[4];

// --- Camera UBO (shared with vertex shader) ---
layout (std140, binding = 0) uniform CameraMatrices {
    mat4 view;
    mat4 projection;
    vec3 camPos;
};

const float PI = 3.14159265359;

// ----------------------------------------------------------------
//  D — Normal Distribution Function (Trowbridge-Reitz GGX)
//  Models the statistical distribution of micro-facet normals
// ----------------------------------------------------------------
float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a      = roughness * roughness;
    float a2     = a * a;
    float NdotH2 = pow(max(dot(N, H), 0.0), 2.0);
    float denom  = (NdotH2 * (a2 - 1.0) + 1.0);
    return a2 / max(PI * denom * denom, 1e-7);
}

// ----------------------------------------------------------------
//  G — Geometry Function (Smith + Schlick-GGX)
//  Models self-shadowing of micro-facets
// ----------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness) {
    float k = pow(roughness + 1.0, 2.0) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    return GeometrySchlickGGX(max(dot(N, V), 0.0), roughness) *
           GeometrySchlickGGX(max(dot(N, L), 0.0), roughness);
}

// ----------------------------------------------------------------
//  F — Fresnel Equation (Fresnel-Schlick approximation)
//  Models the reflectivity angle dependence
// ----------------------------------------------------------------
vec3 FresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// ----------------------------------------------------------------
void main() {
    vec3 N = normalize(Normal);
    vec3 V = normalize(camPos - WorldPos);

    // Base reflectivity: 0.04 for dielectrics; albedo for metals
    vec3 F0 = mix(vec3(0.04), albedo, metallic);

    vec3 Lo = vec3(0.0); // Accumulated outgoing radiance

    for (int i = 0; i < 4; ++i) {
        vec3  L           = normalize(lightPositions[i] - WorldPos);
        vec3  H           = normalize(V + L);
        float dist        = length(lightPositions[i] - WorldPos);
        float attenuation = 1.0 / (dist * dist); // Inverse square law
        vec3  radiance    = lightColors[i] * attenuation;

        // Cook-Torrance BRDF
        float D = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        vec3  F = FresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3  kD      = (vec3(1.0) - F) * (1.0 - metallic);
        vec3  specular = (D * G * F) /
                         max(4.0 * max(dot(N, V), 0.0) *
                                   max(dot(N, L), 0.0), 0.0001);

        Lo += (kD * albedo / PI + specular) * radiance * max(dot(N, L), 0.0);
    }

    // Ambient approximation — IBL irradiance maps will replace this
    vec3 color = vec3(0.03) * albedo * ao + Lo;

    // HDR Reinhard tonemapping + gamma correction (γ = 2.2)
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));

    FragColor = vec4(color, 1.0);
}
