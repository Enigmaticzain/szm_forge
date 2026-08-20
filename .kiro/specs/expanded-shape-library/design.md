# Design Document: Expanded Shape Library

## Overview

This design specifies the architecture for expanding SZM Forge's shape library with comprehensive 3D geometric primitives through OpenCascade BRepPrimAPI integration. The system will provide eight new primitive shapes (cylinder, cone, torus, wedge, pyramid, prism, ellipsoid, paraboloid) with eager mesh generation, parameter validation, and boolean operation support. The design ensures immediate mesh availability for rendering while maintaining type safety and error handling through a unified shape data structure.

## Architecture

### System Components

The expanded shape library consists of four primary architectural layers:

```
┌─────────────────────────────────────────────────────────┐
│              CAD Modeler (UI Layer)                     │
│  - Shape creation dialogs                               │
│  - Parameter input validation                           │
│  - 3D viewport rendering                                │
└─────────────────────────┬───────────────────────────────┘
                          │
┌─────────────────────────▼───────────────────────────────┐
│           Shape Library (API Layer)                     │
│  - Primitive creation functions                         │
│  - Parameter validation                                 │
│  - Boolean operations                                   │
│  - Error handling and logging                           │
└─────────────────────────┬───────────────────────────────┘
                          │
          ┌───────────────┴──────────────┐
          │                              │
┌─────────▼──────────┐        ┌──────────▼─────────┐
│  BRepPrimAPI       │        │  Mesh Generator    │
│  (OpenCascade)     │        │  - BRepMesh API    │
│  - MakeCylinder    │        │  - Quality config  │
│  - MakeCone        │        │  - Deflection      │
│  - MakeTorus       │        │  - Parallelization │
│  - MakeWedge       │        └────────────────────┘
│  - MakeSphere      │
│  - BRepBuilderAPI  │
└────────────────────┘
```

### Component Descriptions

#### 1. Shape Library (Core API)

The Shape Library provides the primary interface for creating and manipulating primitive shapes. It encapsulates OpenCascade API calls, performs parameter validation, coordinates eager mesh generation, and manages error handling.

**Responsibilities:**
- Expose creation functions for all eight primitive types
- Validate dimensional parameters before shape construction
- Coordinate synchronous mesh generation
- Provide boolean operation wrappers
- Manage error codes and logging
- Handle serialization/deserialization

#### 2. Mesh Generator

The Mesh Generator converts boundary representation (BRep) shapes into triangle meshes suitable for rendering. It uses OpenCascade's BRepMesh_IncrementalMesh with configurable quality settings.

**Responsibilities:**
- Generate triangle meshes from TopoDS_Shape
- Apply quality-based deflection tolerance
- Utilize parallel processing when available
- Complete within performance requirements
- Handle mesh generation failures

#### 3. BRepPrimAPI Integration

Direct integration with OpenCascade's BRepPrimAPI for native primitive construction and BRepBuilderAPI for complex shapes requiring custom face/edge operations.

**Shapes by API:**
- **BRepPrimAPI:** Cylinder, Cone, Torus, Wedge, Sphere (for ellipsoid)
- **BRepBuilderAPI:** Pyramid, Prism, Paraboloid (revolution operations)


#### 4. CAD Modeler UI Integration

The CAD Modeler provides user-facing interfaces for shape creation through menu-driven dialogs with parameter inputs and immediate 3D viewport visualization.

## Data Structures

### PrimitiveShape Structure

The `PrimitiveShape` structure serves as the unified container for all primitive shape data, ensuring consistency across shape types.

```cpp
namespace SZM::Geometry {

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

struct MeshData {
    std::vector<float> vertices;      // x, y, z coordinates
    std::vector<uint32_t> indices;    // Triangle indices
    std::vector<float> normals;       // Vertex normals
    size_t triangleCount;
    
    bool IsValid() const {
        return !vertices.empty() && !indices.empty() && 
               indices.size() % 3 == 0;
    }
};


struct ShapeParameters {
    std::map<std::string, double> dimensions;
    std::chrono::system_clock::time_point creationTime;
    
    // Helper methods for common parameters
    void SetRadius(double r) { dimensions["radius"] = r; }
    void SetHeight(double h) { dimensions["height"] = h; }
    double GetRadius() const { return dimensions.at("radius"); }
    double GetHeight() const { return dimensions.at("height"); }
};

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

} // namespace SZM::Geometry
```

