#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>

#include "ast.h"
#include "logger.h"
#include "bulitins.h"

int32_t execution(AST *ast, bool forked, bool parallel)
{
  if (ast == NULL)
    return EXIT_FAILURE;
  AST ast_value = *ast;
  switch (ast_value.tag)
  {
  case AST_COMMAND:
  {
    struct AST_COMMAND command = ast_value.data.AST_COMMAND;
    if (command.argc == 0)
      logger(LOG_ERROR, "No command provided.\n");
    char **arguments = calloc(sizeof(char *) * (command.argc + 1), sizeof(char *));
    arguments[0] = command.executable;
    if (command.argc > 1)
      for (size_t i = 1; i <= command.argc - 1; i++)
      {
        struct AST_ARGUMENT argument = command.arguments[i]->data.AST_ARGUMENT;
        arguments[i] = argument.value;
      }
    arguments[command.argc] = NULL;
    // Because of the spec, our builtins are preferred over system commands
    if (scan_builtin(command.executable))
      return run_builtin(command.argc, arguments);
    if (forked)
    {
      {
        int32_t result = execvpe(command.executable, arguments, environ);
        if (result == -1)
          logger(LOG_WARNING, "Failed to execute command.\n");
        exit(result);
      }
      exit(execvpe(command.executable, arguments, environ));
    }
    else
    {
      pid_t pid = fork();
      if (pid == -1)
        logger(LOG_ERROR, "Failed to fork.\n");
      if (pid == 0)
      {
        int32_t result = execvpe(command.executable, arguments, environ);
        if (result == -1)
          logger(LOG_WARNING, "Failed to execute command.\n");
        exit(result);
      }
    }
    return EXIT_SUCCESS;
    break;
  }
  case AST_ARGUMENT:
    logger(LOG_ERROR, "Unreachable code reached. Arguments should always be leaf nodes.\n");
    break;
  case AST_REDIRECTION:
  {
    struct AST_REDIRECTION redirection = ast_value.data.AST_REDIRECTION;
    pid_t pid = fork();
    if (pid == -1)
      logger(LOG_ERROR, "Failed to fork.\n");
    if (pid == 0)
    {
      int32_t file_descriptor = -1;
      switch (redirection.AST_REDIRECTION_TYPE)
      {
      case AST_REDIRECTION_APPEND_LEFT:
        file_descriptor = open(redirection.file, O_RDONLY);
        break;
      case AST_REDIRECTION_APPEND_RIGHT:
        file_descriptor = open(redirection.file, O_WRONLY | O_CREAT | O_APPEND, 0644);
        break;
      case AST_REDIRECTION_LEFT:
        file_descriptor = open(redirection.file, O_RDONLY);
        break;
      case AST_REDIRECTION_RIGHT:
        file_descriptor = open(redirection.file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        break;
      default:
        logger(LOG_ERROR, "Unknown redirection type\n");
        break;
      }
      if (file_descriptor == -1)
        logger(LOG_ERROR, "Failed to open file\n");
      if (dup2(file_descriptor, redirection.AST_REDIRECTION_TYPE == AST_REDIRECTION_LEFT ? STDIN_FILENO : STDOUT_FILENO) == -1)
        logger(LOG_ERROR, "Failed to duplicate file descriptor\n");
      exit(execution(redirection.command, true, true));
    }
    int32_t status = 0;
    waitpid(pid, &status, 0);
    return WEXITSTATUS(status);
    break;
  }
    logger(LOG_ERROR, "Redirection not implemented\n");
    break;
  case AST_PIPE:
  {
    pid_t left_pid = fork(), right_pid = -1;
    int32_t left_status = 0, right_status = 0;
    int32_t pipe_between_process[2];
    pipe(pipe_between_process);
    if (left_pid == -1)
      logger(LOG_ERROR, "Failed to fork left leaf.\n");
    if (left_pid == 0)
    {
      dup2(pipe_between_process[1], STDOUT_FILENO);
      close(pipe_between_process[0]);
      exit(execution(ast_value.data.AST_PIPE.left, 0, true));
    }
    right_pid = fork();
    if (right_pid == -1)
      logger(LOG_ERROR, "Failed to fork right leaf.\n");
    if (right_pid == 0)
    {
      dup2(pipe_between_process[0], STDIN_FILENO);
      close(pipe_between_process[1]);
      exit(execution(ast_value.data.AST_PIPE.right, 0, true));
    }

    close(pipe_between_process[0]);
    close(pipe_between_process[1]);
    waitpid(left_pid, &left_status, 0);
    waitpid(right_pid, &right_status, 0);

    return WEXITSTATUS(left_status) | WEXITSTATUS(right_status);
    break;
  }
  case AST_LIST:
  {
    struct AST_LIST list = ast_value.data.AST_LIST;
    // Left leaf
    pid_t left_pid = fork(), right_pid = -1;
    int32_t left_status = 0, right_status = 0;
    if (left_pid == -1)
      logger(LOG_ERROR, "Failed to fork left leaf.\n");
    if (left_pid == 0)
      exit(execution(list.left, 0, true));
    if (!(list.AST_LIST_TYPE == AST_LIST_PARALLEL))
      waitpid(left_pid, &left_status, 0);
    // Right leaf
    // If the left leaf successes and the list is OR, don't execute the right leaf
    // If the left leaf fails and the list is AND, don't execute the right leaf
    if (!((WEXITSTATUS(left_status) == EXIT_SUCCESS && list.AST_LIST_TYPE == AST_LIST_OR) ||
          (WEXITSTATUS(left_status) != EXIT_SUCCESS && list.AST_LIST_TYPE == AST_LIST_AND)))
    {
      right_pid = fork();
      if (right_pid == -1)
        logger(LOG_ERROR, "Failed to fork right leaf.\n");
      if (right_pid == 0)
        exit(execution(list.right, 0, true));
    }
    if (list.AST_LIST_TYPE == AST_LIST_PARALLEL)
      waitpid(left_pid, &left_status, 0);
    else
      waitpid(right_pid, &right_status, 0);

    return WEXITSTATUS(left_status) | WEXITSTATUS(right_status);
    break;
  }
  case AST_FD:
    logger(LOG_ERROR, "File descriptors not implemented\n");
    break;
  case AST_LITERAL:
    logger(LOG_ERROR, "Literals not implemented\n");
    break;
  default:
    logger(LOG_ERROR, "Unknown AST tag\n");
    break;
  }

  return EXIT_FAILURE;
}
