#include "ingest/parsers.h"
#include <cstdint>
#include <string>
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size){ std::string s(reinterpret_cast<const char*>(data),size); logharbor::ingest::ParseOptions o; (void)logharbor::ingest::parse_mixed_lines(s,o); (void)logharbor::ingest::parse_csv_events(s,o); (void)logharbor::ingest::parse_ini_events(s,o); return 0; }
