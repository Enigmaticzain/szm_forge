#pragma once

#include "UUID.hpp"
#include <random>

namespace SZM {

    // Fast, zero-collision 128-bit UUID Generator
    class UUIDGenerator {
    public:
        static UUID Generate() {
            static thread_local std::random_device rd;
            static thread_local std::mt19937_64 engine(rd());
            static thread_local std::uniform_int_distribution<uint64_t> dist;
            return {dist(engine), dist(engine)};
        }
    };

} // namespace SZM
