#include "index/store.h"
#include "core/checksum.h"
#include "core/slice.h"
#include "core/string_util.h"
#include <fstream>
#include <set>

namespace logharbor::index {
std::vector<std::uint8_t> encode_varint(std::uint64_t v) { std::vector<std::uint8_t> out; do { std::uint8_t b = static_cast<std::uint8_t>(v & 0x7f); v >>= 7; if (v) b |= 0x80; out.push_back(b); } while(v); return out; }
core::Result<std::uint64_t> decode_varint(const std::vector<std::uint8_t>& bytes, std::size_t& pos) { std::uint64_t v=0; int shift=0; while(pos<bytes.size()&&shift<64){ auto b=bytes[pos++]; v |= static_cast<std::uint64_t>(b&0x7f) << shift; if(!(b&0x80)) return core::Result<std::uint64_t>::success(v); shift+=7;} return core::Result<std::uint64_t>::failure(core::make_error(core::ErrorCode::invalid_format,"bad varint",pos)); }
namespace {
void put(std::vector<std::uint8_t>& out, const std::string& s){ auto n=encode_varint(s.size()); out.insert(out.end(),n.begin(),n.end()); out.insert(out.end(),s.begin(),s.end()); }
core::Result<std::string> get(const std::vector<std::uint8_t>& b, std::size_t& p){ auto n=decode_varint(b,p); if(!n)return core::Result<std::string>::failure(n.error()); if(p+n.value()>b.size())return core::Result<std::string>::failure(core::make_error(core::ErrorCode::truncated_input,"string outside record",p)); std::string s(reinterpret_cast<const char*>(b.data()+p), static_cast<std::size_t>(n.value())); p+=static_cast<std::size_t>(n.value()); return core::Result<std::string>::success(s); }
}
std::vector<std::uint8_t> serialize_event(const ingest::Event& e) {
    std::vector<std::uint8_t> out; out.push_back(static_cast<std::uint8_t>(e.kind)); out.push_back(e.tombstone?1:0);
    for(auto s: encode_varint(static_cast<std::uint64_t>(core::unix_seconds(e.timestamp)))) out.push_back(s);
    for(auto s: encode_varint(e.ordinal)) out.push_back(s);
    put(out,e.id); put(out,e.source); put(out,e.host); put(out,e.service); put(out,e.severity); put(out,e.message); auto fc=encode_varint(e.fields.size()); out.insert(out.end(),fc.begin(),fc.end()); for(const auto& kv:e.fields){put(out,kv.first);put(out,kv.second);} return out;
}
core::Result<ingest::Event> deserialize_event(const std::vector<std::uint8_t>& b) {
    if(b.size()<2)return core::Result<ingest::Event>::failure(core::make_error(core::ErrorCode::truncated_input,"short record")); std::size_t p=0; ingest::Event e; e.kind=static_cast<ingest::EventKind>(b[p++]); e.tombstone=b[p++]!=0; auto ts=decode_varint(b,p); if(!ts)return core::Result<ingest::Event>::failure(ts.error()); e.timestamp=core::from_unix_seconds(static_cast<std::int64_t>(ts.value())); auto ord=decode_varint(b,p); if(!ord)return core::Result<ingest::Event>::failure(ord.error()); e.ordinal=ord.value();
    auto id=get(b,p); if(!id)return core::Result<ingest::Event>::failure(id.error()); e.id=id.value(); auto src=get(b,p); if(!src)return core::Result<ingest::Event>::failure(src.error()); e.source=src.value(); auto host=get(b,p); if(!host)return core::Result<ingest::Event>::failure(host.error()); e.host=host.value(); auto svc=get(b,p); if(!svc)return core::Result<ingest::Event>::failure(svc.error()); e.service=svc.value(); auto sev=get(b,p); if(!sev)return core::Result<ingest::Event>::failure(sev.error()); e.severity=sev.value(); auto msg=get(b,p); if(!msg)return core::Result<ingest::Event>::failure(msg.error()); e.message=msg.value(); auto fc=decode_varint(b,p); if(!fc)return core::Result<ingest::Event>::failure(fc.error()); for(std::uint64_t i=0;i<fc.value()&&i<100000;++i){auto k=get(b,p);auto v=get(b,p);if(!k||!v)return core::Result<ingest::Event>::failure(core::make_error(core::ErrorCode::truncated_input,"bad field")); e.fields[k.value()]=v.value();} return core::Result<ingest::Event>::success(e);
}
void RecordStore::append(const ingest::Event& e){ events_.push_back(e); }
bool RecordStore::erase_id(const std::string& id){ for(auto& e:events_) if(e.id==id&&!e.tombstone){ e.tombstone=true; return true; } return false; }
std::vector<ingest::Event> RecordStore::live_events() const { std::vector<ingest::Event> out; for(const auto& e:events_) if(!e.tombstone) out.push_back(e); return out; }
StoreStats RecordStore::stats() const { StoreStats s; std::set<std::string> strings; for(const auto& e:events_){++s.records;if(e.tombstone)++s.tombstones; auto b=serialize_event(e); s.bytes+=b.size(); strings.insert(e.source); strings.insert(e.host); strings.insert(e.service); strings.insert(e.severity); for(const auto& kv:e.fields){strings.insert(kv.first);strings.insert(kv.second);}} s.strings=strings.size(); return s; }
core::Result<void> RecordStore::save(const std::string& path) const { std::ofstream f(path, std::ios::binary); if(!f)return core::Result<void>::failure(core::make_error(core::ErrorCode::io_error,"cannot open store for write")); f.write("LHST1\n",6); for(const auto& e:events_){ auto rec=serialize_event(e); auto len=encode_varint(rec.size()); f.write(reinterpret_cast<const char*>(len.data()),len.size()); f.write(reinterpret_cast<const char*>(rec.data()),rec.size()); std::uint32_t c=core::crc32(rec.data(),rec.size()); f.write(reinterpret_cast<const char*>(&c),sizeof(c)); } return core::Result<void>::success(); }
core::Result<void> RecordStore::load(const std::string& path) { std::ifstream f(path,std::ios::binary); if(!f)return core::Result<void>::failure(core::make_error(core::ErrorCode::io_error,"cannot open store")); std::vector<std::uint8_t> b((std::istreambuf_iterator<char>(f)),{}); if(b.size()<6||std::string(reinterpret_cast<char*>(b.data()),6)!="LHST1\n")return core::Result<void>::failure(core::make_error(core::ErrorCode::storage_corrupt,"bad store header")); events_.clear(); std::size_t p=6; while(p<b.size()){ auto len=decode_varint(b,p); if(!len)return core::Result<void>::failure(len.error()); if(p+len.value()+4>b.size())return core::Result<void>::failure(core::make_error(core::ErrorCode::truncated_input,"short page")); std::vector<std::uint8_t> rec(b.begin()+p,b.begin()+p+static_cast<std::size_t>(len.value())); p+=static_cast<std::size_t>(len.value())+4; auto e=deserialize_event(rec); if(e)events_.push_back(e.value()); } return core::Result<void>::success(); }
core::Result<void> RecordStore::compact(const std::string& path) const { RecordStore tmp; for(const auto& e:events_) if(!e.tombstone) tmp.append(e); return tmp.save(path); }
}
