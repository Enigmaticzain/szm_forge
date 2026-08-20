# Requirements Document

## Introduction

This document specifies requirements for expanding the SZM Forge shape library with comprehensive 3D geometric primitives. The system will integrate OpenCascade's BRepPrimAPI to provide immediate mesh generation for advanced shapes including cylinders, cones, tori, wedges, pyramids, prisms, ellipsoids, and paraboloids. The feature enables seamless creation and boolean operation workflows for CAD modeling within the application.

## Glossary

- **Shape_Library**: The SZM Forge component responsible for creating and managing geometric primitives
- **Primitive_Shape**: A fundamental 3D geometric form (cylinder, cone, torus, wedge, pyramid, prism, ellipsoid, paraboloid)
- **BRepPrimAPI**: OpenCascade library API for creating boundary representation primitive shapes
- **TopoDS_Shape**: OpenCascade topology data structure representing a geometric shape
- **Mesh_Generator**: Component that converts boundary representation shapes into triangle meshes
- **Boolean_Operation**: CSG operations (union, difference, intersection) applied to shapes
- **Eager_Mesh_Generation**: Immediate mesh creation at shape instantiation time rather than deferred generation
- **CAD_Modeler**: User interface component for shape creation and manipulation

## Requirements

### Requirement 1: Cylinder Primitive

**User Story:** As a CAD user, I want to create cylindrical shapes with configurable radius and height, so that I can model mechanical components like shafts and tubes.

#### Acceptance Criteria

1. THE Shape_Library SHALL provide a cylinder creation function accepting radius and height parameters
2. WHEN a cylinder is created, THE Shape_Library SHALL generate the TopoDS_Shape using BRepPrimAPI_MakeCylinder
3. WHEN a cylinder TopoDS_Shape is created, THE Mesh_Generator SHALL immediately generate a triangle mesh representation
4. THE Shape_Library SHALL validate that radius is greater than zero millimeters
5. THE Shape_Library SHALL validate that height is greater than zero millimeters
6. WHEN invalid parameters are provided, THE Shape_Library SHALL return an error code indicating the parameter violation

### Requirement 2: Cone Primitive

**User Story:** As a CAD user, I want to create conical shapes with configurable base radius, top radius, and height, so that I can model tapered components and transition sections.

#### Acceptance Criteria

1. THE Shape_Library SHALL provide a cone creation function accepting base radius, top radius, and height parameters
2. WHEN a cone is created, THE Shape_Library SHALL generate the TopoDS_Shape using BRepPrimAPI_MakeCone
3. WHEN a cone TopoDS_Shape is created, THE Mesh_Generator SHALL immediately generate a triangle mesh representation
4. THE Shape_Library SHALL validate that base radius is greater than or equal to zero millimeters
5. THE Shape_Library SHALL validate that top radius is greater than or equal to zero millimeters
6. THE Shape_Library SHALL validate that height is greater than zero millimeters
7. THE Shape_Library SHALL validate that at least one radius (base or top) is greater than zero
8. WHEN invalid parameters are provided, THE Shape_Library SHALL return an error code indicating the parameter violation

### Requirement 3: Torus Primitive

**User Story:** As a CAD user, I want to create toroidal shapes with configurable major and minor radii, so that I can model pipe bends and donut-shaped components.

#### Acceptance Criteria

1. THE Shape_Library SHALL provide a torus creation function accepting major radius and minor radius parameters
2. WHEN a torus is created, THE Shape_Library SHALL generate the TopoDS_Shape using BRepPrimAPI_MakeTorus
3. WHEN a torus TopoDS_Shape is created, THE Mesh_Generator SHALL immediately generate a triangle mesh representation
4. THE Shape_Library SHALL validate that major radius is greater than zero millimeters
5. THE Shape_Library SHALL validate that minor radius is greater than zero millimeters
6. THE Shape_Library SHALL validate that minor radius is less than major radius
7. WHEN invalid parameters are provided, THE Shape_Library SHALL return an error code indicating the parameter violation

### Requirement 4: Wedge Primitive

**User Story:** As a CAD user, I want to create wedge shapes with configurable dimensions, so that I can model sloped components and ramps.

#### Acceptance Criteria

1. THE Shape_Library SHALL provide a wedge creation function accepting length, width, and height parameters
2. WHEN a wedge is created, THE Shape_Library SHALL generate the TopoDS_Shape using BRepPrimAPI_MakeWedge
3. WHEN a wedge TopoDS_Shape is created, THE Mesh_Generator SHALL immediately generate a triangle mesh representation
4. THE Shape_Library SHALL validate that length is greater than zero millimeters
5. THE Shape_Library SHALL validate that width is greater than zero millimeters
6. THE Shape_Library SHALL validate that height is greater than zero millimeters
7. WHEN invalid parameters are provided, THE Shape_Library SHALL return an error code indicating the parameter violation

### Requirement 5: Pyramid Primitive

**User Story:** As a CAD user, I want to create pyramid shapes with configurable base dimensions and height, so that I can model pointed structures and tapered forms.

