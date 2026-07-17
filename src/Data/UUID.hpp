#pragma once

#include <cstdint>
#include <functional>

namespace SZM {

    // 128-bit UUID Structure
    struct UUID {
        uint64_t high;
        uint64_t low;

        static constexpr UUID Null() {
            return UUID{0u, 0u};
        }

        constexpr bool IsNull() const {
            return high == 0u && low == 0u;
        }

        bool operator==(const UUID& other) const {
            return high == other.high && low == other.low;
        }

        bool operator!=(const UUID& other) const {
            return !(*this == other);
        }

        bool operator<(const UUID& other) const {
            return (high < other.high) || (high == other.high && low < other.low);
        }
    };

} // namespace SZM

// Custom Hash for unordered_map compatibility
namespace std {
    template <>
    struct hash<SZM::UUID> {
        size_t operator()(const SZM::UUID& uuid) const {
            return hash<uint64_t>()(uuid.high) ^ (hash<uint64_t>()(uuid.low) << 1);
        }
    };
}
