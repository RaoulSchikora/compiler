#ifndef MCC_IR_H
#define MCC_IR_H

#include <stdbool.h>

#include "mcc/ast.h"
#include "mcc/symbol_table.h"

//---------------------------------------------------------------------------------------- Data structure: IR

enum mcc_ir_instruction {
	MCC_IR_INSTR_ASSIGN,
	MCC_IR_INSTR_LABEL,
	MCC_IR_INSTR_FUNC_LABEL,
	MCC_IR_INSTR_FUNC_CALL,
	MCC_IR_INSTR_JUMP,
	MCC_IR_INSTR_CALL,
	MCC_IR_INSTR_JUMPFALSE,
	MCC_IR_INSTR_PUSH,
	MCC_IR_INSTR_POP,
	MCC_IR_INSTR_PLUS,
	MCC_IR_INSTR_MINUS,
	MCC_IR_INSTR_MULTIPLY,
	MCC_IR_INSTR_DIVIDE,
	MCC_IR_INSTR_MODULO,
	MCC_IR_INSTR_EQUALS,
	MCC_IR_INSTR_NOTEQUALS,
	MCC_IR_INSTR_SMALLER,
	MCC_IR_INSTR_GREATER,
	MCC_IR_INSTR_SMALLEREQ,
	MCC_IR_INSTR_GREATEREQ,
	MCC_IR_INSTR_AND,
	MCC_IR_INSTR_OR,
	MCC_IR_INSTR_RETURN,
	MCC_IR_INSTR_ARRAY,
	MCC_IR_INSTR_NEGATIV,
	MCC_IR_INSTR_NOT,
	MCC_IR_INSTR_UNKNOWN
};

enum mcc_ir_arg_type {
	MCC_IR_TYPE_ROW,
	MCC_IR_TYPE_LIT_INT,
	MCC_IR_TYPE_LIT_FLOAT,
	MCC_IR_TYPE_LIT_BOOL,
	MCC_IR_TYPE_LIT_STRING,
	MCC_IR_TYPE_IDENTIFIER,
	MCC_IR_TYPE_ARR_ELEM,
	MCC_IR_TYPE_LABEL,
	MCC_IR_TYPE_FUNC_LABEL,
};

struct mcc_ir_arg {
	enum mcc_ir_arg_type type;

	union {
		long lit_int;
		double lit_float;
		bool lit_bool;
		char *lit_string;
		struct mcc_ir_row *row;
		unsigned label;
		struct mcc_ast_identifier *ident;
		struct {
			struct mcc_ast_identifier *arr_ident;
			struct mcc_ir_arg *index;
		};
		char *func_label;
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

struct mcc_ir_row *mcc_ir_generate_entry_point(struct mcc_parser_result *result,
                                               struct mcc_symbol_table *table,
                                               enum mcc_parser_entry_point entry_point);
struct mcc_ir_row *mcc_ir_generate(struct mcc_ast_program *ast, struct mcc_symbol_table *table);

//---------------------------------------------------------------------------------------- Cleanup

void mcc_ir_delete_ir(struct mcc_ir_row *head);

void mcc_ir_delete_ir_row(struct mcc_ir_row *row);

void mcc_ir_delete_ir_arg(struct mcc_ir_arg *arg);

// clang-format off

#define mcc_ir_new_arg(x, y) _Generic((x), \
		long:									new_arg_int, \
		double:									new_arg_float, \
		bool:									new_arg_bool, \
        char* :                                 new_arg_string, \
        struct mcc_ir_row *:          			new_arg_row, \
		unsigned:								new_arg_label, \
		struct mcc_ast_identifier *:			new_arg_identifier \
    )(x,y)

// clang-format on

#endif
