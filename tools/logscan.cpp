#include "ingest/parsers.h"
#include <fstream>
#include <iostream>
using namespace logharbor;
int main(int argc, char** argv){ if(argc<2){std::cerr<<"usage: logscan <file>\n";return 2;} std::ifstream f(argv[1]); if(!f){std::cerr<<"cannot open input\n";return 1;} std::string text((std::istreambuf_iterator<char>(f)),{}); ingest::ParseOptions opt; opt.source_name=argv[1]; auto events=ingest::parse_mixed_lines(text,opt); for(const auto& e:events) std::cout<<e.to_json()<<"\n"; return 0; }
