#ifndef GENERATORS_BOOK_H
#define GENERATORS_BOOK_H

#include <map>
#include <string>
#include <vector>

#include "metadata_repo.h"

namespace md2 {

class BookGenerator {
 public:
  std::string GenerateMainTex(std::string_view start_file_num,
                              const std::vector<std::string>& tex_files,
                              const MetadataRepo& repo) const;
};

}  // namespace md2

#endif