#### Acceptance Criteria

1. THE Shape_Library SHALL provide a pyramid creation function accepting base width, base depth, and height parameters
2. WHEN a pyramid is created, THE Shape_Library SHALL construct the TopoDS_Shape using BRepBuilderAPI face and edge operations
3. WHEN a pyramid TopoDS_Shape is created, THE Mesh_Generator SHALL immediately generate a triangle mesh representation
4. THE Shape_Library SHALL validate that base width is greater than zero millimeters
5. THE Shape_Library SHALL validate that base depth is greater than zero millimeters
6. THE Shape_Library SHALL validate that height is greater than zero millimeters
7. WHEN invalid parameters are provided, THE Shape_Library SHALL return an error code indicating the parameter violation

### Requirement 6: Prism Primitive

**User Story:** As a CAD user, I want to create prism shapes with configurable base polygon and extrusion height, so that I can model hexagonal bolts and polygonal structures.

#### Acceptance Criteria

1. THE Shape_Library SHALL provide a prism creation function accepting number of sides, radius, and height parameters
2. WHEN a prism is created with number of sides between 3 and 32, THE Shape_Library SHALL construct the TopoDS_Shape using BRepBuilderAPI operations
3. WHEN a prism TopoDS_Shape is created, THE Mesh_Generator SHALL immediately generate a triangle mesh representation
4. THE Shape_Library SHALL validate that number of sides is between 3 and 32
5. THE Shape_Library SHALL validate that radius is greater than zero millimeters
6. THE Shape_Library SHALL validate that height is greater than zero millimeters
7. WHEN invalid parameters are provided, THE Shape_Library SHALL return an error code indicating the parameter violation

### Requirement 7: Ellipsoid Primitive

**User Story:** As a CAD user, I want to create ellipsoidal shapes with configurable radii along three axes, so that I can model oblate and prolate spheroids.

#### Acceptance Criteria

1. THE Shape_Library SHALL provide an ellipsoid creation function accepting x-radius, y-radius, and z-radius parameters
2. WHEN an ellipsoid is created, THE Shape_Library SHALL generate the TopoDS_Shape using BRepPrimAPI_MakeSphere with scaling transformations
3. WHEN an ellipsoid TopoDS_Shape is created, THE Mesh_Generator SHALL immediately generate a triangle mesh representation
4. THE Shape_Library SHALL validate that x-radius is greater than zero millimeters
5. THE Shape_Library SHALL validate that y-radius is greater than zero millimeters
6. THE Shape_Library SHALL validate that z-radius is greater than zero millimeters
7. WHEN invalid parameters are provided, THE Shape_Library SHALL return an error code indicating the parameter violation

### Requirement 8: Paraboloid Primitive

**User Story:** As a CAD user, I want to create paraboloid shapes with configurable focal length and height, so that I can model reflective dishes and aerodynamic surfaces.

#### Acceptance Criteria

1. THE Shape_Library SHALL provide a paraboloid creation function accepting focal length and height parameters
2. WHEN a paraboloid is created, THE Shape_Library SHALL construct the TopoDS_Shape using BRepBuilderAPI revolution operations with parabolic profile
3. WHEN a paraboloid TopoDS_Shape is created, THE Mesh_Generator SHALL immediately generate a triangle mesh representation
4. THE Shape_Library SHALL validate that focal length is greater than zero millimeters
5. THE Shape_Library SHALL validate that height is greater than zero millimeters
6. WHEN invalid parameters are provided, THE Shape_Library SHALL return an error code indicating the parameter violation

### Requirement 9: Eager Mesh Generation

**User Story:** As a CAD user, I want shape meshes to be available immediately after creation, so that I can visualize and manipulate shapes without waiting for deferred mesh generation.

#### Acceptance Criteria

1. WHEN any Primitive_Shape is created, THE Mesh_Generator SHALL generate the triangle mesh within the same function call
2. THE Mesh_Generator SHALL use BRepMesh_IncrementalMesh with configurable deflection tolerance
3. WHEN mesh generation completes, THE Shape_Library SHALL store both the TopoDS_Shape and the triangle mesh in the shape object
4. THE Shape_Library SHALL return the shape object with mesh data available for immediate rendering
5. WHEN mesh generation fails, THE Shape_Library SHALL log the error and return an error code
6. THE Mesh_Generator SHALL complete mesh generation within 500 milliseconds for primitives with default parameters

### Requirement 10: Boolean Operation Integration

**User Story:** As a CAD user, I want to perform boolean operations on newly created shapes, so that I can build complex geometries through constructive solid geometry workflows.

#### Acceptance Criteria

