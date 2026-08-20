# Implementation Plan: Expanded Shape Library

## Overview

This implementation plan converts the expanded shape library design into actionable coding tasks. The plan follows an incremental approach: first establishing core data structures and infrastructure, then implementing each primitive shape with eager mesh generation, and finally integrating boolean operations and UI components. Each task builds on previous work to ensure a solid, testable foundation throughout development.

## Tasks

- [ ] 1. Set up core data structures and error handling infrastructure
  - Create ShapeType enum, ShapeError enum, and error handling structures
  - Implement PrimitiveShape structure with TopoDS_Shape, MeshData, and ShapeParameters
  - Create ShapeResult structure for operation result handling
  - Define MeshData structure with vertices, indices, normals, and validation
  - Implement UUID generation utility function
  - Create Logger integration for shape library operations
  - _Requirements: 14.1, 14.2, 14.3, 12.1, 12.5, 12.6_

- [ ] 2. Implement mesh generator infrastructure
  - [ ] 2.1 Create MeshGenerator class with quality configuration
    - Implement MeshQuality enum (Coarse, Normal, Fine)
    - Create SetQuality/GetQuality methods with static storage
    - Implement DeflectionFromQuality helper function
    - _Requirements: 13.1, 13.2, 13.3, 13.4, 13.5_
  
  - [ ] 2.2 Implement GenerateMesh function with BRepMesh integration
    - Use BRepMesh_IncrementalMesh for mesh generation
    - Extract vertices, indices, and normals from Poly_Triangulation
    - Implement vertex normal computation
    - Add performance logging for mesh generation duration
    - Return std::optional<MeshData> for error handling
    - _Requirements: 9.1, 9.2, 9.3, 9.6, 15.7_

- [ ] 3. Checkpoint - Verify mesh generator compiles and basic structure is correct
  - Ensure all tests pass, ask the user if questions arise.

- [ ] 4. Implement cylinder primitive
  - [ ] 4.1 Create ShapeLibrary::CreateCylinder function
    - Validate radius > 0 and height > 0
    - Use BRepPrimAPI_MakeCylinder to create BRep shape
    - Call MeshGenerator::GenerateMesh immediately after BRep creation
    - Populate PrimitiveShape structure with all fields
    - Return ShapeResult with appropriate error codes
    - _Requirements: 1.1, 1.2, 1.3, 1.4, 1.5, 1.6_
  
  - [ ]* 4.2 Write unit tests for cylinder creation
    - Test valid cylinder creation
    - Test invalid radius (zero and negative)
    - Test invalid height (zero and negative)
    - Test mesh availability after creation
    - Test performance within 100ms threshold
    - _Requirements: 1.6, 15.1_

- [ ] 5. Implement cone primitive
  - [ ] 5.1 Create ShapeLibrary::CreateCone function
    - Validate baseRadius >= 0, topRadius >= 0, height > 0
    - Validate at least one radius > 0
    - Use BRepPrimAPI_MakeCone to create BRep shape
    - Call MeshGenerator::GenerateMesh immediately after BRep creation
    - Populate PrimitiveShape structure with all fields
    - Return ShapeResult with appropriate error codes
    - _Requirements: 2.1, 2.2, 2.3, 2.4, 2.5, 2.6, 2.7, 2.8_
  
  - [ ]* 5.2 Write unit tests for cone creation
    - Test valid cone creation with various radius combinations
    - Test invalid parameter combinations
    - Test mesh availability
    - Test performance within 100ms threshold
    - _Requirements: 2.8, 15.2_

- [ ] 6. Implement torus primitive
  - [ ] 6.1 Create ShapeLibrary::CreateTorus function
    - Validate majorRadius > 0, minorRadius > 0
    - Validate minorRadius < majorRadius
    - Use BRepPrimAPI_MakeTorus to create BRep shape
    - Call MeshGenerator::GenerateMesh immediately after BRep creation
    - Populate PrimitiveShape structure with all fields
    - Return ShapeResult with appropriate error codes
    - _Requirements: 3.1, 3.2, 3.3, 3.4, 3.5, 3.6, 3.7_
  
  - [ ]* 6.2 Write unit tests for torus creation
    - Test valid torus creation
    - Test invalid radius relationships
    - Test mesh availability
    - Test performance within 200ms threshold
    - _Requirements: 3.7, 15.3_

- [ ] 7. Implement wedge primitive
  - [ ] 7.1 Create ShapeLibrary::CreateWedge function
    - Validate length > 0, width > 0, height > 0
    - Use BRepPrimAPI_MakeWedge to create BRep shape
    - Call MeshGenerator::GenerateMesh immediately after BRep creation
    - Populate PrimitiveShape structure with all fields
    - Return ShapeResult with appropriate error codes
    - _Requirements: 4.1, 4.2, 4.3, 4.4, 4.5, 4.6, 4.7_
  
  - [ ]* 7.2 Write unit tests for wedge creation
    - Test valid wedge creation
    - Test invalid dimension parameters
    - Test mesh availability
    - _Requirements: 4.7_

