# untitled_shell

A simple shell implemented with abstract syntax tree written in pure C with GNU extensions.

This project is not intended to be a full-featured shell, but rather a simple shell with a simple AST implementation. Note that many features are missing, and the code may crash or behave unexpectedly. Please use it as a learning resource only.

## (Expected) Behavior

### Parsing

The `ast_parse_command()` function is responsible for parsing the input string and building the AST. The input string is scanned in the following order:

- If the input string is empty, return `NULL`.
- If the input string contains `;` or `&`, allocate a `AST_LIST`, split the input string into two parts and parse each part separately.
- If the input string contains `&&` or `||`, allocate a `AST_LIST`, split the input string and parse it.
- If the input string contains `|`, allocate a `AST_PIPE`, split the input string and parse it.
- If the input string contains `<`, `>`, `<<` or `>>`, allocate a `AST_REDIRECTION`, parse the left side as command and right side as a file.
- If the input string contains `"` or `'`, allocate a `AST_LITERAL`, parse the quoted content as literals.
- Tokenize remaining string with `<space>`, allocate a `AST_COMMAND`, and parse as command followed by arguments.
  - `argc` >= 1
  - `arguments[0]` is not defined.
  - `arguments[n]`, 1 >= n > argc are pointers to `AST_ARGUMENT`. `AST_ARGUMENT` should always to be leaf nodes.

### Dumping and Freeing

The `ast_print()` and `ast_free()` will dump the content (to stdout) and free the AST, respectively.

### Executing

The `execution()` function accepts a AST and executes it.

- `AST_COMMAND` `fork()` (if not forked) and `execvpe()` the command with `AST_ARGUMENTS`.
- `AST_ARGUMENTS` should not be passed into this function.
- `AST_LIST` `fork()` and calls the `execution()` function. `waitpid()` if specified.
- `AST_PIPE` open pipes, `fork()` and calls the `execution()`.
- Other tags are not implemented.

### Builtin functions

The shell checks if a function by passing argv[0] to `scan_builtin()`. If the executable match a builtin command, `run_builtin()` is called and builtin is executed.

- `bye` or `exit` exits the shell.
- `cd` wraps the `chdir()` function.
- `env` dumps the `extern char **environ` to stdout.
- `path` sets the `PATH` environment variable. Parameters are separated by space.

## Build

```bash
make
```

or

```bash
make dev
```

For verbose output and printing AST by default.

## Credits

[Abstract Syntax Tree: An Example in C](https://keleshev.com/abstract-syntax-tree-an-example-in-c/) for the nice example and base `Tagged union` AST implementation.
