// Assembly Code generation
//
// Here we define the data structures and functions used to generate the assembly code
// It is targeting an x86 system and written in AT&T syntax.

#ifndef MCC_ASM_H
#define MCC_ASM_H

#include "mcc/ir.h"

//---------------------------------------------------------------------------------------- Data structure: ASM

struct mcc_asm {
	struct mcc_asm_text_section *text_section;
	struct mcc_asm_data_section *data_section;
};

struct mcc_asm_text_section {
	struct mcc_asm_function *function;
};

struct mcc_asm_data_section {
	struct mcc_asm_declaration *head;
};

enum mcc_asm_declaration_type {
	MCC_ASM_DECLARATION_TYPE_DB,
	MCC_ASM_DECLARATION_TYPE_FLOAT,
};

struct mcc_asm_declaration {
	char *identifier;
	enum mcc_asm_declaration_type type;
	union {
		float float_value;
		char *db_value;
	};
	struct mcc_asm_declaration *next;
};

struct mcc_asm_function {
	char *label;
	struct mcc_asm_assembly_line *head;
	struct mcc_asm_function *next;
};

enum mcc_asm_opcode {
	MCC_ASM_MOVL,
	MCC_ASM_PUSHL,
};

struct mcc_asm_assembly_line {
	enum mcc_asm_opcode opcode;
	struct mcc_asm_operand *first;
	struct mcc_asm_operand *second;
};

enum mcc_asm_operand_type {
	MCC_ASM_OPERAND_REGISTER,
	MCC_ASM_OPERAND_DATA,
	MCC_ASM_OPERAND_LITERAL,
};

enum mcc_asm_register {
	MCC_ASM_EAX,
	MCC_ASM_EBX,
	MCC_ASM_ECX,
	MCC_ASM_EDX,
	MCC_ASM_ESP,
	MCC_ASM_EBP,
};

struct mcc_asm_operand {
	enum mcc_asm_operand_type type;
	union {
		int literal;
		enum mcc_asm_register reg;
		struct mcc_asm_declaration *decl;
	};
};

//---------------------------------------------------------------------------------------- Functions: ASM

struct mcc_asm *mcc_asm_generate(struct mcc_ir_row *ir);

void mcc_asm_delete_asm(struct mcc_asm *head);

#endif // MCC_ASM_H
