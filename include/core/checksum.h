#pragma once
#include <cstdint>
#include <string>
#include <vector>
namespace logharbor::core {
class Crc32 {
    std::uint32_t value_{0xffffffffu};
public:
    void update(const void* data, std::size_t size);
    void update(const std::string& s) { update(s.data(), s.size()); }
    std::uint32_t value() const { return value_ ^ 0xffffffffu; }
};
std::uint32_t crc32(const void* data, std::size_t size);
std::uint64_t fnv1a64(const void* data, std::size_t size);
std::uint64_t fnv1a64(const std::string& s);
std::string stable_id(const std::string& source, const std::string& payload, std::uint64_t ordinal);
}
