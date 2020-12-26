#ifndef GENERATORS_GENERATOR_CONTEXT_H
#define GENERATORS_GENERATOR_CONTEXT_H

#include <mutex>
#include <unordered_map>
#include <zmq.hpp>

#include "metadata_repo.h"
#include "parse_tree_nodes/paragraph.h"

namespace md2 {

// Common object that is shared by generators.
class GeneratorContext {
 public:
  GeneratorContext(const MetadataRepo& repo, const std::string& image_path,
                   bool use_clang_server, int clang_server_port,
                   zmq::context_t* context)
      : repo_(repo),
        image_path_(image_path),
        use_clang_server_(use_clang_server),
        clang_server_port_(clang_server_port),
        context_(context) {}

  std::string_view GetClangFormatted(const ParseTreeTextNode* node,
                                     std::string_view md);

  // Find the link to the name (if possible) and the reference name (e.g
  // find$vector --> find). If not found, then the file_name (first) would be
  // empty.
  std::pair<std::string_view, std::string_view> FindReference(
      std::string_view name) const;

  std::string_view FindImage(const std::string& image_url);

  const Metadata* FindMetadataByFilename(std::string_view filename) const;

 private:
  std::mutex m_format_map;
  std::unordered_map<const ParseTreeTextNode*, std::string>
      verbatim_to_formatted_;

  // Map from image link url to the actual path.
  std::unordered_map<std::string, std::string> image_url_to_actual_url_;

  const MetadataRepo& repo_;
  std::string image_path_;

  bool use_clang_server_;
  int clang_server_port_;

  // ZMQ context (if used)
  zmq::context_t* context_;
};

}  // namespace md2

#endif
