#include "PartLibrary.hpp"
#include "CADImporter.hpp"
#include "Data/UUIDGenerator.hpp"
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace fs = std::filesystem;

namespace {

std::string TrimCopy(const std::string& value) {
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) {
        return "";
    }
    const auto last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1U);
}

std::string ToLowerCopy(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return value;
}

std::string EscapeField(std::string value) {
    std::replace(value.begin(), value.end(), '\t', ' ');
    std::replace(value.begin(), value.end(), '\n', ' ');
    std::replace(value.begin(), value.end(), '\r', ' ');
    return value;
}

std::string UUIDToString(const SZM::UUID& id) {
    std::ostringstream stream;
    stream << std::hex << std::setfill('0')
           << std::setw(16) << id.high
           << std::setw(16) << id.low;
    return stream.str();
}

} // namespace

namespace SZM::Workshop3D {

    Part::Part(const std::string& name) : m_ID(UUIDGenerator::Generate()) {
        m_Properties.name = name;
    }

    void Part::AddArtifactPath(const std::string& path) {
        if (!path.empty()) {
            m_ArtifactPaths.push_back(path);
        }
    }

    float Part::CalculateVolume() const {
        if (m_Indices.size() < 3U || m_Vertices.empty()) {
            const auto extents = m_BBoxMax - m_BBoxMin;
            return static_cast<float>(std::max(0.0, extents.x) *
                                      std::max(0.0, extents.y) *
                                      std::max(0.0, extents.z));
        }

        double volume = 0.0;
        for (size_t i = 0; i + 2U < m_Indices.size(); i += 3U) {
            const auto& p0 = m_Vertices[m_Indices[i]];
            const auto& p1 = m_Vertices[m_Indices[i + 1U]];
            const auto& p2 = m_Vertices[m_Indices[i + 2U]];
            volume += p0.Dot((p1 - p0).Cross(p2 - p0));
        }

        return static_cast<float>(std::abs(volume) / 6.0);
    }

    void Part::CalculateMass(float materialDensity) {
        m_Properties.density = materialDensity;
        m_Properties.mass = CalculateVolume() * materialDensity;
    }

    void Part::CalculateBounds() {
        if (m_Vertices.empty()) {
            SetBounds(Geometry::Vector3(0.0, 0.0, 0.0), Geometry::Vector3(0.0, 0.0, 0.0));
            return;
        }

        Geometry::Vector3 minBounds = m_Vertices.front();
        Geometry::Vector3 maxBounds = m_Vertices.front();

        for (const auto& vertex : m_Vertices) {
            minBounds.x = std::min(minBounds.x, vertex.x);
            minBounds.y = std::min(minBounds.y, vertex.y);
            minBounds.z = std::min(minBounds.z, vertex.z);

            maxBounds.x = std::max(maxBounds.x, vertex.x);
            maxBounds.y = std::max(maxBounds.y, vertex.y);
            maxBounds.z = std::max(maxBounds.z, vertex.z);
        }

        SetBounds(minBounds, maxBounds);
    }

    void Part::SetBounds(const Geometry::Vector3& minBounds, const Geometry::Vector3& maxBounds) {
        m_BBoxMin = minBounds;
        m_BBoxMax = maxBounds;

        const Geometry::Vector3 center(
            (minBounds.x + maxBounds.x) * 0.5,
            (minBounds.y + maxBounds.y) * 0.5,
            (minBounds.z + maxBounds.z) * 0.5
        );

        m_BBoxRadius = 0.0;
        if (!m_Vertices.empty()) {
            for (const auto& vertex : m_Vertices) {
                m_BBoxRadius = std::max(m_BBoxRadius, (vertex - center).Magnitude());
            }
        } else {
            const auto extents = maxBounds - minBounds;
            m_BBoxRadius = std::sqrt(extents.x * extents.x +
                                     extents.y * extents.y +
                                     extents.z * extents.z) * 0.5;
        }
    }

    bool Part::LoadFromSTEP(const std::string& filepath) {
        return fs::exists(filepath);
    }

    bool Part::LoadFromGEO(const std::string& filepath) {
        return fs::exists(filepath);
    }

    bool Part::SaveToJSON(const std::string& filepath) const {
        std::ofstream output(filepath);
        if (!output.is_open()) {
            return false;
        }

        output << "{\n"
               << "  \"id\": \"" << UUIDToString(m_ID) << "\",\n"
               << "  \"name\": \"" << EscapeField(m_Properties.name) << "\",\n"
               << "  \"category\": \"" << EscapeField(m_Properties.category) << "\",\n"
               << "  \"material\": \"" << EscapeField(m_Properties.material) << "\"\n"
               << "}\n";
        return true;
    }

    bool Part::LoadFromJSON(const std::string& filepath) {
        return fs::exists(filepath);
    }

    std::shared_ptr<Part> PartLibrary::CreatePart(const std::string& name) {
        return RegisterPart(std::make_shared<Part>(name));
    }

    std::shared_ptr<Part> PartLibrary::RegisterPart(const std::shared_ptr<Part>& part) {
        if (!part) {
            return nullptr;
        }

        const auto& props = part->GetProperties();
        const auto existing = GetPartByName(props.name);
        if (existing && existing->GetID() != part->GetID()) {
            RemovePart(existing->GetID());
        }

        m_Parts[part->GetID()] = part;
        m_NameIndex[props.name] = part->GetID();
        return part;
    }

    std::shared_ptr<Part> PartLibrary::GetPart(const UUID& id) {
        const auto found = m_Parts.find(id);
        return found != m_Parts.end() ? found->second : nullptr;
    }

