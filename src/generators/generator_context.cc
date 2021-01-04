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

  return files.back(); // The last one is webp (if exists).
}

std::string GeneratorContext::FindImageForLatex(
    const std::string& image_url) {
  const std::vector<std::string>& files = FindImage(image_url);
  if (files.empty()) {
    return image_url;
  }

  std::string file = files.front();
  if (file.substr(0, 4) == "/img") {
    file = file.substr(4);
  }

  return image_path_ + file; // First one is never webp.
}

const std::vector<std::string>& GeneratorContext::FindImage(
    const std::string& image_url) {
  if (const auto& actual_url = image_url_to_actual_url_.find(image_url);
      actual_url != image_url_to_actual_url_.end()) {
    return actual_url->second;
  }

  if (image_url.find(kDaumImageURL) != std::string_view::npos) {
    size_t id_start = image_url.find("image%2F");
    MD2_ASSERT(id_start != std::string_view::npos,
               "Daum image url is malformed. " + image_url);

    std::string image_name = image_url.substr(id_start + 8);
    std::string image_path;
    for (std::string_view ext : kImageFileExtCandidate) {
      std::string actual_image_path = StrCat(image_path_, "/", image_name, ext);
      if (IsFileExist(actual_image_path)) {
        image_name = StrCat("/img/", image_name, ext);
        image_url_to_actual_url_[image_url].push_back(image_name);

        image_path = actual_image_path;
        break;
      }
    }

    // Check whether webp exists.
    size_t ext_pos = image_path.find_last_of('.');
    if (ext_pos != std::string::npos) {
      std::string webp_image_path = image_path.substr(0, ext_pos) + ".webp";
      if (IsFileExist(webp_image_path)) {
        image_name =
            StrCat(image_name.substr(0, image_name.find_last_of('.')), ".webp");
        image_url_to_actual_url_[image_url].push_back(image_name);
      }
    }

    return image_url_to_actual_url_[image_url];
  }

  image_url_to_actual_url_[image_url] = {image_url};
  return image_url_to_actual_url_[image_url];
}

const Metadata* GeneratorContext::FindMetadataByFilename(
    std::string_view filename) const {
  return repo_.FindMetadataByFilename(filename);
}

}  // namespace md2
