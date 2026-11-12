#include "CreatorMode.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <random>
#include <sstream>

namespace SZM::AI {

namespace {

constexpr size_t kMinCreatorPasswordLength = 12;
constexpr size_t kMaxActionLogEntries = 10000;

std::string BytesToHex(const uint8_t* data, size_t len) {
    static constexpr char kHex[] = "0123456789abcdef";
    std::string out;
    out.reserve(len * 2);
    for (size_t i = 0; i < len; ++i) {
        out.push_back(kHex[(data[i] >> 4) & 0xF]);
        out.push_back(kHex[data[i] & 0xF]);
    }
    return out;
}

bool HexToBytes(const std::string& hex, std::vector<uint8_t>& out) {
    if (hex.empty() || (hex.size() % 2) != 0) {
        return false;
    }
    auto nibble = [](char c) -> int {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
        if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
        return -1;
    };
    out.clear();
    out.reserve(hex.size() / 2);
    for (size_t i = 0; i < hex.size(); i += 2) {
        int hi = nibble(hex[i]);
        int lo = nibble(hex[i + 1]);
        if (hi < 0 || lo < 0) {
            out.clear();
            return false;
        }
        out.push_back(static_cast<uint8_t>((hi << 4) | lo));
    }
    return true;
}

bool ConstantTimeEqual(const std::vector<uint8_t>& a, const std::vector<uint8_t>& b) {
    if (a.size() != b.size() || a.empty()) {
        return false;
    }
    volatile uint8_t diff = 0;
    for (size_t i = 0; i < a.size(); ++i) {
        diff |= static_cast<uint8_t>(a[i] ^ b[i]);
    }
    return diff == 0;
}

std::array<uint8_t, 32> Sha256(const uint8_t* data, size_t len) {
    static constexpr std::array<uint32_t, 64> kRound = {
        0x428a2f98u, 0x71374491u, 0xb5c0fbcfu, 0xe9b5dba5u, 0x3956c25bu, 0x59f111f1u, 0x923f82a4u, 0xab1c5ed5u,
        0xd807aa98u, 0x12835b01u, 0x243185beu, 0x550c7dc3u, 0x72be5d74u, 0x80deb1feu, 0x9bdc06a7u, 0xc19bf174u,
        0xe49b69c1u, 0xefbe4786u, 0x0fc19dc6u, 0x240ca1ccu, 0x2de92c6fu, 0x4a7484aau, 0x5cb0a9dcu, 0x76f988dau,
        0x983e5152u, 0xa831c66du, 0xb00327c8u, 0xbf597fc7u, 0xc6e00bf3u, 0xd5a79147u, 0x06ca6351u, 0x14292967u,
        0x27b70a85u, 0x2e1b2138u, 0x4d2c6dfcu, 0x53380d13u, 0x650a7354u, 0x766a0abbu, 0x81c2c92eu, 0x92722c85u,
        0xa2bfe8a1u, 0xa81a664bu, 0xc24b8b70u, 0xc76c51a3u, 0xd192e819u, 0xd6990624u, 0xf40e3585u, 0x106aa070u,
        0x19a4c116u, 0x1e376c08u, 0x2748774cu, 0x34b0bcb5u, 0x391c0cb3u, 0x4ed8aa4au, 0x5b9cca4fu, 0x682e6ff3u,
        0x748f82eeu, 0x78a5636fu, 0x84c87814u, 0x8cc70208u, 0x90befffau, 0xa4506cebu, 0xbef9a3f7u, 0xc67178f2u
    };

    auto rotr = [](uint32_t x, uint32_t n) { return (x >> n) | (x << (32 - n)); };

    std::array<uint32_t, 8> h = {
        0x6a09e667u, 0xbb67ae85u, 0x3c6ef372u, 0xa54ff53au,
        0x510e527fu, 0x9b05688cu, 0x1f83d9abu, 0x5be0cd19u
    };

    const uint64_t bitLen = static_cast<uint64_t>(len) * 8ULL;
    const size_t paddedLen = ((len + 9 + 63) / 64) * 64;
    std::vector<uint8_t> buf(paddedLen, 0);
    if (len > 0) {
        std::copy_n(data, len, buf.begin());
    }
    buf[len] = 0x80;
    for (int i = 0; i < 8; ++i) {
        buf[paddedLen - 1 - i] = static_cast<uint8_t>((bitLen >> (8 * i)) & 0xFF);
    }

    for (size_t offset = 0; offset < paddedLen; offset += 64) {
        std::array<uint32_t, 64> w{};
        for (size_t i = 0; i < 16; ++i) {
            const size_t idx = offset + i * 4;
            w[i] = (static_cast<uint32_t>(buf[idx]) << 24) |
                   (static_cast<uint32_t>(buf[idx + 1]) << 16) |
                   (static_cast<uint32_t>(buf[idx + 2]) << 8) |
                   static_cast<uint32_t>(buf[idx + 3]);
        }
        for (size_t i = 16; i < 64; ++i) {
            uint32_t s0 = rotr(w[i - 15], 7) ^ rotr(w[i - 15], 18) ^ (w[i - 15] >> 3);
            uint32_t s1 = rotr(w[i - 2], 17) ^ rotr(w[i - 2], 19) ^ (w[i - 2] >> 10);
            w[i] = w[i - 16] + s0 + w[i - 7] + s1;
        }

        uint32_t a = h[0], b = h[1], c = h[2], d = h[3];
        uint32_t e = h[4], f = h[5], g = h[6], hh = h[7];

        for (size_t i = 0; i < 64; ++i) {
            uint32_t S1 = rotr(e, 6) ^ rotr(e, 11) ^ rotr(e, 25);
            uint32_t ch = (e & f) ^ ((~e) & g);
            uint32_t temp1 = hh + S1 + ch + kRound[i] + w[i];
            uint32_t S0 = rotr(a, 2) ^ rotr(a, 13) ^ rotr(a, 22);
            uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
            uint32_t temp2 = S0 + maj;

            hh = g;
            g = f;
            f = e;
            e = d + temp1;
            d = c;
            c = b;
            b = a;
            a = temp1 + temp2;
        }

        h[0] += a; h[1] += b; h[2] += c; h[3] += d;
        h[4] += e; h[5] += f; h[6] += g; h[7] += hh;
    }

    std::array<uint8_t, 32> digest{};
    for (size_t i = 0; i < 8; ++i) {
        digest[i * 4 + 0] = static_cast<uint8_t>((h[i] >> 24) & 0xFF);
        digest[i * 4 + 1] = static_cast<uint8_t>((h[i] >> 16) & 0xFF);
        digest[i * 4 + 2] = static_cast<uint8_t>((h[i] >> 8) & 0xFF);
        digest[i * 4 + 3] = static_cast<uint8_t>(h[i] & 0xFF);
    }
    return digest;
}

std::array<uint8_t, 32> HmacSha256(
    const uint8_t* key, size_t keyLen,
    const uint8_t* msg, size_t msgLen) {
    std::array<uint8_t, 64> keyBlock{};
    if (keyLen > 64) {
        auto hashed = Sha256(key, keyLen);
        std::copy(hashed.begin(), hashed.end(), keyBlock.begin());
    } else if (keyLen > 0) {
        std::copy_n(key, keyLen, keyBlock.begin());
    }

    std::vector<uint8_t> inner(64 + msgLen);
    std::array<uint8_t, 64 + 32> outer{};
    for (size_t i = 0; i < 64; ++i) {
        inner[i] = static_cast<uint8_t>(keyBlock[i] ^ 0x36);
        outer[i] = static_cast<uint8_t>(keyBlock[i] ^ 0x5c);
    }
    if (msgLen > 0) {
        std::copy_n(msg, msgLen, inner.begin() + 64);
    }
    auto innerHash = Sha256(inner.data(), inner.size());
    std::copy(innerHash.begin(), innerHash.end(), outer.begin() + 64);
    return Sha256(outer.data(), outer.size());
}

std::array<uint8_t, 32> Pbkdf2HmacSha256(
    const std::string& password,
    const std::vector<uint8_t>& salt,
    uint32_t iterations) {
    std::vector<uint8_t> saltBlock(salt.size() + 4);
    std::copy(salt.begin(), salt.end(), saltBlock.begin());
    saltBlock[salt.size() + 0] = 0;
    saltBlock[salt.size() + 1] = 0;
    saltBlock[salt.size() + 2] = 0;
    saltBlock[salt.size() + 3] = 1;

    const auto* pwdBytes = reinterpret_cast<const uint8_t*>(password.data());
    auto u = HmacSha256(pwdBytes, password.size(), saltBlock.data(), saltBlock.size());
    auto acc = u;
    for (uint32_t iter = 1; iter < iterations; ++iter) {
        u = HmacSha256(pwdBytes, password.size(), u.data(), u.size());
        for (size_t j = 0; j < acc.size(); ++j) {
            acc[j] ^= u[j];
        }
    }
    return acc;
}

bool VerifyCredentialSpec(const std::string& candidate, const std::string& spec) {
    if (candidate.empty() || spec.empty()) {
        return false;
    }
    // Supported formats:
    //   $pbkdf2-sha256$<iterations>$<salt_hex>$<hash_hex>
    //   $sha256$<salt_hex>$<hash_hex>
    //   <salt_hex>:<hash_hex>
    if (spec.rfind("$pbkdf2-sha256$", 0) == 0) {
        std::stringstream ss(spec.substr(15));
        std::string iterStr, saltHex, hashHex;
        if (!std::getline(ss, iterStr, '$') ||
            !std::getline(ss, saltHex, '$') ||
            !std::getline(ss, hashHex, '$') ||
            ss.rdbuf()->in_avail() != 0) {
            return false;
        }
        uint32_t iterations = 0;
        try {
            unsigned long parsed = std::stoul(iterStr);
            if (parsed < 10000 || parsed > 5000000) {
                return false;
            }
            iterations = static_cast<uint32_t>(parsed);
        } catch (...) {
            return false;
        }
        std::vector<uint8_t> salt, expected;
        if (!HexToBytes(saltHex, salt) || salt.size() < 8 ||
            !HexToBytes(hashHex, expected) || expected.size() != 32) {
            return false;
        }
        auto derived = Pbkdf2HmacSha256(candidate, salt, iterations);
        std::vector<uint8_t> actual(derived.begin(), derived.end());
        return ConstantTimeEqual(actual, expected);
    }

    std::string saltHex, hashHex;
    if (spec.rfind("$sha256$", 0) == 0) {
        std::string rest = spec.substr(8);
        size_t sep = rest.find('$');
        if (sep == std::string::npos || rest.find('$', sep + 1) != std::string::npos) {
            return false;
        }
        saltHex = rest.substr(0, sep);
        hashHex = rest.substr(sep + 1);
    } else {
        size_t sep = spec.find(':');
        if (sep == std::string::npos || spec.find(':', sep + 1) != std::string::npos) {
            return false;
        }
        saltHex = spec.substr(0, sep);
        hashHex = spec.substr(sep + 1);
    }

    std::vector<uint8_t> salt, expected;
    if (!HexToBytes(saltHex, salt) || salt.size() < 8 ||
        !HexToBytes(hashHex, expected) || expected.size() != 32) {
        return false;
    }
    std::vector<uint8_t> input;
    input.reserve(salt.size() + candidate.size());
    input.insert(input.end(), salt.begin(), salt.end());
    input.insert(input.end(), candidate.begin(), candidate.end());
    auto digest = Sha256(input.data(), input.size());
    std::vector<uint8_t> actual(digest.begin(), digest.end());
    return ConstantTimeEqual(actual, expected);
}

std::string ReadEnvString(const char* name) {
    const char* val = std::getenv(name);
    return val ? std::string(val) : std::string();
}

uint64_t GenerateSessionId() {
    std::random_device rd;
    uint64_t hi = static_cast<uint64_t>(rd());
    uint64_t lo = static_cast<uint64_t>(rd());
    uint64_t combined = (hi << 32) ^ lo;
    return combined != 0 ? combined : 1ULL;
}

} // namespace

CreatorModeManager& CreatorModeManager::GetInstance() {
    static CreatorModeManager instance;
    return instance;
}

void CreatorModeManager::ConfigureCredentials(
    const std::string& passwordHashSpec,
    const std::vector<std::string>& authorizedDevices,
    const std::string& confirmationHashSpec) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_ConfiguredPasswordHash = passwordHashSpec;
    m_ConfiguredConfirmationHash = confirmationHashSpec;
    m_AuthorizedDevices.clear();
    for (const auto& dev : authorizedDevices) {
        if (!dev.empty()) {
            m_AuthorizedDevices.insert(dev);
        }
    }
}