    std::shared_ptr<Part> PartLibrary::GetPart(const UUID& id) const {
        const auto found = m_Parts.find(id);
        return found != m_Parts.end() ? found->second : nullptr;
    }

    std::shared_ptr<Part> PartLibrary::GetPartByName(const std::string& name) {
        const auto found = m_NameIndex.find(name);
        if (found == m_NameIndex.end()) {
            return nullptr;
        }
        return GetPart(found->second);
    }

    std::shared_ptr<Part> PartLibrary::GetPartByName(const std::string& name) const {
        const auto found = m_NameIndex.find(name);
        if (found == m_NameIndex.end()) {
            return nullptr;
        }
        return GetPart(found->second);
    }

    bool PartLibrary::RemovePart(const UUID& id) {
        const auto found = m_Parts.find(id);
        if (found == m_Parts.end()) {
            return false;
        }

        m_NameIndex.erase(found->second->GetProperties().name);
        m_Parts.erase(found);
        return true;
    }

    void PartLibrary::ClearAll() {
        m_Parts.clear();
        m_NameIndex.clear();
    }

    std::shared_ptr<Part> PartLibrary::ImportFromSTEP(const std::string& filepath) {
        return RegisterPart(CADImporter::ImportSTEP(filepath));
    }

    std::shared_ptr<Part> PartLibrary::ImportFromGEO(const std::string& filepath) {
        return RegisterPart(CADImporter::ImportGEO(filepath));
    }

    std::shared_ptr<Part> PartLibrary::ImportFromCode8(const std::string& code8Path) {
        const size_t before = m_Parts.size();
        Code8AssetManager::LoadAllExamples(code8Path, *this);
        if (m_Parts.size() <= before) {
            return nullptr;
        }
        return std::prev(m_Parts.end())->second;
    }

    std::vector<std::shared_ptr<Part>> PartLibrary::GetPartsByCategory(const std::string& category) const {
        std::vector<std::shared_ptr<Part>> parts;
        const std::string expected = ToLowerCopy(category);

        for (const auto& [id, part] : m_Parts) {
            (void)id;
            if (ToLowerCopy(part->GetProperties().category) == expected) {
                parts.push_back(part);
            }
        }

        return parts;
    }

    bool PartLibrary::SaveLibrary(const std::string& dirpath) const {
        std::error_code error;
        fs::create_directories(dirpath, error);
        if (error) {
            return false;
        }

        std::ofstream output(fs::path(dirpath) / "parts.tsv");
        if (!output.is_open()) {
            return false;
        }

        output << "uuid\tname\tcategory\tanalysis\tmaterial\tdensity\tmass\tcolor\tsource\tpreview\treference\tdescription\tartifacts\n";
        for (const auto& [id, part] : m_Parts) {
            const auto& props = part->GetProperties();
            std::ostringstream artifacts;
            for (size_t i = 0; i < part->GetArtifactPaths().size(); ++i) {
                if (i > 0U) {
                    artifacts << ';';
                }
                artifacts << EscapeField(part->GetArtifactPaths()[i]);
            }

            output << UUIDToString(id) << '\t'
                   << EscapeField(props.name) << '\t'
                   << EscapeField(props.category) << '\t'
                   << EscapeField(props.analysisType) << '\t'
                   << EscapeField(props.material) << '\t'
                   << props.density << '\t'
                   << props.mass << '\t'
                   << props.color << '\t'
                   << EscapeField(props.sourcePath) << '\t'
                   << EscapeField(props.previewPath) << '\t'
                   << EscapeField(props.referencePath) << '\t'
                   << EscapeField(props.description) << '\t'
                   << artifacts.str() << '\n';
        }

        return true;
    }

    bool PartLibrary::LoadLibrary(const std::string& dirpath) {
        std::ifstream input(fs::path(dirpath) / "parts.tsv");
        if (!input.is_open()) {
            return false;
        }

        ClearAll();

        std::string line;
        std::getline(input, line);
        while (std::getline(input, line)) {
            if (TrimCopy(line).empty()) {
                continue;
            }

            std::vector<std::string> fields;
            std::stringstream parser(line);
            std::string field;
            while (std::getline(parser, field, '\t')) {
                fields.push_back(field);
            }

            if (fields.size() < 12U) {
                continue;
            }

            auto part = std::make_shared<Part>(fields[1]);
            Part::Properties props = part->GetProperties();
            props.name = fields[1];
            props.category = fields[2];
            props.analysisType = fields[3];
            props.material = fields[4];
            props.density = fields[5].empty() ? 0.0f : std::stof(fields[5]);
            props.mass = fields[6].empty() ? 0.0f : std::stof(fields[6]);
            props.color = fields[7].empty() ? 0xFFFFFFFFU : static_cast<uint32_t>(std::stoul(fields[7]));
            props.sourcePath = fields[8];
            props.previewPath = fields[9];
            props.referencePath = fields[10];
            props.description = fields[11];
            part->SetProperties(props);

            if (fields.size() >= 13U) {
                std::stringstream artifacts(fields[12]);
                std::string artifact;
                while (std::getline(artifacts, artifact, ';')) {
                    if (!artifact.empty()) {
                        part->AddArtifactPath(artifact);
                    }
                }
            }

            part->SetBounds(Geometry::Vector3(-0.5, -0.5, -0.5), Geometry::Vector3(0.5, 0.5, 0.5));
            RegisterPart(part);
        }

        return true;
    }

} // namespace SZM::Workshop3D
