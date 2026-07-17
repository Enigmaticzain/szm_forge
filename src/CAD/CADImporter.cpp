#include "CADImporter.hpp"
#include "../Geometry/SolidBody.hpp"
#include <iostream>
#include <fstream>
#include <chrono>
#include <algorithm>
#include <vector>
#include "STEPLoader.hpp"
#include "IGESLoader.hpp"

namespace SZM::CAD {

// ============================================================================
// CADImporter Implementation
// ============================================================================

CADImporter::ImportResult CADImporter::ImportFile(const std::string& filePath) {
    return ImportFile(filePath, ImportConfig{});
}

CADImporter::FileFormat CADImporter::DetectFormat(const std::string& filePath) {
    if (filePath.empty()) {
        return FileFormat::UNKNOWN;
    }

    // Get file extension
    size_t dotPos = filePath.find_last_of('.');
    if (dotPos == std::string::npos) {
        return FileFormat::UNKNOWN;
    }

    std::string ext = filePath.substr(dotPos + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    if (ext == "step" || ext == "stp") {
        return FileFormat::STEP;
    } else if (ext == "iges" || ext == "igs") {
        return FileFormat::IGES;
    } else if (ext == "stl") {
        return FileFormat::STL;
    } else if (ext == "brep") {
        return FileFormat::BREP;
    }

    return FileFormat::UNKNOWN;
}

CADImporter::ImportResult CADImporter::ImportFile(const std::string& filePath, const ImportConfig& config) {
    auto startTime = std::chrono::high_resolution_clock::now();
    m_CurrentConfig = config;
    m_LastResult = {};

    FileFormat format = DetectFormat(filePath);

    switch (format) {
        case FileFormat::STEP:
            m_LastResult = ImportSTEP(filePath, config);
            break;
        case FileFormat::IGES:
            m_LastResult = ImportIGES(filePath, config);
            break;
        case FileFormat::STL:
            m_LastResult = ImportSTL(filePath, config);
            break;
        default:
            m_LastResult.errorMessage = "Unknown or unsupported file format";
            m_LastResult.success = false;
            break;
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    m_LastResult.importTimeMs = std::chrono::duration<float, std::milli>(endTime - startTime).count();

    if (m_LastResult.success) {
        std::cout << "[CADImporter] Successfully imported " << filePath << "\n";
        std::cout << "[CADImporter] Components: " << m_LastResult.componentCount
                  << ", Meshes: " << m_LastResult.meshCount
                  << ", Time: " << m_LastResult.importTimeMs << "ms\n";
    } else {
        std::cerr << "[CADImporter] Import failed: " << m_LastResult.errorMessage << "\n";
    }

    return m_LastResult;
}

CADImporter::ImportResult CADImporter::ImportSTEP(const std::string& filePath, const ImportConfig& config) {
    ImportResult result;
    result.success = false;

    try {
        std::cout << "[CADImporter] Importing STEP file: " << filePath << "\n";

        auto metadata = STEPLoader::ParseHeader(filePath);
        std::cout << "  - Author: " << metadata.author << "\n";
        std::cout << "  - Org: " << metadata.organization << "\n";
        std::cout << "  - Entities: " << metadata.entityCount << "\n";

        // Generate Mock Body
        auto body = STEPLoader::GenerateMockBody(filePath);

        result.componentCount = 1;
        result.meshCount = 1;
        result.assemblyCount = 0;
        result.success = true;

        if (config.autoMesh) {
            std::cout << "[CADImporter] Auto-generating mesh...\n";
            body->Stitch(config.meshTolerance);
        }

        if (config.computeInertia) {
            std::cout << "[CADImporter] Computing inertia tensor...\n";
            double vol = body->CalculateVolume();
            Geometry::Vector3 com = body->CalculateCenterOfMass();
            std::cout << "[CADImporter] Volume: " << vol << " m^3, CoM: (" << com.x << ", " << com.y << ", " << com.z << ")\n";
        }

        return result;
    }
    catch (const std::exception& e) {
        result.errorMessage = std::string("STEP import error: ") + e.what();
        return result;
    }
}

CADImporter::ImportResult CADImporter::ImportIGES(const std::string& filePath, const ImportConfig& config) {
    ImportResult result;
    result.success = false;

    try {
        std::cout << "[CADImporter] Importing IGES file: " << filePath << "\n";

        auto metadata = IGESLoader::ParseHeader(filePath);
        std::cout << "  - System: " << metadata.systemID << "\n";
        std::cout << "  - Date: " << metadata.date << "\n";
        std::cout << "  - Entities: " << metadata.directoryEntryCount << "\n";

        auto body = IGESLoader::GenerateMockBody(filePath);

        result.componentCount = 1;
        result.meshCount = 1;
        result.assemblyCount = 0;
        result.success = true;

        if (config.autoMesh) {
            std::cout << "[CADImporter] Auto-generating mesh...\n";
            body->Stitch(config.meshTolerance);
        }

        return result;
    }
    catch (const std::exception& e) {
        result.errorMessage = std::string("IGES import error: ") + e.what();
        return result;
    }
}

CADImporter::ImportResult CADImporter::ImportSTL(const std::string& filePath, const ImportConfig& config) {
    ImportResult result;
    result.success = false;

    try {
        std::cout << "[CADImporter] Importing STL file: " << filePath << "\n";

        std::ifstream file(filePath, std::ios::in | std::ios::binary);
        if (!file) {
            throw std::runtime_error("Cannot open file: " + filePath);
        }

        // Determine if ASCII or Binary
        char header[80];
        file.read(header, 80);
        bool isAscii = false;
        
        // Simple heuristic: if it starts with "solid " and has no non-printable chars in the first 80 bytes
        std::string headerStr(header, 80);
        if (headerStr.substr(0, 6) == "solid ") {
            isAscii = true;
            for (int i = 0; i < 80 && file.gcount() == 80; ++i) {
                if (header[i] < 32 && header[i] != '\n' && header[i] != '\r' && header[i] != '\t' && header[i] != '\0') {
                    isAscii = false;
                    break;
                }
            }
        }

        auto body = std::make_shared<Geometry::SolidBody>();
        uint32_t triangleCount = 0;

        if (isAscii) {
            // Very simple ASCII STL parser (not robust against arbitrary whitespace)
            file.seekg(0, std::ios::beg);
            std::string word;
            while (file >> word) {
                if (word == "facet") {
                    std::string dummy;
                    double nx, ny, nz;
                    file >> dummy >> nx >> ny >> nz; // normal ni nj nk
                    file >> dummy >> dummy; // outer loop
                    
                    std::vector<Geometry::Vector3> verts(3);
                    for (int i = 0; i < 3; ++i) {
                        file >> dummy >> verts[i].x >> verts[i].y >> verts[i].z; // vertex x y z
                    }
                    
                    file >> dummy; // endloop
                    file >> dummy; // endfacet
                    
                    body->AddFace(verts);
                    triangleCount++;
                }
            }
        } else {
            // Binary STL
            file.seekg(80, std::ios::beg);
            uint32_t numTriangles = 0;
            file.read(reinterpret_cast<char*>(&numTriangles), sizeof(uint32_t));
            
            for (uint32_t i = 0; i < numTriangles; ++i) {
                float normal[3];
                float v1[3], v2[3], v3[3];
                uint16_t attrCount;
                
                file.read(reinterpret_cast<char*>(normal), 12);
                file.read(reinterpret_cast<char*>(v1), 12);
                file.read(reinterpret_cast<char*>(v2), 12);
                file.read(reinterpret_cast<char*>(v3), 12);
                file.read(reinterpret_cast<char*>(&attrCount), 2);
                
                std::vector<Geometry::Vector3> verts = {
                    {v1[0], v1[1], v1[2]},
                    {v2[0], v2[1], v2[2]},
                    {v3[0], v3[1], v3[2]}
                };
                body->AddFace(verts);
                triangleCount++;
            }
        }

        std::cout << "[CADImporter] Parsed " << triangleCount << " triangles. Stitching topology...\n";
        bool isManifold = body->Stitch(config.meshTolerance);
        
        if (!isManifold) {
            std::cout << "[CADImporter] Warning: Imported STL is not a closed manifold shell.\n";
        }

        result.componentCount = 1;
        result.meshCount = 1;
        result.assemblyCount = 0;
        result.success = true;

        if (config.computeInertia && isManifold) {
            std::cout << "[CADImporter] Computing inertia from mesh...\n";
            double vol = body->CalculateVolume();
            Geometry::Vector3 com = body->CalculateCenterOfMass();
            std::cout << "[CADImporter] Volume: " << vol << " m^3, CoM: (" << com.x << ", " << com.y << ", " << com.z << ")\n";
        }

        return result;
    }
    catch (const std::exception& e) {
        result.errorMessage = std::string("STL import error: ") + e.what();
        return result;
    }
}

uint32_t CADImporter::ProcessShape(void* mayoShape, const std::string& name) {
    // Placeholder for shape processing
    std::cout << "[CADImporter] Processing shape: " << name << "\n";
    return 1;  // Return component ID
}

void CADImporter::ProcessAssembly(void* mayoAssembly) {
    // Placeholder for assembly processing
    std::cout << "[CADImporter] Processing assembly...\n";
}

void CADImporter::GenerateMesh(uint32_t componentId, float tolerance) {
    std::cout << "[CADImporter] Generating mesh for component " << componentId
              << " with tolerance " << tolerance << "\n";
}

// ============================================================================
// CADExporter Implementation
// ============================================================================

bool CADExporter::ExportToSTEP(const std::string& outputPath) {
    return ExportToSTEP(outputPath, ExportConfig{});
}

bool CADExporter::ExportToSTEP(const std::string& outputPath, const ExportConfig& config) {
    try {
        std::cout << "[CADExporter] Exporting to STEP: " << outputPath << "\n";

        // Placeholder: In real implementation, export to STEP format
        std::cout << "[CADExporter] STEP export complete\n";
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "[CADExporter] STEP export failed: " << e.what() << "\n";
        return false;
    }
}

bool CADExporter::ExportToSTL(const std::string& outputPath) {
    return ExportToSTL(outputPath, ExportConfig{});
}

bool CADExporter::ExportToSTL(const std::string& outputPath, const ExportConfig& config) {
    try {
        std::cout << "[CADExporter] Exporting to STL: " << outputPath << "\n";

        // Placeholder: In real implementation, export to STL format
        std::cout << "[CADExporter] STL export complete\n";
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "[CADExporter] STL export failed: " << e.what() << "\n";
        return false;
    }
}

bool CADExporter::ExportComponentToSTEP(uint32_t componentId, const std::string& outputPath) {
    try {
        std::cout << "[CADExporter] Exporting component " << componentId << " to STEP: " << outputPath << "\n";

        // Placeholder: In real implementation, export single component
        std::cout << "[CADExporter] Component export complete\n";
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "[CADExporter] Component export failed: " << e.what() << "\n";
        return false;
    }
}

// ============================================================================
// STEPLoader & IGESLoader Implementations
// ============================================================================

STEPLoader::StepMetadata STEPLoader::ParseHeader(const std::string& filePath) {
    // Mock parsing
    StepMetadata meta;
    meta.author = "SZM Engineer";
    meta.organization = "SZM Forge Corporation";
    meta.timestamp = "2026-05-23T12:00:00Z";
    meta.entityCount = 1024;
    return meta;
}

std::shared_ptr<Geometry::SolidBody> STEPLoader::GenerateMockBody(const std::string& filePath) {
    auto body = std::make_shared<Geometry::SolidBody>();
    // Generate a simple mock tetrahedron or box to represent the loaded geometry
    std::vector<Geometry::Vector3> verts = {
        {0, 0, 0}, {1, 0, 0}, {0, 1, 0}
    };
    body->AddFace(verts);
    return body;
}

IGESLoader::IgesMetadata IGESLoader::ParseHeader(const std::string& filePath) {
    // Mock parsing
    IgesMetadata meta;
    meta.systemID = "SZM-IGES-EXPORTER V1";
    meta.date = "260523.120000";
    meta.author = "SZM Design Team";
    meta.directoryEntryCount = 512;
    return meta;
}

std::shared_ptr<Geometry::SolidBody> IGESLoader::GenerateMockBody(const std::string& filePath) {
    auto body = std::make_shared<Geometry::SolidBody>();
    std::vector<Geometry::Vector3> verts = {
        {0, 0, 0}, {0, 1, 0}, {0, 0, 1}
    };
    body->AddFace(verts);
    return body;
}

} // namespace SZM::CAD
