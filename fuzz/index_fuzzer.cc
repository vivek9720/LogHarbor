#include "index/store.h"
#include <cstdint>
#include <string>
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size){ std::vector<std::uint8_t> b(data,data+size); (void)logharbor::index::deserialize_event(b); std::size_t p=0; while(p<b.size()) { auto r=logharbor::index::decode_varint(b,p); if(!r) break; } return 0; }
