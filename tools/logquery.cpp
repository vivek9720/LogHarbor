#include "index/store.h"
#include "query/query.h"
#include <iostream>
using namespace logharbor;
int main(int argc, char** argv){ if(argc<3){std::cerr<<"usage: logquery <store> <query>\n";return 2;} index::RecordStore store; auto r=store.load(argv[1]); if(!r){std::cerr<<r.error().describe()<<"\n";return 1;} auto events=query::filter(store.live_events(), argv[2]); for(const auto& e:events) std::cout<<e.to_json()<<"\n"; return 0; }
