#include "string_util.h"

#include <cstring>

namespace md2 {

size_t GetStringSize(const char* s) { return std::strlen(s); }

size_t GetStringSize(const std::string& s) { return s.size(); }

size_t GetStringSize(const std::string_view& s) { return s.size(); }

void AppendToString(std::string* concat_str) { return; }

}  // namespace md2
