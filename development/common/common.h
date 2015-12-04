#ifndef CPP_UTILS_COMMON_H_
#define CPP_UTILS_COMMON_H_

#include <cstdarg>
#include <string>
#include <exception>

/*!
 * \brief Namespace of the project.
 */
#ifdef PROJECT_NAMESPACE
#define COMMON_NAMESPACE ::PROJECT_NAMESPACE::common
#else  // PROJECT_NAMESPACE
#define COMMON_NAMESPACE ::common
#endif  // PROJECT_NAMESPACE

/*!
 * \brief Log string.
 */
#define LOG(fmt...)                                                            \
  COMMON_NAMESPACE::Log(COMMON_NAMESPACE::LogLevel::kInfo, __FILE__, __LINE__, \
                        __func__, fmt)
/*!
 * \brief Log string as warning.
 */
#define LOG_WARN(fmt...)                                                       \
  COMMON_NAMESPACE::Log(COMMON_NAMESPACE::LogLevel::kWarn, __FILE__, __LINE__, \
                        __func__, fmt)
/*!
 * \brief Log string as fatal error.
 */
#define LOG_FATAL(fmt...)                                             \
  COMMON_NAMESPACE::Log(COMMON_NAMESPACE::LogLevel::kFatal, __FILE__, \
                        __LINE__, __func__, fmt)
/*!
 * \brief Whether to enable debug logging.
 */
#ifdef ENABLE_LOG_DEBUG
/*!
 * \brief Log string as debugging info.
 */
#define LOG_DEBUG(fmt...)                                             \
  COMMON_NAMESPACE::Log(COMMON_NAMESPACE::LogLevel::kDebug, __FILE__, \
                        __LINE__, __func__, fmt)
#else  // ENABLE_LOG_DEBUG
#define LOG_DEBUG(fmt...) static_cast<void>(sizeof(fmt))
#endif

#define LIKELY(v) __builtin_expect(static_cast<bool>(v), 1)
#define UNLIKELY(v) __builtin_expect(static_cast<bool>(v), 0)

/*!
 * \brief Assert to be true.
 */
#define ASSERT(expr, msg...)                                            \
  {                                                                     \
    if (UNLIKELY(!(expr))) {                                            \
      COMMON_NAMESPACE::AssertFail(__FILE__, __LINE__, __func__, #expr, \
                                   ##msg);                              \
    }                                                                   \
  }
/*!
 * \brief Assert pointer to be not null.
 */
#define NOT_NULL(expr) \
  COMMON_NAMESPACE::NotNull(__FILE__, __LINE__, __func__, (expr), #expr)

#ifdef PROJECT_NAMESPACE
namespace PROJECT_NAMESPACE {
#endif  // PROJECT_NAMESPACE

namespace common {

enum class LogLevel { kInfo, kWarn, kDebug, kFatal };

void Log(LogLevel, char const*, int, char const*, char const*, ...)
    __attribute__((format(printf, 5, 6)));

void AssertFail(char const*, int, char const*, char const*, char const* = 0,
                ...) __attribute__((format(printf, 5, 6), noreturn));

template <typename T>
T* NotNull(char const* file, int line, char const* func, T* t,
           char const* expr) {
  if (UNLIKELY(t == nullptr)) {
    AssertFail(file, line, func, expr, "must not be `nullptr`");
  }
  return t;
}

std::string FormatString(char const*, ...)
    __attribute__((format(printf, 1, 2)));

std::string FormatStringVariadic(char const*, std::va_list);

class Exception : public std::exception {
 protected:
  std::string msg_;

 public:
  Exception(std::string const&);
  ~Exception();
  char const* what() const noexcept override;
};  // class Exception

class AssertionError : public Exception {
 public:
  using Exception::Exception;
};  // class AssertionError

class FatalError : public Exception {
 public:
  using Exception::Exception;
};  // class FatalError

}  // namespace common

#ifdef PROJECT_NAMESPACE
}  // namespace PROJECT_NAMESPACE
#endif  // PROJECT_NAMESPACE

#endif  // CPP_UTILS_COMMON_H_
