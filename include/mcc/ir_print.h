// IR Print Infrastructure
//
// This module provides basic printing infrastructure for the IR data structure.
//
// Set escape_quotes to escape quotes as \".
// Set doubly_escaped to escape backslashes twice: \,\n,\t become \\\\, \\\\n, \\\\t
// Rows are printed without newline.

#ifndef MCC_IR_PRINT_H
#define MCC_IR_PRINT_H

#include <stdbool.h>

#include "mcc/ir.h"

void mcc_ir_print_table_begin(FILE *out);

void mcc_ir_print_table_end(FILE *out);

void mcc_ir_print_ir(FILE *out, struct mcc_ir_row *head, bool escape_quotes, bool doubly_escaped);

void mcc_ir_print_ir_row(FILE *out, struct mcc_ir_row *row, bool escape_quotes, bool doubly_escaped);

#endif