std::string CreatorModeManager::CreateCredentialSpec(
    const std::string& secret,
    const std::string& saltHex,
    uint32_t iterations) {
    std::vector<uint8_t> salt;
    if (secret.empty() || iterations < 10000 || !HexToBytes(saltHex, salt) || salt.size() < 8) {
        return "";
    }
    auto derived = Pbkdf2HmacSha256(secret, salt, iterations);
    return "$pbkdf2-sha256$" + std::to_string(iterations) + "$" +
           BytesToHex(salt.data(), salt.size()) + "$" +
           BytesToHex(derived.data(), derived.size());
}

bool CreatorModeManager::AuthenticateCreator(const std::string& password, const std::string& deviceId) {
    std::lock_guard<std::mutex> lock(m_Mutex);

    if (!ValidatePassword(password)) {
        LogAction("FAILED_AUTH: Invalid or unconfigured credentials");
        return false;
    }

    if (!ValidateDeviceBinding(deviceId)) {
        LogAction("FAILED_AUTH: Device not authorized");
        return false;
    }

    ResetSessionLocked();
    m_CurrentSession.sessionId = GenerateSessionId();
    m_CurrentSession.level = PermissionLevel::ROOT_CREATOR;
    m_CurrentSession.createdAt = std::chrono::system_clock::now();
    m_CurrentSession.expiresAt = m_CurrentSession.createdAt + std::chrono::hours(8);
    m_CurrentSession.deviceId = deviceId;
    m_CurrentSession.isActive = true;
    m_CurrentSession.requiresApproval = true;

    // Grant baseline creator permissions; MODIFY_AI / ROOT_ACCESS still require confirmation
    m_CurrentSession.permissions =
        static_cast<uint32_t>(Permission::READ_FILES) |
        static_cast<uint32_t>(Permission::WRITE_FILES) |
        static_cast<uint32_t>(Permission::EXECUTE_COMMANDS) |
        static_cast<uint32_t>(Permission::TRAIN_AI) |
        static_cast<uint32_t>(Permission::SYSTEM_ACCESS) |
        static_cast<uint32_t>(Permission::EMERGENCY_STOP);

    m_IsAuthenticated = true;
    LogAction("AUTH_SUCCESS: Creator authenticated");

    std::cout << "[CreatorMode] Authentication successful\n";
    std::cout << "[CreatorMode] Session ID: " << m_CurrentSession.sessionId << "\n";
    std::cout << "[CreatorMode] Expires in 8 hours\n";

    return true;
}

