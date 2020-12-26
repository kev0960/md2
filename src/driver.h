#ifndef DRIVER_H
#define DRIVER_H

#include <atomic>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <zmq.hpp>

#include "metadata_repo.h"

namespace md2 {

struct DriverOptions {
  std::vector<std::string> input_dirs;
  std::vector<std::string> input_files;

  // Book start file and the tex output directory.
  std::vector<std::pair<std::string, std::string>> book_file_and_dir;

  std::string output_dir;

  std::string image_path;

  // Output directory for json files that will be picked up by the server.
  std::string json_output_dir;

  bool generate_html = true;
  bool generate_latex = true;

  size_t num_threads = 1;

  std::string clang_format_server_path;
  bool use_clang_format_server = false;
  int clang_format_server_port = 3001;

  // JSON file that contains the authentication info.
  std::string auth_file_path;

  // If true, articles will be updated to Database.
  bool update_database = false;
};

class Driver {
 public:
  // string file content, pos, rel_path
  using FileInfo = std::tuple<std::string, size_t, std::string>;

  Driver(const DriverOptions& options) : options_(options) {}

  // Run the driver.
  void Run();

  ~Driver();

 private:
  // Read files in the dirs.
  void ReadFilesInDirectory();

  // Build the file metadata repository.
  void BuildFileMetadataRepo();

  void DoParse();

  // Note that file_name is not a full path (e.g 251.md)
  void DoParse(std::string_view content, std::string_view file_name);

  // Figure out md files that needed to be parsed for book.
  void BuildBookFilesMap();

  // Emit the main tex file for each book.
  void GenerateBookMainPage() const;

  // Generate page_path.json and file_headers.json
  void GenerateJSONFiles() const;

  // Start the Clang format server.
  void StartClangFormatServer(const std::string& server_location);

  // Update articles to the database.
  void UpdateDatabase() const;

  DriverOptions options_;

  // Map between file name to the file content and the current reading position.
  std::unordered_map<std::string, FileInfo> file_contents_;

  MetadataRepo repo_;

  std::atomic<int> num_parsed_ = 0;

  // Map between the file name to the book directory.
  std::unordered_map<std::string, std::string> book_dir_to_files_;

  // Map between the start file and the included tex files in order.
  std::unordered_map<std::string, std::vector<std::string>>
      book_start_to_remaining_;

  bool clang_format_server_spanwed_ = false;
  int clang_format_pid_ = 0;
  std::unique_ptr<zmq::context_t> zmq_context_;
};

}  // namespace md2

#endif
