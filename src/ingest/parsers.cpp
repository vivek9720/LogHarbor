#include "ingest/parsers.h"
#include "core/checksum.h"
#include "core/string_util.h"
#include <cctype>
#include <sstream>

namespace logharbor::ingest {
namespace {
Event base(EventKind kind, const std::string& line, const ParseOptions& o, std::uint64_t ordinal) { Event e; e.kind=kind; e.source=o.source_name; e.ordinal=ordinal; e.id=core::stable_id(o.source_name,line,ordinal); return e; }
std::vector<std::string> words(const std::string& s) { return core::split(core::collapse_spaces(s), ' ', false); }
}
core::Result<Event> parse_syslog_line(const std::string& line, const ParseOptions& o, std::uint64_t ordinal) {
    auto w = words(line); if (w.size() < 5) return core::Result<Event>::failure(core::make_error(core::ErrorCode::invalid_format,"short syslog line"));
    Event e = base(EventKind::syslog,line,o,ordinal); auto ts = core::parse_syslog_timestamp(w[0]+" "+w[1]+" "+w[2], o.default_year); if (ts) e.timestamp=ts.value();
    e.host = w[3]; std::size_t msgpos = line.find(w[4]); std::string tag = w[4]; if (!tag.empty() && tag.back()==':') tag.pop_back(); auto bracket = tag.find('['); e.service = bracket==std::string::npos ? tag : tag.substr(0, bracket);
    e.message = msgpos==std::string::npos ? "" : core::trim(line.substr(msgpos + w[4].size())); e.severity = core::contains_case_insensitive(e.message,"error") ? "error" : (core::contains_case_insensitive(e.message,"warn") ? "warning" : "info");
    for (auto& kv : core::parse_key_values(e.message)) e.fields[kv.first]=kv.second;
    return core::Result<Event>::success(e);
}
core::Result<Event> parse_web_access_line(const std::string& line, const ParseOptions& o, std::uint64_t ordinal) {
    Event e = base(EventKind::web_access,line,o,ordinal); std::size_t p = 0; auto next=[&](char d){ auto q=line.find(d,p); std::string r=q==std::string::npos?line.substr(p):line.substr(p,q-p); p=q==std::string::npos?line.size():q+1; return r; };
    e.host = next(' '); next(' '); std::string user = next(' '); auto lb=line.find('[',p); auto rb=line.find(']',lb); if(lb==std::string::npos||rb==std::string::npos) return core::Result<Event>::failure(core::make_error(core::ErrorCode::invalid_format,"missing web timestamp"));
    auto ts = core::parse_web_timestamp(line.substr(lb, rb-lb+1)); if(ts) e.timestamp=ts.value(); p=rb+2; if(p>=line.size()||line[p-1]!='"') return core::Result<Event>::failure(core::make_error(core::ErrorCode::invalid_format,"missing request"));
    auto rqend=line.find('"',p); if(rqend==std::string::npos) return core::Result<Event>::failure(core::make_error(core::ErrorCode::invalid_format,"unterminated request")); std::string req=line.substr(p,rqend-p); p=rqend+2;
    auto rest=words(line.substr(p)); if(rest.size()>=2){ e.fields["status"]=rest[0]; e.fields["bytes"]=rest[1]; std::int64_t status=0; if(core::parse_i64(rest[0],status)) e.severity=status>=500?"error":(status>=400?"warning":"info"); }
    auto parts=core::split(req,' ',false); if(parts.size()>0)e.fields["method"]=parts[0]; if(parts.size()>1){e.fields["path"]=core::percent_decode(parts[1]); e.message=parts[0]+" "+e.fields["path"];} if(parts.size()>2)e.fields["protocol"]=parts[2]; if(user!="-") e.fields["user"]=user;
    return core::Result<Event>::success(e);
}
std::map<std::string,std::string> parse_flat_json_object(const std::string& line) {
    std::map<std::string,std::string> out; std::string s=core::trim(line); if(s.size()<2||s.front()!='{'||s.back()!='}') return out; std::size_t i=1;
    while(i+1<s.size()){ while(i<s.size()&&(std::isspace(static_cast<unsigned char>(s[i]))||s[i]==','))++i; if(i>=s.size()||s[i]!='"')break; std::size_t k=++i; while(i<s.size()&&s[i]!='"') { if(s[i]=='\\'&&i+1<s.size())++i; ++i; } std::string key=s.substr(k,i-k); ++i; while(i<s.size()&&(std::isspace(static_cast<unsigned char>(s[i]))||s[i]==':'))++i; std::string val; if(i<s.size()&&s[i]=='"'){ std::size_t v=++i; while(i<s.size()&&s[i]!='"'){ if(s[i]=='\\'&&i+1<s.size())++i; ++i;} val=s.substr(v,i-v); ++i; } else { std::size_t v=i; while(i<s.size()&&s[i]!=','&&s[i]!='}')++i; val=core::trim(s.substr(v,i-v)); } out[core::normalize_field_name(key)]=val; }
    return out;
}
core::Result<Event> parse_json_line(const std::string& line, const ParseOptions& o, std::uint64_t ordinal) {
    auto obj=parse_flat_json_object(line); if(obj.empty()) return core::Result<Event>::failure(core::make_error(core::ErrorCode::invalid_format,"not a flat json object")); Event e=base(EventKind::json,line,o,ordinal); e.fields=obj;
    for(auto key:{"timestamp","time","ts","date"}){ auto it=obj.find(key); if(it!=obj.end()){ auto t=core::parse_timestamp(it->second); if(t)e.timestamp=t.value(); } }
    if(obj.count("host"))e.host=obj["host"]; if(obj.count("service"))e.service=obj["service"]; if(obj.count("level"))e.severity=core::lower(obj["level"]); if(obj.count("severity"))e.severity=core::lower(obj["severity"]); if(obj.count("message"))e.message=obj["message"]; else if(obj.count("msg"))e.message=obj["msg"];
    return core::Result<Event>::success(e);
}
std::vector<std::string> parse_csv_record(const std::string& line) {
    std::vector<std::string> out; std::string cur; bool q=false; for(std::size_t i=0;i<line.size();++i){ char c=line[i]; if(q&&c=='"'&&i+1<line.size()&&line[i+1]=='"'){cur.push_back('"');++i;} else if(c=='"') q=!q; else if(c==','&&!q){out.push_back(cur);cur.clear();} else cur.push_back(c);} out.push_back(cur); return out;
}
core::Result<std::vector<Event>> parse_csv_events(const std::string& text, const ParseOptions& o) {
    std::istringstream in(text); std::string line; if(!std::getline(in,line)) return core::Result<std::vector<Event>>::success({}); auto header=parse_csv_record(line); std::vector<Event> events; std::uint64_t ord=0;
    while(std::getline(in,line)){ if(core::trim(line).empty())continue; auto cells=parse_csv_record(line); Event e=base(EventKind::csv,line,o,ord++); for(std::size_t i=0;i<header.size()&&i<cells.size();++i)e.set_field(header[i],cells[i]); auto t=e.field("timestamp"); if(!t.empty()){auto r=core::parse_timestamp(t); if(r)e.timestamp=r.value();} events.push_back(e);}
    return core::Result<std::vector<Event>>::success(events);
}
core::Result<std::vector<Event>> parse_ini_events(const std::string& text, const ParseOptions& o) {
    std::istringstream in(text); std::string line, section; std::vector<Event> events; std::uint64_t ord=0;
    while(std::getline(in,line)){ std::string t=core::trim(line); if(t.empty()||t[0]=='#'||t[0]==';')continue; if(t.front()=='['&&t.back()==']'){section=t.substr(1,t.size()-2);continue;} auto eq=t.find('='); if(eq==std::string::npos)continue; Event e=base(EventKind::ini,line,o,ord++); e.service=section; e.fields["key"]=core::trim(t.substr(0,eq)); e.fields["value"]=core::trim(t.substr(eq+1)); e.message=section+"."+e.fields["key"]+"="+e.fields["value"]; events.push_back(e);}
    return core::Result<std::vector<Event>>::success(events);
}
std::vector<Event> parse_mixed_lines(const std::string& text, const ParseOptions& o) {
    std::istringstream in(text); std::string line; std::vector<Event> events; std::uint64_t ord=0;
    while(std::getline(in,line)){ if(core::trim(line).empty())continue; core::Result<Event> r = parse_json_line(line,o,ord); if(!r) r=parse_web_access_line(line,o,ord); if(!r) r=parse_syslog_line(line,o,ord); if(r){ events.push_back(r.value()); ++ord; } }
    return events;
}
}
