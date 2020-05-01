// IR Print Infrastructure
//
// This module provides basic printing infrastructure for the AST data
// structure. The DOT printer enables easy visualisation of an AST.

#ifndef MCC_IR_PRINT_H
#define MCC_IR_PRINT_H

#include <stdio.h>

#include "mcc/ir.h"

void mcc_ir_print_table_begin(FILE *out);

void mcc_ir_print_table_end(FILE *out);

void mcc_ir_print_ir_row(FILE *out, struct mcc_ir_row *row);

void mcc_ir_print_ir(FILE *out, struct mcc_ir_row *head);

#endif

