#pragma once
#include "core/result.h"
#include "ingest/event.h"
#include <memory>
#include <string>
#include <vector>
namespace logharbor::query {
struct Token { std::string kind; std::string text; };
std::vector<Token> lex(const std::string& q);
class Expression { public: virtual ~Expression()=default; virtual bool eval(const ingest::Event& e) const=0; };
using ExprPtr = std::unique_ptr<Expression>;
core::Result<ExprPtr> parse(const std::string& q);
bool matches(const ingest::Event& e, const std::string& q);
std::vector<ingest::Event> filter(const std::vector<ingest::Event>& events, const std::string& q);
}