bool CreatorModeManager::IsAuthenticatedLocked() const {
    if (!m_IsAuthenticated || !m_CurrentSession.isActive) {
        return false;
    }
    const auto now = std::chrono::system_clock::now();
    return now < m_CurrentSession.expiresAt;
}

bool CreatorModeManager::IsAuthenticated() const {
    std::lock_guard<std::mutex> lock(m_Mutex);
    return IsAuthenticatedLocked();
}

PermissionLevel CreatorModeManager::GetPermissionLevel() const {
    std::lock_guard<std::mutex> lock(m_Mutex);
    if (!IsAuthenticatedLocked()) {
        return PermissionLevel::VIEWER;
    }
    return m_CurrentSession.level;
}

bool CreatorModeManager::HasPermission(Permission perm) const {
    std::lock_guard<std::mutex> lock(m_Mutex);
    if (!IsAuthenticatedLocked()) {
        return false;
    }
    return (m_CurrentSession.permissions & static_cast<uint32_t>(perm)) != 0;
}

bool CreatorModeManager::RequestPermissionEscalation(Permission perm) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    if (!IsAuthenticatedLocked()) {
        LogAction("ESCALATION_DENIED: Not authenticated");
        return false;
    }

    if (perm == Permission::MODIFY_AI || perm == Permission::ROOT_ACCESS) {
        LogAction("ESCALATION_PENDING: Requires confirmation");
        std::cout << "[CreatorMode] Permission escalation required\n";
        std::cout << "[CreatorMode] Awaiting creator confirmation...\n";
        return false;
    }

    m_CurrentSession.permissions |= static_cast<uint32_t>(perm);
    LogAction("ESCALATION_GRANTED: " + std::to_string(static_cast<uint32_t>(perm)));
    return true;
}

