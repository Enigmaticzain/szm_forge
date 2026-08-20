#include "ShapeLibrary.hpp"
#include "../Core/Logger.hpp"

// OpenCascade includes
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepPrimAPI_MakeCone.hxx>
#include <BRepPrimAPI_MakeTorus.hxx>
#include <BRepPrimAPI_MakeWedge.hxx>
#include <BRepPrimAPI_MakeSphere.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <BRepPrimAPI_MakeRevol.hxx>
#include <BRepBuilderAPI_MakePolygon.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_Sewing.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Common.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <BRep_Tool.hxx>
#include <BRep_Builder.hxx>
#include <BRepTools.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Compound.hxx>
#include <TopLoc_Location.hxx>
#include <Poly_Triangulation.hxx>
#include <Poly_Array1OfTriangle.hxx>
#include <TColgp_Array1OfPnt.hxx>
#include <gp_Trsf.hxx>
#include <gp_Ax1.hxx>
#include <gp_Ax2.hxx>
#include <gp_Parab.hxx>
#include <Standard_Failure.hxx>

#include <sstream>
#include <random>
#include <iomanip>
#include <cmath>

namespace SZM::Geometry {

// Static member initialization
MeshQuality MeshGenerator::s_currentQuality = MeshQuality::Normal;

// ============================================================================
// Error Handling
// ============================================================================

const char* GetErrorDescription(ShapeError error) {
    switch (error) {
        case ShapeError::Success: return "Success";
        case ShapeError::InvalidRadius: return "Invalid radius";
        case ShapeError::InvalidHeight: return "Invalid height";
        case ShapeError::InvalidLength: return "Invalid length";
        case ShapeError::InvalidWidth: return "Invalid width";
        case ShapeError::InvalidDepth: return "Invalid depth";
        case ShapeError::InvalidFocalLength: return "Invalid focal length";
        case ShapeError::InvalidSideCount: return "Invalid side count";
        case ShapeError::InvalidRadiusRatio: return "Invalid radius ratio";
        case ShapeError::MeshGenerationFailed: return "Mesh generation failed";
        case ShapeError::BRepOperationFailed: return "BRep operation failed";
        case ShapeError::BooleanOperationFailed: return "Boolean operation failed";
        case ShapeError::InvalidShapeType: return "Invalid shape type";
        case ShapeError::SerializationFailed: return "Serialization failed";
        case ShapeError::DeserializationFailed: return "Deserialization failed";
        default: return "Unknown error";
    }
}

// ============================================================================
// UUID Generation
// ============================================================================

std::string GenerateUUID() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<uint32_t> dis(0, 0xFFFFFFFF);
    
    uint32_t data[4];
    for (int i = 0; i < 4; ++i) {
        data[i] = dis(gen);
    }
    
    // Set version to 4 (random UUID)
    data[1] = (data[1] & 0xFFFF0FFF) | 0x00004000;
    // Set variant to RFC4122
    data[2] = (data[2] & 0x3FFFFFFF) | 0x80000000;
    
    std::ostringstream oss;
    oss << std::hex << std::setfill('0')
        << std::setw(8) << data[0] << "-"
        << std::setw(4) << (data[1] >> 16) << "-"
        << std::setw(4) << (data[1] & 0xFFFF) << "-"
        << std::setw(4) << (data[2] >> 16) << "-"
        << std::setw(4) << (data[2] & 0xFFFF)
        << std::setw(8) << data[3];
    
    return oss.str();
}

// ============================================================================
// ShapeParameters Implementation
// ============================================================================

double ShapeParameters::GetRadius() const {
    auto it = dimensions.find("radius");
    if (it != dimensions.end()) {
        return it->second;
    }
    return 0.0;
}

double ShapeParameters::GetHeight() const {
    auto it = dimensions.find("height");
    if (it != dimensions.end()) {
        return it->second;
    }
    return 0.0;
}

// ============================================================================
// MeshGenerator Implementation
// ============================================================================

void MeshGenerator::SetQuality(MeshQuality quality) {
    s_currentQuality = quality;
}

MeshQuality MeshGenerator::GetQuality() {
    return s_currentQuality;
}

double MeshGenerator::GetDeflection(MeshQuality quality) {
    return DeflectionFromQuality(quality);
}

