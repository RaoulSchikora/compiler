# Development Notes

## Test Wrappers

Meson provides a `--wrapper` flag to run tests through arbitrary programs, for instance:

    $ meson test --wrapper 'valgrind --error-exitcode=1 --leak-check=full'

Full output can be obtained by also passing `--verbose`.

If you encounter segfaults happening at random, catch them by repeating unit tests multiple times with GDB attached:

    $ meson test --repeat 1000000 --gdb

## Printing and Debugging

Several printers for the [Dot Format](https://en.wikipedia.org/wiki/DOT_(graph_description_language)) are provided.
Together with [Graphviz](https://graphviz.gitlab.io/), ASTs, symbol tables and control flow graphs can be visualised.

    $ ./mc_ast_to_dot ../test/integration/fib/fib.mc | dot -Tpng > fib_ast.png
    $ ./mc_symbol_table -d ../test/integration/fib/fib.mc | dot -Tpng > fib_ast.png
    $ ./mc_cfg_to_dot ../test/integration/fib/fib.mc | dot -Tpng > fib_ast.png

Symbol tables, intermediate representation, and assembly code can be output in plain format directly:

    $ ./mc_ir ../test/integration/fib/fib.mc
    $ ./mc_symbol_table ../test/integration/fib/fib.mc
    $ ./mc_asm ../test/integration/fib/fib.mc

## Testing error execution paths

The script `run_mallocfail` in "scripts/" provides a gdb wrapper, that runs a given program repeatedly and 
systematically lets one `malloc` after the other fail. 

It requires the installation of [mallocfail](https://github.com/ralight/mallocfail) and compilation with debug option.
See usage info for how to run it (`run_mallocfail -h`). 

When encoutering a segfaulting program, gdb will halt and ask if you want to quit. 
Press "n" and continue debugging in gdb.

