#include "CADImporter.hpp"
#include "FEA/CalculiXPipeline.hpp"
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <unordered_map>

namespace fs = std::filesystem;

namespace {

using SZM::Geometry::Vector3;
using SZM::Workshop3D::Part;

std::string ToLowerCopy(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return value;
}

std::string TrimCopy(const std::string& value) {
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) {
        return "";
    }
    const auto last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1U);
}

bool HasExtension(const fs::path& path, const std::initializer_list<const char*>& extensions) {
    const std::string ext = ToLowerCopy(path.extension().string());
    for (const char* candidate : extensions) {
        if (ext == candidate) {
            return true;
        }
    }
    return false;
}

float MaterialDensityFor(const std::string& materialName) {
    const std::string material = ToLowerCopy(materialName);
    if (material.find("steel") != std::string::npos) return 7850.0f;
    if (material.find("aluminum") != std::string::npos || material.find("aluminium") != std::string::npos) return 2700.0f;
    if (material.find("titanium") != std::string::npos) return 4500.0f;
    if (material.find("copper") != std::string::npos) return 8960.0f;
    if (material.find("poly") != std::string::npos || material.find("plastic") != std::string::npos) return 1180.0f;
    return 1600.0f;
}

uint32_t CategoryColorFor(const std::string& category) {
    const std::string normalized = ToLowerCopy(category);
    if (normalized == "thermal") return 0xFF5858FFU;
    if (normalized == "contact") return 0xFF55A7FFU;
    if (normalized == "dynamics") return 0xFF5BC6FFU;
    if (normalized == "nonlinear") return 0xFF58C58CU;
    if (normalized == "linear") return 0xFF6AA8D8U;
    if (normalized == "elements") return 0xFFE0B15AU;
    if (normalized == "cad") return 0xFFB477FFU;
    if (normalized == "rve") return 0xFF7ED37EU;
    return 0xFFB8B8C8U;
}

Vector3 FallbackHalfExtentsFor(const std::string& seedText, const std::string& category) {
    const std::size_t seed = std::hash<std::string>{}(seedText + "::" + category);
    const double width = 0.25 + static_cast<double>((seed >> 0U) & 0x0FU) * 0.035;
    const double height = 0.18 + static_cast<double>((seed >> 4U) & 0x0FU) * 0.028;
    const double depth = 0.12 + static_cast<double>((seed >> 8U) & 0x0FU) * 0.022;
    return Vector3(width, height, depth);
}

void CreateBoxMesh(
    std::shared_ptr<Part> part,
    const Vector3& halfExtents
) {
    if (!part) {
        return;
    }

    auto& vertices = part->GetVertices();
    auto& indices = part->GetIndices();
    vertices = {
        {-halfExtents.x, -halfExtents.y, -halfExtents.z},
        { halfExtents.x, -halfExtents.y, -halfExtents.z},
        { halfExtents.x,  halfExtents.y, -halfExtents.z},
        {-halfExtents.x,  halfExtents.y, -halfExtents.z},
        {-halfExtents.x, -halfExtents.y,  halfExtents.z},
        { halfExtents.x, -halfExtents.y,  halfExtents.z},
        { halfExtents.x,  halfExtents.y,  halfExtents.z},
        {-halfExtents.x,  halfExtents.y,  halfExtents.z}
    };

    indices = {
        0, 1, 2, 0, 2, 3,
        1, 5, 6, 1, 6, 2,
        5, 4, 7, 5, 7, 6,
        4, 0, 3, 4, 3, 7,
        3, 2, 6, 3, 6, 7,
        4, 5, 1, 4, 1, 0
    };

    part->SetBounds(
        Vector3(-halfExtents.x, -halfExtents.y, -halfExtents.z),
        Vector3(halfExtents.x, halfExtents.y, halfExtents.z)
    );
}

void ComputeNormals(
    const std::vector<Vector3>& vertices,
    const std::vector<uint32_t>& indices,
    std::vector<Vector3>& normals
) {
    normals.assign(vertices.size(), Vector3(0.0, 0.0, 0.0));
    if (indices.size() < 3U) {
        return;
    }

    for (size_t i = 0; i + 2U < indices.size(); i += 3U) {
        const auto i0 = indices[i];
        const auto i1 = indices[i + 1U];
        const auto i2 = indices[i + 2U];
        if (i0 >= vertices.size() || i1 >= vertices.size() || i2 >= vertices.size()) {
            continue;
        }

        const Vector3 normal = (vertices[i1] - vertices[i0]).Cross(vertices[i2] - vertices[i0]).Normalize();
        normals[i0] = normals[i0] + normal;
        normals[i1] = normals[i1] + normal;
        normals[i2] = normals[i2] + normal;
    }

    for (auto& normal : normals) {
        normal = normal.Normalize();
    }
}

