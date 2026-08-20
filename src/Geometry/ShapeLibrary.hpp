#pragma once

#include <string>
#include <vector>
#include <map>
#include <optional>
#include <chrono>
#include <memory>

// OpenCascade includes
#include <TopoDS_Shape.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>

namespace SZM::Geometry {

// Forward declarations
class PrimitiveShape;

/**
 * @brief Shape type enumeration for all supported primitives
 */
enum class ShapeType {
    Cylinder,
    Cone,
    Torus,
    Wedge,
    Pyramid,
    Prism,
    Ellipsoid,
    Paraboloid,
    BooleanResult
};

/**
 * @brief Error codes for shape operations
 */
enum class ShapeError {
    Success = 0,
    InvalidRadius,
    InvalidHeight,
    InvalidLength,
    InvalidWidth,
    InvalidDepth,
    InvalidFocalLength,
    InvalidSideCount,
    InvalidRadiusRatio,          // For torus: minor >= major
    MeshGenerationFailed,
    BRepOperationFailed,
    BooleanOperationFailed,
    InvalidShapeType,
    SerializationFailed,
    DeserializationFailed
};

/**
 * @brief Get human-readable description for error code
 */
const char* GetErrorDescription(ShapeError error);

/**
 * @brief Mesh data structure for rendering
 */
struct MeshData {
    std::vector<float> vertices;      // x, y, z coordinates (interleaved)
    std::vector<uint32_t> indices;    // Triangle indices
    std::vector<float> normals;       // Vertex normals (interleaved)
    size_t triangleCount;
    
    bool IsValid() const {
        return !vertices.empty() && !indices.empty() && 
               indices.size() % 3 == 0 &&
               vertices.size() / 3 == normals.size() / 3;
    }
};

/**
 * @brief Parameters for shape creation
 */
struct ShapeParameters {
    std::map<std::string, double> dimensions;
    std::chrono::system_clock::time_point creationTime;
    
    // Helper methods for common parameters
    void SetRadius(double r) { dimensions["radius"] = r; }
    void SetHeight(double h) { dimensions["height"] = h; }
    double GetRadius() const;
    double GetHeight() const;
};

/**
 * @brief Unified primitive shape structure
 */
struct PrimitiveShape {
    std::string id;                    // Unique identifier (UUID)
    ShapeType type;                    // Shape type enumeration
    TopoDS_Shape brepShape;            // OpenCascade boundary representation
    MeshData mesh;                     // Triangle mesh for rendering
    ShapeParameters parameters;         // Creation parameters and metadata
    
    // Accessor methods
    const TopoDS_Shape& GetBRepShape() const { return brepShape; }
    const MeshData& GetMesh() const { return mesh; }
    bool HasValidMesh() const { return mesh.IsValid(); }
    
    // Serialization
    std::string ToJSON() const;
    static PrimitiveShape FromJSON(const std::string& json);
};

/**
 * @brief Result wrapper for shape operations
 */
struct ShapeResult {
    ShapeError errorCode;
    std::string errorMessage;
    std::optional<PrimitiveShape> shape;
    
    bool IsSuccess() const { return errorCode == ShapeError::Success; }
    
    static ShapeResult Success(PrimitiveShape&& s) {
        return ShapeResult{ShapeError::Success, "", std::move(s)};
    }
    
    static ShapeResult Error(ShapeError code, const std::string& msg) {
        return ShapeResult{code, msg, std::nullopt};
    }
};

/**
 * @brief Mesh quality settings
 */
enum class MeshQuality {
    Coarse,   // 1.0 mm deflection
    Normal,   // 0.1 mm deflection (default)
    Fine      // 0.01 mm deflection
};

/**
 * @brief Mesh generator for converting BRep shapes to triangle meshes
 */
class MeshGenerator {
public:
    // Set global mesh quality
    static void SetQuality(MeshQuality quality);
    static MeshQuality GetQuality();
    
    // Generate mesh from TopoDS_Shape
    static std::optional<MeshData> GenerateMesh(
        const TopoDS_Shape& shape,
        MeshQuality quality = MeshQuality::Normal
    );
    
    // Get deflection tolerance for quality level
    static double GetDeflection(MeshQuality quality);
    
private:
    static MeshQuality s_currentQuality;
    static double DeflectionFromQuality(MeshQuality quality);
};

/**
 * @brief Main shape library API for primitive creation
 */
class ShapeLibrary {
public:
    // Cylinder: radius > 0, height > 0
    static ShapeResult CreateCylinder(double radius, double height);
    
    // Cone: height > 0, at least one radius > 0, both >= 0
    static ShapeResult CreateCone(
        double baseRadius, 
        double topRadius, 
        double height
    );
    
    // Torus: majorRadius > minorRadius > 0
    static ShapeResult CreateTorus(
        double majorRadius, 
        double minorRadius
    );
    
    // Wedge: length, width, height > 0
    static ShapeResult CreateWedge(
        double length, 
        double width, 
        double height
    );
    
    // Pyramid: baseWidth, baseDepth, height > 0
    static ShapeResult CreatePyramid(
        double baseWidth, 
        double baseDepth, 
        double height
    );
    
    // Prism: 3 <= sides <= 32, radius > 0, height > 0
    static ShapeResult CreatePrism(
        int sides, 
        double radius, 
        double height
    );
    
    // Ellipsoid: all radii > 0
    static ShapeResult CreateEllipsoid(
        double xRadius, 
        double yRadius, 
        double zRadius
    );
    
    // Paraboloid: focalLength > 0, height > 0
    static ShapeResult CreateParaboloid(
        double focalLength, 
        double height
    );
    
    // Boolean operations
    static ShapeResult Union(
        const PrimitiveShape& shape1, 
        const PrimitiveShape& shape2
    );
    
    static ShapeResult Difference(
        const PrimitiveShape& shape1, 
        const PrimitiveShape& shape2
    );
    
    static ShapeResult Intersection(
        const PrimitiveShape& shape1, 
        const PrimitiveShape& shape2
    );
};

/**
 * @brief Configuration management for shape library
 */
class ShapeConfig {
public:
    static void Load();
    static void Save();
    
    static void SetMeshQuality(MeshQuality quality);
    static MeshQuality GetMeshQuality();
    
    static std::map<ShapeType, ShapeParameters> GetDefaultParameters();
    
private:
    static const std::string CONFIG_FILE_PATH;
};

/**
 * @brief Generate a UUID string
 */
std::string GenerateUUID();

} // namespace SZM::Geometry
