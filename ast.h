#pragma once

#define _GNU_SOURCE

#include <stdint.h>

typedef struct AST AST;

struct AST
{
  enum
  {
    AST_COMMAND,
    AST_ARGUMENT,
    AST_REDIRECTION,
    AST_PIPE,
    AST_LIST,
    AST_FD,
    AST_LITERAL,
  } tag;
  union
  {
    struct AST_COMMAND
    {
      char *executable;
      AST **arguments;
      int32_t argc;
    } AST_COMMAND;
    struct AST_ARGUMENT
    {
      char *value;
    } AST_ARGUMENT;
    struct AST_REDIRECTION
    {
      AST *command;
      char *file;
      enum
      {
        AST_REDIRECTION_APPEND_LEFT,
        AST_REDIRECTION_APPEND_RIGHT,
        AST_REDIRECTION_LEFT,
        AST_REDIRECTION_RIGHT,
      } AST_REDIRECTION_TYPE;
    } AST_REDIRECTION;
    struct AST_PIPE
    {
      AST *left;
      AST *right;
    } AST_PIPE;
    struct AST_LIST
    {
      AST *left;
      AST *right;
      enum
      {
        AST_LIST_AND,
        AST_LIST_OR,
        AST_LIST_PARALLEL,
        AST_LIST_SEQUENTIAL,
      } AST_LIST_TYPE;
    } AST_LIST;
    struct AST_FD
    {
      int32_t fd;
    } AST_FD;
    struct AST_LITERAL
    {
      char *value;
      AST *next;
    } AST_LITERAL;
  } data;
};

AST *ast_new();
AST *ast_parse_command(AST *ast, char *command);
void ast_print(AST *ast);

void ast_free(AST *ast);