bool ParseFaceIndex(const std::string& token, int vertexCount, uint32_t& index) {
    const std::string value = token.substr(0, token.find('/'));
    if (value.empty()) {
        return false;
    }

    const int parsed = std::stoi(value);
    if (parsed == 0) {
        return false;
    }

    if (parsed < 0) {
        index = static_cast<uint32_t>(vertexCount + parsed);
    } else {
        index = static_cast<uint32_t>(parsed - 1);
    }
    return true;
}

std::string FindReadmePath(const fs::path& directory) {
    for (const auto& filename : { "README.md", "Readme.md", "README.txt", "Readme.txt" }) {
        const fs::path candidate = directory / filename;
        if (fs::exists(candidate)) {
            return candidate.string();
        }
    }
    return "";
}

std::string ReadSummaryFromReadme(const fs::path& directory) {
    const std::string readmePath = FindReadmePath(directory);
    if (readmePath.empty()) {
        return "";
    }

    std::ifstream input(readmePath);
    if (!input.is_open()) {
        return "";
    }

    std::string line;
    bool sawHeading = false;
    while (std::getline(input, line)) {
        const std::string trimmed = TrimCopy(line);
        if (trimmed.empty()) {
            continue;
        }

        if (!sawHeading && trimmed[0] == '#') {
            sawHeading = true;
            continue;
        }

        if (trimmed[0] == '#' || trimmed[0] == '[' || trimmed[0] == '!' || trimmed[0] == '|') {
            continue;
        }

        return trimmed;
    }

    return "";
}

std::string DetectMaterial(const fs::path& directory) {
    for (const auto& entry : fs::directory_iterator(directory)) {
        if (!entry.is_regular_file() || !HasExtension(entry.path(), { ".inp" })) {
            continue;
        }

        std::ifstream input(entry.path());
        if (!input.is_open()) {
            continue;
        }

        std::string line;
        while (std::getline(input, line)) {
            const std::string normalized = ToLowerCopy(line);
            const auto marker = normalized.find("*material");
            const auto nameMarker = normalized.find("name=");
            if (marker != std::string::npos && nameMarker != std::string::npos) {
                const std::string suffix = line.substr(nameMarker + 5U);
                return TrimCopy(suffix);
            }
        }
    }

    const std::string readme = ToLowerCopy(ReadSummaryFromReadme(directory));
    if (readme.find("steel") != std::string::npos) return "Steel";
    if (readme.find("aluminum") != std::string::npos || readme.find("aluminium") != std::string::npos) return "Aluminum";
    if (readme.find("titanium") != std::string::npos) return "Titanium";
    if (readme.find("copper") != std::string::npos) return "Copper";
    return "Generic";
}

