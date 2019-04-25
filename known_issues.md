# Known Issues

## mc_ast_to_dot

Leaks memory on some invalid inputs(e.g. expression as input to stdin w/o test mode) and performs invalid reads and writes
Leaks memory if files are passed as input

## Parser

### Grammar

The grammar contains 4 reduce/reduce conflicts and 13 shift/reduce conflicts, which indicates ambiguity and needs to be solved, by re-writing or precedence rules (see Bison documentation).


