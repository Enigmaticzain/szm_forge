#pragma once

#include <cstdlib>
#include <filesystem>
#include <string>
#include <vector>

namespace SZM::Knowledge {

/// Candidate directories containing data/knowledge/*.json (checked in order).
[[nodiscard]] inline std::vector<std::filesystem::path> GetKnowledgeSearchRoots() {
    std::vector<std::filesystem::path> roots;

    if (const char* env = std::getenv("SZM_FORGE_ROOT")) {
        roots.emplace_back(env);
    }

    roots.emplace_back(".");
    roots.emplace_back("..");
    roots.emplace_back("../..");

    return roots;
}

/// Resolve data/knowledge directory; returns empty path if not found.
[[nodiscard]] inline std::filesystem::path ResolveKnowledgeDirectory() {
    for (const auto& root : GetKnowledgeSearchRoots()) {
        const auto dir = root / "data" / "knowledge";
        if (std::filesystem::is_directory(dir)) {
            return std::filesystem::weakly_canonical(dir);
        }
    }
    return {};
}

} // namespace SZM::Knowledge
