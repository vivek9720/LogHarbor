#include "query/query.h"
#include <cassert>
int main(){ logharbor::ingest::Event e; e.message="disk full"; e.severity="error"; e.fields["status"]="503"; assert(logharbor::query::matches(e,"severity=error and status>=500")); assert(!logharbor::query::matches(e,"severity=info")); return 0; }
