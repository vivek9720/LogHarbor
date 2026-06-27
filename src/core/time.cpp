#include "core/time.h"
#include "core/string_util.h"
#include <array>
#include <iomanip>
#include <sstream>

namespace logharbor::core {
namespace {
bool read_int(const std::string& s, std::size_t pos, std::size_t n, int& out) {
    if (pos + n > s.size()) return false; int v = 0;
    for (std::size_t i = 0; i < n; ++i) { char c = s[pos + i]; if (c < '0' || c > '9') return false; v = v * 10 + c - '0'; }
    out = v; return true;
}
bool month_name(const std::string& s, unsigned& month) {
    static const std::array<std::string, 12> months{"jan","feb","mar","apr","may","jun","jul","aug","sep","oct","nov","dec"};
    std::string t = lower(s.substr(0, 3));
    for (std::size_t i = 0; i < months.size(); ++i) if (months[i] == t) { month = static_cast<unsigned>(i + 1); return true; }
    return false;
}
TimePoint compose_utc(int y, unsigned m, unsigned d, int hh, int mm, int ss) {
    return from_unix_seconds(days_from_civil(y, m, d) * 86400 + hh * 3600 + mm * 60 + ss);
}
}
std::int64_t days_from_civil(int y, unsigned m, unsigned d) {
    y -= m <= 2; const int era = (y >= 0 ? y : y - 399) / 400; const unsigned yoe = static_cast<unsigned>(y - era * 400);
    const unsigned doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1; const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return era * 146097 + static_cast<int>(doe) - 719468;
}
bool civil_from_days(std::int64_t z, int& y, unsigned& m, unsigned& d) {
    z += 719468; const std::int64_t era = (z >= 0 ? z : z - 146096) / 146097; const unsigned doe = static_cast<unsigned>(z - era * 146097);
    const unsigned yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365; y = static_cast<int>(yoe) + static_cast<int>(era) * 400;
    const unsigned doy = doe - (365 * yoe + yoe / 4 - yoe / 100); const unsigned mp = (5 * doy + 2) / 153; d = doy - (153 * mp + 2) / 5 + 1; m = mp + (mp < 10 ? 3 : -9); y += (m <= 2); return true;
}
TimePoint from_unix_seconds(std::int64_t v) { return TimePoint(std::chrono::seconds(v)); }
std::int64_t unix_seconds(TimePoint tp) { return std::chrono::duration_cast<std::chrono::seconds>(tp.time_since_epoch()).count(); }
Result<TimePoint> parse_rfc3339(const std::string& text) {
    std::string s = trim(text); int y=0, mo=0, d=0, hh=0, mi=0, ss=0;
    if (s.size() < 19 || !read_int(s,0,4,y) || s[4] != '-' || !read_int(s,5,2,mo) || s[7] != '-' || !read_int(s,8,2,d) || (s[10] != 'T' && s[10] != ' ') || !read_int(s,11,2,hh) || s[13] != ':' || !read_int(s,14,2,mi) || s[16] != ':' || !read_int(s,17,2,ss)) return Result<TimePoint>::failure(make_error(ErrorCode::invalid_format, "invalid timestamp"));
    int off = 0; std::size_t pos = 19; if (pos < s.size() && s[pos] == '.') while (++pos < s.size() && std::isdigit(static_cast<unsigned char>(s[pos]))) {}
    if (pos < s.size() && s[pos] != 'Z') { if (pos + 5 <= s.size() && (s[pos] == '+' || s[pos] == '-')) { int oh=0, om=0; read_int(s,pos+1,2,oh); read_int(s,pos+4,2,om); off = (oh*3600+om*60) * (s[pos]=='-' ? -1 : 1); } }
    return Result<TimePoint>::success(compose_utc(y, static_cast<unsigned>(mo), static_cast<unsigned>(d), hh, mi, ss) - std::chrono::seconds(off));
}
Result<TimePoint> parse_syslog_timestamp(const std::string& text, int default_year) {
    auto parts = split(collapse_spaces(text), ' ', false); if (parts.size() < 3) return Result<TimePoint>::failure(make_error(ErrorCode::invalid_format, "bad syslog timestamp"));
    unsigned mon = 0; if (!month_name(parts[0], mon)) return Result<TimePoint>::failure(make_error(ErrorCode::invalid_format, "bad syslog month"));
    std::int64_t day=0, hh=0, mi=0, ss=0; auto clock = split(parts[2], ':'); if (clock.size() != 3 || !parse_i64(parts[1], day) || !parse_i64(clock[0], hh) || !parse_i64(clock[1], mi) || !parse_i64(clock[2], ss)) return Result<TimePoint>::failure(make_error(ErrorCode::invalid_format, "bad syslog clock"));
    return Result<TimePoint>::success(compose_utc(default_year, mon, static_cast<unsigned>(day), static_cast<int>(hh), static_cast<int>(mi), static_cast<int>(ss)));
}
Result<TimePoint> parse_web_timestamp(const std::string& text) {
    std::string s = trim(text); if (!s.empty() && s.front() == '[') s.erase(s.begin()); if (!s.empty() && s.back() == ']') s.pop_back();
    auto sp = s.find(' '); auto main = sp == std::string::npos ? s : s.substr(0, sp); auto parts = split(main, ':'); if (parts.size() < 4) return Result<TimePoint>::failure(make_error(ErrorCode::invalid_format, "bad web timestamp"));
    auto date = split(parts[0], '/'); unsigned mon = 0; std::int64_t day=0, year=0, hh=0, mi=0, ss=0;
    if (date.size()!=3 || !month_name(date[1], mon) || !parse_i64(date[0], day) || !parse_i64(date[2], year) || !parse_i64(parts[1], hh) || !parse_i64(parts[2], mi) || !parse_i64(parts[3], ss)) return Result<TimePoint>::failure(make_error(ErrorCode::invalid_format, "bad web date"));
    return Result<TimePoint>::success(compose_utc(static_cast<int>(year), mon, static_cast<unsigned>(day), static_cast<int>(hh), static_cast<int>(mi), static_cast<int>(ss)));
}
Result<TimePoint> parse_timestamp(const std::string& text) { auto a = parse_rfc3339(text); if (a) return a; auto b = parse_web_timestamp(text); if (b) return b; return parse_syslog_timestamp(text, 1970); }
std::string format_rfc3339(TimePoint tp) {
    auto sec = unix_seconds(tp); std::int64_t days = sec / 86400; std::int64_t rem = sec % 86400; if (rem < 0) { rem += 86400; --days; }
    int y=0; unsigned m=0,d=0; civil_from_days(days,y,m,d); std::ostringstream out; out << std::setfill('0') << std::setw(4) << y << '-' << std::setw(2) << m << '-' << std::setw(2) << d << 'T' << std::setw(2) << (rem/3600) << ':' << std::setw(2) << ((rem%3600)/60) << ':' << std::setw(2) << (rem%60) << 'Z'; return out.str();
}
} // namespace logharbor::core
