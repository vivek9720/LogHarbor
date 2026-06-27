#pragma once
#include <string>
#include <vector>
namespace logharbor::core {
struct RuleEntry { const char* name; const char* category; const char* value; int weight; };
const std::vector<RuleEntry>& rule_catalog();
bool is_sensitive_field_name(const std::string& name);
int severity_weight_for_token(const std::string& token);
std::vector<std::string> aliases_for_field(const std::string& field);
std::vector<std::string> tokens_for_category(const std::string& category);
}