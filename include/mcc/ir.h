#ifndef MCC_IR_H
#define MCC_IR_H

#include "mcc/ast.h"
#include "mcc/symbol_table.h"

//---------------------------------------------------------------------------------------- Data structure: IR

enum mcc_ir_instruction {
	MCC_IR_INSTR_LABEL,
	MCC_IR_INSTR_JUMP,
	MCC_IR_INSTR_JUMPFALSE,
	MCC_IR_INSTR_PUSH,
	MCC_IR_INSTR_POP,
	MCC_IR_INSTR_PLUS,
	MCC_IR_INSTR_MINUS,
	MCC_IR_INSTR_MULTIPLY,
	MCC_IR_INSTR_DIVIDE,
	MCC_IR_INSTR_MODULO,
	MCC_IR_INSTR_EQUALS,
	MCC_IR_INSTR_LESS_THAN,
	MCC_IR_INSTR_AND,
	MCC_IR_INSTR_OR,
	MCC_IR_INSTR_RETURN,
	MCC_IR_INSTR_ARRAY_EL
};

enum mcc_ir_arg_type {
	MCC_IR_TYPE_ROW,
	MCC_IR_TYPE_LIT,
};

struct mcc_ir_arg {
	enum mcc_ir_arg_type type;

	union {
		char *lit;
		struct mcc_ir_row *row;
	};
};

struct mcc_ir_row {
	unsigned row_no;
	enum mcc_ir_instruction instr;

	struct mcc_ir_arg *arg1;
	struct mcc_ir_arg *arg2;


	struct mcc_ir_row *prev_row;
	struct mcc_ir_row *next_row;
};

//---------------------------------------------------------------------------------------- Generate IR datastructures

struct mcc_ir_row *mcc_ir_generate_entry_point(struct mcc_parser_result *result, struct mcc_symbol_table *table, enum mcc_parser_entry_point entry_point);
struct mcc_ir_row *mcc_ir_generate(struct mcc_ast_program *ast, struct mcc_symbol_table *table);

//---------------------------------------------------------------------------------------- Cleanup

void mcc_ir_delete_ir(struct mcc_ir_row *head);

void mcc_ir_delete_ir_row(struct mcc_ir_row *row);

void mcc_ir_delete_ir_arg(struct mcc_ir_arg *arg);

// clang-format off

#define mcc_ir_new_arg(x) _Generic((x), \
        char* :                                 mcc_ir_new_arg_lit, \
        struct mcc_ir_row *:          			mcc_ir_new_arg_row \
    )(x)

// clang-format on

#endif
