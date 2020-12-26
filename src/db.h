#ifndef DB_H
#define DB_H

#include <memory>
#include <pqxx/pqxx>
#include <unordered_map>

#include "metadata_repo.h"

namespace md2 {
class Database {
 public:
  // Using the credentials in auth_file_path, try to connect to the database.
  Database(const std::string& auth_file_path, const MetadataRepo& repo);

  // article_url (== path name). e.g "314"
  bool TryUpdateFileToDatabase(const std::string& article_url,
                               const std::string& content);

  struct ArticleHeaderInfo {
    std::string current_content_sha256;
    std::string creation_date;
    std::string is_published;
    std::string is_deleted;
  };

  struct ArticleContent {
    std::string content_or_diff;
    bool is_diff;
  };

 private:

  // If the hash value of the content does not match to the one in the database,
  // try to add patch. Note that path name is identical to "article_url" field.
  bool MaybeUpdateContent(
      std::unordered_map<std::string, ArticleHeaderInfo>::iterator itr,
      const std::string& content);
  bool MaybeUpdateMetadata(
      std::unordered_map<std::string, ArticleHeaderInfo>::iterator itr);

  bool CreateNewArticle(const std::string& article_path_name,
                        const std::string& content);

  std::unique_ptr<pqxx::connection> conn_;

  // url to sha256 hash.
  std::unordered_map<std::string, ArticleHeaderInfo> articles_in_db_;

  const MetadataRepo& repo_;
};
}  // namespace md2

#endif
