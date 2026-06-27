#include "ingest/parsers.h"
#include "core/string_util.h"
#include "core/rule_catalog.h"
#include <fstream>
#include <iostream>
#include <regex>
using namespace logharbor;
int main(int argc, char** argv){ if(argc<2){std::cerr<<"usage: logredact <file>\n";return 2;} std::ifstream f(argv[1]); if(!f){std::cerr<<"cannot open input\n";return 1;} std::string text((std::istreambuf_iterator<char>(f)),{}); text=std::regex_replace(text,std::regex(R"([A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\.[A-Za-z]{2,})"),"[email]"); text=std::regex_replace(text,std::regex(R"((\b\d{1,3}\.){3}\d{1,3}\b)"),"[ip]"); for(const auto& key: core::tokens_for_category("sensitive")){ if(key.size()>32) continue; text=std::regex_replace(text,std::regex("(" + key + R"()=([^ \t,&]+))",std::regex::icase),"$1=[secret]"); } std::cout<<text; return 0; }