### Error Handling Structure

```cpp
namespace SZM::Geometry {

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

const char* GetErrorDescription(ShapeError error);

} // namespace SZM::Geometry
```

## Interface Design

### Shape Creation Functions

Each primitive shape has a dedicated creation function following a consistent pattern with parameter validation, BRep construction, and eager mesh generation.

```cpp
namespace SZM::Geometry {

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

} // namespace SZM::Geometry
```


### Mesh Generation Interface

```cpp
namespace SZM::Geometry {

enum class MeshQuality {
    Coarse,   // 1.0 mm deflection
    Normal,   // 0.1 mm deflection (default)
    Fine      // 0.01 mm deflection
};

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

} // namespace SZM::Geometry
```

### Configuration and Persistence

```cpp
namespace SZM::Geometry {

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

} // namespace SZM::Geometry
```


## Implementation Details

### Primitive Shape Construction Algorithms

#### Cylinder Implementation

```cpp
ShapeResult ShapeLibrary::CreateCylinder(double radius, double height) {
    // 1. Parameter validation
    if (radius <= 0.0) {
        return ShapeResult::Error(
            ShapeError::InvalidRadius,
            "Cylinder radius must be greater than zero (provided: " + 
            std::to_string(radius) + ")"
        );
    }
    if (height <= 0.0) {
        return ShapeResult::Error(
            ShapeError::InvalidHeight,
            "Cylinder height must be greater than zero (provided: " + 
            std::to_string(height) + ")"
        );
    }
    
    // 2. Create BRep shape using OpenCascade
    try {
        BRepPrimAPI_MakeCylinder cylinderMaker(radius, height);
        TopoDS_Shape brepShape = cylinderMaker.Shape();
        
        // 3. Generate mesh immediately
        auto meshData = MeshGenerator::GenerateMesh(brepShape);
        if (!meshData.has_value()) {
            return ShapeResult::Error(
                ShapeError::MeshGenerationFailed,
                "Failed to generate mesh for cylinder"
            );
        }
        
        // 4. Construct PrimitiveShape
        PrimitiveShape shape;
        shape.id = GenerateUUID();
        shape.type = ShapeType::Cylinder;
        shape.brepShape = brepShape;
        shape.mesh = std::move(meshData.value());
        shape.parameters.SetRadius(radius);
        shape.parameters.SetHeight(height);
        shape.parameters.creationTime = std::chrono::system_clock::now();
        
        return ShapeResult::Success(std::move(shape));
    }
    catch (const Standard_Failure& e) {
        return ShapeResult::Error(
            ShapeError::BRepOperationFailed,
            "OpenCascade error: " + std::string(e.GetMessageString())
        );
    }
}
```


#### Cone Implementation

```cpp
ShapeResult ShapeLibrary::CreateCone(
    double baseRadius, 
    double topRadius, 
    double height
) {
    // Validate parameters
    if (baseRadius < 0.0) {
        return ShapeResult::Error(
            ShapeError::InvalidRadius,
            "Cone base radius must be non-negative"
        );
    }
    if (topRadius < 0.0) {
        return ShapeResult::Error(
            ShapeError::InvalidRadius,
            "Cone top radius must be non-negative"
        );
    }
    if (height <= 0.0) {
        return ShapeResult::Error(
            ShapeError::InvalidHeight,
            "Cone height must be greater than zero"
        );
    }
    if (baseRadius == 0.0 && topRadius == 0.0) {
        return ShapeResult::Error(
            ShapeError::InvalidRadius,
            "At least one cone radius must be greater than zero"
        );
    }
    
    try {
        BRepPrimAPI_MakeCone coneMaker(baseRadius, topRadius, height);
        TopoDS_Shape brepShape = coneMaker.Shape();
        
        auto meshData = MeshGenerator::GenerateMesh(brepShape);
        if (!meshData.has_value()) {
            return ShapeResult::Error(
                ShapeError::MeshGenerationFailed,
                "Failed to generate mesh for cone"
            );
        }
        
        PrimitiveShape shape;
        shape.id = GenerateUUID();
        shape.type = ShapeType::Cone;
        shape.brepShape = brepShape;
        shape.mesh = std::move(meshData.value());
        shape.parameters.dimensions["baseRadius"] = baseRadius;
        shape.parameters.dimensions["topRadius"] = topRadius;
        shape.parameters.SetHeight(height);
        shape.parameters.creationTime = std::chrono::system_clock::now();
        
        return ShapeResult::Success(std::move(shape));
    }
    catch (const Standard_Failure& e) {
        return ShapeResult::Error(
            ShapeError::BRepOperationFailed,
            "OpenCascade error: " + std::string(e.GetMessageString())
        );
    }
}
```


