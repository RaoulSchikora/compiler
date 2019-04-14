# Known Issues

## Parser

### Function Call Expression

* There is no system in place in the data structure or functions to detect function calls with empty arguments-list
* Currently using a function without arguments fails on "assert(arguments);"

### Function Definition

* There is no system to store wether a function has a type or void. Currently the visitor/printer doesn't print the type

### Memory Leaks and failed Unit test

* Function_defs, program, parameters, arguments and expression_function_call are not tested, some are buggy and have read/write-errors --> Memory Leaks
* Unit test FunctionCall fails due to memory errors and are commented out in the macro
* Unit test CompoundStatement leaks memory
