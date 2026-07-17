#include "StructuralSolveService.hpp"
#include "FastAPIBridge.hpp"
#include "../Materials/MaterialDatabase.hpp"

#include <algorithm>
#include <cmath>
#include <string>

namespace SZM::WebAPI {

namespace {

constexpr float kG = 9.81f;

struct Member {
    std::string id;
    std::string name;
    std::string kind;
    float length = 0.75f;
    float width = 0.08f;
    float height = 0.08f;
    float cx = 0.0f;
    float cy = 0.0f;
    float cz = 0.0f;
    std::string materialId;
};

struct CrossSection {
    float area = 0.0f;
    float I = 0.0f;
    float c = 0.0f;
};

CrossSection crossSection(const Member& m) {
    const float L = std::max(m.length, 1e-4f);
    const float W = std::max(m.width, 1e-4f);
    const float H = std::max(m.height, 1e-4f);

    if (m.kind == "rod") {
        const float d = W;
        const float r = d * 0.5f;
        const float area = static_cast<float>(M_PI) * r * r;
        const float I = static_cast<float>(M_PI) * std::pow(d, 4.0f) / 64.0f;
        return {area, I, d * 0.5f};
    }
    if (m.kind == "plate") {
        return {W * H, (W * H * H * H) / 12.0f, H * 0.5f};
    }
    const float area = W * H;
    float I = (W * H * H * H) / 12.0f;
    if (I <= 0.0f) {
        I = area * L * L / 12.0f;
    }
    return {area, I, H * 0.5f};
}

float vonMises(float axial, float bending) {
    return std::sqrt(axial * axial + bending * bending);
}

std::string statusFromRatio(float ratio) {
    if (ratio >= 1.0f) {
        return "FAIL";
    }
    if (ratio >= 0.8f) {
        return "WARN";
    }
    return "SAFE";
}

float loadShare(const Member& member, const std::vector<Member>& members) {
    float totalW = 0.0f;
    std::vector<float> weights;
    weights.reserve(members.size());
    for (const auto& m : members) {
        const auto cs = crossSection(m);
        const float massProxy = cs.area * m.length;
        weights.push_back(massProxy);
        totalW += massProxy;
    }

    const auto it = std::find_if(members.begin(), members.end(),
        [&](const Member& m) { return m.id == member.id; });
    const std::size_t idx = static_cast<std::size_t>(std::distance(members.begin(), it));
    const float base = totalW > 0.0f ? weights[idx] / totalW : 1.0f / static_cast<float>(members.size());

    float maxY = 0.0f;
    for (const auto& m : members) {
        maxY = std::max(maxY, m.cy + m.length);
    }
    const float topness = (member.cy + member.length) / (maxY > 0.0f ? maxY : 1.0f);
    const float blend = base * 0.4f + topness * 0.6f;

    float sum = 0.0f;
    for (const auto& m : members) {
        const auto cs = crossSection(m);
        const float my = std::max(maxY, 1e-6f);
        const float t = (m.cy + m.length) / my;
        const float w = base * 0.4f + t * 0.6f;
        sum += w * cs.area * m.length;
    }
    const auto csM = crossSection(member);
    return sum > 0.0f ? (blend * csM.area * member.length) / sum : 1.0f / static_cast<float>(members.size());
}

Member parseMember(const nlohmann::json& j) {
    Member m;
    m.id = j.value("id", "");
    m.name = j.value("name", "Member");
    m.kind = j.value("kind", "beam");
    m.length = j.value("length", 0.75f);
    m.width = j.value("width", 0.08f);
    m.height = j.value("height", 0.08f);
    m.cx = j.value("cx", 0.0f);
    m.cy = j.value("cy", 0.0f);
    m.cz = j.value("cz", 0.0f);
    m.materialId = j.value("materialId", "MAT-STEEL-STRUCT");
    return m;
}

} // namespace

nlohmann::json StructuralSolveService::Solve(const nlohmann::json& body) {
    auto& db = Materials::MaterialDatabase::GetInstance();
    db.LoadStandardLibraries();

    const auto membersJson = body.value("members", nlohmann::json::array());
    if (membersJson.empty()) {
        return {{"ok", false}, {"error", "No members provided"}};
    }

    const std::string defaultMatId = body.value("materialId", "MAT-STEEL-STRUCT");
    const float loadKg = body.value("loadKg", 200.0f);
    const float totalN = loadKg * kG;

    std::vector<Member> members;
    members.reserve(membersJson.size());
    for (const auto& j : membersJson) {
        members.push_back(parseMember(j));
    }

    const auto defaultMat = db.GetMaterial(defaultMatId);

    nlohmann::json parts = nlohmann::json::array();
    for (const auto& m : members) {
        const auto matOpt = db.GetMaterial(m.materialId.empty() ? defaultMatId : m.materialId);
        const auto& mat = matOpt.has_value() ? *matOpt : (defaultMat.has_value() ? *defaultMat : Materials::IMaterial{});

        const float yieldMPa = mat.yieldStrength > 0.0 ? mat.yieldStrength : 250.0;
        const float E_GPa = mat.youngsModulus > 0.0 ? mat.youngsModulus : 200.0;

        const auto cs = crossSection(m);
        const float share = loadShare(m, members);
        const float F = totalN * share;
        const float L = m.length;

        const float sigmaAxial = (F / std::max(cs.area, 1e-9f)) / 1e6f;
        const float M = (F * L) / 4.0f;
        const float sigmaBend = cs.I > 0.0f ? ((M * cs.c) / cs.I) / 1e6f : 0.0f;

        float sigma = vonMises(std::abs(sigmaAxial), std::abs(sigmaBend));

        if (m.kind == "rod") {
            const float E = E_GPa * 1e9f;
            const float r = std::max(m.width * 0.5f, 1e-4f);
            const float slenderness = L / r;
            if (slenderness > 80.0f) {
                const float Pcr = (static_cast<float>(M_PI * M_PI) * E * cs.I) / (4.0f * L * L);
                if (F > Pcr * 0.5f) {
                    sigma *= 1.0f + (F / std::max(Pcr, 1.0f)) * 0.35f;
                }
            }
        }

        const float ratio = yieldMPa > 0.0f ? sigma / static_cast<float>(yieldMPa) : 0.0f;
        const std::string status = statusFromRatio(ratio);

        parts.push_back({
            {"name", m.name},
            {"stress_MPa", sigma},
            {"yield_MPa", yieldMPa},
            {"stressRatio", ratio},
            {"status", status},
            {"force_N", F},
            {"material", mat.name.empty() ? m.materialId : mat.name},
            {"kind", m.kind},
            {"cx", m.cx},
            {"cy", m.cy},
            {"cz", m.cz},
            {"length", m.length},
            {"width", m.width},
            {"height", m.height},
        });

        APIManager::GetInstance().PushComponentSnapshot(
            m.name, sigma, 25.0f, ratio, 0.2f, 0.0);
    }

    return {
        {"ok", true},
        {"parts", parts},
        {"memberCount", members.size()},
        {"loadKg", loadKg},
        {"materialId", defaultMatId},
        {"solver", "SZM Structural (MaterialDatabase + beam/rod mechanics)"},
    };
}

} // namespace SZM::WebAPI