#### Torus Implementation

```cpp
ShapeResult ShapeLibrary::CreateTorus(
    double majorRadius, 
    double minorRadius
) {
    if (majorRadius <= 0.0) {
        return ShapeResult::Error(
            ShapeError::InvalidRadius,
            "Torus major radius must be greater than zero"
        );
    }
    if (minorRadius <= 0.0) {
        return ShapeResult::Error(
            ShapeError::InvalidRadius,
            "Torus minor radius must be greater than zero"
        );
    }
    if (minorRadius >= majorRadius) {
        return ShapeResult::Error(
            ShapeError::InvalidRadiusRatio,
            "Torus minor radius must be less than major radius"
        );
    }
    
    try {
        BRepPrimAPI_MakeTorus torusMaker(majorRadius, minorRadius);
        TopoDS_Shape brepShape = torusMaker.Shape();
        
        auto meshData = MeshGenerator::GenerateMesh(brepShape);
        if (!meshData.has_value()) {
            return ShapeResult::Error(
                ShapeError::MeshGenerationFailed,
                "Failed to generate mesh for torus"
            );
        }
        
        PrimitiveShape shape;
        shape.id = GenerateUUID();
        shape.type = ShapeType::Torus;
        shape.brepShape = brepShape;
        shape.mesh = std::move(meshData.value());
        shape.parameters.dimensions["majorRadius"] = majorRadius;
        shape.parameters.dimensions["minorRadius"] = minorRadius;
        shape.parameters.creationTime = std::chrono::system_clock::now();
        
        return ShapeResult::Success(std::move(shape));
    }
    catch (const Standard_Failure& e) {
        return ShapeResult::Error(
            ShapeError::BRepOperationFailed,
            "OpenCascade error: " + std::string(e.GetMessageString())
        );
    }
}
```


#### Pyramid Implementation (Using BRepBuilderAPI)

```cpp
ShapeResult ShapeLibrary::CreatePyramid(
    double baseWidth, 
    double baseDepth, 
    double height
) {
    if (baseWidth <= 0.0 || baseDepth <= 0.0 || height <= 0.0) {
        return ShapeResult::Error(
            ShapeError::InvalidRadius,
            "Pyramid dimensions must all be greater than zero"
        );
    }
    
    try {
        // Create base vertices
        gp_Pnt p1(-baseWidth/2, -baseDepth/2, 0);
        gp_Pnt p2(baseWidth/2, -baseDepth/2, 0);
        gp_Pnt p3(baseWidth/2, baseDepth/2, 0);
        gp_Pnt p4(-baseWidth/2, baseDepth/2, 0);
        gp_Pnt apex(0, 0, height);
        
        // Build base face
        BRepBuilderAPI_MakePolygon baseWire;
        baseWire.Add(p1);
        baseWire.Add(p2);
        baseWire.Add(p3);
        baseWire.Add(p4);
        baseWire.Close();
        
        BRepBuilderAPI_MakeFace baseFace(baseWire.Wire());
        
        // Build side faces and combine
        BRep_Builder builder;
        TopoDS_Compound compound;
        builder.MakeCompound(compound);
        builder.Add(compound, baseFace.Face());
        
        // Add triangular side faces
        for (int i = 0; i < 4; ++i) {
            gp_Pnt v1 = (i == 0) ? p1 : (i == 1) ? p2 : (i == 2) ? p3 : p4;
            gp_Pnt v2 = (i == 0) ? p2 : (i == 1) ? p3 : (i == 2) ? p4 : p1;
            
            BRepBuilderAPI_MakePolygon sideWire;
            sideWire.Add(v1);
            sideWire.Add(v2);
            sideWire.Add(apex);
            sideWire.Close();
            
            BRepBuilderAPI_MakeFace sideFace(sideWire.Wire());
            builder.Add(compound, sideFace.Face());
        }
        
        // Create solid from shell
        BRepBuilderAPI_Sewing sewing;
        sewing.Add(compound);
        sewing.Perform();
        TopoDS_Shape brepShape = sewing.SewedShape();
        
        auto meshData = MeshGenerator::GenerateMesh(brepShape);
        if (!meshData.has_value()) {
            return ShapeResult::Error(
                ShapeError::MeshGenerationFailed,
                "Failed to generate mesh for pyramid"
            );
        }
        
        PrimitiveShape shape;
        shape.id = GenerateUUID();
        shape.type = ShapeType::Pyramid;
        shape.brepShape = brepShape;
        shape.mesh = std::move(meshData.value());
        shape.parameters.dimensions["baseWidth"] = baseWidth;
        shape.parameters.dimensions["baseDepth"] = baseDepth;
        shape.parameters.SetHeight(height);
        shape.parameters.creationTime = std::chrono::system_clock::now();
        
        return ShapeResult::Success(std::move(shape));
    }
    catch (const Standard_Failure& e) {
        return ShapeResult::Error(
            ShapeError::BRepOperationFailed,
            "OpenCascade error: " + std::string(e.GetMessageString())
        );
    }
}
```