bool CreatorModeManager::EnableAIModificationMode(const std::string& confirmationCode) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    if (!IsAuthenticatedLocked()) {
        LogAction("AI_MOD_DENIED: Not authenticated");
        return false;
    }

    if (!ValidateConfirmationCode(confirmationCode)) {
        LogAction("AI_MOD_DENIED: Invalid or unconfigured confirmation secret");
        return false;
    }

    m_CurrentSession.permissions |= static_cast<uint32_t>(Permission::MODIFY_AI);
    m_CurrentSession.permissions |= static_cast<uint32_t>(Permission::ROOT_ACCESS);
    m_AIModificationEnabled = true;

    LogAction("AI_MODIFICATION_ENABLED");
    std::cout << "[CreatorMode] AI MODIFICATION MODE ENABLED\n";
    std::cout << "[CreatorMode] The AI system can now modify SZM Forge\n";
    std::cout << "[CreatorMode] All changes will be logged and reversible\n";

    return true;
}

void CreatorModeManager::DisableAIModificationMode() {
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_CurrentSession.permissions &= ~static_cast<uint32_t>(Permission::MODIFY_AI);
    m_CurrentSession.permissions &= ~static_cast<uint32_t>(Permission::ROOT_ACCESS);
    m_AIModificationEnabled = false;

    LogAction("AI_MODIFICATION_DISABLED");
    std::cout << "[CreatorMode] AI modification mode disabled\n";
}

