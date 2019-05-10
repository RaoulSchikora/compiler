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

## Parser

### Grammar

The grammar contains 13 shift/reduce conflicts.
Also, since the unit test matches any grammar rule, there are multiple reduce/reduce-conflicts. These will however not arise, when the testing mode isn't specified, because the parser can only match the unit-test grammar rule with the use of special "~" characters.


