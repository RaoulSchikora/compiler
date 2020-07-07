// CFG Print Infrastructure
//
// This module provides basic printing infrastructure for the CFG data
// structure. The DOT printer enables easy visualisation of a CFG.

#ifndef MCC_CFG_PRINT_H
#define MCC_CFG_PRINT_H

#include "mcc/cfg.h"

//---------------------------------------------------------------------------------------- Functions: Print CFT as .dot

void mcc_cfg_print_dot_begin(FILE *out);

void mcc_cfg_print_dot_end(FILE *out);

void mcc_cfg_print_dot_bb(FILE *out, struct mcc_basic_block *block);

void mcc_cfg_print_dot_ir_row(FILE *out, struct mcc_ir_row *leader);

void mcc_cfg_print_dot_ir(FILE *out, struct mcc_ir_row *leader);

void mcc_cfg_print_dot_cfg(FILE *out, struct mcc_basic_block *head);

#endif // MCC_CFG_PRINT_H
