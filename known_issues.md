# Known Issues

## Parser

### Grammar

The grammar contains 4 reduce/reduce conflicts and 13 shift/reduce conflicts, which indicates ambiguity and needs to be solved, by re-writing or precedence rules (see Bison documentation).

### Return statement

Return statements are supported by the datastructure, but currently lead to segmentation faults

