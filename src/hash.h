#ifndef HASH_H
#define HASH_H

#include <optional>
#include <string>

namespace md2 {

std::optional<std::string> GenerateSha256Hash(const std::string& s);

}  // namespace md2

#endif
