#include "common.h"
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <ctime>
#include <cstddef>
#include <cstdio>
#include <mutex>
#include <memory>

#ifdef PROJECT_NAMESPACE
namespace PROJECT_NAMESPACE {
#endif  // PROJECT_NAMESPACE

namespace common {

namespace {

const bool use_color = isatty(fileno(stderr));
thread_local const int pid = getpid();

}  // unnamed namespace

char const* LogLevelSymbol(LogLevel l) {
  switch (l) {
    case LogLevel::kInfo:
      return "I";
    case LogLevel::kWarn:
      return "W";
    case LogLevel::kDebug:
      return "D";
    case LogLevel::kFatal:
      return "F";
  }
  return "";
}

char const* LogLevelColorMaybe(LogLevel l) {
  if (use_color) {
    switch (l) {
      case LogLevel::kInfo:
        return "\x1b[32m";
      case LogLevel::kWarn:
        return "\x1b[31m";
      case LogLevel::kDebug:
        return "\x1b[34m";
      case LogLevel::kFatal:
        return "\x1b[31m";
    }
  }
  return "";
}

char const* LogLevelClearMaybe() {
  if (use_color) {
    return "\x1b[0m";
  } else {
    return "";
  }
}

void Log(LogLevel level, char const* file, int line, char const* func,
         char const* fmt, ...) {
  static std::mutex m;
  char const* format_string = "%s%s%s.%06d %d %s:%d: %s]%s ";
  std::time_t time_now;
  std::time(&time_now);
  char time_str[64];
  timeval tv;
  gettimeofday(&tv, nullptr);
  {
    std::lock_guard<std::mutex> lock{m};
    std::strftime(time_str, sizeof(time_str), "%m%d %H:%M:%S",
                  std::localtime(&time_now));
    std::fprintf(stderr, format_string, LogLevelColorMaybe(level),
                 LogLevelSymbol(level), time_str, tv.tv_usec, pid, file, line,
                 func, LogLevelClearMaybe());
    std::va_list ap;
    va_start(ap, fmt);
    std::vfprintf(stderr, fmt, ap);
    va_end(ap);
    std::fputc('\n', stderr);
  }
  if (level == LogLevel::kFatal) {
    throw FatalError{"Fatal error"};
  }
}

void AssertFail(char const* file, int line, char const* func, char const* expr,
                char const* fmt, ...) {
  std::string msg = FormatString("Assertion `%s' failed at %s:%d: %s", expr,
                                 file, line, func);
  if (fmt) {
    msg.append("\nextra message: ");
    std::va_list ap;
    va_start(ap, fmt);
    msg.append(FormatString(fmt, ap));
    va_end(ap);
  }
  throw AssertionError{msg};
}

std::string FormatString(char const* fmt, ...) {
  std::va_list ap;
  va_start(ap, fmt);
  auto&& rst = FormatStringVariadic(fmt, ap);
  va_end(ap);
  return rst;
}

std::string FormatStringVariadic(char const* fmt, std::va_list ap_orig) {
  constexpr std::size_t stack_size = 128;
  auto size = stack_size;
  char stack_ptr[stack_size];
  std::unique_ptr<char[]> heap_ptr{};
  char* ptr = stack_ptr;

  while (true) {
    std::va_list ap;
    va_copy(ap, ap_orig);
    auto n = std::vsnprintf(ptr, size, fmt, ap);
    va_end(ap);
    if (n < 0) {
      throw std::runtime_error{"Encoding error"};
    } else if (static_cast<std::size_t>(n) < size) {
      return std::string(ptr);
    } else {
      size = n + 1;
      heap_ptr.reset(new char[size]);
      ptr = heap_ptr.get();
    }
  }
}

Exception::Exception(std::string const& s) : msg_{s} {}

Exception::~Exception() = default;

char const* Exception::what() const noexcept { return msg_.c_str(); }

}  // namespace common

#ifdef PROJECT_NAMESPACE
}  // namespace PROJECT_NAMESPACE
#endif  // PROJECT_NAMESPACE
