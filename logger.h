#pragma once
typedef enum
{
  LOG_DEBUG,
  LOG_INFO,
  LOG_WARNING,
  LOG_ERROR
} log_level;

void logger(log_level level, char *message);