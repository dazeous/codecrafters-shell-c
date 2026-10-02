[![progress-banner](https://backend.codecrafters.io/progress/shell/3b11de0e-8725-4395-87db-02240e00cc27)](https://app.codecrafters.io/users/dazeous?r=2qF)

# Build Your Own Shell in C

A POSIX-ish shell written in C, built as part of the
["Build Your Own Shell" challenge](https://app.codecrafters.io/courses/shell/overview)
on [CodeCrafters](https://codecrafters.io).

The goal of the challenge is to build a shell that can interpret user input, run
built-in commands, and execute external programs found on the system `PATH` —
covering command parsing, a REPL, `fork`/`exec` process management, and I/O
redirection along the way.

## Implemented stages

- **Interactive REPL** — reads lines from stdin, prints a `$` prompt, exits
  cleanly on EOF or `exit`
- **Built-ins** — `echo`, `exit`, `type`, `pwd`, `cd` (including `~` expansion)
- **Quoting** — single quotes, double quotes, and backslash escaping
- **Redirection** — `>`, `1>`, `>>`, `1>>`, `2>`, `2>>` with `dup`/`dup2` to
  swap and restore `stdout`/`stderr`
- **External commands** — resolves binaries through `PATH` via `access(X_OK)`,
  then runs them with `fork` + `execv` and `wait`

## Project layout

```
src/main.c            the entire shell
CMakeLists.txt        build definition (C23, links readline)
your_program.sh       build + run the shell locally
.codecrafters/        build + run scripts used by CodeCrafters
codecrafters.yml      pins the C buildpack version on CodeCrafters
```

## Implementation notes

Parsing is done in a single pass in place: `read` walks the raw input line while
`write` compacts it, so quoted and escaped characters are unwritten as the
argument vector (`char *argList[10]`) is built directly inside the input buffer
with no extra allocation. Quoting state is tracked with
`in_single_quotes` / `in_double_quotes` / `in_word` / `escape_sequenced` flags,
which is what allows `echo "hello   world"` to collapse to a single argument
while `echo 'it\'s'` stays literal.

Redirections are detected in a pass over the finished argument list. When one is
found, the original descriptor is saved with `dup` and replaced with `dup2` onto
an `open`ed file, then restored after the command finishes — so a redirection
only affects the command it was attached to. `O_TRUNC` vs `O_APPEND` selects
truncate (`>`) versus append (`>>`) semantics.

Builtin detection is a linear scan over the `builtins` table, and external
commands are resolved by walking the colon-separated `PATH` entries and testing
each candidate with `access(fullpath, X_OK)`.

## Running locally

Requires `cmake`, a C23-capable compiler, and `VCPKG_ROOT` pointing at a vcpkg
installation.

```sh
./your_program.sh
```
or you can just compile and run the src/main.c file.

To build without running:

```sh
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake
cmake --build ./build
```

## Submitting to CodeCrafters

```sh
codecrafters submit
```

Test output is streamed to your terminal. Note that CodeCrafters uses
`.codecrafters/compile.sh` and `.codecrafters/run.sh` — edits to
`your_program.sh` affect only local runs.

