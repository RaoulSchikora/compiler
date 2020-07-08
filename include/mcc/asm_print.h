// Assembly Print Infrastructure
//
// This module provides basic printing infrastructure for the generated assembly code.

#ifndef MCC_ASM_PRINT_H
#define MCC_ASM_PRINT_H

#include "mcc/asm.h"

//---------------------------------------------------------------------------------------- Functions: Print ASM

void mcc_asm_print_line(FILE *out, struct mcc_asm_line *line);

void mcc_asm_print_func(FILE *out, struct mcc_asm_function *func);

void mcc_asm_print_decl(FILE *out, struct mcc_asm_declaration *decl);

void mcc_asm_print_text_sec(FILE *out, struct mcc_asm_text_section *text);

void mcc_asm_print_data_sec(FILE *out, struct mcc_asm_data_section *data);

void mcc_asm_print_asm(FILE *out, struct mcc_asm *head);

void mcc_asm_print_end(FILE *out);

#endif // MCC_ASM_PRINT_H
