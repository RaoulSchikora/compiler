# mC Compiler


## Prerequisites

- [Meson](http://mesonbuild.com/) in a recent version (`0.44.0`)
  (you may want to install it via `pip3 install --user meson`)
- [Ninja](https://ninja-build.org/)
- `time`, typically located at `/usr/bin/time`, do not confuse this with the Bash built-in
- `flex` for generating the lexer
- `bison` for generating the parser
- A compiler supporting C11 — typically GCC or Clang

## Build Instructions

First, generate the build directory.

    $ meson builddir
    $ cd builddir

Meson creates Ninja build files.
Let's build.

    $ ninja

Unit tests can be run directly with Ninja (or Meson).

    $ ninja test

For integration testing we try to compile mC programs and compare their output for a given input.

    $ ../scripts/run_integration_tests

# Known Issues

## mc_ast_to_dot

- mc_ast_to_dot leaks memory
- if "-f" is specified and multiple functions match the specified name, currently the first one is used
- in function limit_result_to_function_scope, a pointer is introduced and set to the corresponding program struct. In order for printing to work correctly, the actual value inside the AST of "program.has_next_function" needs to be set to false (which is what happens right now.) In order to free the memory later, a copy of the program struct in question should be made, which will be returned by value. Adjustements in the main method in order to free allocated memory need to be made afterwards.

## Parser

### Grammar

The grammar contains 12 shift/reduce conflicts.
Also, since the unit test matches any grammar rule, there are multiple reduce/reduce-conflicts. These will however not arise, when the testing mode isn't specified, because the parser can only match the unit-test grammar rule when a dedicated global variable is set (which can't happen due to user input)


