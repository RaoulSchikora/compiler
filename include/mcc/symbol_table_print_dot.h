// Symbol Table Printer (dot format)
//
// This module provides basic printing infrastructure for the symbol_table into dot-format.
// It produces nested HTML tables.
// The obtained data can be visualized via GraphViz.

#ifndef MCC_SYMBOL_TABLE_PRINT_DOT_H
#define MCC_SYMBOL_TABLE_PRINT_DOT_H

#include <stdio.h>

#include "mcc/symbol_table.h"

void mcc_symbol_table_print_dot_begin(struct mcc_symbol_table_scope *scope, FILE *out);

void mcc_symbol_table_print_dot_end(FILE *out);

void mcc_symbol_table_print_dot_open_new_scope(FILE *out);

void mcc_symbol_table_print_dot_close_new_scope(FILE *out);

void mcc_symbol_table_print_dot_row(struct mcc_symbol_table_row *row, const char *leading_spaces, FILE *out);

void mcc_symbol_table_print_dot_scope(struct mcc_symbol_table_scope *scope, const char *leading_spaces, FILE *out);

void mcc_symbol_table_print_dot(struct mcc_symbol_table *table, FILE *out);

#endif // MCC_SYMBOL_TABLE_PRINT_DOT_H

