# Known Issues

## Parser

The grammar works without memory leaks, with the following issues still unadressed:

### Function Definition

Since empty parameter-lists are allowed in the grammar, currently a NULL pointer is passed

### Return statement

Empty return statements are supported by the datastructure, but currently lead to segmentation faults

## Printer

Printer currently fails on empty parameter list
