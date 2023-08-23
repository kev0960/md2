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

bool IsValidUTF8(const std::string& s) {
  const unsigned char* bytes = (const unsigned char*)s.c_str();
  unsigned int cp;
  int num;

  while (*bytes != 0x00) {
    if ((*bytes & 0x80) == 0x00) {
      // U+0000 to U+007F
      cp = (*bytes & 0x7F);
      num = 1;
    } else if ((*bytes & 0xE0) == 0xC0) {
      // U+0080 to U+07FF
      cp = (*bytes & 0x1F);
      num = 2;
    } else if ((*bytes & 0xF0) == 0xE0) {
      // U+0800 to U+FFFF
      cp = (*bytes & 0x0F);
      num = 3;
    } else if ((*bytes & 0xF8) == 0xF0) {
      // U+10000 to U+10FFFF
      cp = (*bytes & 0x07);
      num = 4;
    } else
      return false;

    bytes += 1;
    for (int i = 1; i < num; ++i) {
      if ((*bytes & 0xC0) != 0x80) return false;
      cp = (cp << 6) | (*bytes & 0x3F);
      bytes += 1;
    }

    if ((cp > 0x10FFFF) || ((cp >= 0xD800) && (cp <= 0xDFFF)) ||
        ((cp <= 0x007F) && (num != 1)) ||
        ((cp >= 0x0080) && (cp <= 0x07FF) && (num != 2)) ||
        ((cp >= 0x0800) && (cp <= 0xFFFF) && (num != 3)) ||
        ((cp >= 0x10000) && (cp <= 0x1FFFFF) && (num != 4)))
      return false;
  }

  return true;
}

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
  if (b) {
    return "true";
  }
  return "false";
}

}  // namespace

Database::Database(const std::string& auth_file_path, const MetadataRepo& repo,
                   bool use_new_schema)
    : repo_(repo), use_new_schema_(use_new_schema) {
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

  pqxx::result articles;
  if (use_new_schema) {
    articles = read_articles.exec(
        "SELECT article_url, current_content_sha256, create_time, "
        "is_published, is_deleted FROM article;");
  } else {
    articles = read_articles.exec(
        "SELECT article_url, current_content_sha256, creation_date, "
        "is_published, is_deleted FROM Articles;");
  }

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

  if (!IsValidUTF8(content)) {
    LOG(0) << "Rejecting invalid content [" << article_path_name << "] ";
    return false;
  }

  bool updated = false;

  const std::string& prev_hash = info.current_content_sha256;
  std::string current_hash = GenerateSha256Hash(content).value_or("");

  if (prev_hash != current_hash) {
    try {
      // Now insert into the database.
      pqxx::work append_new_article_info(*conn_);

      if (use_new_schema_) {
        append_new_article_info.exec(StrCat(
            "UPDATE article SET current_content_sha256 = '", current_hash,
            "', content = '", append_new_article_info.esc(content),
            "', update_time = now() WHERE article_url = '",
            append_new_article_info.esc(article_path_name), "';"));
      } else {
        append_new_article_info.exec(
            StrCat("UPDATE articles SET contents = contents || "
                   "row(now(), '",
                   /* content */ append_new_article_info.esc(content),
                   "', '', false)::article_content WHERE article_url = '",
                   append_new_article_info.esc(article_path_name), "';"));

        append_new_article_info.exec(
            StrCat("UPDATE articles SET current_content_sha256 = '",
                   current_hash, "' WHERE article_url = '",
                   append_new_article_info.esc(article_path_name), "';"));
      }

      append_new_article_info.commit();
      updated = true;
    } catch (std::exception& e) {
      LOG(0) << "DB Update Error : " << article_path_name << " " << e.what();
    }
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

    if (use_new_schema_) {
      modify_article_metadata.exec(StrCat(
          "UPDATE article SET create_time='",
          /* creation_date */
          DefaultDateWhenEmpty(metadata->GetPublishDate()), "', is_published=",
          /* is_published */
          BooleanString(metadata->IsPublished()), ", is_deleted=false",
          /* is_deleted */
          " WHERE article_url = '",
          modify_article_metadata.esc(article_path_name), "';"));
    } else {
      modify_article_metadata.exec(StrCat(
          "UPDATE articles SET creation_date='",
          /* creation_date */
          DefaultDateWhenEmpty(metadata->GetPublishDate()), "', is_published=",
          /* is_published */
          BooleanString(metadata->IsPublished()), ", is_deleted=false",
          /* is_deleted */
          " WHERE article_url = '",
          modify_article_metadata.esc(article_path_name), "';"));
    }
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

  if (!IsValidUTF8(content)) {
    LOG(0) << "Rejecting invalid content [" << article_path_name << "] ";
    return false;
  }

  std::string current_hash = GenerateSha256Hash(content).value_or("");

  if (use_new_schema_) {
    create_new_article.exec(
        StrCat("INSERT INTO article(article_url, create_time, update_time, "
               "is_published, "
               "is_deleted, current_content_sha256, content) VALUES ('",
               create_new_article.esc(article_path_name), "', '",
               /* create_time */
               DefaultDateWhenEmpty(metadata->GetPublishDate()), "', ",
               /* update_time */
               "now(), ",
               /* is_published */
               BooleanString(metadata->IsPublished()), ", ",
               /* is_deleted */
               "false, '",
               /* current_content_sha256 */ current_hash, "', ",
               /*  content */ "'", create_new_article.esc(content), "'", ");"));

  } else {
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
  }

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
