// IR Print Infrastructure
//
// This module provides basic printing infrastructure for the IR data
// structure. The DOT printer enables easy visualisation of the IR.

#ifndef MCC_IR_PRINT_H
#define MCC_IR_PRINT_H

#include <stdbool.h>

#include "mcc/ir.h"

void mcc_ir_print_table_begin(FILE *out);

void mcc_ir_print_table_end(FILE *out);

// Set bool to print quotes as \" instead of ". The row is printed without newline.
void mcc_ir_print_ir_row(FILE *out, struct mcc_ir_row *row, bool escape_quotes, bool doubly_escaped);

void mcc_ir_print_ir(FILE *out, struct mcc_ir_row *head, bool escape_quotes, bool doubly_escaped);

#endif
