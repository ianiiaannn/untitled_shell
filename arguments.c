#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>

#include "main.h"

void print_help_and_exit()
{
  printf("Usage: ./" PROGRAM_NAME " [-v] [-h] [-c command] [file]\n");
  exit(EXIT_SUCCESS);
}
void print_version_and_exit()
{
  printf(PROGRAM_NAME " version " VERSION ".\n");
  exit(EXIT_SUCCESS);
}