#pragma once
#include "core/time.h"
#include <map>
#include <string>
#include <vector>
namespace logharbor::ingest {
enum class EventKind { unknown, syslog, web_access, json, csv, ini, stream };
struct Event {
    std::string id;
    EventKind kind{EventKind::unknown};
    core::TimePoint timestamp{};
    std::string source;
    std::string host;
    std::string service;
    std::string severity;
    std::string message;
    std::map<std::string, std::string> fields;
    std::uint64_t ordinal{0};
    bool tombstone{false};
    std::string field(const std::string& name) const;
    void set_field(std::string name, std::string value);
    std::string to_json() const;
    std::string to_csv_row(const std::vector<std::string>& columns) const;
};
std::string event_kind_name(EventKind kind);
EventKind event_kind_from_name(const std::string& name);
std::vector<std::string> default_export_columns();
std::string event_summary(const Event& event);
}
