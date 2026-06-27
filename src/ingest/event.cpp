#include "ingest/event.h"
#include "core/string_util.h"
#include <sstream>
namespace logharbor::ingest {
std::string Event::field(const std::string& name) const {
    std::string key = core::normalize_field_name(name);
    if (key=="id") return id; if (key=="kind") return event_kind_name(kind); if (key=="timestamp"||key=="time") return core::format_rfc3339(timestamp); if (key=="source") return source; if (key=="host") return host; if (key=="service") return service; if (key=="severity") return severity; if (key=="message") return message;
    auto it = fields.find(key); return it == fields.end() ? std::string{} : it->second;
}
void Event::set_field(std::string name, std::string value) {
    std::string key = core::normalize_field_name(name); if (key.empty()) return;
    if (key=="host") host = std::move(value); else if (key=="service") service = std::move(value); else if (key=="severity") severity = std::move(value); else if (key=="message") message = std::move(value); else if (key=="source") source = std::move(value); else fields[key] = std::move(value);
}
std::string Event::to_json() const {
    std::ostringstream out; out << "{\"id\":\"" << core::json_escape(id) << "\",\"kind\":\"" << event_kind_name(kind) << "\",\"timestamp\":\"" << core::format_rfc3339(timestamp) << "\",\"source\":\"" << core::json_escape(source) << "\",\"host\":\"" << core::json_escape(host) << "\",\"service\":\"" << core::json_escape(service) << "\",\"severity\":\"" << core::json_escape(severity) << "\",\"message\":\"" << core::json_escape(message) << "\",\"fields\":{";
    bool first=true; for (const auto& kv: fields) { if (!first) out << ','; first=false; out << "\"" << core::json_escape(kv.first) << "\":\"" << core::json_escape(kv.second) << "\""; }
    out << "}}"; return out.str();
}
std::string Event::to_csv_row(const std::vector<std::string>& columns) const { std::vector<std::string> cells; for (const auto& c: columns) cells.push_back(core::csv_escape(field(c))); return core::join(cells, ","); }
std::string event_kind_name(EventKind k) { switch(k){case EventKind::syslog:return"syslog";case EventKind::web_access:return"web_access";case EventKind::json:return"json";case EventKind::csv:return"csv";case EventKind::ini:return"ini";case EventKind::stream:return"stream";default:return"unknown";} }
EventKind event_kind_from_name(const std::string& n) { auto s=core::lower(n); if(s=="syslog")return EventKind::syslog; if(s=="web_access"||s=="access")return EventKind::web_access; if(s=="json"||s=="jsonl")return EventKind::json; if(s=="csv")return EventKind::csv; if(s=="ini")return EventKind::ini; if(s=="stream")return EventKind::stream; return EventKind::unknown; }
std::vector<std::string> default_export_columns(){ return {"timestamp","kind","source","host","service","severity","message"}; }
std::string event_summary(const Event& e){ std::ostringstream out; out << core::format_rfc3339(e.timestamp) << ' ' << event_kind_name(e.kind) << ' '; if(!e.host.empty()) out << e.host << ' '; if(!e.service.empty()) out << e.service << ' '; if(!e.severity.empty()) out << e.severity << ' '; out << e.message; return out.str(); }
}
