#ifndef LSM1X0A_LOGGER_H
#define LSM1X0A_LOGGER_H

#include "LSM1x0A_Types.h"
#include <stdarg.h>

/**
 * @brief User-defined callback for receiving log events.
 * @param level   The severity level of the log message.
 * @param module  A short string identifying the source module (e.g., "CTRL", "UART", "LORA").
 * @param message The formatted log message.
 */
typedef void (*LsmLogCallback)(LsmLogLevel level, const char* module, const char* message);

// -----------------------------------------------------------------------------------------
// Global Logger Configuration
// -----------------------------------------------------------------------------------------

namespace LsmLogger
{
/**
 * @brief Sets the global callback to receive log messages.
 * @param cb Callback function pointer.
 */
void setCallback(LsmLogCallback cb);

/**
 * @brief Sets the maximum log level to be emitted at runtime.
 * Messages with a level higher than this will be ignored, even if compiled.
 * @param level Maximum allowed log level.
 */
void setLevel(LsmLogLevel level);

/**
 * @brief Gets the current runtime log level.
 * @return The current LsmLogLevel.
 */
LsmLogLevel getLevel();

/**
 * @brief Internal function to format and dispatch logs.
 * DO NOT call this directly; use the LSM_LOG_* macros instead.
 */
void internalLog(LsmLogLevel level, const char* module, const char* format, ...);
} // namespace LsmLogger

// -----------------------------------------------------------------------------------------
// Compile-Time Logging Macros
// -----------------------------------------------------------------------------------------
// Define LSM_MAX_COMPILE_LOG_LEVEL globally via build_flags (e.g. -D LSM_MAX_COMPILE_LOG_LEVEL=7)
// to compile out verbose strings to save Flash memory.
// Defaults to 6 (VERBOSE) if not specified by the user.
#ifndef LSM_MAX_COMPILE_LOG_LEVEL
#define LSM_MAX_COMPILE_LOG_LEVEL 6
#endif

// ERROR Level (Level 1)
#if LSM_MAX_COMPILE_LOG_LEVEL >= 1
#define LSM_LOG_ERROR(module, format, ...) LsmLogger::internalLog(LsmLogLevel::ERROR, module, format, ##__VA_ARGS__)
#else
#define LSM_LOG_ERROR(module, format, ...)                                                                                                           \
  do {                                                                                                                                               \
  } while (0)
#endif

// WARN Level (Level 2)
#if LSM_MAX_COMPILE_LOG_LEVEL >= 2
#define LSM_LOG_WARN(module, format, ...) LsmLogger::internalLog(LsmLogLevel::WARN, module, format, ##__VA_ARGS__)
#else
#define LSM_LOG_WARN(module, format, ...)                                                                                                            \
  do {                                                                                                                                               \
  } while (0)
#endif

// INFO Level (Level 3)
#if LSM_MAX_COMPILE_LOG_LEVEL >= 3
#define LSM_LOG_INFO(module, format, ...) LsmLogger::internalLog(LsmLogLevel::INFO, module, format, ##__VA_ARGS__)
#else
#define LSM_LOG_INFO(module, format, ...)                                                                                                            \
  do {                                                                                                                                               \
  } while (0)
#endif

// DEBUG Level (Level 4)
#if LSM_MAX_COMPILE_LOG_LEVEL >= 4
#define LSM_LOG_DEBUG(module, format, ...) LsmLogger::internalLog(LsmLogLevel::DEBUG, module, format, ##__VA_ARGS__)
#else
#define LSM_LOG_DEBUG(module, format, ...)                                                                                                           \
  do {                                                                                                                                               \
  } while (0)
#endif

// VERBOSE Level (Level 5)
#if LSM_MAX_COMPILE_LOG_LEVEL >= 5
#define LSM_LOG_VERBOSE(module, format, ...) LsmLogger::internalLog(LsmLogLevel::VERBOSE, module, format, ##__VA_ARGS__)
#else
#define LSM_LOG_VERBOSE(module, format, ...)                                                                                                         \
  do {                                                                                                                                               \
  } while (0)
#endif

#endif // LSM1X0A_LOGGER_H
