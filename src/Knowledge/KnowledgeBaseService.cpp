#include "KnowledgeBaseService.hpp"
#include "KnowledgePathUtils.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>

namespace SZM::Knowledge {

namespace {

std::string ToLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}

bool JsonContainsQuery(const nlohmann::json& entry, const std::string& qLower) {
    auto check = [&](const std::string& text) {
        return ToLower(text).find(qLower) != std::string::npos;
    };

    if (entry.contains("id") && entry["id"].is_string() && check(entry["id"].get<std::string>())) {
        return true;
    }
    if (entry.contains("name") && entry["name"].is_string() && check(entry["name"].get<std::string>())) {
        return true;
    }
    if (entry.contains("category") && entry["category"].is_string() &&
        check(entry["category"].get<std::string>())) {
        return true;
    }
    if (entry.contains("notes") && entry["notes"].is_string() && check(entry["notes"].get<std::string>())) {
        return true;
    }
    if (entry.contains("expression") && entry["expression"].is_string() &&
        check(entry["expression"].get<std::string>())) {
        return true;
    }
    if (entry.contains("tags") && entry["tags"].is_array()) {
        for (const auto& t : entry["tags"]) {
            if (t.is_string() && check(t.get<std::string>())) {
                return true;
            }
        }
    }
    return false;
}

nlohmann::json ReadJsonFile(const std::filesystem::path& path) {
    std::ifstream in(path);
    if (!in) {
        return nlohmann::json{};
    }
    nlohmann::json j;
    in >> j;
    return j;
}

} // namespace

void KnowledgeBaseService::Load() {
    std::unique_lock lock(m_Mutex);
    if (m_Loaded) {
        return;
    }

    const auto dir = ResolveKnowledgeDirectory();
    if (dir.empty()) {
        std::cerr << "[SZM Knowledge] data/knowledge not found — KB endpoints will be empty.\n";
        m_Loaded = true;
        return;
    }

    LoadFromDirectory(dir);
    m_Loaded = true;
}

bool KnowledgeBaseService::IsLoaded() const {
    std::shared_lock lock(m_Mutex);
    return m_Loaded;
}

void KnowledgeBaseService::LoadFromDirectory(const std::filesystem::path& dir) {
    m_Index   = ReadJsonFile(dir / "index.json");
    m_Domains.clear();

    if (m_Index.contains("domains") && m_Index["domains"].is_array()) {
        for (const auto& d : m_Index["domains"]) {
            if (!d.contains("id") || !d.contains("file")) {
                continue;
            }
            const std::string domainId = d["id"].get<std::string>();
            const std::string fileName = d["file"].get<std::string>();
            auto domainJson            = ReadJsonFile(dir / fileName);
            if (!domainJson.is_object()) {
                continue;
            }
            m_Domains[domainId] = std::move(domainJson);
        }
    } else {
        // Fallback: load every *.json except index
        for (const auto& entry : std::filesystem::directory_iterator(dir)) {
            if (!entry.is_regular_file()) {
                continue;
            }
            const auto fname = entry.path().filename().string();
            if (fname == "index.json" || entry.path().extension() != ".json") {
                continue;
            }
            const std::string domainId = entry.path().stem().string();
            auto domainJson            = ReadJsonFile(entry.path());
            if (domainJson.is_object()) {
                m_Domains[domainId] = std::move(domainJson);
            }
        }
    }

    std::cout << "[SZM Knowledge] Loaded " << m_Domains.size() << " domain(s) from "
              << dir.string() << ".\n";
}

nlohmann::json KnowledgeBaseService::ListDomains() const {
    std::shared_lock lock(m_Mutex);
    if (m_Index.contains("domains")) {
        return m_Index["domains"];
    }

    nlohmann::json domains = nlohmann::json::array();
    for (const auto& [id, data] : m_Domains) {
        const std::size_t count =
            data.contains("entries") && data["entries"].is_array() ? data["entries"].size() : 0;
        domains.push_back({{"id", id}, {"name", id}, {"entryCount", count}});
    }
    return domains;
}

nlohmann::json KnowledgeBaseService::GetDomain(const std::string& domainId) const {
    std::shared_lock lock(m_Mutex);
    auto it = m_Domains.find(domainId);
    if (it == m_Domains.end()) {
        return nlohmann::json{{"ok", false}, {"error", "domain not found"}};
    }
    return nlohmann::json{{"ok", true}, {"domain", domainId}, {"data", it->second}};
}

std::optional<nlohmann::json> KnowledgeBaseService::GetEntry(const std::string& domainId,
                                                             const std::string& entryId) const {
    std::shared_lock lock(m_Mutex);
    auto it = m_Domains.find(domainId);
    if (it == m_Domains.end() || !it->second.contains("entries") ||
        !it->second["entries"].is_array()) {
        return std::nullopt;
    }
    for (const auto& e : it->second["entries"]) {
        if (e.contains("id") && e["id"].is_string() && e["id"].get<std::string>() == entryId) {
            return e;
        }
    }
    return std::nullopt;
}

nlohmann::json KnowledgeBaseService::Search(const std::string& query,
                                            const std::string& domainFilter) const {
    std::shared_lock lock(m_Mutex);

    nlohmann::json results = nlohmann::json::array();
    if (query.empty()) {
        return nlohmann::json{{"ok", true}, {"query", query}, {"results", results}};
    }

    const std::string qLower = ToLower(query);

    for (const auto& [domainId, data] : m_Domains) {
        if (!domainFilter.empty() && domainId != domainFilter) {
            continue;
        }
        if (!data.contains("entries") || !data["entries"].is_array()) {
            continue;
        }
        for (const auto& entry : data["entries"]) {
            if (JsonContainsQuery(entry, qLower)) {
                results.push_back({{"domain", domainId}, {"entry", entry}});
            }
        }
    }

    return nlohmann::json{{"ok", true}, {"query", query}, {"count", results.size()}, {"results", results}};
}

} // namespace SZM::Knowledge
