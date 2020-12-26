#include "db.h"

#include <algorithm>
#include <fstream>
#include <nlohmann/json.hpp>
#include <unordered_map>

#include "hash.h"
#include "logger.h"
#include "string_util.h"

namespace md2 {
namespace {

using json = nlohmann::json;

// Returns true if the metadata has not changed.
bool CheckMetadataMatches(std::string_view article_path_name,
                          const Database::ArticleHeaderInfo& prev_version,
                          const MetadataRepo& repo) {
  const Metadata* metadata = repo.FindMetadataByPathname(article_path_name);
  if (metadata->IsPublished() && (prev_version.is_published == "false")) {
    return false;
  } else if (!metadata->IsPublished() && (prev_version.is_published == "true" ||
                                          prev_version.is_published.empty())) {
    // Note that default for "is_published" is true.
    return false;
  }

  return true;
}

std::string PostgresBoolToString(const std::string& s) {
  if (s == "t") {
    return "true";
  } else if (s == "f") {
    return "false";
  }

  LOG(0) << "Something is wrong " << s;
  return "false";
}

std::string_view DefaultDateWhenEmpty(std::string_view s) {
  if (s.empty()) {
    return "now()";
  } else {
    return s;
  }
}

std::string BooleanString(bool b) {
  if (b) { return "true"; }
  return "false";
}

}  // namespace

Database::Database(const std::string& auth_file_path, const MetadataRepo& repo)
    : repo_(repo) {
  std::ifstream auth_file_in(auth_file_path);
  json auth_file = json::parse(auth_file_in);

  // Initiate the connection to the PSQL server.
  std::string pg_user;
  std::string pg_password;
  std::string pg_database;

  if (auth_file.count("PGUSER")) {
    pg_user = auth_file["PGUSER"].get<std::string>();
  }
  if (auth_file.count("PGPASSWORD")) {
    pg_password = auth_file["PGPASSWORD"].get<std::string>();
  }
  if (auth_file.count("PGDATABASE")) {
    pg_database = auth_file["PGDATABASE"].get<std::string>();
  }

  const std::string conn_str =
      StrCat("dbname=", pg_database, " user=", pg_user,
             " password=", pg_password, " hostaddr=127.0.0.1 port=5432");
  conn_ = std::make_unique<pqxx::connection>(conn_str);

  // Read all the hashes of current saved articles.
  pqxx::work read_articles(*conn_);
  pqxx::result articles = read_articles.exec(
      "SELECT article_url, current_content_sha256, creation_date, "
      "is_published, is_deleted FROM Articles;");

  for (auto itr = articles.begin(); itr != articles.end(); ++itr) {
    articles_in_db_[itr[0].as<std::string>()] = {
        itr[1].as<std::string>(), itr[2].as<std::string>(),
        PostgresBoolToString(itr[3].as<std::string>()),
        PostgresBoolToString(itr[4].as<std::string>())};
  }
}

bool Database::MaybeUpdateContent(
    std::unordered_map<std::string, ArticleHeaderInfo>::iterator itr,
    const std::string& content) {
  const auto& [article_path_name, info] = *itr;

  bool updated = false;

  const std::string& prev_hash = info.current_content_sha256;
  std::string current_hash = GenerateSha256Hash(content).value_or("");

  if (prev_hash != current_hash) {
    // Now insert into the database.
    pqxx::work append_new_article_info(*conn_);
    append_new_article_info.exec(
        StrCat("UPDATE articles SET contents = contents || "
               "row(now(), '",
               /* content */ append_new_article_info.esc(content),
               "', '', false)::article_content WHERE article_url = '",
               append_new_article_info.esc(article_path_name), "';"));

    append_new_article_info.exec(
        StrCat("UPDATE articles SET current_content_sha256 = '", current_hash,
               "' WHERE article_url = '",
               append_new_article_info.esc(article_path_name), "';"));
    append_new_article_info.commit();
    updated = true;
  }
  return updated;
}

bool Database::MaybeUpdateMetadata(
    std::unordered_map<std::string, ArticleHeaderInfo>::iterator itr) {
  const auto& [article_path_name, info] = *itr;

  const Metadata* metadata = repo_.FindMetadataByPathname(article_path_name);
  if (metadata == nullptr) {
    LOG(0) << "Error! " << article_path_name
           << " is in Database but not in files.";
    return false;
  }

  // TODO Remove is_deleted field? (setting is_published to false is good
  // enough)
  if (!CheckMetadataMatches(article_path_name, itr->second, repo_)) {
    pqxx::work modify_article_metadata(*conn_);
    modify_article_metadata.exec(StrCat(
        "UPDATE articles SET creation_date='",
        /* creation_date */
        DefaultDateWhenEmpty(metadata->GetPublishDate()), "', is_published=",
        /* is_published */
        BooleanString(metadata->IsPublished()), ", is_deleted=false",
        /* is_deleted */
        " WHERE article_url = '",
        modify_article_metadata.esc(article_path_name), "';"));
    modify_article_metadata.commit();

    return true;
  }

  return false;
}

bool Database::CreateNewArticle(const std::string& article_path_name,
                                const std::string& content) {
  // If the entry does not even exist, we should create one.
  pqxx::work create_new_article(*conn_);

  const Metadata* metadata = repo_.FindMetadataByPathname(article_path_name);
  if (metadata == nullptr) {
    return false;
  }

  std::string current_hash = GenerateSha256Hash(content).value_or("");
  create_new_article.exec(
      StrCat("INSERT INTO articles(article_url, creation_date, is_published, "
             "is_deleted, current_content_sha256) VALUES ('",
             create_new_article.esc(article_path_name), "', '",
             /* creation_date */
             DefaultDateWhenEmpty(metadata->GetPublishDate()), "', ",
             /* is_published */
             BooleanString(metadata->IsPublished()), ", ",
             /* is_deleted */
             "false, '",
             /* current_content_sha256 */ current_hash, "');"));

  create_new_article.exec(
      StrCat("UPDATE articles SET contents = contents || "
             "row(now(), '",
             create_new_article.esc(content),
             "', '', false)::article_content WHERE article_url = '",
             create_new_article.esc(article_path_name), "';"));

  create_new_article.commit();

  return true;
}

// Returns true if the file is changed.
bool Database::TryUpdateFileToDatabase(const std::string& article_url,
                                       const std::string& content) {
  // First request for the current status of the articles.
  std::string current_hash;

  // Now retrieve the hash of the current file.
  auto itr = articles_in_db_.find(article_url);
  if (itr != articles_in_db_.end()) {
    bool updated = MaybeUpdateContent(itr, content);
    updated |= MaybeUpdateMetadata(itr);

    return updated;
  } else {
    return CreateNewArticle(article_url, content);
  }

  return false;
}

}  // namespace md2
