#define _GNU_SOURCE

#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <errno.h>
#include <stdint.h>

#include "logger.h"
#include "main.h"

/**
 * @brief Log the message to the standard error output.
 * If the level is greater than or equal to the `LOGGING_LEVEL`, the message will be printed.
 * If the level is greater than or equal to the `ASSERT_LEVEL`, the program will be terminated.
 * @param level The level of the message.
 * @param message The message to log.
 */
void logger(log_level level, char *message)
{
  if (level >= LOGGING_LEVEL)
    write(STDERR_FILENO, message, strlen(message));
  if (level >= LOG_WARNING)
    fprintf(stderr, "%s\n", strerror(errno));
  assert(level <= ASSERT_LEVEL);
}