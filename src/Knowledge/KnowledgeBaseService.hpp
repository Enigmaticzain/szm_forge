#pragma once

#include <nlohmann/json.hpp>
#include <filesystem>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace SZM::Knowledge {

/**
 * @brief Loads and serves engineering reference data from data/knowledge/*.json.
 */
class KnowledgeBaseService {
public:
    static KnowledgeBaseService& GetInstance() {
        static KnowledgeBaseService instance;
        return instance;
    }

    KnowledgeBaseService(const KnowledgeBaseService&)            = delete;
    KnowledgeBaseService& operator=(const KnowledgeBaseService&) = delete;

    /// Load index + all domain files (idempotent).
    void Load();

    [[nodiscard]] bool IsLoaded() const;

    [[nodiscard]] nlohmann::json ListDomains() const;

    [[nodiscard]] nlohmann::json GetDomain(const std::string& domainId) const;

    [[nodiscard]] std::optional<nlohmann::json> GetEntry(const std::string& domainId,
                                                           const std::string& entryId) const;

    /// Case-insensitive substring search across id, name, category, notes, tags.
    [[nodiscard]] nlohmann::json Search(const std::string& query,
                                        const std::string& domainFilter = "") const;

private:
    KnowledgeBaseService() = default;

    void LoadFromDirectory(const std::filesystem::path& dir);

    mutable std::shared_mutex                  m_Mutex;
    nlohmann::json                             m_Index = nlohmann::json::object();
    std::unordered_map<std::string, nlohmann::json> m_Domains;
    bool                                       m_Loaded = false;
};

} // namespace SZM::Knowledge