double MeshGenerator::DeflectionFromQuality(MeshQuality quality) {
    switch (quality) {
        case MeshQuality::Coarse: return 1.0;
        case MeshQuality::Normal: return 0.1;
        case MeshQuality::Fine:   return 0.01;
        default: return 0.1;
    }
}

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
            SZM::Core::Logger::Error("BRepMesh_IncrementalMesh failed");
            return std::nullopt;
        }
        
        // Extract triangle mesh data
        MeshData meshData;
        meshData.triangleCount = 0;
        
        for (TopExp_Explorer faceExp(shape, TopAbs_FACE); faceExp.More(); faceExp.Next()) {
            TopoDS_Face face = TopoDS::Face(faceExp.Current());
            TopLoc_Location loc;
            Handle(Poly_Triangulation) triangulation = BRep_Tool::Triangulation(face, loc);
            
            if (triangulation.IsNull()) continue;
            
            size_t vertexOffset = meshData.vertices.size() / 3;
            
            // Extract vertices
            const TColgp_Array1OfPnt& nodes = triangulation->Nodes();
            for (int i = 1; i <= nodes.Length(); ++i) {
                gp_Pnt p = nodes(i).Transformed(loc);
                meshData.vertices.push_back(static_cast<float>(p.X()));
                meshData.vertices.push_back(static_cast<float>(p.Y()));
                meshData.vertices.push_back(static_cast<float>(p.Z()));
            }
            
            // Extract triangle indices
            const Poly_Array1OfTriangle& triangles = triangulation->Triangles();
            bool reversed = face.Orientation() == TopAbs_REVERSED;
            
            for (int i = 1; i <= triangles.Length(); ++i) {
                Standard_Integer n1, n2, n3;
                triangles(i).Get(n1, n2, n3);
                
                if (reversed) {
                    meshData.indices.push_back(static_cast<uint32_t>(vertexOffset + n1 - 1));
                    meshData.indices.push_back(static_cast<uint32_t>(vertexOffset + n3 - 1));
                    meshData.indices.push_back(static_cast<uint32_t>(vertexOffset + n2 - 1));
                } else {
                    meshData.indices.push_back(static_cast<uint32_t>(vertexOffset + n1 - 1));
                    meshData.indices.push_back(static_cast<uint32_t>(vertexOffset + n2 - 1));
                    meshData.indices.push_back(static_cast<uint32_t>(vertexOffset + n3 - 1));
                }
                
                meshData.triangleCount++;
            }
        }
        
        // Compute vertex normals (simple per-face normal)
        meshData.normals.resize(meshData.vertices.size(), 0.0f);
        std::vector<int> normalCounts(meshData.vertices.size() / 3, 0);
        
        for (size_t i = 0; i < meshData.indices.size(); i += 3) {
            uint32_t i0 = meshData.indices[i];
            uint32_t i1 = meshData.indices[i + 1];
            uint32_t i2 = meshData.indices[i + 2];
            
            float* v0 = &meshData.vertices[i0 * 3];
            float* v1 = &meshData.vertices[i1 * 3];
            float* v2 = &meshData.vertices[i2 * 3];
            
            // Compute triangle normal
            float e1[3] = {v1[0] - v0[0], v1[1] - v0[1], v1[2] - v0[2]};
            float e2[3] = {v2[0] - v0[0], v2[1] - v0[1], v2[2] - v0[2]};
            float n[3] = {
                e1[1] * e2[2] - e1[2] * e2[1],
                e1[2] * e2[0] - e1[0] * e2[2],
                e1[0] * e2[1] - e1[1] * e2[0]
            };
            
            // Accumulate normals for each vertex
            for (uint32_t idx : {i0, i1, i2}) {
                meshData.normals[idx * 3 + 0] += n[0];
                meshData.normals[idx * 3 + 1] += n[1];
                meshData.normals[idx * 3 + 2] += n[2];
                normalCounts[idx]++;
            }
        }
        
        // Normalize averaged normals
        for (size_t i = 0; i < normalCounts.size(); ++i) {
            if (normalCounts[i] > 0) {
                float* n = &meshData.normals[i * 3];
                float len = std::sqrt(n[0]*n[0] + n[1]*n[1] + n[2]*n[2]);
                if (len > 1e-6f) {
                    n[0] /= len;
                    n[1] /= len;
                    n[2] /= len;
                }
            }
        }
        
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            endTime - startTime
        ).count();
        
        if (duration > 500) {
            SZM::Core::Logger::Warning(
                "Mesh generation took " + std::to_string(duration) + 
                "ms (exceeds 500ms threshold)"
            );
        }
        
        return meshData;
    }
    catch (const Standard_Failure& e) {
        SZM::Core::Logger::Error(
            "Mesh generation failed: " + 
            std::string(e.GetMessageString())
        );
        return std::nullopt;
    }
}

// ============================================================================
// ShapeLibrary - Primitive Creation Functions
// ============================================================================

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

ShapeResult ShapeLibrary::CreateWedge(
    double length, 
    double width, 
    double height
) {
    if (length <= 0.0) {
        return ShapeResult::Error(
            ShapeError::InvalidLength,
            "Wedge length must be greater than zero"
        );
    }
    if (width <= 0.0) {
        return ShapeResult::Error(
            ShapeError::InvalidWidth,
            "Wedge width must be greater than zero"
        );
    }
    if (height <= 0.0) {
        return ShapeResult::Error(
            ShapeError::InvalidHeight,
            "Wedge height must be greater than zero"
        );
    }
    
    try {
        // Create wedge at origin
        gp_Ax2 ax2(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1));
        BRepPrimAPI_MakeWedge wedgeMaker(ax2, length, width, height);
        TopoDS_Shape brepShape = wedgeMaker.Shape();
        
        auto meshData = MeshGenerator::GenerateMesh(brepShape);
        if (!meshData.has_value()) {
            return ShapeResult::Error(
                ShapeError::MeshGenerationFailed,
                "Failed to generate mesh for wedge"
            );
        }
        
        PrimitiveShape shape;
        shape.id = GenerateUUID();
        shape.type = ShapeType::Wedge;
        shape.brepShape = brepShape;
        shape.mesh = std::move(meshData.value());
        shape.parameters.dimensions["length"] = length;
        shape.parameters.dimensions["width"] = width;
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
        std::vector<gp_Pnt> basePoints = {p1, p2, p3, p4};
        for (size_t i = 0; i < 4; ++i) {
            gp_Pnt v1 = basePoints[i];
            gp_Pnt v2 = basePoints[(i + 1) % 4];
            
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