#### Prism Implementation

```cpp
ShapeResult ShapeLibrary::CreatePrism(int sides, double radius, double height) {
    if (sides < 3 || sides > 32) {
        return ShapeResult::Error(
            ShapeError::InvalidSideCount,
            "Prism sides must be between 3 and 32 (provided: " + 
            std::to_string(sides) + ")"
        );
    }
    if (radius <= 0.0) {
        return ShapeResult::Error(
            ShapeError::InvalidRadius,
            "Prism radius must be greater than zero"
        );
    }
    if (height <= 0.0) {
        return ShapeResult::Error(
            ShapeError::InvalidHeight,
            "Prism height must be greater than zero"
        );
    }
    
    try {
        // Create regular polygon base
        BRepBuilderAPI_MakePolygon baseWire;
        const double angleStep = 2.0 * M_PI / sides;
        
        for (int i = 0; i < sides; ++i) {
            double angle = i * angleStep;
            gp_Pnt p(radius * cos(angle), radius * sin(angle), 0);
            baseWire.Add(p);
        }
        baseWire.Close();
        
        // Create face and extrude
        BRepBuilderAPI_MakeFace baseFace(baseWire.Wire());
        gp_Vec extrusionVec(0, 0, height);
        BRepPrimAPI_MakePrism prismMaker(baseFace.Face(), extrusionVec);
        TopoDS_Shape brepShape = prismMaker.Shape();
        
        auto meshData = MeshGenerator::GenerateMesh(brepShape);
        if (!meshData.has_value()) {
            return ShapeResult::Error(
                ShapeError::MeshGenerationFailed,
                "Failed to generate mesh for prism"
            );
        }
        
        PrimitiveShape shape;
        shape.id = GenerateUUID();
        shape.type = ShapeType::Prism;
        shape.brepShape = brepShape;
        shape.mesh = std::move(meshData.value());
        shape.parameters.dimensions["sides"] = sides;
        shape.parameters.SetRadius(radius);
        shape.parameters.SetHeight(height);
        shape.parameters.creationTime = std::chrono::system_clock::now();
        
        return ShapeResult::Success(std::move(shape));
    }
    catch (const Standard_Failure& e) {
        return ShapeResult::Error(
            ShapeError::BRepOperationFailed,
            "OpenCascade error: " + std::string(e.GetMessageString())
        );
    }
}
```


### Mesh Generation Implementation

