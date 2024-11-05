#define _GNU_SOURCE

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "ast.h"
#include "logger.h"

/**
 * @brief Trim the string provided.
 */
char *trim(char *str)
{
  while (isspace(*str) || *str == '\n')
    str++;
  if (*str == 0)
    return str;
  char *end = str + strlen(str) - 1;
  while (end > str && (isspace(*end) || *end == '\n'))
    end--;
  end[1] = '\0';
  return str;
}

/**
 * @brief Create a new (void) AST.
 * @return The pointer to the new AST (NULL).
 */
AST *ast_new()
{
  return NULL;
}

/**
 * @brief Free the AST memory by walking thought every nodes.
 * @param ast_pointer The pointer to the AST.
 */
void ast_free(AST *ast_pointer)
{
  if (ast_pointer == NULL)
    return;
  AST ast = *ast_pointer;
  switch (ast.tag)
  {
  case AST_COMMAND:
    struct AST_COMMAND command = ast.data.AST_COMMAND;
    free(command.executable);
    for (size_t i = 1; i <= command.argc - 1; i++)
      ast_free(command.arguments[i]);
    free(command.arguments);
    break;
  case AST_ARGUMENT:
    struct AST_ARGUMENT argument = ast.data.AST_ARGUMENT;
    free(argument.value);
    break;
  case AST_REDIRECTION:
    struct AST_REDIRECTION redirection = ast.data.AST_REDIRECTION;
    ast_free(redirection.command);
    // free(redirection.file);
    // TODO: Fix this somehow leaking memory
    break;
  case AST_PIPE:
    struct AST_PIPE pipe = ast.data.AST_PIPE;
    ast_free(pipe.left);
    ast_free(pipe.right);
    break;
  case AST_LIST:
    struct AST_LIST list = ast.data.AST_LIST;
    ast_free(list.left);
    ast_free(list.right);
    break;
  case AST_FD:
    // Nothing to free
    break;
  case AST_LITERAL:
    struct AST_LITERAL literal = ast.data.AST_LITERAL;
    ast_free(literal.next);
    free(literal.value);
    break;
  default:
    logger(LOG_ERROR, "Unknown AST tag\n");
    break;
  }
  free(ast_pointer);
}

/**
 * @brief Print the AST struture to the standard output.
 */
void ast_print(AST *ast)
{
  if (ast == NULL)
    return;
  AST ast_value = *ast;
  switch (ast_value.tag)
  {
  case AST_COMMAND:
    struct AST_COMMAND command = ast_value.data.AST_COMMAND;
    printf("AST_COMMAND: %s\n", command.executable);
    printf("argc: %d\n", command.argc);
    for (size_t i = 1; i <= command.argc - 1; i++)
      ast_print(command.arguments[i]);
    break;
  case AST_ARGUMENT:
    struct AST_ARGUMENT argument = ast_value.data.AST_ARGUMENT;
    printf("AST_ARGUMENT: %s\n", argument.value);
    break;
  case AST_REDIRECTION:
    struct AST_REDIRECTION redirection = ast_value.data.AST_REDIRECTION;
    printf("AST_REDIRECTION: %d\n", redirection.AST_REDIRECTION_TYPE);
    ast_print(redirection.command);
    printf("target file: %s\n", redirection.file);
    break;
  case AST_PIPE:
    struct AST_PIPE pipe = ast_value.data.AST_PIPE;
    printf("AST_PIPE\n");
    ast_print(pipe.left);
    ast_print(pipe.right);
    break;
  case AST_LIST:
    struct AST_LIST list = ast_value.data.AST_LIST;
    printf("AST_LIST: %d\n", list.AST_LIST_TYPE);
    ast_print(list.left);
    ast_print(list.right);
    break;
  case AST_FD:
    struct AST_FD fd = ast_value.data.AST_FD;
    printf("AST_FD: %d\n", fd.fd);
    break;
  case AST_LITERAL:
    struct AST_LITERAL literal = ast_value.data.AST_LITERAL;
    printf("AST_LITERAL: %s\n", literal.value);
    ast_print(literal.next);
    break;
  default:
    logger(LOG_ERROR, "Unknown AST tag\n");
    break;
  }
}

/**
 * @brief Parse the command string to an AST.
 * @param ast The AST to append to (Likely a NULL pointer).
 * @param command The command string.
 * @return The generated AST of the given command.
 */