- [ ] 8. Checkpoint - Verify BRepPrimAPI primitives work correctly
  - Ensure all tests pass, ask the user if questions arise.

- [ ] 9. Implement pyramid primitive using BRepBuilderAPI
  - [ ] 9.1 Create ShapeLibrary::CreatePyramid function
    - Validate baseWidth > 0, baseDepth > 0, height > 0
    - Create base vertices and apex using gp_Pnt
    - Build base face using BRepBuilderAPI_MakePolygon
    - Create four triangular side faces
    - Assemble using BRepBuilderAPI_Sewing
    - Call MeshGenerator::GenerateMesh immediately after BRep creation
    - Populate PrimitiveShape structure with all fields
    - Return ShapeResult with appropriate error codes
    - _Requirements: 5.1, 5.2, 5.3, 5.4, 5.5, 5.6, 5.7_
  
  - [ ]* 9.2 Write unit tests for pyramid creation
    - Test valid pyramid creation
    - Test invalid dimension parameters
    - Test mesh availability
    - _Requirements: 5.7_

- [ ] 10. Implement prism primitive
  - [ ] 10.1 Create ShapeLibrary::CreatePrism function
    - Validate 3 <= sides <= 32, radius > 0, height > 0
    - Generate regular polygon base vertices using trigonometry
    - Create base wire using BRepBuilderAPI_MakePolygon
    - Create face and extrude using BRepPrimAPI_MakePrism
    - Call MeshGenerator::GenerateMesh immediately after BRep creation
    - Populate PrimitiveShape structure with all fields
    - Return ShapeResult with appropriate error codes
    - _Requirements: 6.1, 6.2, 6.3, 6.4, 6.5, 6.6, 6.7_
  
  - [ ]* 10.2 Write unit tests for prism creation
    - Test valid prism creation with various side counts
    - Test invalid side count (< 3, > 32)
    - Test invalid radius and height
    - Test mesh availability
    - _Requirements: 6.7_

- [ ] 11. Implement ellipsoid primitive
  - [ ] 11.1 Create ShapeLibrary::CreateEllipsoid function
    - Validate xRadius > 0, yRadius > 0, zRadius > 0
    - Create sphere using BRepPrimAPI_MakeSphere
    - Apply scaling transformation for ellipsoid shape
    - Call MeshGenerator::GenerateMesh immediately after BRep creation
    - Populate PrimitiveShape structure with all fields
    - Return ShapeResult with appropriate error codes
    - _Requirements: 7.1, 7.2, 7.3, 7.4, 7.5, 7.6, 7.7_
  
  - [ ]* 11.2 Write unit tests for ellipsoid creation
    - Test valid ellipsoid creation with various radii
    - Test invalid radius parameters
    - Test mesh availability
    - Test performance within 150ms threshold
    - _Requirements: 7.7, 15.4_

- [ ] 12. Implement paraboloid primitive using revolution
  - [ ] 12.1 Create ShapeLibrary::CreateParaboloid function
    - Validate focalLength > 0, height > 0
    - Create parabolic profile curve
    - Use BRepPrimAPI_MakeRevol for surface of revolution
    - Call MeshGenerator::GenerateMesh immediately after BRep creation
    - Populate PrimitiveShape structure with all fields
    - Return ShapeResult with appropriate error codes
    - _Requirements: 8.1, 8.2, 8.3, 8.4, 8.5, 8.6_
  
  - [ ]* 12.2 Write unit tests for paraboloid creation
    - Test valid paraboloid creation
    - Test invalid focal length and height
    - Test mesh availability
    - _Requirements: 8.6_

- [ ] 13. Checkpoint - Verify all primitive shapes create correctly
  - Ensure all tests pass, ask the user if questions arise.

- [ ] 14. Implement boolean operations
  - [ ] 14.1 Create ShapeLibrary::Union function
    - Use BRepAlgoAPI_Fuse for boolean union
    - Check IsDone() status and handle failures
    - Generate mesh for result shape
    - Create PrimitiveShape with BooleanResult type
    - Return ShapeResult with appropriate error codes
    - _Requirements: 10.1, 10.4, 10.5, 10.6, 10.7_
  
  - [ ] 14.2 Create ShapeLibrary::Difference function
    - Use BRepAlgoAPI_Cut for boolean difference
    - Check IsDone() status and handle failures
    - Generate mesh for result shape
    - Create PrimitiveShape with BooleanResult type
    - Return ShapeResult with appropriate error codes
    - _Requirements: 10.2, 10.4, 10.5, 10.6, 10.7_
  
  - [ ] 14.3 Create ShapeLibrary::Intersection function
    - Use BRepAlgoAPI_Common for boolean intersection
    - Check IsDone() status and handle failures
    - Generate mesh for result shape
    - Create PrimitiveShape with BooleanResult type
    - Return ShapeResult with appropriate error codes
    - _Requirements: 10.3, 10.4, 10.5, 10.6, 10.7_
  
  - [ ]* 14.4 Write unit tests for boolean operations
    - Test union of two cylinders
    - Test difference of overlapping shapes
    - Test intersection of overlapping shapes
    - Test error handling for invalid geometry
    - Test performance within 500ms threshold
    - _Requirements: 10.6, 15.5_

