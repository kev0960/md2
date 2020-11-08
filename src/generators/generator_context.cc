#include "generator_context.h"

#include <unistd.h>

#include <optional>
#include <string_view>
#include <thread>
#include <utility>

#include "logger.h"

namespace md2 {
namespace {

static char kClangFormatName[] = "clang-format";
static char kClangFormatConfig[] = "-style=google";

constexpr size_t kBufferSize = 4096;

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

}  // namespace md2
