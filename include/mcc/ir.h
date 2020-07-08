// Intermediate Representation (IR)
//
// This module provides infrastructure for the Intermediate Representation.
// It is implemented as Three Address Code in the form of triples.
// The result of an instruction that takes two or less arguments is assigned to one variable (either temporary or a
// variable from the original code).

#ifndef MCC_IR_H
#define MCC_IR_H

#include <stdbool.h>

#include "mcc/ast.h"
#include "mcc/symbol_table.h"

//---------------------------------------------------------------------------------------- Data structure: Helper for
//---------------------------------------------------------------------------------------- IR generation

struct ir_generation_userdata {
	// First row of IR
	struct mcc_ir_row *head;
	// Current (last) row of IR, set during execution
	struct mcc_ir_row *current;
	// Flag for indicating errors
	bool has_failed;
	// Counts number of (non-function) labels
	unsigned label_counter;
	// Counts float temporaries
	unsigned tmp_counter;
};

//---------------------------------------------------------------------------------------- Data structure: IR

enum mcc_ir_instruction {
	MCC_IR_INSTR_ASSIGN,
	MCC_IR_INSTR_LABEL,
	MCC_IR_INSTR_FUNC_LABEL,
	MCC_IR_INSTR_JUMP,
	MCC_IR_INSTR_CALL,
	MCC_IR_INSTR_JUMPFALSE,
	MCC_IR_INSTR_PUSH,
	MCC_IR_INSTR_POP,
	MCC_IR_INSTR_PLUS,
	MCC_IR_INSTR_MINUS,
	MCC_IR_INSTR_MULTIPLY,
	MCC_IR_INSTR_DIVIDE,
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
	MCC_IR_TYPE_LIT_INT,
	MCC_IR_TYPE_LIT_FLOAT,
	MCC_IR_TYPE_LIT_BOOL,
	MCC_IR_TYPE_LIT_STRING,
	MCC_IR_TYPE_ROW,
	MCC_IR_TYPE_LABEL,
	MCC_IR_TYPE_IDENTIFIER,
	MCC_IR_TYPE_ARR_ELEM,
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
		char *ident;
		struct {
			char *arr_ident;
			struct mcc_ir_arg *index;
		};
		char *func_label;
	};
};

enum mcc_ir_row_types {
	MCC_IR_ROW_INT,
	MCC_IR_ROW_FLOAT,
	MCC_IR_ROW_BOOL,
	MCC_IR_ROW_STRING,
	MCC_IR_ROW_TYPELESS,
};

struct mcc_ir_row_type {
	enum mcc_ir_row_types type;
	signed array_size; // -1 if no array
};

struct mcc_ir_row {
	unsigned row_no;
	enum mcc_ir_instruction instr;
	struct mcc_ir_row_type *type;

	struct mcc_ir_arg *arg1;
	struct mcc_ir_arg *arg2;

	struct mcc_ir_row *prev_row;
	struct mcc_ir_row *next_row;
};

//---------------------------------------------------------------------------------------- Generate IR datastructures

void mcc_ir_generate_arguments(struct mcc_ast_arguments *arguments, struct ir_generation_userdata *data);

void mcc_ir_generate_comp_statement(struct mcc_ast_compound_statement *cmp_stmt, struct ir_generation_userdata *data);

void mcc_ir_generate_assignment(struct mcc_ast_assignment *asgn, struct ir_generation_userdata *data);

void mcc_ir_generate_statememt_while_stmt(struct mcc_ast_statement *stmt, struct ir_generation_userdata *data);

void mcc_ir_generate_statememt_if_else_stmt(struct mcc_ast_statement *stmt, struct ir_generation_userdata *data);

void mcc_ir_generate_statememt_if_stmt(struct mcc_ast_statement *stmt, struct ir_generation_userdata *data);

void mcc_ir_generate_statement_return(struct mcc_ast_statement *stmt, struct ir_generation_userdata *data);

void mcc_ir_generate_declaration(struct mcc_ast_declaration *decl, struct ir_generation_userdata *data);

void mcc_ir_generate_statement(struct mcc_ast_statement *stmt, struct ir_generation_userdata *data);

void mcc_ir_generate_program(struct mcc_ast_program *program, struct ir_generation_userdata *data);

void mcc_ir_generate_function_definition(struct mcc_ast_function_definition *def, struct ir_generation_userdata *data);

struct mcc_ir_arg *mcc_ir_generate_arg_lit(struct mcc_ast_literal *literal, struct ir_generation_userdata *data);

struct mcc_ir_arg *mcc_ir_generate_expression_binary_op(struct mcc_ast_expression *expression,
                                                        struct ir_generation_userdata *data);

struct mcc_ir_arg *mcc_ir_generate_expression_unary_op(struct mcc_ast_expression *expression,
                                                       struct ir_generation_userdata *data);

struct mcc_ir_arg *mcc_ir_generate_expression_var(struct mcc_ast_expression *expression,
                                                  struct ir_generation_userdata *data);

struct mcc_ir_arg *mcc_ir_generate_expression_func_call(struct mcc_ast_expression *expression,
                                                        struct ir_generation_userdata *data);

struct mcc_ir_arg *mcc_ir_generate_expression(struct mcc_ast_expression *expression,
                                              struct ir_generation_userdata *data);

//---------------------------------------------------------------------------------------- Generate IR

struct mcc_ir_row *mcc_ir_generate(struct mcc_ast_program *ast);

//---------------------------------------------------------------------------------------- Cleanup

void mcc_ir_delete_ir_arg(struct mcc_ir_arg *arg);

void mcc_ir_delete_ir_row_type(struct mcc_ir_row_type *type);

void mcc_ir_delete_ir_row(struct mcc_ir_row *row);

void mcc_ir_delete_ir(struct mcc_ir_row *head);

#endif
