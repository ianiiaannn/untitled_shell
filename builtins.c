#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#include "logger.h"
#include "main.h"

/**
 * @brief exit the shell.
 */
void bye()
{
  exit(EXIT_SUCCESS);
}

/**
 * @brief Wrapper for the `chdir()` function.
 */
int32_t cd(char *path)
{
  if (path == NULL || strlen(path) == 0)
    path = getenv("HOME");
  if (path == NULL || strlen(path) == 0)
  {
    logger(LOG_WARNING, "Neither $HOME nor path is supplied.\n");
    return EXIT_FAILURE;
  }
  if (chdir(path) == -1)
  {
    logger(LOG_WARNING, strerror(errno));
    logger(LOG_WARNING, "Failed to change directory.");
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

/**
 * @brief Print the environment variables to the stream.
 */
int32_t env(FILE *stream)
{
  while (*environ)
  {
    fprintf(stream, "%s\n", *environ);
    environ++;
  }
  return EXIT_SUCCESS;
}

/**
 * @brief Set the `PATH` environment variable.
 * @param path The new `PATH` variable. Empty string will clear the `PATH`.
 * @return The return value of the `setenv()` function.
 */
int32_t path(char **path)
{
  char *new_path = "";
  path++;
  while (*path)
  {
    if (new_path == NULL)
      new_path = *path;
    else
    {
      new_path = realloc(new_path, strlen(new_path) + strlen(*path) + 2);
      strcat(new_path, ":");
      strcat(new_path, *path);
    }
    path++;
  }
  return setenv("PATH", new_path, 1);
}

/**
 * @brief Scan if the command is a built-in command.
 * @return `true` if the command is a built-in command, `false` otherwise.
 */
bool scan_builtin(char *search)
{
  if (strcmp(search, "bye") == 0 || strcmp(search, "exit") == 0)
    return true;
  if (strcmp(search, "cd") == 0)
    return true;
  if (strcmp(search, "path") == 0)
    return true;
  if (strcmp(search, "env") == 0)
    return true;
  return false;
}

/**
 * @brief Run the built-in command.
 * This function SHOULD only be called after `scan_builtin()` and the result is true.
 * @return The return value of the built-in command.
 */
int32_t run_builtin(int32_t argc, char **argv)
{
  if (argc == 0 || argv == NULL || argv[0] == NULL)
    logger(LOG_ERROR, "No arguments provided for the built-in command.\n");
  if (strcmp(argv[0], "bye") == 0 || strcmp(argv[0], "exit") == 0)
  {
    bye();
  }
  if (strcmp(argv[0], "cd") == 0)
  {
    if (argc == 1)
      return cd(NULL);
    return cd(argv[1]);
  }
  if (strcmp(argv[0], "path") == 0)
  {
    return path(argv);
  }
  if (strcmp(argv[0], "env") == 0)
  {
    return env(stdout);
  }
  logger(LOG_ERROR, "Unknown built-in command.\n");
  return EXIT_FAILURE;
}
