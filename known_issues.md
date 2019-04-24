# Known Issues

## mc_ast_to_dot

Leaks memory on invalid inputs and performs 3 invalid reads and writes

## Parser

### Grammar

The grammar contains 4 reduce/reduce conflicts and 13 shift/reduce conflicts, which indicates ambiguity and needs to be solved, by re-writing or precedence rules (see Bison documentation).

