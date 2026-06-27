#pragma once
#include "core/result.h"
#include "ingest/event.h"
#include <cstdint>
#include <map>
#include <string>
#include <vector>
namespace logharbor::index {
struct StoreStats { std::size_t records{0}; std::size_t tombstones{0}; std::size_t bytes{0}; std::size_t strings{0}; };
std::vector<std::uint8_t> encode_varint(std::uint64_t v);
core::Result<std::uint64_t> decode_varint(const std::vector<std::uint8_t>& bytes, std::size_t& pos);
std::vector<std::uint8_t> serialize_event(const ingest::Event& e);
core::Result<ingest::Event> deserialize_event(const std::vector<std::uint8_t>& bytes);
class RecordStore {
    std::vector<ingest::Event> events_;
public:
    void append(const ingest::Event& e);
    bool erase_id(const std::string& id);
    std::vector<ingest::Event> live_events() const;
    const std::vector<ingest::Event>& all_events() const { return events_; }
    StoreStats stats() const;
    core::Result<void> save(const std::string& path) const;
    core::Result<void> load(const std::string& path);
    core::Result<void> compact(const std::string& path) const;
};
}
