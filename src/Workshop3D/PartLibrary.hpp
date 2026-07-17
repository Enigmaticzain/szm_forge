#pragma once

#include "Data/UUID.hpp"
#include "Geometry/MathTypes.hpp"
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace SZM::Workshop3D {

    class Part {
    public:
        struct Properties {
            std::string name;
            std::string description;
            std::string material;
            std::string category;
            std::string analysisType;
            std::string sourcePath;
            std::string previewPath;
            std::string referencePath;
            float mass = 0.0f;
            float density = 0.0f;
            uint32_t color = 0xFFFFFFFF;
            bool isAssembly = false;
        };

        explicit Part(const std::string& name);
        ~Part() = default;

        const UUID& GetID() const { return m_ID; }
        const Properties& GetProperties() const { return m_Properties; }
        void SetProperties(const Properties& props) { m_Properties = props; }

        std::vector<Geometry::Vector3>& GetVertices() { return m_Vertices; }
        const std::vector<Geometry::Vector3>& GetVertices() const { return m_Vertices; }
        std::vector<uint32_t>& GetIndices() { return m_Indices; }
        const std::vector<uint32_t>& GetIndices() const { return m_Indices; }
        std::vector<Geometry::Vector3>& GetNormals() { return m_Normals; }
        const std::vector<Geometry::Vector3>& GetNormals() const { return m_Normals; }

        void AddArtifactPath(const std::string& path);
        const std::vector<std::string>& GetArtifactPaths() const { return m_ArtifactPaths; }

        float CalculateVolume() const;
        void CalculateMass(float materialDensity);
        float GetMass() const { return m_Properties.mass; }

        void CalculateBounds();
        void SetBounds(const Geometry::Vector3& minBounds, const Geometry::Vector3& maxBounds);
        const Geometry::Vector3& GetBBoxMin() const { return m_BBoxMin; }
        const Geometry::Vector3& GetBBoxMax() const { return m_BBoxMax; }
        double GetBBoxRadius() const { return m_BBoxRadius; }

        bool LoadFromSTEP(const std::string& filepath);
        bool LoadFromGEO(const std::string& filepath);
        bool SaveToJSON(const std::string& filepath) const;
        bool LoadFromJSON(const std::string& filepath);

    private:
        UUID m_ID;
        Properties m_Properties;
        std::vector<Geometry::Vector3> m_Vertices;
        std::vector<uint32_t> m_Indices;
        std::vector<Geometry::Vector3> m_Normals;
        Geometry::Vector3 m_BBoxMin;
        Geometry::Vector3 m_BBoxMax;
        double m_BBoxRadius = 0.0;
        std::vector<std::string> m_ArtifactPaths;
    };

    class PartLibrary {
    public:
        PartLibrary() = default;
        ~PartLibrary() = default;

        std::shared_ptr<Part> CreatePart(const std::string& name);
        std::shared_ptr<Part> RegisterPart(const std::shared_ptr<Part>& part);
        std::shared_ptr<Part> GetPart(const UUID& id);
        std::shared_ptr<Part> GetPart(const UUID& id) const;
        std::shared_ptr<Part> GetPartByName(const std::string& name);
        std::shared_ptr<Part> GetPartByName(const std::string& name) const;
        bool RemovePart(const UUID& id);
        void ClearAll();

        std::shared_ptr<Part> ImportFromSTEP(const std::string& filepath);
        std::shared_ptr<Part> ImportFromGEO(const std::string& filepath);
        std::shared_ptr<Part> ImportFromCode8(const std::string& code8Path);

        size_t GetPartCount() const { return m_Parts.size(); }
        const std::map<UUID, std::shared_ptr<Part>>& GetAllParts() const { return m_Parts; }
        std::vector<std::shared_ptr<Part>> GetPartsByCategory(const std::string& category) const;

        bool SaveLibrary(const std::string& dirpath) const;
        bool LoadLibrary(const std::string& dirpath);

    private:
        std::map<UUID, std::shared_ptr<Part>> m_Parts;
        std::map<std::string, UUID> m_NameIndex;
    };

} // namespace SZM::Workshop3D
