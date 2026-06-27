#pragma once
#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace logharbor::core {
std::string trim(std::string s);
std::string lower(std::string s);
std::string upper(std::string s);
bool starts_with(const std::string& s, const std::string& prefix);
bool ends_with(const std::string& s, const std::string& suffix);
std::vector<std::string> split(const std::string& s, char delim, bool keep_empty = true);
std::string join(const std::vector<std::string>& parts, const std::string& delim);
std::string replace_all(std::string s, const std::string& needle, const std::string& repl);
std::string collapse_spaces(const std::string& s);
bool parse_i64(const std::string& s, std::int64_t& out);
bool parse_u64(const std::string& s, std::uint64_t& out);
std::string json_escape(const std::string& s);
std::string csv_escape(const std::string& s);
std::string unquote(std::string s);
bool contains_case_insensitive(const std::string& haystack, const std::string& needle);
std::vector<std::string> tokenize_words(const std::string& s);
std::map<std::string, std::string> parse_key_values(const std::string& s);
std::string normalize_field_name(const std::string& s);
std::string percent_decode(const std::string& s);
} // namespace logharbor::core
