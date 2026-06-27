#include "query/query.h"
#include <cstdint>
#include <string>
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size){ std::string q(reinterpret_cast<const char*>(data),size); logharbor::ingest::Event e; e.message=q; e.severity="info"; e.fields["status"]="200"; (void)logharbor::query::lex(q); (void)logharbor::query::matches(e,q); return 0; }