bool TryReadInpBounds(const fs::path& directory, Vector3& outHalfExtents) {
    Vector3 minBounds(0.0, 0.0, 0.0);
    Vector3 maxBounds(0.0, 0.0, 0.0);
    bool hasPoint = false;

    for (const auto& entry : fs::directory_iterator(directory)) {
        if (!entry.is_regular_file() || !HasExtension(entry.path(), { ".inp" })) {
            continue;
        }

        std::ifstream input(entry.path());
        if (!input.is_open()) {
            continue;
        }

        bool inNodeBlock = false;
        std::string line;
        while (std::getline(input, line)) {
            const std::string trimmed = TrimCopy(line);
            if (trimmed.empty()) {
                continue;
            }

            if (!trimmed.empty() && trimmed[0] == '*') {
                inNodeBlock = ToLowerCopy(trimmed).rfind("*node", 0U) == 0U;
                continue;
            }

            if (!inNodeBlock) {
                continue;
            }

            std::stringstream parser(trimmed);
            std::string token;
            std::vector<std::string> fields;
            while (std::getline(parser, token, ',')) {
                fields.push_back(TrimCopy(token));
            }

            if (fields.size() < 4U) {
                continue;
            }

            const Vector3 point(
                std::stod(fields[1]),
                std::stod(fields[2]),
                std::stod(fields[3])
            );

            if (!hasPoint) {
                minBounds = point;
                maxBounds = point;
                hasPoint = true;
            } else {
                minBounds.x = std::min(minBounds.x, point.x);
                minBounds.y = std::min(minBounds.y, point.y);
                minBounds.z = std::min(minBounds.z, point.z);

                maxBounds.x = std::max(maxBounds.x, point.x);
                maxBounds.y = std::max(maxBounds.y, point.y);
                maxBounds.z = std::max(maxBounds.z, point.z);
            }
        }
    }

    if (!hasPoint) {
        return false;
    }

    Vector3 extents = (maxBounds - minBounds) * 0.5;
    const double largest = std::max({ std::abs(extents.x), std::abs(extents.y), std::abs(extents.z) });
    if (largest > 10.0) {
        extents = extents * 0.001;
    }

    if (extents.x <= 0.0 || extents.y <= 0.0 || extents.z <= 0.0) {
        return false;
    }

    outHalfExtents = extents;
    return true;
}

std::string ChoosePreviewImage(const fs::path& directory) {
    fs::path bestPath;
    int bestScore = -1;

    for (const auto& entry : fs::recursive_directory_iterator(directory)) {
        if (!entry.is_regular_file() || !HasExtension(entry.path(), { ".png", ".jpg", ".jpeg", ".gif" })) {
            continue;
        }

        const std::string filename = ToLowerCopy(entry.path().filename().string());
        int score = 1;
        if (filename.find("preview") != std::string::npos) score += 6;
        if (filename.find("contact") != std::string::npos) score += 5;
        if (filename.find("mesh") != std::string::npos) score += 4;
        if (filename.find("def") != std::string::npos || filename.find("deformed") != std::string::npos) score += 4;
        if (filename.find("exp") != std::string::npos || filename.find("se") != std::string::npos) score += 3;
        if (entry.path().parent_path().filename() == "Refs") score += 3;

        if (score > bestScore || (score == bestScore && entry.path().string() < bestPath.string())) {
            bestScore = score;
            bestPath = entry.path();
        }
    }

    return bestPath.empty() ? "" : bestPath.string();
}

std::vector<std::string> CollectArtifacts(const fs::path& directory) {
    std::vector<std::string> artifacts;
    for (const auto& entry : fs::directory_iterator(directory)) {
        if (!entry.is_regular_file()) {
            continue;
        }

        if (HasExtension(entry.path(), { ".inp", ".fbd", ".sm", ".png", ".gif", ".pdf", ".py", ".md" })) {
            artifacts.push_back(entry.path().string());
        }
    }

    std::sort(artifacts.begin(), artifacts.end());
    if (artifacts.size() > 12U) {
        artifacts.resize(12U);
    }
    return artifacts;
}

bool DirectoryHasCoreFiles(const fs::path& directory) {
    for (const auto& entry : fs::directory_iterator(directory)) {
        if (entry.is_regular_file() &&
            HasExtension(entry.path(), { ".inp", ".fbd", ".sm", ".geo", ".step", ".stp", ".stl", ".obj" })) {
            return true;
        }
    }
    return false;
}

fs::path NormalizeExampleDirectory(const fs::path& path) {
    if (path.filename() == "Refs") {
        return path.parent_path();
    }
    return path;
}