1. THE Shape_Library SHALL provide a union operation function accepting two TopoDS_Shape parameters
2. THE Shape_Library SHALL provide a difference operation function accepting two TopoDS_Shape parameters
3. THE Shape_Library SHALL provide an intersection operation function accepting two TopoDS_Shape parameters
4. WHEN a Boolean_Operation is performed, THE Shape_Library SHALL use BRepAlgoAPI boolean operations
5. WHEN a Boolean_Operation completes, THE Mesh_Generator SHALL immediately generate a triangle mesh for the resulting shape
6. WHEN a Boolean_Operation fails due to invalid geometry, THE Shape_Library SHALL return an error code indicating the operation failure
7. THE Shape_Library SHALL preserve the original shapes after Boolean_Operation execution

### Requirement 11: CAD Modeler Integration

**User Story:** As a CAD user, I want to create primitive shapes through the application interface, so that I can access the expanded shape library without writing code.

#### Acceptance Criteria

1. THE CAD_Modeler SHALL provide menu options for creating each Primitive_Shape type
2. WHEN a user selects a primitive shape creation option, THE CAD_Modeler SHALL display a parameter input dialog
3. THE CAD_Modeler SHALL display parameter labels with units (millimeters) for all dimensional inputs
4. WHEN a user confirms shape creation with valid parameters, THE CAD_Modeler SHALL invoke the Shape_Library creation function
5. WHEN shape creation succeeds, THE CAD_Modeler SHALL display the shape in the 3D viewport
6. WHEN shape creation fails, THE CAD_Modeler SHALL display an error message describing the validation failure
7. THE CAD_Modeler SHALL provide default parameter values for each Primitive_Shape type

### Requirement 12: Parameter Validation and Error Handling

**User Story:** As a CAD user, I want clear error messages when I provide invalid shape parameters, so that I can quickly correct my inputs and create valid shapes.

#### Acceptance Criteria

1. THE Shape_Library SHALL validate all dimensional parameters before invoking BRepPrimAPI operations
2. WHEN a dimensional parameter is zero or negative, THE Shape_Library SHALL return an error code with the parameter name
3. WHEN torus minor radius exceeds major radius, THE Shape_Library SHALL return an error code indicating the geometric constraint violation
4. WHEN prism sides parameter is outside the valid range, THE Shape_Library SHALL return an error code indicating the valid range
5. THE Shape_Library SHALL provide string descriptions for all error codes
6. WHEN BRepPrimAPI operations fail, THE Shape_Library SHALL capture the OpenCascade exception and return an appropriate error code
7. THE Shape_Library SHALL log detailed error information for debugging purposes

### Requirement 13: Mesh Quality Configuration

**User Story:** As a CAD user, I want to configure mesh generation quality, so that I can balance visual fidelity against performance for different use cases.

#### Acceptance Criteria

1. THE Shape_Library SHALL provide a mesh quality setting with options for coarse, normal, and fine quality levels
2. WHEN mesh quality is set to coarse, THE Mesh_Generator SHALL use a linear deflection of 1.0 millimeters
3. WHEN mesh quality is set to normal, THE Mesh_Generator SHALL use a linear deflection of 0.1 millimeters
4. WHEN mesh quality is set to fine, THE Mesh_Generator SHALL use a linear deflection of 0.01 millimeters
5. THE Shape_Library SHALL apply the configured mesh quality to all Primitive_Shape mesh generation operations
6. THE CAD_Modeler SHALL provide a settings interface for selecting mesh quality
7. THE Shape_Library SHALL persist the mesh quality setting across application sessions

### Requirement 14: Shape Data Structure

**User Story:** As a developer, I want a consistent data structure for all primitive shapes, so that I can uniformly process shapes regardless of their type.

#### Acceptance Criteria

1. THE Shape_Library SHALL define a PrimitiveShape structure containing TopoDS_Shape, triangle mesh, and shape type fields
2. THE PrimitiveShape structure SHALL include a unique identifier field for shape tracking
3. THE PrimitiveShape structure SHALL include creation timestamp and parameter fields
4. THE Shape_Library SHALL provide accessor functions for retrieving TopoDS_Shape from PrimitiveShape
5. THE Shape_Library SHALL provide accessor functions for retrieving triangle mesh from PrimitiveShape
6. THE Shape_Library SHALL provide a function to serialize PrimitiveShape to JSON format
7. THE Shape_Library SHALL provide a function to deserialize PrimitiveShape from JSON format

### Requirement 15: Performance Requirements

**User Story:** As a CAD user, I want responsive shape creation, so that I can iterate quickly on my designs without waiting for shape generation.

#### Acceptance Criteria

1. THE Shape_Library SHALL complete cylinder creation with mesh generation within 100 milliseconds for default parameters
2. THE Shape_Library SHALL complete cone creation with mesh generation within 100 milliseconds for default parameters
3. THE Shape_Library SHALL complete torus creation with mesh generation within 200 milliseconds for default parameters
4. THE Shape_Library SHALL complete ellipsoid creation with mesh generation within 150 milliseconds for default parameters
5. THE Shape_Library SHALL complete Boolean_Operation with mesh generation within 500 milliseconds for two primitive shapes
6. WHEN shape creation exceeds expected duration, THE Shape_Library SHALL log a performance warning with execution time
7. THE Mesh_Generator SHALL utilize parallel processing for mesh generation when available
