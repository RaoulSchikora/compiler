# Known Issues

## Parser

The grammar works without memory leaks, with the following issues still unadressed:

### Function Call Expression

* There is no system in place to detect function calls with empty arguments-list
* Currently a NULL pointer is passed in the grammar and set in the datastructure. 

### Function Definition

* There is an enum to store wether a function has a type or void. Currently the visitor/printer doesn't print the type
* Since empty parameter-lists are allowed in the grammar, currently a NULL pointer is passed

### Return statement

Like Function Call expression, for "return;" without return values, a NULL pointer is passed

## Printer

Printer currently fails on empty parameter list
