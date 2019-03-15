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