```cpp
std::optional<MeshData> MeshGenerator::GenerateMesh(
    const TopoDS_Shape& shape,
    MeshQuality quality
) {
    auto startTime = std::chrono::high_resolution_clock::now();
    
    try {
        double deflection = DeflectionFromQuality(quality);
        
        // Use BRepMesh_IncrementalMesh for mesh generation
        BRepMesh_IncrementalMesh mesher(
            shape,
            deflection,
            Standard_False,  // relative
            0.5,             // angular deflection
            Standard_True    // parallel processing
        );
        
        if (!mesher.IsDone()) {
            Logger::Error("BRepMesh_IncrementalMesh failed");
            return std::nullopt;
        }
        
        // Extract triangle mesh data
        MeshData meshData;
        
        for (TopExp_Explorer faceExp(shape, TopAbs_FACE); faceExp.More(); faceExp.Next()) {
            TopoDS_Face face = TopoDS::Face(faceExp.Current());
            TopLoc_Location loc;
            Handle(Poly_Triangulation) triangulation = BRep_Tool::Triangulation(face, loc);
            
            if (triangulation.IsNull()) continue;
            
            // Extract vertices and normals
            const TColgp_Array1OfPnt& nodes = triangulation->Nodes();
            for (int i = 1; i <= nodes.Length(); ++i) {
                gp_Pnt p = nodes(i).Transformed(loc);
                meshData.vertices.push_back(static_cast<float>(p.X()));
                meshData.vertices.push_back(static_cast<float>(p.Y()));
                meshData.vertices.push_back(static_cast<float>(p.Z()));
            }
            
            // Extract triangle indices
            const Poly_Array1OfTriangle& triangles = triangulation->Triangles();
            for (int i = 1; i <= triangles.Length(); ++i) {
                Standard_Integer n1, n2, n3;
                triangles(i).Get(n1, n2, n3);
                
                meshData.indices.push_back(static_cast<uint32_t>(n1 - 1));
                meshData.indices.push_back(static_cast<uint32_t>(n2 - 1));
                meshData.indices.push_back(static_cast<uint32_t>(n3 - 1));
            }
        }
        
        // Compute normals (simplified - use face normals)
        ComputeVertexNormals(meshData);
        
        meshData.triangleCount = meshData.indices.size() / 3;
        
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            endTime - startTime
        ).count();
        
        if (duration > 500) {
            Logger::Warning(
                "Mesh generation took " + std::to_string(duration) + 
                "ms (exceeds 500ms threshold)"
            );
        }
        
        return meshData;
    }
    catch (const Standard_Failure& e) {
        Logger::Error(
            "Mesh generation failed: " + 
            std::string(e.GetMessageString())
        );
        return std::nullopt;
    }
}

double MeshGenerator::DeflectionFromQuality(MeshQuality quality) {
    switch (quality) {
        case MeshQuality::Coarse: return 1.0;
        case MeshQuality::Normal: return 0.1;
        case MeshQuality::Fine:   return 0.01;
        default: return 0.1;
    }
}
```


### Boolean Operations Implementation

