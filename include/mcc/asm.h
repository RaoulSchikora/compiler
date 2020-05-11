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
	struct mcc_asm_assembly_line *next;
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

//------------------------------------------------------------------------------------ Functions: Create data structures

struct mcc_asm *mcc_asm_new_asm(struct mcc_asm_data_section *data, struct mcc_asm_text_section *text);

struct mcc_asm_data_section *mcc_asm_new_data_section(struct mcc_asm_declaration *head);

struct mcc_asm_text_section *mcc_asm_new_text_section(struct mcc_asm_function *function);

struct mcc_asm_declaration *
mcc_asm_new_float_declaration(char *identifier, float float_value, struct mcc_asm_declaration *next);

struct mcc_asm_declaration *
mcc_asm_new_db_declaration(char *identifier, char *db_value, struct mcc_asm_declaration *next);

struct mcc_asm_function *
mcc_asm_new_function(char *label, struct mcc_asm_assembly_line *head, struct mcc_asm_function *next);

struct mcc_asm_assembly_line *mcc_asm_new_assembly_line(enum mcc_asm_opcode opcode,
                                                        struct mcc_asm_operand *first,
                                                        struct mcc_asm_operand *second,
                                                        struct mcc_asm_assembly_line *next);

struct mcc_asm_operand *mcc_asm_new_literal_operand(int literal);

struct mcc_asm_operand *mcc_asm_new_register_operand(enum mcc_asm_register reg);

struct mcc_asm_operand *mcc_asm_new_data_operand(struct mcc_asm_declaration *decl);

//------------------------------------------------------------------------------------ Functions: Delete data structures

void mcc_asm_delete_asm(struct mcc_asm *head);

void mcc_asm_delete_text_section(struct mcc_asm_text_section *text_section);

void mcc_asm_delete_data_section(struct mcc_asm_data_section *data_section);

void mcc_asm_delete_all_declarations(struct mcc_asm_declaration *decl);

void mcc_asm_delete_declaration(struct mcc_asm_declaration *decl);

void mcc_asm_delete_all_functions(struct mcc_asm_function *function);

void mcc_asm_delete_function(struct mcc_asm_function *function);

void mcc_asm_delete_all_assembly_lines(struct mcc_asm_assembly_line *line);

void mcc_asm_delete_assembly_line(struct mcc_asm_assembly_line *line);

void mcc_asm_delete_operand(struct mcc_asm_operand *operand);

//---------------------------------------------------------------------------------------- Functions: ASM generation

struct mcc_asm *mcc_asm_generate(struct mcc_ir_row *ir);

#endif // MCC_ASM_H
