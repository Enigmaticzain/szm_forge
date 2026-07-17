namespace SZM::Graphics {

    // 32-byte aligned vertex for optimal GPU memory streaming
    struct RenderVertex {
        glm::vec3 Position;
        glm::vec3 Normal;
        glm::vec2 TexCoords;
    };

    struct RenderMesh {
        std::vector<RenderVertex> Vertices;
        std::vector<uint32_t> Indices;
        
        // VRAM Handles (Set by the Graphics Context later)
        uint32_t VBO = 0;
        uint32_t IBO = 0;
        uint32_t VAO = 0;
    };

    struct TessellationConfig {
        double MaxChordalError = 0.01; // ϵ (meters)
        bool EnableDynamicLOD = true;
        double CameraDistance = 1.0;   // Used to scale ϵ dynamically
    };

    class MeshGenerator {
    public:
        // Converts a mathematical SolidBody into a GPU-ready triangle mesh
        // Forward declaring SolidBody from Module 110.4
        static RenderMesh TessellateSolid(const class Geometry::SolidBody& body, const TessellationConfig& config);
        
        // Calculates required segments for a curve based on error tolerance
        static uint32_t CalculateSegmentCount(double radius, double chordalError);
    };

} // namespace SZM::Graphics
