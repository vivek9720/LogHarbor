#include "stream/decoder.h"
#include <cstdint>
#include <string>
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size){ std::string s(reinterpret_cast<const char*>(data),size); logharbor::stream::StreamDecoder d; auto f=d.feed(s); auto tail=d.finish(); f.insert(f.end(),tail.begin(),tail.end()); (void)logharbor::stream::reconstruct_messages(f); (void)logharbor::stream::split_rotated_segments(s); return 0; }
