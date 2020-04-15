#ifndef MCC_IR_H
#define MCC_IR_H

#include "mcc/ast.h"
#include "mcc/symbol_table.h"

//---------------------------------------------------------------------------------------- Data structure: IR

enum mcc_ir_instruction {
	MCC_IR_INSTR_JUMP,
	MCC_IR_INSTR_JUMPFALSE,
	// Remember to update ir_print.c if you add more here
};

enum mcc_ir_arg_type {
	MCC_IR_TYPE_ROW,
	MCC_IR_TYPE_VAR,
};

struct mcc_ir_arg {
	enum mcc_ir_arg_type type;

	union {
		char *var;
		struct mcc_ir_row *row;
	};
};

struct mcc_ir_row {
	enum mcc_ir_instruction instr;

	struct mcc_ir_arg *arg1;
	struct mcc_ir_arg *arg2;

	unsigned row_no;

	struct mcc_ir_row *prev_row;
	struct mcc_ir_row *next_row;
};

//---------------------------------------------------------------------------------------- Generate IR datastructures

struct mcc_ir_row *mcc_ir_generate(struct mcc_ast_program *ast, struct mcc_symbol_table *table);

struct mcc_ir_row *mcc_ir_new_row(int row_no, struct mcc_ir_arg *arg1, struct mcc_ir_arg *arg2, enum mcc_ir_instruction instr);

struct mcc_ir_arg *mcc_ir_new_arg_row(struct mcc_ir_row *row);

struct mcc_ir_arg *mcc_ir_new_arg_var(char *var);

//---------------------------------------------------------------------------------------- Generate IR for different AST nodes



//---------------------------------------------------------------------------------------- Cleanup

void mcc_ir_delete_ir(struct mcc_ir_row *head);

void mcc_ir_delete_ir_row(struct mcc_ir_row *row);

void mcc_ir_delete_ir_arg(struct mcc_ir_arg *arg);

// clang-format off

#define mcc_ir_new_arg(x) _Generic((x), \
        char* :                                 mcc_ir_new_arg_var, \
        struct mcc_ir_row *:          			mcc_ir_new_arg_row \
    )(x)

// clang-format on

#endif