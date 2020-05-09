// Assembly Print Infrastructure
//
// This module provides basic printing infrastructure for the generated assembly code.

#ifndef MCC_ASM_PRINT_H
#define MCC_ASM_PRINT_H

#include "mcc/asm.h"

//---------------------------------------------------------------------------------------- Functions: Print ASM

void mcc_asm_print_dot_begin(FILE *out);

void mcc_asm_print_dot_end(FILE *out);

void mcc_asm_print_dot_asm(FILE *out, struct mcc_asm *head);

#endif // MCC_ASM_PRINT_H