```cpp
ShapeResult ShapeLibrary::Union(
    const PrimitiveShape& shape1,
    const PrimitiveShape& shape2
) {
    try {
        // Perform boolean union
        BRepAlgoAPI_Fuse fuseOp(shape1.brepShape, shape2.brepShape);
        if (!fuseOp.IsDone()) {
            return ShapeResult::Error(
                ShapeError::BooleanOperationFailed,
                "Boolean union operation failed"
            );
        }
        
        TopoDS_Shape resultShape = fuseOp.Shape();
        
        // Generate mesh for result
        auto meshData = MeshGenerator::GenerateMesh(resultShape);
        if (!meshData.has_value()) {
            return ShapeResult::Error(
                ShapeError::MeshGenerationFailed,
                "Failed to generate mesh for boolean result"
            );
        }
        
        // Create result primitive shape
        PrimitiveShape result;
        result.id = GenerateUUID();
        result.type = ShapeType::BooleanResult;
        result.brepShape = resultShape;
        result.mesh = std::move(meshData.value());
        result.parameters.dimensions["operation"] = 0; // Union
        result.parameters.creationTime = std::chrono::system_clock::now();
        
        return ShapeResult::Success(std::move(result));
    }
    catch (const Standard_Failure& e) {
        return ShapeResult::Error(
            ShapeError::BooleanOperationFailed,
            "OpenCascade error: " + std::string(e.GetMessageString())
        );
    }
}

ShapeResult ShapeLibrary::Difference(
    const PrimitiveShape& shape1,
    const PrimitiveShape& shape2
) {
    try {
        BRepAlgoAPI_Cut cutOp(shape1.brepShape, shape2.brepShape);
        if (!cutOp.IsDone()) {
            return ShapeResult::Error(
                ShapeError::BooleanOperationFailed,
                "Boolean difference operation failed"
            );
        }
        
        TopoDS_Shape resultShape = cutOp.Shape();
        auto meshData = MeshGenerator::GenerateMesh(resultShape);
        if (!meshData.has_value()) {
            return ShapeResult::Error(
                ShapeError::MeshGenerationFailed,
                "Failed to generate mesh for boolean result"
            );
        }
        
        PrimitiveShape result;
        result.id = GenerateUUID();
        result.type = ShapeType::BooleanResult;
        result.brepShape = resultShape;
        result.mesh = std::move(meshData.value());
        result.parameters.dimensions["operation"] = 1; // Difference
        result.parameters.creationTime = std::chrono::system_clock::now();
        
        return ShapeResult::Success(std::move(result));
    }
    catch (const Standard_Failure& e) {
        return ShapeResult::Error(
            ShapeError::BooleanOperationFailed,
            "OpenCascade error: " + std::string(e.GetMessageString())
        );
    }
}

ShapeResult ShapeLibrary::Intersection(
    const PrimitiveShape& shape1,
    const PrimitiveShape& shape2
) {
    try {
        BRepAlgoAPI_Common commonOp(shape1.brepShape, shape2.brepShape);
        if (!commonOp.IsDone()) {
            return ShapeResult::Error(
                ShapeError::BooleanOperationFailed,
                "Boolean intersection operation failed"
            );
        }
        
        TopoDS_Shape resultShape = commonOp.Shape();
        auto meshData = MeshGenerator::GenerateMesh(resultShape);
        if (!meshData.has_value()) {
            return ShapeResult::Error(
                ShapeError::MeshGenerationFailed,
                "Failed to generate mesh for boolean result"
            );
        }
        
        PrimitiveShape result;
        result.id = GenerateUUID();
        result.type = ShapeType::BooleanResult;
        result.brepShape = resultShape;
        result.mesh = std::move(meshData.value());
        result.parameters.dimensions["operation"] = 2; // Intersection
        result.parameters.creationTime = std::chrono::system_clock::now();
        
        return ShapeResult::Success(std::move(result));
    }
    catch (const Standard_Failure& e) {
        return ShapeResult::Error(
            ShapeError::BooleanOperationFailed,
            "OpenCascade error: " + std::string(e.GetMessageString())
        );
    }
}
```


### Serialization Implementation

```cpp
std::string PrimitiveShape::ToJSON() const {
    json j;
    j["id"] = id;
    j["type"] = static_cast<int>(type);
    j["parameters"] = parameters.dimensions;
    j["creationTime"] = std::chrono::system_clock::to_time_t(
        parameters.creationTime
    );
    
    // Serialize mesh data
    j["mesh"]["vertices"] = mesh.vertices;
    j["mesh"]["indices"] = mesh.indices;
    j["mesh"]["triangleCount"] = mesh.triangleCount;
    
    // Serialize BRep shape to BREP format string
    std::ostringstream brepStream;
    BRepTools::Write(brepShape, brepStream);
    j["brepData"] = brepStream.str();
    
    return j.dump();
}

PrimitiveShape PrimitiveShape::FromJSON(const std::string& jsonStr) {
    json j = json::parse(jsonStr);
    
    PrimitiveShape shape;
    shape.id = j["id"];
    shape.type = static_cast<ShapeType>(j["type"]);
    shape.parameters.dimensions = j["parameters"];
    
    auto timeT = j["creationTime"].get<std::time_t>();
    shape.parameters.creationTime = std::chrono::system_clock::from_time_t(timeT);
    
    // Deserialize mesh
    shape.mesh.vertices = j["mesh"]["vertices"].get<std::vector<float>>();
    shape.mesh.indices = j["mesh"]["indices"].get<std::vector<uint32_t>>();
    shape.mesh.triangleCount = j["mesh"]["triangleCount"];
    
    // Deserialize BRep shape
    std::string brepData = j["brepData"];
    std::istringstream brepStream(brepData);
    BRep_Builder builder;
    BRepTools::Read(shape.brepShape, brepStream, builder);
    
    return shape;
}
```

