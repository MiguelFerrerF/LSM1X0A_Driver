#include "LSM1x0A_Logger.h"
#include <stdio.h>

namespace LsmLogger
{

static LsmLogCallback _callback     = nullptr;
static LsmLogLevel    _currentLevel = LsmLogLevel::INFO;

void setCallback(LsmLogCallback cb)
{
  _callback = cb;
}

void setLevel(LsmLogLevel level)
{
  _currentLevel = level;
}

LsmLogLevel getLevel()
{
  return _currentLevel;
}

void internalLog(LsmLogLevel level, const char* module, const char* format, ...)
{
  if (_callback == nullptr)
    return;
  if (level > _currentLevel)
    return; // Dynamic filter

  // Use a local buffer to be thread-safe/reentrant.
  // 192 bytes is usually enough for a log message, but adjust if needed.
  char _logBuffer[192];

  va_list args;
  va_start(args, format);
  vsnprintf(_logBuffer, sizeof(_logBuffer), format, args);
  va_end(args);

  // Call the user callback
  _callback(level, module, _logBuffer);
}

} // namespace LsmLogger
