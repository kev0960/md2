#include "generator_context.h"

#include <unistd.h>

#include <fstream>
#include <optional>
#include <string_view>
#include <thread>
#include <unordered_set>
#include <utility>

#include "logger.h"
#include "string_util.h"

namespace md2 {
namespace {

static char kClangFormatName[] = "clang-format";
static char kClangFormatConfig[] = "-style=google";

constexpr size_t kBufferSize = 4096;
constexpr std::string_view kDaumImageURL = "http://img1.daumcdn.net";

static std::vector<std::string_view> kImageFileExtCandidate{
    "png", "jpg", "jpeg", "gif", "svg", "webp"};

static std::unordered_set<std::string_view> kLatexNotAllowedFileExt{
    "gif", "svg", "webp"};

std::vector<std::unordered_set<std::string_view>> kPreferredExtOrdering = {
    {"webp"}, {"png"}, {"jpg", "jpeg"}, {"gif"}};

bool IsFileExist(const std::string& file_name) {
  std::ifstream in(file_name);
  return in.good();
}

std::pair<std::string_view, std::string_view> GetFileNameAndExtension(
    std::string_view file_name) {
  size_t delim = file_name.find_last_of('.');
  if (delim == std::string_view::npos) {
    return std::make_pair(file_name, "");
  }

  return std::make_pair(file_name.substr(0, delim),
                        file_name.substr(delim + 1));
}

void ReadFromPipe(int pipe, std::string* formatted_code) {
  char buf[kBufferSize];
  int read_cnt;
  while ((read_cnt = read(pipe, buf, kBufferSize)) > 0) {
    auto current_size = formatted_code->size();
    formatted_code->reserve(current_size + read_cnt + 1);
    for (int i = 0; i < read_cnt; i++) {
      formatted_code->push_back(buf[i]);
    }
  }
  close(pipe);
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
      return;
    }
    close(pipe_p2c[1]);

    // Retrieve the formatted code from the child process.
    ReadFromPipe(pipe_c2p[0], formatted_code);
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

void DoClangFormatUsingFormatServer(int server_port, zmq::context_t& context,
                                    std::string_view code,
                                    std::string* formatted_code) {
  zmq::socket_t sock(context, ZMQ_REQ);
  sock.connect("tcp://localhost:" + std::to_string(server_port));

  sock.send(zmq::buffer(code));

  zmq::message_t formatted;
  auto result = sock.recv(formatted);

  if (!result) {
    LOG(0) << "Clang format result : " << result.value();
  }

  *formatted_code = formatted.to_string();
}

// file_path is relative path from the image_dir_path.
std::vector<std::string> FindFileAndCandidates(
    std::string_view file_path, std::string_view image_dir_path) {
  auto [file_name, file_ext] = GetFileNameAndExtension(file_path);

  std::vector<std::string> candidates;
  if (!file_ext.empty()) {
    candidates.push_back(std::string(file_path));
  }

  for (auto ext : kImageFileExtCandidate) {
    std::string candidate_file_name = StrCat(file_name, ".", ext);
    if (file_path != candidate_file_name) {
      candidates.push_back(candidate_file_name);
    }
  }

  std::vector<std::string> image_files;
  for (const auto& file : candidates) {
    if (IsFileExist(StrCat(image_dir_path, "/", file))) {
      image_files.push_back(file);
    }
  }

  // If no candidates are found, then just return the file.
  if (image_files.empty()) {
    image_files.push_back(std::string(file_path));
  }

  return image_files;
}

}  // namespace

std::string_view GeneratorContext::GetClangFormatted(
    const ParseTreeTextNode* node, std::string_view md) {
  {
    std::lock_guard<std::mutex> lk(m_format_map);
    if (const auto itr = verbatim_to_formatted_.find(node);
        itr != verbatim_to_formatted_.end()) {
      return itr->second;
    }
  }

  std::string_view code = md.substr(node->Start(), node->End() - node->Start());
  std::string formatted_code;

  if (use_clang_server_) {
    DoClangFormatUsingFormatServer(clang_server_port_, *context_, code,
                                   &formatted_code);
  } else {
    // Otherwise actually do formatting.
    DoClangFormat(md.substr(node->Start(), node->End() - node->Start()),
                  &formatted_code);
  }

  {
    std::lock_guard<std::mutex> lk(m_format_map);
    verbatim_to_formatted_[node] = formatted_code;
    return verbatim_to_formatted_[node];
  }
}

std::pair<std::string_view, std::string_view> GeneratorContext::FindReference(
    std::string_view name) const {
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

std::string_view GeneratorContext::FindImageForHtml(
    const std::string& image_url) {
  const std::vector<std::string>& files = FindImage(image_url);
  if (files.empty()) {
    return image_url;
  }

  // Web image priority
  // webp > png > jpeg, jpg > ...
  for (const auto& preferred_ext_set : kPreferredExtOrdering) {
    for (std::string_view file : files) {
      auto [name, ext] = GetFileNameAndExtension(file);
      if (preferred_ext_set.count(ext)) {
        return file;
      }
    }
  }

  return files.back();  // The last one is webp (if exists).
}

std::string GeneratorContext::FindImageForLatex(const std::string& image_url) {
  const std::vector<std::string>& files = FindImage(image_url);
  if (files.empty()) {
    return image_url;
  }

  std::string file;

  for (auto& file_name : files) {
    auto [name, ext] = GetFileNameAndExtension(file_name);
    if (kLatexNotAllowedFileExt.find(ext) == kLatexNotAllowedFileExt.end()) {
      file = file_name;
      break;
    }
  }

  return image_dir_path_ + file;
}

const std::vector<std::string>& GeneratorContext::FindImage(
    const std::string& image_url) {
  if (const auto& actual_path = image_url_to_actual_path_.find(image_url);
      actual_path != image_url_to_actual_path_.end()) {
    return actual_path->second;
  }

  if (image_url.find(kDaumImageURL) != std::string_view::npos) {
    size_t id_start = image_url.find("image%2F");
    MD2_ASSERT(id_start != std::string_view::npos,
               "Daum image url is malformed. " + image_url);

    std::string image_name = image_url.substr(id_start + 8);
    image_url_to_actual_path_[image_url] =
        FindFileAndCandidates(StrCat("/img/", image_name), image_dir_path_);
  } else {
    image_url_to_actual_path_[image_url] =
        FindFileAndCandidates(image_url, image_dir_path_);
  }

  return image_url_to_actual_path_[image_url];
}

const Metadata* GeneratorContext::FindMetadataByFilename(
    std::string_view filename) const {
  return repo_.FindMetadataByFilename(filename);
}

}  // namespace md2
