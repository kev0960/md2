#include "metadata.h"

#include <cctype>
#include <iostream>
#include <utility>

#include "string_util.h"

namespace md2 {
namespace {

std::pair<std::string_view, std::string_view> GetNameAndField(
    std::string_view line) {
  int delimiter = line.find(':');

  std::string_view field_name = Strip(line.substr(0, delimiter));
  std::string_view field = Strip(line.substr(delimiter + 1));

  return std::make_pair(field_name, field);
}

std::vector<std::string> SplitString(std::string_view line) {
  std::vector<std::string> elems;
  int current = 0;

  while (true) {
    size_t end = 0;

    // We should escape , with \.
    while (true) {
      end = line.find(',', current);
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

}  // namespace

std::optional<Metadata> MetadataFactory::ParseMetadata(std::string_view content,
                                                       int& end) {
  size_t current = 0;

  // Ignore until it sees non whitespace.
  while (std::isspace(content[current])) {
    current++;
  }

  // Metadata should start with "----"s.
  if (content.substr(current, 3) != "---") {
    return std::nullopt;
  }

  // Go to the next line.
  while (content[current] != '\n' && current < content.size()) {
    current++;
  }

  current += 1;
  if (current > content.size()) {
    return std::nullopt;
  }

  Metadata metadata;
  while (true) {
    size_t line_end = content.find('\n', current);
    if (line_end == std::string_view::npos) {
      return std::nullopt;
    }

    std::string_view line = content.substr(current, line_end - current);

    // End of the metadata.
    if (line.substr(0, 3) == "---") {
      end = line_end + 1;
      return metadata;
    }

    auto [field_name, field] = GetNameAndField(line);
    if (field_name == "title") {
      metadata.title_ = field;
    } else if (field_name == "cat_title") {
      metadata.cat_title_ = field;
    } else if (field_name == "path") {
      metadata.path_ = field;
    } else if (field_name == "publish_date") {
      metadata.publish_date_ = field;
    } else if (field_name == "is_published") {
      metadata.is_published_ = (field == "true");
    } else if (field_name == "ref_name") {
      metadata.ref_names_ = SplitString(field);
    }

    current = line_end + 1;
  }

  // End of the metadata is not marked.
  return std::nullopt;
}

}  // namespace md2