- [ ] 15. Implement serialization support
  - [ ] 15.1 Implement PrimitiveShape::ToJSON function
    - Serialize all PrimitiveShape fields to JSON
    - Convert mesh data to JSON arrays
    - Serialize BRep shape using BRepTools::Write
    - Use nlohmann/json or similar library
    - _Requirements: 14.6_
  
  - [ ] 15.2 Implement PrimitiveShape::FromJSON function
    - Deserialize JSON to PrimitiveShape structure
    - Reconstruct mesh data from JSON arrays
    - Deserialize BRep shape using BRepTools::Read
    - Handle deserialization errors gracefully
    - _Requirements: 14.7_
  
  - [ ]* 15.3 Write unit tests for serialization
    - Test round-trip serialization for each shape type
    - Test deserialization error handling
    - Verify mesh and BRep data integrity after round-trip
    - _Requirements: 14.6, 14.7_

- [ ] 16. Implement configuration and persistence
  - [ ] 16.1 Create ShapeConfig class
    - Implement Load/Save methods for configuration file
    - Store mesh quality setting persistently
    - Provide default parameter access for each shape type
    - _Requirements: 13.7_
  
  - [ ]* 16.2 Write unit tests for configuration
    - Test configuration persistence across sessions
    - Test default parameter retrieval
    - _Requirements: 13.7_

- [ ] 17. Integrate with CAD Modeler UI
  - [ ] 17.1 Create shape creation menu items
    - Add menu entries for all eight primitive shapes
    - Register menu callbacks with appropriate handlers
    - _Requirements: 11.1_
  
  - [ ] 17.2 Create parameter input dialogs
    - Design dialog layout for each shape type
    - Add input fields with units (millimeters)
    - Provide default values for each parameter
    - Implement parameter validation in dialog
    - _Requirements: 11.2, 11.3, 11.7_
  
  - [ ] 17.3 Wire shape creation to 3D viewport
    - Call ShapeLibrary creation functions with dialog parameters
    - Display created shapes in 3D viewport
    - Show error messages for validation failures with descriptive text
    - _Requirements: 11.4, 11.5, 11.6_
  
  - [ ] 17.4 Add mesh quality settings UI
    - Create settings panel with quality dropdown
    - Wire settings to ShapeConfig
    - Apply settings to all subsequent shape creations
    - _Requirements: 13.6_

- [ ] 18. Update CMakeLists.txt and build configuration
  - Add new shape library source files to CMakeLists.txt
  - Link OpenCascade libraries (TKPrim, TKMesh, TKBRep, TKBool)
  - Verify compilation with all dependencies
  - _Requirements: 1.1, 2.1, 3.1, 4.1, 5.1, 6.1, 7.1, 8.1_

- [ ] 19. Create integration header file
  - Create ShapeLibrary.hpp as main include file
  - Export all public API functions and structures
  - Add namespace documentation
  - _Requirements: 14.4, 14.5_

- [ ] 20. Final checkpoint - Integration testing and verification
  - Ensure all tests pass, ask the user if questions arise.
  - Verify all shapes can be created through UI
  - Test boolean operations on created shapes
  - Verify mesh quality settings work correctly
  - Confirm serialization and persistence work end-to-end

## Notes

- Tasks marked with `*` are optional and can be skipped for faster MVP
- Each task references specific requirements for traceability
- Checkpoints ensure incremental validation
- The implementation follows the architecture specified in the design document
- OpenCascade exception handling is critical for robust operation
- Mesh generation performance must be monitored to meet requirements

## Task Dependency Graph

```json
{
  "waves": [
    { "id": 0, "tasks": ["1.1"] },
    { "id": 1, "tasks": ["2.1"] },
    { "id": 2, "tasks": ["2.2"] },
    { "id": 3, "tasks": ["4.1", "5.1", "6.1", "7.1"] },
    { "id": 4, "tasks": ["4.2", "5.2", "6.2", "7.2"] },
    { "id": 5, "tasks": ["9.1", "10.1"] },
    { "id": 6, "tasks": ["9.2", "10.2", "11.1", "12.1"] },
    { "id": 7, "tasks": ["11.2", "12.2"] },
    { "id": 8, "tasks": ["14.1", "14.2", "14.3"] },
    { "id": 9, "tasks": ["14.4", "15.1"] },
    { "id": 10, "tasks": ["15.2", "15.3"] },
    { "id": 11, "tasks": ["16.1", "16.2"] },
    { "id": 12, "tasks": ["17.1"] },
    { "id": 13, "tasks": ["17.2"] },
    { "id": 14, "tasks": ["17.3", "17.4", "18.1", "19.1"] }
  ]
}
```
