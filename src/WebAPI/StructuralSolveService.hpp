#pragma once

#include <nlohmann/json.hpp>

namespace SZM::WebAPI {

/**
 * @brief Structural kit solver — mirrors codes/ workshop beam/rod physics using MaterialDatabase.
 */
class StructuralSolveService {
public:
    static nlohmann::json Solve(const nlohmann::json& body);
};

} // namespace SZM::WebAPI
