# mC Compiler

## Prerequisites

- [Meson](http://mesonbuild.com/) in version `0.47.0` or higher
  (you may want to install it via `pip3 install --user meson`)
- [Ninja](https://ninja-build.org/)
- `time`, typically located at `/usr/bin/time`, do not confuse this with the Bash built-in
- `flex` for generating the lexer
- `bison` for generating the parser
- A compiler supporting C11 — typically GCC or Clang
- glibc version 2.20 or higher

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

For simple integration testing of the generated assembly code, we compile mC programs and check if their return values
match the expected values:

    $ ../scripts/run_asm_test.sh

# Known Issues

### Grammar

Since the unit test matches any grammar rule, there are 5 reduce/reduce-conflicts. These will however not arise, when 
the testing mode isn't specified, because the parser can only match the unit-test grammar rule when a dedicated global 
variable is set (which can't happen due to user input).

### Assembly Integration tests

Currently some of the assembly integration tests fail, as well as all "regular" integration tets, since we haven't
implement some mC features yet.

### Other stuff

- In the `mcc` app, we currently ignore the return value (not implemented yet), thus the compiler warns during
  compilation

- Some TODO's in the code base are still in progress.
