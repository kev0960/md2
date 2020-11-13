#include "generator_context.h"

#include <unistd.h>

#include <fstream>
#include <optional>
#include <string_view>
#include <thread>
#include <utility>

#include "logger.h"
#include "string_util.h"

namespace md2 {
namespace {

static char kClangFormatName[] = "clang-format";
static char kClangFormatConfig[] = "-style=google";

constexpr size_t kBufferSize = 4096;
constexpr std::string_view kDaumImageURL = "http://img1.daumcdn.net";

static std::vector<std::string_view> kImageFileExtCandidate{".png", ".jpg",
                                                            ".jpeg", ".gif"};

bool IsFileExist(const std::string& file_name) {
  std::ifstream in(file_name);
  return in.good();
}

void DoClangFormat(std::string_view code, std::string* formatted_code) {
  int pipe_p2c[2], pipe_c2p[2];
  if (pipe(pipe_p2c) != 0 || pipe(pipe_c2p) != 0) {
    LOG(0) << "Pipe error!";
    return;
  }
  int pid = fork();
  if (pid < 0) {
    LOG(0) << "Fork error!";
    return;
  }
  // Parent process;
  if (pid > 0) {
    // Close unused pipes.
    close(pipe_p2c[0]);
    close(pipe_c2p[1]);

    // Write the code we are trying to format.
    int result = write(pipe_p2c[1], code.data(), code.size());
    if (result == -1 || static_cast<size_t>(result) != code.size()) {
      LOG(0) << "Error; STDOUT to clang format has not completed succesfully";
    }
    close(pipe_p2c[1]);

    // Retrieve the formatted code from the child process.

    char buf[kBufferSize];
    int read_cnt;
    while ((read_cnt = read(pipe_c2p[0], buf, kBufferSize)) > 0) {
      auto current_size = formatted_code->size();
      formatted_code->reserve(current_size + read_cnt + 1);
      for (int i = 0; i < read_cnt; i++) {
        formatted_code->push_back(buf[i]);
      }
    }
    close(pipe_c2p[0]);
  } else {
    // In child process, call execve into the clang format.

    // Close unused pipes.
    close(pipe_p2c[1]);
    close(pipe_c2p[0]);

    // Bind the input and output stream to the pipe.
    dup2(pipe_p2c[0], STDIN_FILENO);
    dup2(pipe_c2p[1], STDOUT_FILENO);

    close(pipe_p2c[0]);
    close(pipe_c2p[0]);

    char* clang_format_argv[] = {kClangFormatName, kClangFormatConfig, NULL};
    char* env[] = {NULL};
    int ret = execve("/usr/bin/clang-format", clang_format_argv, env);
    LOG(0) << "CLANG FORMAT ERROR : " << ret;
  }
}

}  // namespace

std::string_view GeneratorContext::GetClangFormatted(
    const ParseTreeTextNode* node, std::string_view md) {
  if (const auto itr = verbatim_to_formatted_.find(node);
      itr != verbatim_to_formatted_.end()) {
    return itr->second;
  }

  // Otherwise actually do formatting.
  DoClangFormat(md.substr(node->Start(), node->End() - node->Start()),
                &verbatim_to_formatted_[node]);

  return verbatim_to_formatted_[node];
}

std::pair<std::string_view, std::string_view> GeneratorContext::FindReference(
    std::string_view name) {
  size_t delim = name.find('$');

  std::string_view actual_ref = name;
  std::string_view specified_path;

  // Makefile variable $(VAR) is incorrectly treated as a delimiter.
  if (delim != 0 && delim != std::string_view::npos) {
    actual_ref = name.substr(0, delim);
    specified_path = name.substr(delim + 1);
  }

  const Metadata* metadata = nullptr;
  if (specified_path.empty()) {
    metadata = repo_.FindMetadata(actual_ref);
  } else {
    metadata = repo_.FindMetadata(actual_ref, specified_path);
  }

  if (metadata == nullptr) {
    return std::make_pair("", actual_ref);
  }

  std::string_view file_name = metadata->GetFileName();
  if (file_name.substr(0, 5) == "dump_") {
    file_name.remove_prefix(5);
  }

  if (file_name.size() > 3 &&
      file_name.substr(file_name.size() - 3, 3) == ".md") {
    file_name.remove_suffix(3);
  }

  return std::make_pair(file_name, actual_ref);
}

std::string_view GeneratorContext::FindImage(const std::string& image_url) {
  if (const auto& actual_url = image_url_to_actual_url_.find(image_url);
      actual_url != image_url_to_actual_url_.end()) {
    return actual_url->second;
  }

  if (image_url.find(kDaumImageURL) != std::string_view::npos) {
    size_t id_start = image_url.find("image%2F");
    ASSERT(id_start != std::string_view::npos, "Daum image url is malformed.");

    std::string image_name = image_url.substr(id_start + 8);
    for (std::string_view ext : kImageFileExtCandidate) {
      if (IsFileExist(StrCat(image_path_, "/", image_name, ext))) {
        image_name = StrCat("/img/", image_name, ext);
        break;
      }
    }

    // Check whether webp exists.
    size_t ext_pos = image_name.find_last_of('.');
    if (ext_pos != std::string::npos) {
      std::string webp_image_name = image_name.substr(0, ext_pos) + ".webp";
      if (IsFileExist(webp_image_name)) {
        image_name = webp_image_name;
      }
    }

    image_url_to_actual_url_[image_url] = image_name;
    return image_url_to_actual_url_[image_url];
  }

  return image_url;
}

}  // namespace md2
