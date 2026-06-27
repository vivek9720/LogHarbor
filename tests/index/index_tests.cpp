#include "index/store.h"
#include <cassert>
#include <cstdio>
int main(){ logharbor::ingest::Event e; e.id="a"; e.message="hello"; e.fields["status"]="200"; auto b=logharbor::index::serialize_event(e); auto d=logharbor::index::deserialize_event(b); assert(d); assert(d.value().fields["status"]=="200"); logharbor::index::RecordStore s; s.append(e); assert(s.erase_id("a")); assert(s.stats().tombstones==1); return 0; }
