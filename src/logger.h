#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>

namespace md2 {

class Logger {
 public:
  static Logger& GetLogger() {
    static Logger lg;
    return lg;
  }

  // Setting higher logging level shows *MORE* logs.
  void SetLoggingLevel(int log_level) { log_level_ = log_level; }

  // Lower level means more important.
  Logger& Log(int log_level, const char* file_name, int line) {
    current_log_level_ = log_level;

    if (current_log_level_ <= log_level_) {
      std::cout << "\n[" << file_name << ":" << line << "] ";
    }

    return *this;
  }

  template <typename T>
  Logger& operator<<(const T& t) {
    if (current_log_level_ <= log_level_) {
      std::cout << t;
    }
    return *this;
  }

 private:
  Logger() = default;

  int log_level_ = 1;
  int current_log_level_ = 1;
};

}  // namespace md2

#define LOG(X) md2::Logger::GetLogger().Log(X, __FILE__, __LINE__)

#endif
