#pragma once

#define _GNU_SOURCE

#ifndef PROGRAM_NAME
#define PROGRAM_NAME "untitled_shell"
#endif

#ifndef VERSION
#define VERSION "0.0.1"
#endif

#ifndef LOGGING_LEVEL
#define LOGGING_LEVEL LOG_WARNING
#endif

#ifndef ASSERT_LEVEL
#define ASSERT_LEVEL LOG_ERROR
#endif

extern char **environ;
