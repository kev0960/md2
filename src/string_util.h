#ifndef STRING_UTIL_H
#define STRING_UTIL_H

#include <string>

namespace md2 {

size_t GetStringSize(const char* s);
size_t GetStringSize(const std::string& s);
size_t GetStringSize(const std::string_view& s);

template <typename String, typename... Strings>
size_t GetStringSize(const String& s, Strings... strs) {
  return GetStringSize(s) + GetStringSize(strs...);
}

void AppendToString(std::string* concat_str);

template <typename String, typename... Strings>
void AppendToString(std::string* concat_str, const String& s, Strings... strs) {
  concat_str->append(s);
  AppendToString(concat_str, strs...);
}

template <typename String, typename... Strings>
std::string StrCat(const String& s, Strings... strs) {
  size_t total_size = GetStringSize(s, strs...);
  std::string concat_str;
  concat_str.reserve(total_size);

  concat_str = s;
  AppendToString(&concat_str, strs...);

  return concat_str;
}

// Strip ' '.
std::string_view LStrip(std::string_view s);
std::string_view RStrip(std::string_view s);
std::string_view Strip(std::string_view s);

}  // namespace md2

#endif
