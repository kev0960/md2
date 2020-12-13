#include "string_util.h"

#include <cstring>

namespace md2 {

size_t GetStringSize(const char* s) { return std::strlen(s); }

size_t GetStringSize(const std::string& s) { return s.size(); }

size_t GetStringSize(const std::string_view& s) { return s.size(); }

void AppendToString(std::string* concat_str) {
  (void)(concat_str);
  return;
}

std::string_view LStrip(std::string_view s) {
  s.remove_prefix(std::min(s.find_first_not_of(" "), s.size()));
  return s;
}

std::string_view RStrip(std::string_view s) {
  s.remove_suffix(std::min(s.size() - s.find_last_not_of(" ") - 1, s.size()));
  return s;
}

std::string_view Strip(std::string_view s) { return LStrip(RStrip(s)); }

std::vector<std::string_view> SplitStringByChar(std::string_view line, char c) {
  std::vector<std::string_view> elems;
  int current = 0;

  while (true) {
    size_t end = 0;

    // We should escape , with \.
    while (true) {
      end = line.find(c, current);
      if (end == std::string_view::npos) {
        break;
      } else if (line[end - 1] != '\\') {
        break;
      }
      current = end + 1;
    }

    elems.push_back(Strip(line.substr(current, end - current)));

    if (end == std::string_view::npos) {
      break;
    }

    current = end + 1;
  }

  return elems;
}

std::vector<std::string> SplitStringByCharToStringVec(std::string_view line,
                                                      char c) {
  std::vector<std::string> elems;
  int current = 0;

  while (true) {
    size_t end = 0;

    // We should escape , with \.
    while (true) {
      end = line.find(c, current);
      if (end == std::string_view::npos) {
        break;
      } else if (line[end - 1] != '\\') {
        break;
      }
      current = end + 1;
    }

    elems.push_back(std::string(Strip(line.substr(current, end - current))));

    if (end == std::string_view::npos) {
      break;
    }

    current = end + 1;
  }

  return elems;
}

std::string Join(int count, char* args[]) {
  std::string concat;

  for (int i = 0; i < count; i ++) {
    if (i != 0) {
      concat.push_back(' ');
    }

    concat.append(args[i]);
  }

  return concat;
}

std::string Join(const std::vector<std::string>& vec) {
  size_t total_size = 0;
  for (auto& s : vec) {
    total_size += s.size();
  }

  total_size += (vec.size() - 1);

  std::string concat;
  concat.reserve(total_size);

  for (size_t i = 0; i < vec.size(); i ++) {
    if (i != 0) {
      concat.push_back(' ');
    }

    concat.append(vec[i]);
  }

  return concat;
}
}  // namespace md2
