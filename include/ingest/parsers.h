#pragma once
#include "core/result.h"
#include "ingest/event.h"
#include <map>
#include <string>
#include <vector>
namespace logharbor::ingest {
struct ParseOptions { std::string source_name{"stdin"}; int default_year{1970}; bool strict_json{false}; };
core::Result<Event> parse_syslog_line(const std::string& line, const ParseOptions& options, std::uint64_t ordinal = 0);
core::Result<Event> parse_web_access_line(const std::string& line, const ParseOptions& options, std::uint64_t ordinal = 0);
core::Result<Event> parse_json_line(const std::string& line, const ParseOptions& options, std::uint64_t ordinal = 0);
core::Result<std::vector<Event>> parse_csv_events(const std::string& text, const ParseOptions& options);
core::Result<std::vector<Event>> parse_ini_events(const std::string& text, const ParseOptions& options);
std::vector<Event> parse_mixed_lines(const std::string& text, const ParseOptions& options);
std::vector<std::string> parse_csv_record(const std::string& line);
std::map<std::string, std::string> parse_flat_json_object(const std::string& line);
}
