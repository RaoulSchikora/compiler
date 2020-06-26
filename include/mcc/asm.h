// Assembly Code generation
//
// Here we define the data structures and functions used to generate the assembly code
// It is targeting an x86 system and written in AT&T syntax.

#ifndef MCC_ASM_H
#define MCC_ASM_H

#include "mcc/ir.h"
#include "mcc/stack_size.h"

struct mcc_asm_error {
	bool has_failed;
	struct mcc_asm_data_section *data_section;
	struct mcc_asm_text_section *text_section;
};

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
	MCC_ASM_DECLARATION_TYPE_STRING,
	MCC_ASM_DECLARATION_TYPE_FLOAT,
};

struct mcc_asm_declaration {
	char *identifier;
	enum mcc_asm_declaration_type type;
	union {
		double float_value;
		char *string_value;
	};
	struct mcc_asm_declaration *next;
};

struct mcc_asm_function {
	char *label;
	struct mcc_asm_line *head;
	struct mcc_asm_function *next;
	struct mcc_asm_pos_list *pos_list;
};

enum mcc_asm_opcode {
	MCC_ASM_MOVL,
	MCC_ASM_MOVZBL,
	MCC_ASM_CMPL,
	MCC_ASM_PUSHL,
	MCC_ASM_POPL,
	MCC_ASM_LEAVE,
	MCC_ASM_ADDL,
	MCC_ASM_SUBL,
	MCC_ASM_IMULL,
	MCC_ASM_IDIVL,
	MCC_ASM_SETE,
	MCC_ASM_SETNE,
	MCC_ASM_SETL,
	MCC_ASM_SETG,
	MCC_ASM_SETLE,
	MCC_ASM_SETGE,
	MCC_ASM_SETA,
	MCC_ASM_SETAE,
	MCC_ASM_SETB,
	MCC_ASM_SETBE,
	MCC_ASM_AND,
	MCC_ASM_OR,
	MCC_ASM_RETURN,
	MCC_ASM_CALLL,
	MCC_ASM_XORL,
	MCC_ASM_NEGL,
	MCC_ASM_JE,
	MCC_ASM_JNE,
	MCC_ASM_LABEL,
	MCC_ASM_LEAL,
	MCC_ASM_FLDS,
	MCC_ASM_FSTPS,
	MCC_ASM_FADDP,
	MCC_ASM_FSUBP,
	MCC_ASM_FCOMIP,
	MCC_ASM_FINIT,
	MCC_ASM_FSTP,
	MCC_ASM_FMULP,
	MCC_ASM_FDIVP,
	MCC_ASM_FCHS,
};

struct mcc_asm_line {
	enum mcc_asm_opcode opcode;
	union {
		struct {
			struct mcc_asm_operand *first;
			struct mcc_asm_operand *second;
		};
		// MCC_ASM_LABEL
		unsigned label;
	};
	struct mcc_asm_line *next;
};

enum mcc_asm_operand_type {
	MCC_ASM_OPERAND_REGISTER,
	MCC_ASM_OPERAND_COMPUTED_OFFSET,
	MCC_ASM_OPERAND_DATA,
	MCC_ASM_OPERAND_LITERAL,
	MCC_ASM_OPERAND_FUNCTION,
};

enum mcc_asm_register {
	MCC_ASM_EAX,
	MCC_ASM_EBX,
	MCC_ASM_ECX,
	MCC_ASM_EDX,
	MCC_ASM_ESP,
	MCC_ASM_EBP,
	MCC_ASM_ST,
	MCC_ASM_DL,
};

struct mcc_asm_operand {
	enum mcc_asm_operand_type type;
	union {
		int literal;
		enum mcc_asm_register reg;
		struct mcc_asm_declaration *decl;
		char *func_name;
		// e.g. -24(%ebp, %ebx, 4)
		struct {
			int offset_initial;                  // -24
			enum mcc_asm_register offset_base;   // ebp
			enum mcc_asm_register offset_factor; // ebx
			int offset_size;                  // 4
		};
	};
	int offset;
};

//------------------------------------------------------------------------------------ Functions: Create data structures

struct mcc_asm *
mcc_asm_new_asm(struct mcc_asm_data_section *data, struct mcc_asm_text_section *text, struct mcc_asm_error *err);

struct mcc_asm_data_section *mcc_asm_new_data_section(struct mcc_asm_declaration *head, struct mcc_asm_error *err);

struct mcc_asm_text_section *mcc_asm_new_text_section(struct mcc_asm_function *function, struct mcc_asm_error *err);

struct mcc_asm_declaration *mcc_asm_new_float_declaration(char *identifier,
                                                          float float_value,
                                                          struct mcc_asm_declaration *next,
                                                          struct mcc_asm_error *err);

struct mcc_asm_declaration *mcc_asm_new_string_declaration(char *identifier,
                                                           char *string_value,
                                                           struct mcc_asm_declaration *next,
                                                           struct mcc_asm_error *err);

struct mcc_asm_function *
mcc_asm_new_function(char *label, struct mcc_asm_line *head, struct mcc_asm_function *next, struct mcc_asm_error *err);

struct mcc_asm_line *mcc_asm_new_line(enum mcc_asm_opcode opcode,
                                      struct mcc_asm_operand *first,
                                      struct mcc_asm_operand *second,
                                      struct mcc_asm_line *next,
                                      struct mcc_asm_error *err);

struct mcc_asm_operand *mcc_asm_new_literal_operand(int literal, struct mcc_asm_error *err);

struct mcc_asm_operand *mcc_asm_new_register_operand(enum mcc_asm_register reg, int offset, struct mcc_asm_error *err);

struct mcc_asm_operand *mcc_asm_new_computed_offset_operand(int offset_initial,
                                                            enum mcc_asm_register offset_base,
                                                            enum mcc_asm_register offset_factor,
                                                            int offset_size,
                                                            struct mcc_asm_error *err);

struct mcc_asm_operand *mcc_asm_new_data_operand(struct mcc_asm_declaration *decl, struct mcc_asm_error *err);

struct mcc_asm_pos_list *mcc_asm_new_pos_list(struct mcc_ast_identifier *ident, int offset, struct mcc_asm_error *err);

//------------------------------------------------------------------------------------ Functions: Delete data structures

void mcc_asm_delete_asm(struct mcc_asm *head);

void mcc_asm_delete_text_section(struct mcc_asm_text_section *text_section);

void mcc_asm_delete_data_section(struct mcc_asm_data_section *data_section);

void mcc_asm_delete_all_declarations(struct mcc_asm_declaration *decl);

void mcc_asm_delete_declaration(struct mcc_asm_declaration *decl);

void mcc_asm_delete_all_functions(struct mcc_asm_function *function);

void mcc_asm_delete_function(struct mcc_asm_function *function);

void mcc_asm_delete_all_lines(struct mcc_asm_line *line);

void mcc_asm_delete_line(struct mcc_asm_line *line);

void mcc_asm_delete_operand(struct mcc_asm_operand *operand);

void mcc_asm_delete_pos_list(struct mcc_asm_pos_list *list);

//---------------------------------------------------------------------------------------- Functions: ASM generation

struct mcc_asm *mcc_asm_generate(struct mcc_ir_row *ir);

struct mcc_asm_function *mcc_asm_generate_function(struct mcc_annotated_ir *an_ir, struct mcc_asm_error *err);

#endif // MCC_ASM_H
