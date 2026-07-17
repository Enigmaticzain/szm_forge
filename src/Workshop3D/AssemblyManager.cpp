#include "AssemblyManager.hpp"
#include "Data/UUIDGenerator.hpp"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <limits>
#include <sstream>

namespace fs = std::filesystem;

namespace {

constexpr double kDegreesToRadians = 3.14159265358979323846 / 180.0;
constexpr double kMinimumScale = 0.05;

std::string UUIDToString(const SZM::UUID& id) {
    std::ostringstream stream;
    stream << std::hex << std::setfill('0')
           << std::setw(16) << id.high
           << std::setw(16) << id.low;
    return stream.str();
}

bool ParseUUID(const std::string& text, SZM::UUID& outId) {
    if (text.size() != 32U) {
        return false;
    }

    try {
        outId.high = std::stoull(text.substr(0, 16), nullptr, 16);
        outId.low = std::stoull(text.substr(16), nullptr, 16);
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

std::string SanitizeFilename(std::string text) {
    for (char& c : text) {
        if (!(std::isalnum(static_cast<unsigned char>(c)) || c == '-' || c == '_')) {
            c = '_';
        }
    }
    return text.empty() ? "assembly" : text;
}

SZM::Geometry::Vector3 ClampScale(const SZM::Geometry::Vector3& scale) {
    return {
        std::max(kMinimumScale, std::abs(scale.x)),
        std::max(kMinimumScale, std::abs(scale.y)),
        std::max(kMinimumScale, std::abs(scale.z))
    };
}

} // namespace

namespace SZM::Workshop3D {

    AssemblyInstance::AssemblyInstance(std::shared_ptr<Part> part)
        : m_Part(std::move(part)) {
        if (m_Part) {
            m_InstanceName = m_Part->GetProperties().name;
        }
    }

    void AssemblyInstance::SetTransform(
        const Geometry::Vector3& position,
        const Geometry::Vector3& rotation
    ) {
        m_Position = position;
        m_Rotation = rotation;
    }

    void AssemblyInstance::SetScale(const Geometry::Vector3& scale) {
        m_Scale = ClampScale(scale);
    }

    Graphics::Matrix4x4 AssemblyInstance::GetTransformMatrix() const {
        const double rx = m_Rotation.x * kDegreesToRadians;
        const double ry = m_Rotation.y * kDegreesToRadians;
        const double rz = m_Rotation.z * kDegreesToRadians;

        const double cx = std::cos(rx);
        const double sx = std::sin(rx);
        const double cy = std::cos(ry);
        const double sy = std::sin(ry);
        const double cz = std::cos(rz);
        const double sz = std::sin(rz);
        const Geometry::Vector3 scale = ClampScale(m_Scale);

        Graphics::Matrix4x4 matrix;
        matrix.m[0] = static_cast<float>(cy * cz * scale.x);
        matrix.m[1] = static_cast<float>((sx * sy * cz + cx * sz) * scale.x);
        matrix.m[2] = static_cast<float>((-cx * sy * cz + sx * sz) * scale.x);
        matrix.m[3] = 0.0f;

        matrix.m[4] = static_cast<float>(-cy * sz * scale.y);
        matrix.m[5] = static_cast<float>((-sx * sy * sz + cx * cz) * scale.y);
        matrix.m[6] = static_cast<float>((cx * sy * sz + sx * cz) * scale.y);
        matrix.m[7] = 0.0f;

        matrix.m[8] = static_cast<float>(sy * scale.z);
        matrix.m[9] = static_cast<float>(-sx * cy * scale.z);
        matrix.m[10] = static_cast<float>(cx * cy * scale.z);
        matrix.m[11] = 0.0f;

        matrix.m[12] = static_cast<float>(m_Position.x);
        matrix.m[13] = static_cast<float>(m_Position.y);
        matrix.m[14] = static_cast<float>(m_Position.z);
        matrix.m[15] = 1.0f;
        return matrix;
    }

    Geometry::Vector3 AssemblyInstance::GetScaledBBoxMin() const {
        if (!m_Part) {
            return Geometry::Vector3(0.0, 0.0, 0.0);
        }

        const Geometry::Vector3 scale = ClampScale(m_Scale);
        const auto& minBounds = m_Part->GetBBoxMin();
        return {
            minBounds.x * scale.x,
            minBounds.y * scale.y,
            minBounds.z * scale.z
        };
    }

    Geometry::Vector3 AssemblyInstance::GetScaledBBoxMax() const {
        if (!m_Part) {
            return Geometry::Vector3(0.0, 0.0, 0.0);
        }

        const Geometry::Vector3 scale = ClampScale(m_Scale);
        const auto& maxBounds = m_Part->GetBBoxMax();
        return {
            maxBounds.x * scale.x,
            maxBounds.y * scale.y,
            maxBounds.z * scale.z
        };
    }

    Geometry::Vector3 AssemblyInstance::GetScaledSize() const {
        return GetScaledBBoxMax() - GetScaledBBoxMin();
    }

    double AssemblyInstance::GetScaledMass() const {
        if (!m_Part) {
            return 0.0;
        }

        const Geometry::Vector3 scale = ClampScale(m_Scale);
        return static_cast<double>(m_Part->GetMass()) * scale.x * scale.y * scale.z;
    }

    double AssemblyInstance::GetScaledRadius() const {
        if (!m_Part) {
            return 0.0;
        }

        const Geometry::Vector3 scale = ClampScale(m_Scale);
        return m_Part->GetBBoxRadius() * std::max({ scale.x, scale.y, scale.z });
    }

    void AssemblyInstance::AddConstraint(uint32_t targetInstanceId, const std::string& constraintType) {
        m_Constraints.emplace_back(targetInstanceId, constraintType);
    }

    Assembly::Assembly(const std::string& name)
        : m_ID(UUIDGenerator::Generate()), m_Name(name) {
    }

    uint32_t Assembly::AddInstance(std::shared_ptr<Part> part, const std::string& instanceName) {
        if (!part) {
            return 0U;
        }

        const uint32_t id = m_NextInstanceId++;
        auto instance = std::make_shared<AssemblyInstance>(part);
        if (!instanceName.empty()) {
            instance->SetInstanceName(instanceName);
        } else {
            instance->SetInstanceName(part->GetProperties().name + " #" + std::to_string(id));
        }

        m_Instances[id] = instance;
        return id;
    }

    std::shared_ptr<AssemblyInstance> Assembly::GetInstance(uint32_t instanceId) {
        const auto found = m_Instances.find(instanceId);
        return found != m_Instances.end() ? found->second : nullptr;
    }

    std::shared_ptr<const AssemblyInstance> Assembly::GetInstance(uint32_t instanceId) const {
        const auto found = m_Instances.find(instanceId);
        return found != m_Instances.end() ? found->second : nullptr;
    }

    bool Assembly::RemoveInstance(uint32_t instanceId) {
        const auto found = m_Instances.find(instanceId);
        if (found == m_Instances.end()) {
            return false;
        }

        m_Instances.erase(found);
        m_Constraints.erase(
            std::remove_if(
                m_Constraints.begin(),
                m_Constraints.end(),
                [instanceId](const auto& constraint) {
                    return std::get<0>(constraint) == instanceId ||
                           std::get<1>(constraint) == instanceId;
                }
            ),
            m_Constraints.end()
        );
        return true;
    }

    void Assembly::ClearInstances() {
        m_Instances.clear();
        m_Constraints.clear();
        m_NextInstanceId = 1U;
    }

    std::vector<uint32_t> Assembly::GetInstancesByPart(const UUID& partId) const {
        std::vector<uint32_t> ids;
        for (const auto& [instanceId, instance] : m_Instances) {
            if (instance->GetPart() && instance->GetPart()->GetID() == partId) {
                ids.push_back(instanceId);
            }
        }
        return ids;
    }

    Geometry::Vector3 Assembly::CalculateCenterOfMass() const {
        if (m_Instances.empty()) {
            return Geometry::Vector3(0.0, 0.0, 0.0);
        }

        Geometry::Vector3 weighted(0.0, 0.0, 0.0);
        double totalMass = 0.0;
        for (const auto& [id, instance] : m_Instances) {
            (void)id;
            const double mass = std::max(0.001, instance->GetScaledMass());
            weighted = weighted + (instance->GetPosition() * mass);
            totalMass += mass;
        }

        if (totalMass <= 0.0) {
            return Geometry::Vector3(0.0, 0.0, 0.0);
        }

        return weighted * (1.0 / totalMass);
    }

    float Assembly::CalculateTotalMass() const {
        float totalMass = 0.0f;
        for (const auto& [id, instance] : m_Instances) {
            (void)id;
            totalMass += static_cast<float>(instance->GetScaledMass());
        }
        return totalMass;
    }

    void Assembly::CalculateBounds(Geometry::Vector3& minBounds, Geometry::Vector3& maxBounds) const {
        if (m_Instances.empty()) {
            minBounds = Geometry::Vector3(0.0, 0.0, 0.0);
            maxBounds = Geometry::Vector3(0.0, 0.0, 0.0);
            return;
        }

        const double maxValue = std::numeric_limits<double>::max();
        minBounds = Geometry::Vector3(maxValue, maxValue, maxValue);
        maxBounds = Geometry::Vector3(-maxValue, -maxValue, -maxValue);

        for (const auto& [instanceId, instance] : m_Instances) {
            (void)instanceId;
            const auto& position = instance->GetPosition();
            const auto partMin = instance->GetScaledBBoxMin();
            const auto partMax = instance->GetScaledBBoxMax();

            minBounds.x = std::min(minBounds.x, position.x + partMin.x);
            minBounds.y = std::min(minBounds.y, position.y + partMin.y);
            minBounds.z = std::min(minBounds.z, position.z + partMin.z);

            maxBounds.x = std::max(maxBounds.x, position.x + partMax.x);
            maxBounds.y = std::max(maxBounds.y, position.y + partMax.y);
            maxBounds.z = std::max(maxBounds.z, position.z + partMax.z);
        }
    }

    void Assembly::AddConstraint(uint32_t instance1, uint32_t instance2, const std::string& type) {
        m_Constraints.emplace_back(instance1, instance2, type);
        if (auto source = GetInstance(instance1)) {
            source->AddConstraint(instance2, type);
        }
    }

    bool Assembly::SaveToJSON(const std::string& filepath, const PartLibrary& library) const {
        (void)library;
        std::ofstream output(filepath);
        if (!output.is_open()) {
            return false;
        }

        output << "ASSEMBLY|" << UUIDToString(m_ID) << '|' << m_Name << '\n';
        for (const auto& [instanceId, instance] : m_Instances) {
            output << "INSTANCE|"
                   << instanceId << '|'
                   << UUIDToString(instance->GetPart()->GetID()) << '|'
                   << instance->GetInstanceName() << '|'
                   << instance->GetPosition().x << '|'
                   << instance->GetPosition().y << '|'
                   << instance->GetPosition().z << '|'
                   << instance->GetRotation().x << '|'
                   << instance->GetRotation().y << '|'
                   << instance->GetRotation().z << '|'
                   << instance->GetScale().x << '|'
                   << instance->GetScale().y << '|'
                   << instance->GetScale().z << '|'
                   << (instance->IsVisible() ? 1 : 0) << '\n';
        }

        for (const auto& [instance1, instance2, type] : m_Constraints) {
            output << "CONSTRAINT|" << instance1 << '|' << instance2 << '|' << type << '\n';
        }
        return true;
    }

    bool Assembly::LoadFromJSON(const std::string& filepath, const PartLibrary& library) {
        std::ifstream input(filepath);
        if (!input.is_open()) {
            return false;
        }

        ClearInstances();

        std::string line;
        while (std::getline(input, line)) {
            if (line.empty()) {
                continue;
            }

            std::stringstream parser(line);
            std::string type;
            std::getline(parser, type, '|');

            if (type == "ASSEMBLY") {
                std::string uuidText;
                std::getline(parser, uuidText, '|');
                ParseUUID(uuidText, m_ID);
                std::getline(parser, m_Name);
                continue;
            }

            if (type == "INSTANCE") {
                std::vector<std::string> fields;
                std::string field;
                while (std::getline(parser, field, '|')) {
                    fields.push_back(field);
                }

                if (fields.size() < 8U) {
                    continue;
                }

                const std::string& instanceIdText = fields[0];
                const std::string& partIdText = fields[1];
                const std::string& name = fields[2];
                const std::string& px = fields[3];
                const std::string& py = fields[4];
                const std::string& pz = fields[5];
                const std::string& rx = fields[6];
                const std::string& ry = fields[7];
                const std::string rz = fields.size() > 8U ? fields[8] : "0";
                const bool hasScale = fields.size() >= 12U;
                const Geometry::Vector3 scale(
                    hasScale ? std::stod(fields[9]) : 1.0,
                    hasScale ? std::stod(fields[10]) : 1.0,
                    hasScale ? std::stod(fields[11]) : 1.0
                );
                const std::string visibleText = hasScale
                    ? (fields.size() > 12U ? fields[12] : "1")
                    : (fields.size() > 9U ? fields[9] : "1");

                UUID partId = UUID::Null();
                if (!ParseUUID(partIdText, partId)) {
                    continue;
                }

                auto part = library.GetPart(partId);
                if (!part) {
                    continue;
                }

                const uint32_t instanceId = static_cast<uint32_t>(std::stoul(instanceIdText));
                auto instance = std::make_shared<AssemblyInstance>(part);
                instance->SetInstanceName(name);
                instance->SetTransform(
                    Geometry::Vector3(std::stod(px), std::stod(py), std::stod(pz)),
                    Geometry::Vector3(std::stod(rx), std::stod(ry), std::stod(rz))
                );
                instance->SetScale(scale);
                instance->SetVisible(visibleText == "1");
                m_Instances[instanceId] = instance;
                m_NextInstanceId = std::max(m_NextInstanceId, instanceId + 1U);
                continue;
            }

            if (type == "CONSTRAINT") {
                std::string instance1Text;
                std::string instance2Text;
                std::string constraintType;
                std::getline(parser, instance1Text, '|');
                std::getline(parser, instance2Text, '|');
                std::getline(parser, constraintType);
                AddConstraint(
                    static_cast<uint32_t>(std::stoul(instance1Text)),
                    static_cast<uint32_t>(std::stoul(instance2Text)),
                    constraintType
                );
            }
        }

        return true;
    }

    std::shared_ptr<Assembly> AssemblyManager::CreateAssembly(const std::string& name) {
        auto assembly = std::make_shared<Assembly>(name);
        m_Assemblies[assembly->GetID()] = assembly;
        m_NameIndex[name] = assembly->GetID();

        if (m_ActiveAssemblyId.IsNull()) {
            m_ActiveAssemblyId = assembly->GetID();
        }
        return assembly;
    }

    std::shared_ptr<Assembly> AssemblyManager::GetAssembly(const UUID& id) {
        const auto found = m_Assemblies.find(id);
        return found != m_Assemblies.end() ? found->second : nullptr;
    }

    std::shared_ptr<const Assembly> AssemblyManager::GetAssembly(const UUID& id) const {
        const auto found = m_Assemblies.find(id);
        return found != m_Assemblies.end() ? found->second : nullptr;
    }

    std::shared_ptr<Assembly> AssemblyManager::GetAssemblyByName(const std::string& name) {
        const auto found = m_NameIndex.find(name);
        if (found == m_NameIndex.end()) {
            return nullptr;
        }
        return GetAssembly(found->second);
    }

    bool AssemblyManager::RemoveAssembly(const UUID& id) {
        const auto found = m_Assemblies.find(id);
        if (found == m_Assemblies.end()) {
            return false;
        }

        m_NameIndex.erase(found->second->GetName());
        m_Assemblies.erase(found);

        if (m_ActiveAssemblyId == id) {
            m_ActiveAssemblyId = m_Assemblies.empty() ? UUID::Null() : m_Assemblies.begin()->first;
        }
        return true;
    }

    std::shared_ptr<Assembly> AssemblyManager::GetActiveAssembly() {
        return GetAssembly(m_ActiveAssemblyId);
    }

    std::shared_ptr<const Assembly> AssemblyManager::GetActiveAssembly() const {
        return GetAssembly(m_ActiveAssemblyId);
    }

    bool AssemblyManager::SaveAllAssemblies(const std::string& dirpath, const PartLibrary& library) const {
        std::error_code error;
        fs::create_directories(dirpath, error);
        if (error) {
            return false;
        }

        std::ofstream manifest(fs::path(dirpath) / "assemblies.tsv");
        if (!manifest.is_open()) {
            return false;
        }

        manifest << "uuid\tname\tfile\tactive\n";
        for (const auto& [id, assembly] : m_Assemblies) {
            const std::string filename = SanitizeFilename(assembly->GetName()) + "_" + UUIDToString(id) + ".asm";
            manifest << UUIDToString(id) << '\t'
                     << assembly->GetName() << '\t'
                     << filename << '\t'
                     << ((m_ActiveAssemblyId == id) ? 1 : 0) << '\n';
            assembly->SaveToJSON((fs::path(dirpath) / filename).string(), library);
        }

        return true;
    }

    bool AssemblyManager::LoadAllAssemblies(const std::string& dirpath, const PartLibrary& library) {
        std::ifstream manifest(fs::path(dirpath) / "assemblies.tsv");
        if (!manifest.is_open()) {
            return false;
        }

        m_Assemblies.clear();
        m_NameIndex.clear();
        m_ActiveAssemblyId = UUID::Null();

        std::string line;
        std::getline(manifest, line);
        while (std::getline(manifest, line)) {
            if (line.empty()) {
                continue;
            }

            std::stringstream parser(line);
            std::string uuidText;
            std::string name;
            std::string filename;
            std::string activeText;
            std::getline(parser, uuidText, '\t');
            std::getline(parser, name, '\t');
            std::getline(parser, filename, '\t');
            std::getline(parser, activeText);

            auto assembly = std::make_shared<Assembly>(name);
            assembly->LoadFromJSON((fs::path(dirpath) / filename).string(), library);
            m_Assemblies[assembly->GetID()] = assembly;
            m_NameIndex[assembly->GetName()] = assembly->GetID();

            if (activeText == "1") {
                m_ActiveAssemblyId = assembly->GetID();
            }
        }

        if (m_ActiveAssemblyId.IsNull() && !m_Assemblies.empty()) {
            m_ActiveAssemblyId = m_Assemblies.begin()->first;
        }
        return !m_Assemblies.empty();
    }

} // namespace SZM::Workshop3D
