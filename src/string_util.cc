#include "string_util.h"

#include <cstring>

namespace md2 {

size_t GetStringSize(const char* s) { return std::strlen(s); }

size_t GetStringSize(const std::string& s) { return s.size(); }

size_t GetStringSize(const std::string_view& s) { return s.size(); }

void AppendToString(std::string* concat_str) { return; }

std::string_view LStrip(std::string_view s) {
  s.remove_prefix(std::min(s.find_first_not_of(" "), s.size()));
  return s;
}

std::string_view RStrip(std::string_view s) {
  s.remove_suffix(std::min(s.size() - s.find_last_not_of(" ") - 1, s.size()));
  return s;
}

std::string_view Strip(std::string_view s) { return LStrip(RStrip(s)); }

}  // namespace md2