AST *ast_parse_command(AST *ast, char *command)
{
  // Lists or parallel lists
  char list_or_parallel_lists[2] = {';',
                                    '&'};
  for (size_t i = 0; i <= 2 - 1; i++)
  {
    char *list_or_parallel_list = strchr(command, list_or_parallel_lists[i]);
    if (list_or_parallel_list != NULL)
    {
      AST *new_ast = (AST *)malloc(sizeof(AST));
      char *first_substring = (char *)calloc(list_or_parallel_list - command, sizeof(char));
      strncpy(first_substring, command, list_or_parallel_list - command);
      new_ast->tag = AST_LIST;
      new_ast->data.AST_LIST.AST_LIST_TYPE = i == 0 ? AST_LIST_SEQUENTIAL : AST_LIST_PARALLEL;
      new_ast->data.AST_LIST.left = ast_parse_command(ast_new(), first_substring);
      new_ast->data.AST_LIST.right = ast_parse_command(ast_new(), list_or_parallel_list + 1);
      ast = new_ast;
      return ast;
    }
  }
  // And lists and Or lists
  char and_or_lists[2][2] = {"||",
                             "&&"};
  for (size_t i = 0; i <= 2 - 1; i++)
  {
    char *and_or_list = strstr(command, and_or_lists[i]);
    if (and_or_list != NULL)
    {
      AST *new_ast = (AST *)malloc(sizeof(AST));
      char *first_substring = (char *)calloc(and_or_list - command, sizeof(char));
      strncpy(first_substring, command, and_or_list - command);
      new_ast->tag = AST_LIST;
      new_ast->data.AST_LIST.AST_LIST_TYPE = i == 0 ? AST_LIST_OR : AST_LIST_AND;
      new_ast->data.AST_LIST.left = ast_parse_command(ast_new(), first_substring);
      new_ast->data.AST_LIST.right = ast_parse_command(ast_new(), and_or_list + 2);
      ast = new_ast;
      return ast;
    }
  }
  // Pipes
  char *pipe = strchr(command, '|');
  if (pipe != NULL)
  {
    AST *new_ast = (AST *)malloc(sizeof(AST));
    char *first_substring = (char *)calloc(pipe - command, sizeof(char));
    strncpy(first_substring, command, pipe - command);
    new_ast->tag = AST_PIPE;
    new_ast->data.AST_PIPE.left = ast_parse_command(ast_new(), first_substring);
    new_ast->data.AST_PIPE.right = ast_parse_command(ast_new(), pipe + 1);
    ast = new_ast;
    return ast;
  }
  // Redirections
  char redirections[4][3] = {"<<",
                             ">>",
                             "<",
                             ">"};
  for (size_t i = 0; i <= 4 - 1; i++)
  {
    char *redirection = strstr(command, redirections[i]);
    if (redirection != NULL)
    {
      AST *new_ast = (AST *)malloc(sizeof(AST));
      char *first_substring = (char *)calloc(redirection - command, sizeof(char));
      char *second_substring = (char *)calloc(strlen(redirection), sizeof(char));
      strncpy(first_substring, command, redirection - command);
      strcpy(second_substring, redirection + (i <= 1 ? 2 : 1));
      second_substring = trim(second_substring);
      new_ast->tag = AST_REDIRECTION;
      new_ast->data.AST_REDIRECTION.AST_REDIRECTION_TYPE = i;
      new_ast->data.AST_REDIRECTION.command = ast_parse_command(ast_new(), first_substring);
      new_ast->data.AST_REDIRECTION.file = second_substring;

      ast = new_ast;
      return ast;
    }
  }
  // Literals
  char literals[2] = {'\"',
                      '\''};
  for (size_t i = 0; i <= 2 - 1; i++)
  {
    char *literal = strchr(command, literals[i]);
    if (literal != NULL)
    {
      char *end = strchr(literal + 1, literals[i]);
      if (end == NULL)
        logger(LOG_ERROR, "Failed to parse string literal\n");
      char *value = (char *)calloc(end - literal, sizeof(char));
      strncpy(value, literal + 1, end - literal - 1);
      value[end - literal - 1] = '\0';
      AST *new_ast = (AST *)malloc(sizeof(AST));
      new_ast->tag = AST_LITERAL;
      new_ast->data.AST_LITERAL.value = value;
      new_ast->data.AST_LITERAL.next = ast_parse_command(ast_new(), end + 1);
      ast = new_ast;
      return ast;
    }
  }
  // Commands and arguments
  char *tokens = " ";
  {
    char *trimmed_command = trim(strdup(command));
    if (strlen(trimmed_command) == 0)
      return ast;
    char *token = strtok(trimmed_command, tokens);
    if (token == NULL)
      return ast;
    char *executable = (char *)calloc(strlen(token), sizeof(char));
    strncpy(executable, token, strlen(token));
    AST *new_ast = (AST *)malloc(sizeof(AST));
    new_ast->tag = AST_COMMAND;
    new_ast->data.AST_COMMAND.executable = executable;
    new_ast->data.AST_COMMAND.arguments = (AST **)malloc(sizeof(AST *));
    size_t argc = 1;
    while ((token = strtok(NULL, tokens)) != NULL)
    {
      // printf("%p: %s\n", token, token);
      char *argument = (char *)calloc(strlen(token), sizeof(char));
      strcpy(argument, token);
      AST *argument_ast = (AST *)malloc(sizeof(AST));
      argument_ast->data.AST_ARGUMENT.value = argument;
      argument_ast->tag = AST_ARGUMENT;
      new_ast->data.AST_COMMAND.arguments[argc] = argument_ast;
      new_ast->data.AST_COMMAND.arguments = (AST **)realloc(new_ast->data.AST_COMMAND.arguments, (++argc) * sizeof(AST *));
    }
    new_ast->data.AST_COMMAND.argc = argc;
    return new_ast;
  }

  return ast;
}
