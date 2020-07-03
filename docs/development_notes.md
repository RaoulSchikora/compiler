# Development Notes

## Test Wrappers

Meson provides a `--wrapper` flag to run tests through arbitrary programs, for instance:

    $ meson test --wrapper 'valgrind --error-exitcode=1 --leak-check=full'

Full output can be obtained by also passing `--verbose`.

If you encounter segfaults happening at random, catch them by repeating unit tests multiple times with GDB attached:

    $ meson test --repeat 1000000 --gdb

## Printing and Debugging

An AST printer for the [Dot Format](https://en.wikipedia.org/wiki/DOT_(graph_description_language)) is provided.
Together with [Graphviz](https://graphviz.gitlab.io/), ASTs can be visualised.

    $ ./mc_ast_to_dot ../test/integration/fib/fib.mc | xdot -

or

    $ ./mc_ast_to_dot ../test/integration/fib/fib.mc | dot -Tpng > fib_ast.png
    $ xdg-open fib_ast.png

## `mcc` Stub

A stub for the mC compiler is provided to ease infrastructure development.
It can already be used with the integration test runner.

    $ MCC=../scripts/mcc_stub ../scripts/run_integration_tests

## Checking error execution paths

The script `mallocfail` in "scripts/" provides a gdb wrapper, that runs a given program repeatedly and systematically 
lets one `malloc` after the other fail. 
Requires the installation of [mallocfail](https://github.com/ralight/mallocfail) to "/usr/local/lib/mallocfail.so", 
and compilation with debug option.
See usage info for how to run it (`mallocfail -h`), when encoutering a segfaulting program, gdb will halt and ask if
you want to quit. Press "n" and continue debugging in gdb.

