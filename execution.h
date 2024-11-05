#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "ast.h"

int32_t execution(AST *ast, pid_t forked, bool wait);