std::shared_ptr<Part> BuildCode8Part(
    const fs::path& code8Root,
    const fs::path& exampleDirectory
) {
    std::error_code error;
    if (!fs::exists(exampleDirectory, error) || !fs::is_directory(exampleDirectory, error)) {
        return nullptr;
    }

    const fs::path relativePath = fs::relative(exampleDirectory, code8Root, error);
    if (error || relativePath.empty()) {
        return nullptr;
    }

    const std::string category = relativePath.begin() != relativePath.end()
        ? relativePath.begin()->string()
        : std::string("Code8");

    const std::string displayName = relativePath.generic_string();
    auto part = std::make_shared<Part>(displayName);

    Part::Properties props = part->GetProperties();
    props.name = displayName;
    props.category = category;
    props.analysisType = category;
    props.description = ReadSummaryFromReadme(exampleDirectory);
    props.material = DetectMaterial(exampleDirectory);
    props.density = MaterialDensityFor(props.material);
    props.color = CategoryColorFor(category);
    props.sourcePath = exampleDirectory.string();
    props.previewPath = ChoosePreviewImage(exampleDirectory);
    props.referencePath = FindReadmePath(exampleDirectory);
    props.isAssembly = ToLowerCopy(props.description).find("assembly") != std::string::npos;
    part->SetProperties(props);

    Vector3 halfExtents;
    if (!TryReadInpBounds(exampleDirectory, halfExtents)) {
        halfExtents = FallbackHalfExtentsFor(displayName, category);
    }
    CreateBoxMesh(part, halfExtents);

    ComputeNormals(part->GetVertices(), part->GetIndices(), part->GetNormals());
    part->CalculateMass(props.density);

    for (const auto& artifact : CollectArtifacts(exampleDirectory)) {
        part->AddArtifactPath(artifact);
    }

    return part;
}

} // namespace

namespace SZM::Workshop3D {

    std::shared_ptr<Part> CADImporter::ImportSTEP(const std::string& filepath) {
        return ImportSTEP(filepath, ImportOptions{});
    }

    std::shared_ptr<Part> CADImporter::ImportSTEP(
        const std::string& filepath,
        const ImportOptions& options
    ) {
        if (!fs::exists(filepath)) {
            return nullptr;
        }

        auto part = std::make_shared<Part>(fs::path(filepath).stem().string());
        Part::Properties props = part->GetProperties();
        props.sourcePath = filepath;
        props.referencePath = filepath;
        props.material = options.materialName;
        props.description = "STEP placeholder import. Native tessellation is not available in this build, so SZM Forge created a proxy volume for workshop layout and simulation setup.";
        props.density = MaterialDensityFor(props.material);
        props.color = 0xFFC7C7D2U;
        part->SetProperties(props);

        const Vector3 halfExtents = FallbackHalfExtentsFor(filepath, "step") * options.scale;
        CreateBoxMesh(part, halfExtents);
        CalculateNormals(part->GetVertices(), part->GetIndices(), part->GetNormals());
        part->CalculateMass(props.density);
        return part;
    }

    std::shared_ptr<Part> CADImporter::ImportGEO(const std::string& filepath) {
        return ImportGEO(filepath, ImportOptions{});
    }

    std::shared_ptr<Part> CADImporter::ImportGEO(
        const std::string& filepath,
        const ImportOptions& options
    ) {
        if (!fs::exists(filepath)) {
            return nullptr;
        }

        std::vector<Vector3> vertices;
        std::vector<uint32_t> indices;
        LoadGEOFile(filepath, vertices, indices);

        auto part = std::make_shared<Part>(fs::path(filepath).stem().string());
        Part::Properties props = part->GetProperties();
        props.sourcePath = filepath;
        props.referencePath = filepath;
        props.material = options.materialName;
        props.description = "GEO import with lightweight point parsing.";
        props.density = MaterialDensityFor(props.material);
        props.color = 0xFFC0D6FFU;
        part->SetProperties(props);

        if (vertices.size() >= 3U) {
            part->GetVertices() = vertices;
            part->GetIndices() = indices;
            part->CalculateBounds();
        } else {
            CreateBoxMesh(part, FallbackHalfExtentsFor(filepath, "geo") * options.scale);
        }

        if (options.mergeVertices) {
            MergeVertices(part->GetVertices(), part->GetIndices(), options.mergeThreshold);
        }
        if (options.calculateNormals) {
            CalculateNormals(part->GetVertices(), part->GetIndices(), part->GetNormals());
        }

        part->CalculateMass(props.density);
        return part;
    }

    std::shared_ptr<Part> CADImporter::ImportSTL(const std::string& filepath) {
        return ImportSTL(filepath, ImportOptions{});
    }

