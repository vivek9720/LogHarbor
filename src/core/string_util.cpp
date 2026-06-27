#include "core/string_util.h"
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <sstream>

namespace logharbor::core {
std::string trim(std::string s) {
    auto b = std::find_if(s.begin(), s.end(), [](unsigned char c){ return !std::isspace(c); });
    auto e = std::find_if(s.rbegin(), s.rend(), [](unsigned char c){ return !std::isspace(c); }).base();
    if (b >= e) return {};
    return std::string(b, e);
}
std::string lower(std::string s) { std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return static_cast<char>(std::tolower(c)); }); return s; }
std::string upper(std::string s) { std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return static_cast<char>(std::toupper(c)); }); return s; }
bool starts_with(const std::string& s, const std::string& prefix) { return s.size() >= prefix.size() && std::equal(prefix.begin(), prefix.end(), s.begin()); }
bool ends_with(const std::string& s, const std::string& suffix) { return s.size() >= suffix.size() && std::equal(suffix.rbegin(), suffix.rend(), s.rbegin()); }
std::vector<std::string> split(const std::string& s, char delim, bool keep_empty) {
    std::vector<std::string> out; std::string cur;
    for (char c : s) { if (c == delim) { if (keep_empty || !cur.empty()) out.push_back(cur); cur.clear(); } else cur.push_back(c); }
    if (keep_empty || !cur.empty()) out.push_back(cur);
    return out;
}
std::string join(const std::vector<std::string>& parts, const std::string& delim) {
    std::ostringstream out; for (std::size_t i = 0; i < parts.size(); ++i) { if (i) out << delim; out << parts[i]; } return out.str();
}
std::string replace_all(std::string s, const std::string& needle, const std::string& repl) {
    if (needle.empty()) return s; std::size_t p = 0;
    while ((p = s.find(needle, p)) != std::string::npos) { s.replace(p, needle.size(), repl); p += repl.size(); }
    return s;
}
std::string collapse_spaces(const std::string& s) {
    std::string out; bool sp = false;
    for (unsigned char c : s) { if (std::isspace(c)) { if (!sp) out.push_back(' '); sp = true; } else { out.push_back(static_cast<char>(c)); sp = false; } }
    return trim(out);
}
bool parse_i64(const std::string& s, std::int64_t& out) {
    try { std::string t = trim(s); std::size_t n = 0; long long v = std::stoll(t, &n, 10); if (n != t.size()) return false; out = v; return true; } catch (...) { return false; }
}
bool parse_u64(const std::string& s, std::uint64_t& out) {
    try { std::string t = trim(s); if (!t.empty() && t[0] == '-') return false; std::size_t n = 0; unsigned long long v = std::stoull(t, &n, 10); if (n != t.size()) return false; out = v; return true; } catch (...) { return false; }
}
std::string json_escape(const std::string& s) {
    std::ostringstream out;
    for (unsigned char c : s) {
        if (c == '\\') out << "\\\\";
        else if (c == '"') out << "\\\"";
        else if (c == '\n') out << "\\n";
        else if (c == '\r') out << "\\r";
        else if (c == '\t') out << "\\t";
        else if (c < 0x20) out << "\\u" << std::hex << std::setw(4) << std::setfill('0') << static_cast<int>(c);
        else out << static_cast<char>(c);
    }
    return out.str();
}
std::string csv_escape(const std::string& s) {
    if (s.find_first_of(",\"\n\r") == std::string::npos) return s;
    return "\"" + replace_all(s, "\"", "\"\"") + "\"";
}
std::string unquote(std::string s) {
    s = trim(std::move(s));
    if (s.size() >= 2 && ((s.front() == '"' && s.back() == '"') || (s.front() == '\'' && s.back() == '\''))) return s.substr(1, s.size() - 2);
    return s;
}
bool contains_case_insensitive(const std::string& haystack, const std::string& needle) { return lower(haystack).find(lower(needle)) != std::string::npos; }
std::vector<std::string> tokenize_words(const std::string& s) {
    std::vector<std::string> words; std::string cur;
    for (unsigned char c : s) { if (std::isalnum(c) || c == '_' || c == '-' || c == '.') cur.push_back(static_cast<char>(c)); else if (!cur.empty()) { words.push_back(cur); cur.clear(); } }
    if (!cur.empty()) words.push_back(cur);
    return words;
}
std::string normalize_field_name(const std::string& s) {
    std::string out;
    for (unsigned char c : lower(trim(s))) { if (std::isalnum(c)) out.push_back(static_cast<char>(c)); else if (c == '_' || c == '-' || c == '.') out.push_back('_'); }
    while (out.find("__") != std::string::npos) out = replace_all(out, "__", "_");
    if (!out.empty() && out.front() == '_') out.erase(out.begin());
    if (!out.empty() && out.back() == '_') out.pop_back();
    return out;
}
std::map<std::string, std::string> parse_key_values(const std::string& s) {
    std::map<std::string, std::string> out;
    for (const auto& p : split(s, ' ', false)) { auto eq = p.find('='); if (eq != std::string::npos) out[normalize_field_name(p.substr(0, eq))] = unquote(p.substr(eq + 1)); }
    return out;
}
std::string percent_decode(const std::string& s) {
    std::string out;
    for (std::size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '%' && i + 2 < s.size() && std::isxdigit(static_cast<unsigned char>(s[i+1])) && std::isxdigit(static_cast<unsigned char>(s[i+2]))) {
            int v = std::stoi(s.substr(i + 1, 2), nullptr, 16); out.push_back(static_cast<char>(v)); i += 2;
        } else out.push_back(s[i] == '+' ? ' ' : s[i]);
    }
    return out;
}
} // namespace logharbor::core
