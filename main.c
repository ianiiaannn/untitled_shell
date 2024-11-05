#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/wait.h>

#include "logger.h"
#include "arguments.h"
#include "execution.h"
#include "ast.h"
#include "main.h"

int32_t main(int32_t argc, char **argv, char **envp)
{
  FILE *input = stdin;
  int32_t opt;
  while ((opt = getopt(argc, argv, "c:vh")) != -1)
  {
    switch (opt)
    {
    case 'v':
      print_version_and_exit();
      break;
    case 'c':
      /*
      opt_command = true;
      command = optarg;
      */
      break;
    case 'h':
      print_help_and_exit();
      break;
    case '?':
      logger(LOG_ERROR, "Unknown option\n");
      break;
    default:
      break;
    }
  }
  if (optind < argc)
  {
    input = fopen(argv[optind], "r");
    if (input == NULL)
      logger(LOG_ERROR, "Failed to open file\n");
  }

  char *line = NULL;
  size_t length = 0;
  ssize_t read = 0;
  bool running = true;
  while (running)
  {
    if (input == stdin)
      printf(PROGRAM_NAME " $ ");
    read = getline(&line, &length, input);
    if (read == -1 && !feof(input))
      logger(LOG_ERROR, "Failed to read line.\n");
    logger(LOG_DEBUG, "Parsing AST.\n");
    fflush(stdout);
    AST *ast = ast_parse_command(ast_new(), line);
#ifdef PRINT_AST
    logger(LOG_DEBUG, "Printing AST.\n");
    ast_print(ast);
#endif
    logger(LOG_DEBUG, "Executing AST.\n");
    execution(ast, 0, false);
    // Fallback :)
    waitpid(-1, NULL, WNOHANG);
    logger(LOG_DEBUG, "Freeing AST.\n");
    ast_free(ast);
    free(line);
    line = NULL;
    logger(LOG_DEBUG, "Line finished.\n");
    if (feof(input))
      running = false;
  }
  exit(EXIT_SUCCESS);
}