    std::shared_ptr<Part> CADImporter::ImportSTL(
        const std::string& filepath,
        const ImportOptions& options
    ) {
        if (!fs::exists(filepath)) {
            return nullptr;
        }

        std::vector<Vector3> vertices;
        if (!LoadSTLBinary(filepath, vertices) && !LoadSTLASCII(filepath, vertices)) {
            std::cerr << "[CADImporter] Failed to load STL: " << filepath << '\n';
            return nullptr;
        }

        auto part = std::make_shared<Part>(fs::path(filepath).stem().string());
        part->GetVertices() = vertices;

        auto& indices = part->GetIndices();
        indices.resize(vertices.size());
        for (uint32_t index = 0; index < indices.size(); ++index) {
            indices[index] = index;
        }

        if (options.mergeVertices) {
            MergeVertices(part->GetVertices(), part->GetIndices(), options.mergeThreshold);
        }
        if (options.calculateNormals) {
            CalculateNormals(part->GetVertices(), part->GetIndices(), part->GetNormals());
        }

        Part::Properties props = part->GetProperties();
        props.sourcePath = filepath;
        props.referencePath = filepath;
        props.material = options.materialName;
        props.density = MaterialDensityFor(props.material);
        props.color = 0xFFD9D9D9U;
        part->SetProperties(props);
        part->CalculateBounds();
        part->CalculateMass(props.density);
        return part;
    }

    std::shared_ptr<Part> CADImporter::ImportOBJ(const std::string& filepath) {
        return ImportOBJ(filepath, ImportOptions{});
    }

    std::shared_ptr<Part> CADImporter::ImportOBJ(
        const std::string& filepath,
        const ImportOptions& options
    ) {
        if (!fs::exists(filepath)) {
            return nullptr;
        }

        std::vector<Vector3> vertices;
        std::vector<uint32_t> indices;
        if (!LoadOBJFile(filepath, vertices, indices)) {
            std::cerr << "[CADImporter] Failed to load OBJ: " << filepath << '\n';
            return nullptr;
        }

        auto part = std::make_shared<Part>(fs::path(filepath).stem().string());
        part->GetVertices() = vertices;
        part->GetIndices() = indices;

        if (options.mergeVertices) {
            MergeVertices(part->GetVertices(), part->GetIndices(), options.mergeThreshold);
        }
        if (options.calculateNormals) {
            CalculateNormals(part->GetVertices(), part->GetIndices(), part->GetNormals());
        }

        Part::Properties props = part->GetProperties();
        props.sourcePath = filepath;
        props.referencePath = filepath;
        props.material = options.materialName;
        props.density = MaterialDensityFor(props.material);
        props.color = 0xFFE7D4BEU;
        part->SetProperties(props);
        part->CalculateBounds();
        part->CalculateMass(props.density);
        return part;
    }

    std::shared_ptr<Part> CADImporter::ImportAuto(const std::string& filepath) {
        return ImportAuto(filepath, ImportOptions{});
    }

    std::shared_ptr<Part> CADImporter::ImportAuto(
        const std::string& filepath,
        const ImportOptions& options
    ) {
        const std::string extension = ToLowerCopy(fs::path(filepath).extension().string());
        if (extension == ".step" || extension == ".stp") return ImportSTEP(filepath, options);
        if (extension == ".geo") return ImportGEO(filepath, options);
        if (extension == ".stl") return ImportSTL(filepath, options);
        if (extension == ".obj") return ImportOBJ(filepath, options);

        std::cerr << "[CADImporter] Unsupported format: " << filepath << '\n';
        return nullptr;
    }

    int CADImporter::ImportFromCode8(
        const std::string& code8Dir,
        PartLibrary& library,
        bool recursive
    ) {
        (void)recursive;
        return Code8AssetManager::LoadAllExamples(code8Dir, library);
    }

    bool CADImporter::LoadSTLBinary(const std::string& filepath, std::vector<Vector3>& vertices) {
        std::ifstream input(filepath, std::ios::binary);
        if (!input.is_open()) {
            return false;
        }

        input.seekg(0, std::ios::end);
        const std::streamsize fileSize = input.tellg();
        if (fileSize < 84) {
            return false;
        }

        input.seekg(80, std::ios::beg);
        uint32_t triangleCount = 0;
        input.read(reinterpret_cast<char*>(&triangleCount), sizeof(uint32_t));
        const std::streamsize expectedSize = 84 + static_cast<std::streamsize>(triangleCount) * 50;
        if (expectedSize != fileSize) {
            return false;
        }

        vertices.clear();
        vertices.reserve(static_cast<size_t>(triangleCount) * 3U);
        for (uint32_t triangle = 0; triangle < triangleCount; ++triangle) {
            input.seekg(12, std::ios::cur);
            for (int vertexIndex = 0; vertexIndex < 3; ++vertexIndex) {
                float coords[3];
                input.read(reinterpret_cast<char*>(coords), sizeof(coords));
                vertices.emplace_back(coords[0], coords[1], coords[2]);
            }
            input.seekg(2, std::ios::cur);
        }

        return !vertices.empty();
    }

