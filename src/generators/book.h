#ifndef GENERATORS_BOOK_H
#define GENERATORS_BOOK_H

#include <map>
#include <string>
#include <vector>

namespace md2 {

enum BookType { C, CPP };

class BookManager {
 public:
  BookManager(BookType book_type,
              const std::map<std::string, std::map<std::string, std::string>>*
                  file_info);

  void GenerateMainTex();
  bool IsBookFile(const std::string& filename);
  std::string GetBookType();

 private:
  BookType book_type_;
  const std::map<std::string, std::map<std::string, std::string>>* file_info_;

  std::vector<std::string> book_list_;
};

}  // namespace md2

#endif

