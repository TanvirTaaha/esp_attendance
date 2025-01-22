#ifndef __DEBUG_H__
#define __DEBUG_H__

// Log levels
#define ATTENDACE_LOG_LEVEL_ERROR 1
#define ATTENDACE_LOG_LEVEL_WARN 2
#define ATTENDACE_LOG_LEVEL_INFO 3
#define ATTENDACE_LOG_LEVEL_DEBUG 4

// Set current log level
#define CURRENT_LOG_LEVEL ATTENDACE_LOG_LEVEL_DEBUG

// Base debug macro
#if ACTIVATE_LOGGING
#define DEBUG_PRINT(level, label, fmt, ...)   \
  do                                          \
  {                                           \
    if (level <= CURRENT_LOG_LEVEL)           \
    {                                         \
      /*unsigned long timestamp = millis();*/ \
      Serial.print("[");                      \
      /*Serial.print(timestamp);*/            \
      /*Serial.print("-");*/                  \
      Serial.print(label);                    \
      Serial.print("] ");                     \
      Serial.print(": ");                     \
      Serial.printf(fmt, ##__VA_ARGS__);      \
      Serial.println();                       \
    }                                         \
  } while (0)
#else
#define DEBUG_PRINT(level, label, fmt, ...) \
  do                                        \
  {                                         \
  } while (0)
#endif

// Convenience macros for different log levels
#define LOG_ERROR(fmt, ...) DEBUG_PRINT(ATTENDACE_LOG_LEVEL_ERROR, "ERROR", fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...) DEBUG_PRINT(ATTENDACE_LOG_LEVEL_WARN, "WARN", fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...) DEBUG_PRINT(ATTENDACE_LOG_LEVEL_INFO, "INFO", fmt, ##__VA_ARGS__)
#define LOG_DEBUG(fmt, ...) DEBUG_PRINT(ATTENDACE_LOG_LEVEL_DEBUG, "DEBUG", fmt, ##__VA_ARGS__)

#endif