void CreatorModeManager::ResetSessionLocked() {
    m_CurrentSession = CreatorSession{};
    m_IsAuthenticated = false;
    m_AIModificationEnabled = false;
}

void CreatorModeManager::EmergencyStop() {
    std::lock_guard<std::mutex> lock(m_Mutex);
    ResetSessionLocked();

    LogAction("EMERGENCY_STOP_ACTIVATED");
    std::cout << "[CreatorMode] EMERGENCY STOP ACTIVATED\n";
    std::cout << "[CreatorMode] All AI operations halted\n";
}

const CreatorSession& CreatorModeManager::GetCurrentSession() const {
    std::lock_guard<std::mutex> lock(m_Mutex);
    return m_CurrentSession;
}

void CreatorModeManager::Logout() {
    std::lock_guard<std::mutex> lock(m_Mutex);
    ResetSessionLocked();

    LogAction("LOGOUT");
    std::cout << "[CreatorMode] Creator logged out\n";
}

bool CreatorModeManager::ValidatePassword(const std::string& password) const {
    if (password.size() < kMinCreatorPasswordLength) {
        return false;
    }
    const std::string hashSpec = !m_ConfiguredPasswordHash.empty()
        ? m_ConfiguredPasswordHash
        : ReadEnvString("SZM_CREATOR_PASSWORD_HASH");
    // Fail closed if no externalized salted password hash is configured
    return VerifyCredentialSpec(password, hashSpec);
}

bool CreatorModeManager::ValidateDeviceBinding(const std::string& deviceId) const {
    if (deviceId.empty()) {
        return false;
    }
    if (!m_AuthorizedDevices.empty()) {
        return m_AuthorizedDevices.find(deviceId) != m_AuthorizedDevices.end();
    }
    const std::string envAllowlist = ReadEnvString("SZM_CREATOR_ALLOWED_DEVICES");
    if (envAllowlist.empty()) {
        // Fail closed when no bound device IDs are registered
        return false;
    }
    std::stringstream ss(envAllowlist);
    std::string token;
    while (std::getline(ss, token, ',')) {
        const size_t first = token.find_first_not_of(" \t");
        if (first == std::string::npos) {
            continue;
        }
        const size_t last = token.find_last_not_of(" \t");
        if (token.substr(first, last - first + 1) == deviceId) {
            return true;
        }
    }
    return false;
}

bool CreatorModeManager::ValidateConfirmationCode(const std::string& confirmationCode) const {
    const std::string hashSpec = !m_ConfiguredConfirmationHash.empty()
        ? m_ConfiguredConfirmationHash
        : ReadEnvString("SZM_AI_MOD_CONFIRM_HASH");
    // Fail closed when no externalized confirmation secret hash is configured
    return VerifyCredentialSpec(confirmationCode, hashSpec);
}

void CreatorModeManager::LogAction(const std::string& action) {
    if (m_ActionLog.size() >= kMaxActionLogEntries) {
        m_ActionLog.erase(m_ActionLog.begin());
    }
    auto timestamp = std::chrono::system_clock::now();
    m_ActionLog.push_back("[" + std::to_string(timestamp.time_since_epoch().count()) + "] " + action);
}

} // namespace SZM::AI
