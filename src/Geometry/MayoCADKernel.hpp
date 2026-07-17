#pragma once

#include <string>
#include <memory>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace SZM::Geometry {

    using json = nlohmann::json;

    /**
     * @brief Mayo CAD Kernel integration
     * Professional B-Rep solid modeling with:
     * - STEP/IGES import/export
     * - Boolean operations (union, diff, intersect)
     * - Advanced topology operations
     * - Mesh generation and quality
     * - Part editing and modification
     */
    class MayoCADKernel {
    public:
        enum class FileFormat {
            STEP,   // ISO 10303-21
            IGES,   // Initial Graphics Exchange Specification
            STL,    // Stereolithography
            OBJ,    // Wavefront OBJ
            BREP    // OpenCASCADE B-Rep
        };

        enum class BooleanOp {
            Union,
            Difference,
            Intersection,
            XOR
        };

        struct MeshQuality {
            float angularDeflection = 0.5f;  // degrees
            float linearDeflection = 0.01f;   // mm
            float chordError = 0.0001f;
        };

        MayoCADKernel();
        ~MayoCADKernel();

        /**
         * @brief Load CAD model from file
         */
        bool LoadModel(const std::string& filePath, FileFormat format);

        /**
         * @brief Save model to file
         */
        bool SaveModel(const std::string& outputPath, FileFormat format);

        /**
         * @brief Perform boolean operation
         */
        bool BooleanOperation(uint32_t shapeId1, uint32_t shapeId2, 
                             BooleanOp operation, uint32_t& resultId);

        /**
         * @brief Get bounding box
         */
        bool GetBoundingBox(uint32_t shapeId, 
                           float& minX, float& minY, float& minZ,
                           float& maxX, float& maxY, float& maxZ);

        /**
         * @brief Generate mesh from solid
         */
        bool GenerateMesh(uint32_t shapeId, const MeshQuality& quality,
                         std::vector<float>& vertices,
                         std::vector<uint32_t>& triangles);

        /**
         * @brief Get shape properties
         */
        json GetShapeProperties(uint32_t shapeId);

        /**
         * @brief Offset/shell operation
         */
        bool OffsetShape(uint32_t shapeId, float offset, uint32_t& resultId);

        /**
         * @brief Fillet edges
         */
        bool FilletEdges(uint32_t shapeId, const std::vector<uint32_t>& edgeIds,
                        float radius, uint32_t& resultId);

        /**
         * @brief Chamfer edges
         */
        bool ChamferEdges(uint32_t shapeId, const std::vector<uint32_t>& edgeIds,
                         float distance, uint32_t& resultId);

        /**
         * @brief Get topology info
         */
        json GetTopology(uint32_t shapeId);

    private:
        // Mayo kernel handle
        void* m_KernelHandle = nullptr;
    };

}