    bool CADImporter::LoadSTLASCII(const std::string& filepath, std::vector<Vector3>& vertices) {
        std::ifstream input(filepath);
        if (!input.is_open()) {
            return false;
        }

        vertices.clear();
        std::string line;
        while (std::getline(input, line)) {
            const std::string trimmed = TrimCopy(line);
            if (trimmed.rfind("vertex", 0U) != 0U) {
                continue;
            }

            std::stringstream parser(trimmed);
            std::string label;
            double x = 0.0;
            double y = 0.0;
            double z = 0.0;
            parser >> label >> x >> y >> z;
            vertices.emplace_back(x, y, z);
        }

        return !vertices.empty();
    }

    bool CADImporter::LoadOBJFile(
        const std::string& filepath,
        std::vector<Vector3>& vertices,
        std::vector<uint32_t>& indices
    ) {
        std::ifstream input(filepath);
        if (!input.is_open()) {
            return false;
        }

        vertices.clear();
        indices.clear();

        std::string line;
        while (std::getline(input, line)) {
            const std::string trimmed = TrimCopy(line);
            if (trimmed.empty() || trimmed[0] == '#') {
                continue;
            }

            std::stringstream parser(trimmed);
            std::string keyword;
            parser >> keyword;

            if (keyword == "v") {
                double x = 0.0;
                double y = 0.0;
                double z = 0.0;
                parser >> x >> y >> z;
                vertices.emplace_back(x, y, z);
                continue;
            }

            if (keyword == "f") {
                std::vector<uint32_t> faceIndices;
                std::string token;
                while (parser >> token) {
                    uint32_t index = 0U;
                    if (ParseFaceIndex(token, static_cast<int>(vertices.size()), index)) {
                        faceIndices.push_back(index);
                    }
                }

                if (faceIndices.size() < 3U) {
                    continue;
                }

                for (size_t i = 1; i + 1U < faceIndices.size(); ++i) {
                    indices.push_back(faceIndices[0]);
                    indices.push_back(faceIndices[i]);
                    indices.push_back(faceIndices[i + 1U]);
                }
            }
        }

        return !vertices.empty() && !indices.empty();
    }

    bool CADImporter::LoadGEOFile(
        const std::string& filepath,
        std::vector<Vector3>& vertices,
        std::vector<uint32_t>& indices
    ) {
        std::ifstream input(filepath);
        if (!input.is_open()) {
            return false;
        }

        vertices.clear();
        indices.clear();

        std::string line;
        while (std::getline(input, line)) {
            const std::string trimmed = TrimCopy(line);
            if (trimmed.rfind("Point(", 0U) != 0U) {
                continue;
            }

            const auto openBrace = trimmed.find('{');
            const auto closeBrace = trimmed.find('}');
            if (openBrace == std::string::npos || closeBrace == std::string::npos || closeBrace <= openBrace + 1U) {
                continue;
            }

            std::stringstream parser(trimmed.substr(openBrace + 1U, closeBrace - openBrace - 1U));
            std::string sx;
            std::string sy;
            std::string sz;
            std::getline(parser, sx, ',');
            std::getline(parser, sy, ',');
            std::getline(parser, sz, ',');
            if (sx.empty() || sy.empty() || sz.empty()) {
                continue;
            }

            vertices.emplace_back(std::stod(TrimCopy(sx)), std::stod(TrimCopy(sy)), std::stod(TrimCopy(sz)));
        }

        if (vertices.size() >= 3U) {
            for (uint32_t i = 1U; i + 1U < vertices.size(); ++i) {
                indices.push_back(0U);
                indices.push_back(i);
                indices.push_back(i + 1U);
            }
        }

        return !vertices.empty();
    }

