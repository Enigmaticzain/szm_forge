namespace SZM::Graphics {

    uint32_t MeshGenerator::CalculateSegmentCount(double radius, double chordalError) {
        if (radius <= 0.0 || chordalError <= 0.0) return 3; // Minimum triangle count
        
        // Prevent domain errors if chordal error exceeds radius
        if (chordalError >= radius) return 3; 

        // Formula: N ≈ π / arccos(1 - ϵ/R)
        double ratio = 1.0 - (chordalError / radius);
        double angle = std::acos(ratio);
        
        // N represents half a cylinder (π), multiply by 2 for full 360 degrees (2π)
        double segments = std::numbers::pi / angle; 
        
        // Ensure at least a minimal resolution, clamp at a safe maximum to prevent VRAM overflow
        return std::clamp(static_cast<uint32_t>(std::ceil(segments * 2.0)), 3u, 1024u);
    }

    RenderMesh MeshGenerator::TessellateSolid(const class Geometry::SolidBody& body, const TessellationConfig& config) {
        RenderMesh mesh;
        
        // Edge Case Handling: Dynamic LOD
        // If the camera is far away, we mathematically increase the allowed chordal error.
        double effectiveError = config.MaxChordalError;
        if (config.EnableDynamicLOD) {
            // Scale error by distance (simplified linear scale for the implementation)
            effectiveError *= std::max(1.0, config.CameraDistance * 0.1);
        }

        // Implementation Note for Engineering Team:
        // 1. Iterate through B-Rep Faces.
        // 2. If Planar -> Delaunay Triangulation or Ear-Clipping (for simple polygons).
        // 3. If Cylindrical/Spherical -> Use CalculateSegmentCount(Radius, effectiveError)
        //    to generate the parameterized UV grid, then construct triangles.
        // 4. Calculate proper Vertex Normals (not face normals) for smooth PBR lighting.
        
        // (Placeholder for the extensive geometry-to-mesh iteration loop)
        
        return mesh;
    }

} // namespace SZM::Graphics

#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

// Camera and Model Matrices
layout (std140, binding = 0) uniform CameraMatrices {
    mat4 view;
    mat4 projection;
    vec3 camPos;
};

uniform mat4 model;

// Outputs to Fragment Shader
out vec3 WorldPos;
out vec3 Normal;
out vec2 TexCoords;

void main() {
    // Transform vertex position to world space
    vec4 worldPosition = model * vec4(aPos, 1.0);
    WorldPos = worldPosition.xyz; 
    
    // Transform normal to world space (using Normal Matrix to handle non-uniform scaling)
    Normal = mat3(transpose(inverse(model))) * aNormal;  
    
    TexCoords = aTexCoords;

    // Final screen space position
    gl_Position = projection * view * worldPosition;
}

#version 460 core

out vec4 FragColor;

in vec3 WorldPos;
in vec3 Normal;
in vec2 TexCoords;

// Material System (300-Series)
uniform vec3  albedo;
uniform float metallic;
uniform float roughness;
uniform float ao;

// Lighting Context
uniform vec3 lightPositions[4];
uniform vec3 lightColors[4];

// Camera Context
layout (std140, binding = 0) uniform CameraMatrices {
    mat4 view;
    mat4 projection;
    vec3 camPos;
};

const float PI = 3.14159265359;

// 1. Normal Distribution Function (Trowbridge-Reitz GGX) -> The 'D' in DFG
float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / max(denom, 0.0000001); // Prevent divide by zero
}

// 2. Geometry Function (Schlick-GGX) -> The 'G' in DFG
float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

// 3. Fresnel Equation (Fresnel-Schlick) -> The 'F' in DFG
vec3 FresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main() {
    vec3 N = normalize(Normal);
    vec3 V = normalize(camPos - WorldPos);

    // F0 is the base reflectivity. 0.04 is standard for dielectrics (plastic).
    // If metallic, we use the albedo color as the base reflectivity.
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    vec3 Lo = vec3(0.0); // Total outgoing radiance

    for(int i = 0; i < 4; ++i) {
        // Calculate per-light radiance
        vec3 L = normalize(lightPositions[i] - WorldPos);
        vec3 H = normalize(V + L); // Halfway vector
        
        float distance    = length(lightPositions[i] - WorldPos);
        float attenuation = 1.0 / (distance * distance); // Inverse square law
        vec3 radiance     = lightColors[i] * attenuation;

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, roughness);       
        float G   = GeometrySmith(N, V, L, roughness);      
        vec3 F    = FresnelSchlick(max(dot(H, V), 0.0), F0);       

        vec3 kS = F; // Specular contribution
        vec3 kD = vec3(1.0) - kS; // Diffuse contribution
        kD *= 1.0 - metallic; // Pure metals have no diffuse light

        // Specular Equation: (D * F * G) / (4 * (N.V) * (N.L))
        vec3 numerator    = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        vec3 specular     = numerator / denominator;  

        // Add to outgoing radiance equation
        float NdotL = max(dot(N, L), 0.0);                
        Lo += (kD * albedo / PI + specular) * radiance * NdotL; 
    }

    // Ambient lighting approximation (Will be replaced by IBL / Irradiance Maps for full scene reflections)
    vec3 ambient = vec3(0.03) * albedo * ao;
    vec3 color = ambient + Lo;

    // HDR Tonemapping (Reinhard) and Gamma Correction
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));  

    FragColor = vec4(color, 1.0);
}
