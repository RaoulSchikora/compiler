# Intermediate Representation

What do we do with TAC? 

* Create from AST (linear text, done by visiting AST)
* Transform into basic blocks
* Print CFG

--> Organise TAC in BB from the beginning!

## TAC, represented as triples

* Every line has 5 entries: line number, operation, arg1, arg2, pointer to next line
* arg can be either a variable from the symbol table, or a line number 

## Implementation: IR, linear text

* One IR line : Struct containing line number, operation, arg1 and arg2 and a pointer to the next (and previous?) line
* Operation: Enum (since it's instruction set is finite)
* Arg: Struct, that contains a boolean to indicate if the arg comes from the symbol table. Also contains a union of a symbol_table_row and a pointer to a different line of the IR
* Line number is an unsigned int

## Implementation: IR, basic blocks

* Have another structure on top of IR tables: basic block
* One basic block consists of: IR table, pointer to all child BBs
* Pointer to first line, pointer to all the next blocks

## Procedure suggestion

* Define set of TAC operations
* Draft header, define API (think of error handling) and data structures
* Write unit tests
* Write a printer for the IR (use AST printer for CFG, since it's also a tree)
* Write the actual transformation of AST -> TAC
