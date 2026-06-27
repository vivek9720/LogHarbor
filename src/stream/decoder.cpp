#include "stream/decoder.h"
#include "core/string_util.h"
#include <sstream>
namespace logharbor::stream {
void StreamDecoder::reset(){ pending_.clear(); current_source_="stream"; sequence_=0; }
std::vector<Frame> StreamDecoder::feed(const std::string& chunk) {
    std::vector<Frame> out; pending_ += chunk; std::size_t pos = 0;
    while (true) {
        auto nl = pending_.find('\n', pos); if (nl == std::string::npos) break;
        std::string line = pending_.substr(pos, nl - pos); if (!line.empty() && line.back() == '\r') line.pop_back(); pos = nl + 1;
        if (core::starts_with(line, "==> ") && core::ends_with(line, " <==")) { current_source_ = line.substr(4, line.size() - 8); out.push_back({current_source_, "", false, true, sequence_++}); continue; }
        bool cont = !line.empty() && (line[0] == ' ' || line[0] == '\t' || core::starts_with(line, "..."));
        if (core::starts_with(line, "LEN ")) { auto sp=line.find(' ',4); if(sp!=std::string::npos){ std::uint64_t n=0; if(core::parse_u64(line.substr(4,sp-4),n) && sp+1+n<=line.size()) line=line.substr(sp+1, static_cast<std::size_t>(n)); } }
        out.push_back({current_source_, line, cont, false, sequence_++});
    }
    pending_.erase(0, pos);
    return out;
}
std::vector<Frame> StreamDecoder::finish(){ std::vector<Frame> out; if(!pending_.empty()){ out.push_back({current_source_, pending_, false, false, sequence_++}); pending_.clear(); } return out; }
std::vector<std::string> split_rotated_segments(const std::string& text){ std::vector<std::string> out; std::istringstream in(text); std::string line, cur; while(std::getline(in,line)){ if(core::starts_with(line,"==> ")&&!cur.empty()){out.push_back(cur);cur.clear();} cur+=line; cur+='\n'; } if(!cur.empty())out.push_back(cur); return out; }
std::string reconstruct_messages(const std::vector<Frame>& frames){ std::ostringstream out; bool have=false; for(const auto& f:frames){ if(f.rotated)continue; if(f.continuation&&have) out << "\\n" << f.payload; else { if(have) out << '\n'; out << f.payload; have=true; } } return out.str(); }
}
