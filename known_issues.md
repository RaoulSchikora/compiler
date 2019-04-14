# Known Issues

## Parser

### Function Call Expression

* There is no system in place in the data structure or functions to detect function calls with empty arguments-list
* Currently using a function without arguments fails on "assert(arguments);"

### Memory Leaks and failed Unit test

* Function_defs, program, parameters, arguments and expression_function_call are not tested, some are buggy and have read/write-errors --> Memory Leaks
* Unit tests FunctionCall and CompoundStatement fail due to memory errors and are commented out in the macro