    void CADImporter::MergeVertices(
        std::vector<Vector3>& vertices,
        std::vector<uint32_t>& indices,
        float threshold
    ) {
        if (vertices.empty() || indices.empty()) {
            return;
        }

        std::vector<Vector3> uniqueVertices;
        std::vector<uint32_t> remap(vertices.size(), 0U);

        for (size_t i = 0; i < vertices.size(); ++i) {
            bool merged = false;
            for (size_t j = 0; j < uniqueVertices.size(); ++j) {
                const Vector3 delta = vertices[i] - uniqueVertices[j];
                if (delta.MagnitudeSq() <= static_cast<double>(threshold) * static_cast<double>(threshold)) {
                    remap[i] = static_cast<uint32_t>(j);
                    merged = true;
                    break;
                }
            }

            if (!merged) {
                remap[i] = static_cast<uint32_t>(uniqueVertices.size());
                uniqueVertices.push_back(vertices[i]);
            }
        }

        for (uint32_t& index : indices) {
            if (index < remap.size()) {
                index = remap[index];
            }
        }
        vertices = std::move(uniqueVertices);
    }

    void CADImporter::CalculateNormals(
        const std::vector<Vector3>& vertices,
        const std::vector<uint32_t>& indices,
        std::vector<Vector3>& normals
    ) {
        ComputeNormals(vertices, indices, normals);
    }

    int Code8AssetManager::LoadAllExamples(const std::string& code8Dir, PartLibrary& library) {
        const fs::path root(code8Dir);
        if (!fs::exists(root)) {
            return 0;
        }

        const auto catalog = SZM::FEA::CalculiXPipeline::CatalogExamples(root);
        if (!catalog.empty()) {
            int imported = 0;
            for (const auto& example : catalog) {
                auto part = BuildCode8Part(root, example.Directory);
                if (!part) {
                    continue;
                }

                if (library.GetPartByName(part->GetProperties().name)) {
                    continue;
                }

                library.RegisterPart(part);
                ++imported;
            }

            return imported;
        }

        std::set<std::string> discovered;
        for (const auto& entry : fs::directory_iterator(root)) {
            if (entry.is_directory()) {
                discovered.insert(entry.path().filename().string());
            }
        }

        int imported = 0;
        for (const char* category : CODE8_CATEGORIES) {
            if (discovered.erase(category) > 0U) {
                imported += LoadCategory(code8Dir, category, library);
            }
        }

        for (const auto& category : discovered) {
            imported += LoadCategory(code8Dir, category, library);
        }

        return imported;
    }

    int Code8AssetManager::LoadCategory(
        const std::string& code8Dir,
        const std::string& category,
        PartLibrary& library
    ) {
        const fs::path categoryPath = fs::path(code8Dir) / category;
        if (!fs::exists(categoryPath) || !fs::is_directory(categoryPath)) {
            return 0;
        }

        std::set<fs::path> exampleDirectories;

        if (DirectoryHasCoreFiles(categoryPath)) {
            exampleDirectories.insert(categoryPath);
        }

        for (const auto& entry : fs::recursive_directory_iterator(categoryPath)) {
            if (!entry.is_regular_file()) {
                continue;
            }

            const fs::path parent = NormalizeExampleDirectory(entry.path().parent_path());
            if (HasExtension(entry.path(), { ".inp", ".fbd", ".sm", ".geo", ".step", ".stp", ".stl", ".obj" })) {
                exampleDirectories.insert(parent);
                continue;
            }

            const std::string filename = ToLowerCopy(entry.path().filename().string());
            if ((filename == "readme.md" || filename == "readme.txt") &&
                (DirectoryHasCoreFiles(parent) || !ChoosePreviewImage(parent).empty())) {
                exampleDirectories.insert(parent);
            }
        }

        int imported = 0;
        for (const auto& directory : exampleDirectories) {
            auto part = BuildCode8Part(fs::path(code8Dir), directory);
            if (!part) {
                continue;
            }

            if (library.GetPartByName(part->GetProperties().name)) {
                continue;
            }

            library.RegisterPart(part);
            ++imported;
        }

        return imported;
    }

    std::string Code8AssetManager::GetExampleDescription(
        const std::string& code8Dir,
        const std::string& exampleName
    ) {
        const fs::path root(code8Dir);
        if (const auto example = SZM::FEA::CalculiXPipeline::FindExample(root, exampleName)) {
            return example->Summary;
        }

        const fs::path examplePath = root / fs::path(exampleName);
        return ReadSummaryFromReadme(examplePath);
    }

} // namespace SZM::Workshop3D
