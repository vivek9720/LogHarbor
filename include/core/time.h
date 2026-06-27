#pragma once
#include "core/result.h"
#include <chrono>
#include <cstdint>
#include <string>
namespace logharbor::core {
using TimePoint = std::chrono::system_clock::time_point;
Result<TimePoint> parse_timestamp(const std::string& text);
Result<TimePoint> parse_rfc3339(const std::string& text);
Result<TimePoint> parse_syslog_timestamp(const std::string& text, int default_year = 1970);
Result<TimePoint> parse_web_timestamp(const std::string& text);
std::string format_rfc3339(TimePoint tp);
std::int64_t unix_seconds(TimePoint tp);
TimePoint from_unix_seconds(std::int64_t v);
std::int64_t days_from_civil(int y, unsigned m, unsigned d);
bool civil_from_days(std::int64_t z, int& y, unsigned& m, unsigned& d);
}
