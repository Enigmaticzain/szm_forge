#pragma once

#include <array>
#include <chrono>
#include <cstdint>
#include <iomanip>
#include <random>
#include <sstream>
#include <string>

namespace SZM::WebAPI {

inline std::string GenerateUuidV4() {
    thread_local std::mt19937_64 generator(
        static_cast<std::mt19937_64::result_type>(
            std::chrono::steady_clock::now().time_since_epoch().count()
        )
    );
    std::uniform_int_distribution<uint32_t> distribution(0, 255);

    std::array<uint8_t, 16> bytes{};
    for (auto& byte : bytes) {
        byte = static_cast<uint8_t>(distribution(generator));
    }

    bytes[6] = static_cast<uint8_t>((bytes[6] & 0x0F) | 0x40);
    bytes[8] = static_cast<uint8_t>((bytes[8] & 0x3F) | 0x80);

    std::ostringstream stream;
    stream << std::hex << std::setfill('0');

    for (std::size_t index = 0; index < bytes.size(); ++index) {
        stream << std::setw(2) << static_cast<int>(bytes[index]);
        if (index == 3 || index == 5 || index == 7 || index == 9) {
            stream << '-';
        }
    }

    return stream.str();
}

inline std::string MakeJobId() {
    return "job_" + GenerateUuidV4();
}

} // namespace SZM::WebAPI
