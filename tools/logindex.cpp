#include "index/store.h"
#include "ingest/parsers.h"
#include <fstream>
#include <iostream>
using namespace logharbor;
int main(int argc, char** argv){ if(argc<3){std::cerr<<"usage: logindex <store> <file> [file...]\n";return 2;} index::RecordStore store; for(int i=2;i<argc;++i){ std::ifstream f(argv[i]); if(!f){std::cerr<<"skip unreadable "<<argv[i]<<"\n";continue;} std::string text((std::istreambuf_iterator<char>(f)),{}); ingest::ParseOptions opt; opt.source_name=argv[i]; for(const auto& e:ingest::parse_mixed_lines(text,opt)) store.append(e); } auto r=store.save(argv[1]); if(!r){std::cerr<<r.error().describe()<<"\n";return 1;} auto s=store.stats(); std::cout<<"records="<<s.records<<" bytes="<<s.bytes<<"\n"; return 0; }
