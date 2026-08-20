#pragma once

#include <string>
#include <vector>
#include <memory>
#include <cstdint>

namespace SZM::CAD {

    /**
     * @brief Imports CAD files (STEP, IGES, STL) into SZM scene
     * Integrates with Mayo CAD kernel
     */
    class CADImporter {
    public:
        enum class FileFormat {
            STEP,
            IGES,
            STL,
            BREP,
            UNKNOWN
        };

        struct ImportConfig {
            bool preserveHierarchy = true;
            bool autoMesh = true;
            bool computeInertia = true;
            float meshTolerance = 0.01f;
            bool importMaterials = true;
        };

        struct ImportResult {
            bool success = false;
            uint32_t componentCount = 0;
            uint32_t meshCount = 0;
            uint32_t assemblyCount = 0;
            std::string errorMessage;
            float importTimeMs = 0.0f;
        };

        CADImporter() = default;
        ~CADImporter() = default;

        /**
         * @brief Detect file format from extension
         */
        static FileFormat DetectFormat(const std::string& filePath);

        /**
         * @brief Import CAD file into scene
         * @param filePath Path to CAD file
         * @param config Import configuration
         * @return Import result with statistics
         */
        ImportResult ImportFile(const std::string& filePath, const ImportConfig& config);
        ImportResult ImportFile(const std::string& filePath);

        /**
         * @brief Import STEP file
         */
        ImportResult ImportSTEP(const std::string& filePath, const ImportConfig& config);

        /**
         * @brief Import via FreeCAD headless bridge — builds a parametric part
         *        and imports the resulting STEP into the scene.
         * @param shape  "box" | "cylinder" | "sphere"
         * @param length/width/height  metres
         * @param filletRadius  0 = no fillet
         */
        struct FreeCADParams {
            std::string shape        = "box";
            float length             = 0.1f;
            float width              = 0.05f;
            float height             = 0.02f;
            float filletRadius       = 0.0f;
            std::string outputPath   = "/tmp/szm_freecad_part.step";
        };
        ImportResult ImportFromFreeCAD(const FreeCADParams& params);

        /**
         * @brief Import IGES file
         */
        ImportResult ImportIGES(const std::string& filePath, const ImportConfig& config);

        /**
         * @brief Import STL file
         */
        ImportResult ImportSTL(const std::string& filePath, const ImportConfig& config);

        /**
         * @brief Get last import statistics
         */
        ImportResult GetLastImportResult() const { return m_LastResult; }

    private:
        ImportResult m_LastResult;
        ImportConfig m_CurrentConfig;

        uint32_t ProcessShape(void* mayoShape, const std::string& name);
        void ProcessAssembly(void* mayoAssembly);
        void GenerateMesh(uint32_t componentId, float tolerance);
    };

    /**
     * @brief Exports SZM components to CAD formats
     */
    class CADExporter {
    public:
        struct ExportConfig {
            bool includeAssembly = true;
            bool includeMaterials = true;
            float meshTolerance = 0.01f;
        };

        /**
         * @brief Export scene to STEP file
         */
        bool ExportToSTEP(const std::string& outputPath, const ExportConfig& config);
        bool ExportToSTEP(const std::string& outputPath);

        /**
         * @brief Export scene to STL file
         */
        bool ExportToSTL(const std::string& outputPath, const ExportConfig& config);
        bool ExportToSTL(const std::string& outputPath);

        /**
         * @brief Export single component to STEP
         */
        bool ExportComponentToSTEP(uint32_t componentId, const std::string& outputPath);
    };

} // namespace SZM::CAD
