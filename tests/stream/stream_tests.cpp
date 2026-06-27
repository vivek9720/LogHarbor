#include "stream/decoder.h"
#include <cassert>
int main(){ logharbor::stream::StreamDecoder d; auto f=d.feed("==> app.log <==\nfirst\n continuation\n"); assert(f.size()==3); auto s=logharbor::stream::reconstruct_messages(f); assert(s.find("first")!=std::string::npos); return 0; }
