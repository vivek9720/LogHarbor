#include "core/checksum.h"
#include <iomanip>
#include <sstream>
namespace logharbor::core {
void Crc32::update(const void* data, std::size_t size) {
    const auto* p = static_cast<const std::uint8_t*>(data);
    for (std::size_t i = 0; i < size; ++i) { value_ ^= p[i]; for (int j = 0; j < 8; ++j) value_ = (value_ >> 1) ^ (0xedb88320u & (0u - (value_ & 1u))); }
}
std::uint32_t crc32(const void* data, std::size_t size) { Crc32 c; c.update(data, size); return c.value(); }
std::uint64_t fnv1a64(const void* data, std::size_t size) { const auto* p = static_cast<const std::uint8_t*>(data); std::uint64_t h = 14695981039346656037ull; for (std::size_t i=0;i<size;++i){ h ^= p[i]; h *= 1099511628211ull; } return h; }
std::uint64_t fnv1a64(const std::string& s) { return fnv1a64(s.data(), s.size()); }
std::string stable_id(const std::string& source, const std::string& payload, std::uint64_t ordinal) { std::ostringstream in; in << source << '\n' << ordinal << '\n' << payload; std::ostringstream out; out << std::hex << std::setw(16) << std::setfill('0') << fnv1a64(in.str()); return out.str(); }
}
