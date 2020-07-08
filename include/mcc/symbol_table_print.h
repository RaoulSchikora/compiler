// Symbol Table Printer
//
// This module provides basic printing infrastructure for the symbol table.
// The string leading_spaces is inserted before the line is printed.

#ifndef MCC_SYMBOL_TABLE_PRINT_H
#define MCC_SYMBOL_TABLE_PRINT_H

#include "mcc/symbol_table.h"

// -------------------------------------------------------------------------------- Print entire symbol table

void mcc_symbol_table_print(struct mcc_symbol_table *table, void *data);

// --------------------------------------------------------------------------------

void mcc_symbol_table_print_row(struct mcc_symbol_table_row *row, const char *leading_spaces, FILE *out);

void mcc_symbol_table_print_scope(struct mcc_symbol_table_scope *scope, const char *leading_spaces, FILE *out);

void mcc_symbol_table_print_begin(struct mcc_symbol_table_scope *scope, FILE *out);

void mcc_symbol_table_print_end(FILE *out);

#endif // MCC_SYMBOL_TABLE_PRINT_H
