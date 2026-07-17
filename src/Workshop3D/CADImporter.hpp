#pragma once

#include "PartLibrary.hpp"
#include <memory>
#include <string>
#include <vector>

namespace SZM::Workshop3D {

    class CADImporter {
    public:
        struct ImportOptions {
            float scale = 1.0f;
            bool mergeVertices = true;
            float mergeThreshold = 1e-6f;
            bool calculateNormals = true;
            std::string materialName = "default";
        };

        static std::shared_ptr<Part> ImportSTEP(const std::string& filepath);
        static std::shared_ptr<Part> ImportSTEP(
            const std::string& filepath,
            const ImportOptions& options
        );

        static std::shared_ptr<Part> ImportGEO(const std::string& filepath);
        static std::shared_ptr<Part> ImportGEO(
            const std::string& filepath,
            const ImportOptions& options
        );

        static std::shared_ptr<Part> ImportSTL(const std::string& filepath);
        static std::shared_ptr<Part> ImportSTL(
            const std::string& filepath,
            const ImportOptions& options
        );

        static std::shared_ptr<Part> ImportOBJ(const std::string& filepath);
        static std::shared_ptr<Part> ImportOBJ(
            const std::string& filepath,
            const ImportOptions& options
        );

        static std::shared_ptr<Part> ImportAuto(const std::string& filepath);
        static std::shared_ptr<Part> ImportAuto(
            const std::string& filepath,
            const ImportOptions& options
        );

        static int ImportFromCode8(
            const std::string& code8Dir,
            PartLibrary& library,
            bool recursive = true
        );

    private:
        static bool LoadSTLBinary(
            const std::string& filepath,
            std::vector<Geometry::Vector3>& vertices
        );
        static bool LoadSTLASCII(
            const std::string& filepath,
            std::vector<Geometry::Vector3>& vertices
        );
        static bool LoadOBJFile(
            const std::string& filepath,
            std::vector<Geometry::Vector3>& vertices,
            std::vector<uint32_t>& indices
        );
        static bool LoadGEOFile(
            const std::string& filepath,
            std::vector<Geometry::Vector3>& vertices,
            std::vector<uint32_t>& indices
        );

        static void MergeVertices(
            std::vector<Geometry::Vector3>& vertices,
            std::vector<uint32_t>& indices,
            float threshold
        );
        static void CalculateNormals(
            const std::vector<Geometry::Vector3>& vertices,
            const std::vector<uint32_t>& indices,
            std::vector<Geometry::Vector3>& normals
        );
    };

    class Code8AssetManager {
    public:
        static constexpr const char* CODE8_CATEGORIES[] = {
            "CAD",
            "Contact",
            "Dynamics",
            "Drahtbiegen",
            "Elements",
            "Kasten",
            "Linear",
            "NonLinear",
            "Pillow",
            "RVE",
            "Streifen",
            "Test",
            "Thermal"
        };

        static int LoadAllExamples(const std::string& code8Dir, PartLibrary& library);

        static int LoadCategory(
            const std::string& code8Dir,
            const std::string& category,
            PartLibrary& library
        );

        static std::string GetExampleDescription(
            const std::string& code8Dir,
            const std::string& exampleName
        );
    };

} // namespace SZM::Workshop3D
