#pragma once
#include "core/result.h"
#include <cstdint>
#include <string>
#include <vector>
namespace logharbor::stream {
struct Frame { std::string source; std::string payload; bool continuation{false}; bool rotated{false}; std::uint64_t sequence{0}; };
class StreamDecoder {
    std::string pending_;
    std::string current_source_{"stream"};
    std::uint64_t sequence_{0};
public:
    std::vector<Frame> feed(const std::string& chunk);
    std::vector<Frame> finish();
    void reset();
    std::string source() const { return current_source_; }
};
std::vector<std::string> split_rotated_segments(const std::string& text);
std::string reconstruct_messages(const std::vector<Frame>& frames);
}
