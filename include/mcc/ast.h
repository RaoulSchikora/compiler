// Abstract Syntax Tree (AST)
//
// Here we define the AST data structure of the compiler. It mainly consists of
// *tagged unions* for node types and enums for operators.
//
// In addition to the node type specific members, each node features a common
// member `mmc_ast_node` which serves as a *base-class*. It holds data
// independent from the actual node type, like the source location.
//
// Also note that this makes excessive use of C11's *anonymous structs and
// unions* feature.

#ifndef MCC_AST_H
#define MCC_AST_H

#include <stdbool.h>
#include <string.h>

#include "mcc/parser.h"

// ------------------------------------------------------------------- AST Node

struct mcc_ast_source_location {
	int start_line;
	int start_col;
	int end_line;
	int end_col;
	char *filename;
};

struct mcc_ast_node {
	struct mcc_ast_source_location sloc;
};

// ------------------------------------------------------------------ Operators

enum mcc_ast_binary_op {
	MCC_AST_BINARY_OP_ADD,
	MCC_AST_BINARY_OP_SUB,
	MCC_AST_BINARY_OP_MUL,
	MCC_AST_BINARY_OP_DIV,
	MCC_AST_BINARY_OP_SMALLER,
	MCC_AST_BINARY_OP_GREATER,
	MCC_AST_BINARY_OP_SMALLEREQ,
	MCC_AST_BINARY_OP_GREATEREQ,
	MCC_AST_BINARY_OP_CONJ,
	MCC_AST_BINARY_OP_DISJ,
	MCC_AST_BINARY_OP_EQUAL,
	MCC_AST_BINARY_OP_NOTEQUAL,
};

enum mcc_ast_unary_op {
	MCC_AST_UNARY_OP_NEGATIV,
	MCC_AST_UNARY_OP_NOT,
};

// ---------------------------------------------------------------- Expressions

enum mcc_ast_expression_type {
	MCC_AST_EXPRESSION_TYPE_LITERAL,
	MCC_AST_EXPRESSION_TYPE_BINARY_OP,
	MCC_AST_EXPRESSION_TYPE_PARENTH,
	MCC_AST_EXPRESSION_TYPE_UNARY_OP,
	MCC_AST_EXPRESSION_TYPE_VARIABLE,
	MCC_AST_EXPRESSION_TYPE_ARRAY_ELEMENT,
	MCC_AST_EXPRESSION_TYPE_FUNCTION_CALL,
};

struct mcc_ast_expression {
	struct mcc_ast_node node;

	enum mcc_ast_expression_type type;
	union {
		// MCC_AST_EXPRESSION_TYPE_LITERAL
		struct mcc_ast_literal *literal;

		// MCC_AST_EXPRESSION_TYPE_BINARY_OP
		struct {
			enum mcc_ast_binary_op op;
			struct mcc_ast_expression *lhs;
			struct mcc_ast_expression *rhs;
		};

		// MCC_AST_EXPRESSION_TYPE_PARENTH
		struct mcc_ast_expression *expression;

		// MCC_AST_EXPRESSION_TYPE_UNARY_OP
		struct {
			enum mcc_ast_unary_op u_op;
			struct mcc_ast_expression *child;
		};

		// MCC_AST_EXPRESSION_TYPE_VARIABLE
		struct {
			struct mcc_ast_identifier *identifier;
			struct mcc_symbol_table_row *variable_row;
		};

		// MCC AST_EXPRESSION_TYPE_ARRAY_ELEMENT
		struct {
			struct mcc_ast_identifier *array_identifier;
			struct mcc_ast_expression *index;
			struct mcc_symbol_table_row *array_row;
		};

		// MCC AST_EXPRESSION_TYPE_FUNCTION_CALL
		struct {
			struct mcc_ast_identifier *function_identifier;
			struct mcc_ast_arguments *arguments;
			struct mcc_symbol_table_row *function_row;
		};
	};
};

struct mcc_ast_expression *mcc_ast_new_expression_literal(struct mcc_ast_literal *literal);

struct mcc_ast_expression *mcc_ast_new_expression_binary_op(enum mcc_ast_binary_op op,
                                                            struct mcc_ast_expression *lhs,
                                                            struct mcc_ast_expression *rhs);

struct mcc_ast_expression *mcc_ast_new_expression_parenth(struct mcc_ast_expression *expression);

struct mcc_ast_expression *mcc_ast_new_expression_unary_op(enum mcc_ast_unary_op u_op,
                                                           struct mcc_ast_expression *expression);

struct mcc_ast_expression *mcc_ast_new_expression_variable(struct mcc_ast_identifier *identifier);

struct mcc_ast_identifier *mcc_ast_new_identifier(char *identifier);

struct mcc_ast_expression *mcc_ast_new_expression_array_element(struct mcc_ast_identifier *identifier,
                                                                struct mcc_ast_expression *index);

struct mcc_ast_expression *mcc_ast_new_expression_function_call(struct mcc_ast_identifier *identifier,
                                                                struct mcc_ast_arguments *arguments);

void mcc_ast_delete_identifier(struct mcc_ast_identifier *identifier);

void mcc_ast_delete_expression(struct mcc_ast_expression *expression);

// ------------------------------------------------------------------- Types

enum mcc_ast_types {
	INT,
	FLOAT,
	BOOL,
	STRING,
	VOID,
};

struct mcc_ast_type {

	struct mcc_ast_node node;
	enum mcc_ast_types type_value;
};

void mcc_ast_delete_type(struct mcc_ast_type *type);

// ------------------------------------------------------------------- Declarations

enum mcc_ast_declaration_type {
	MCC_AST_DECLARATION_TYPE_VARIABLE,
	MCC_AST_DECLARATION_TYPE_ARRAY,
};

struct mcc_ast_declaration {

	struct mcc_ast_node node;
	enum mcc_ast_declaration_type declaration_type;
	union {
		struct {
			struct mcc_ast_type *variable_type;
			struct mcc_ast_identifier *variable_identifier;
		};
		struct {
			struct mcc_ast_literal *array_size;
			struct mcc_ast_type *array_type;
			struct mcc_ast_identifier *array_identifier;
		};
	};
};

struct mcc_ast_declaration *mcc_ast_new_variable_declaration(enum mcc_ast_types, struct mcc_ast_identifier *identifier);

void mcc_ast_delete_variable_declaration(struct mcc_ast_declaration *decl);

struct mcc_ast_declaration *mcc_ast_new_array_declaration(enum mcc_ast_types type,
                                                          struct mcc_ast_literal *size,
                                                          struct mcc_ast_identifier *identifier);

void mcc_ast_delete_array_declaration(struct mcc_ast_declaration *array_decl);

void mcc_ast_delete_declaration(struct mcc_ast_declaration *decl);

// ------------------------------------------------------------------ Assignments

enum mcc_ast_assignment_type {
	MCC_AST_ASSIGNMENT_TYPE_VARIABLE,
	MCC_AST_ASSIGNMENT_TYPE_ARRAY,
};

struct mcc_ast_assignment {
	struct mcc_ast_node node;
	enum mcc_ast_assignment_type assignment_type;
	union {
		// MCC_AST_ASSIGNMENT_TYPE_VARIABLE
		struct {
			struct mcc_ast_identifier *variable_identifier;
			struct mcc_ast_expression *variable_assigned_value;
		};
		// MCC_AST_ASSIGNMENT_TYPE_ARRAY
		struct {
			struct mcc_ast_identifier *array_identifier;
			struct mcc_ast_expression *array_index;
			struct mcc_ast_expression *array_assigned_value;
		};
	};
	// pointer to symbol table
	struct mcc_symbol_table_row *row;
};

struct mcc_ast_assignment *mcc_ast_new_variable_assignment(struct mcc_ast_identifier *identifier,
                                                           struct mcc_ast_expression *assigned_value);

struct mcc_ast_assignment *mcc_ast_new_array_assignment(struct mcc_ast_identifier *identifier,
                                                        struct mcc_ast_expression *index,
                                                        struct mcc_ast_expression *assigned_value);

void mcc_ast_delete_assignment(struct mcc_ast_assignment *assignment);

void mcc_ast_delete_variable_assignment(struct mcc_ast_assignment *assignment);

void mcc_ast_delete_array_assignment(struct mcc_ast_assignment *assignment);
//-------------------------------------------------------------------- Identifier

struct mcc_ast_identifier {

	struct mcc_ast_node node;
	char *identifier_name;
};

struct mcc_ast_identifier *mcc_ast_new_identifier(char *identifier);

//-------------------------------------------------------------------- Statements

enum mcc_ast_statement_type {
	MCC_AST_STATEMENT_TYPE_IF_STMT,
	MCC_AST_STATEMENT_TYPE_IF_ELSE_STMT,
	MCC_AST_STATEMENT_TYPE_EXPRESSION,
	MCC_AST_STATEMENT_TYPE_WHILE,
	MCC_AST_STATEMENT_TYPE_DECLARATION,
	MCC_AST_STATEMENT_TYPE_ASSIGNMENT,
	MCC_AST_STATEMENT_TYPE_RETURN,
	MCC_AST_STATEMENT_TYPE_COMPOUND_STMT,
};

struct mcc_ast_statement {
	struct mcc_ast_node node;

	enum mcc_ast_statement_type type;

	union {
		// MCC_AST_STATEMENT_TYPE_IF_STMT,
		struct {
			struct mcc_ast_expression *if_condition;
			struct mcc_ast_statement *if_on_true;
		};
		// MCC_AST_STATEMENT_TYPE_IF_ELSE_STMT
		struct {
			struct mcc_ast_expression *if_else_condition;
			struct mcc_ast_statement *if_else_on_true;
			struct mcc_ast_statement *if_else_on_false;
		};
		// MCC_AST_STATEMENT_TYPE_EXPRESSION
		struct mcc_ast_expression *stmt_expression;
		// MCC_AST_STATEMENT_TYPE_WHILE
		struct {
			struct mcc_ast_expression *while_condition;
			struct mcc_ast_statement *while_on_true;
		};
		// MCC_AST_STATEMENT_TYPE_DECLARATION,
		struct mcc_ast_declaration *declaration;
		// MCC_AST_STATEMENT_TYPE_ASSIGNMENT,
		struct mcc_ast_assignment *assignment;
		// MCC_AST_STATEMENT_TYPE_RETURN
		struct {
			bool is_empty_return;
			struct mcc_ast_expression *return_value;
		};
		// MCC_AST_STATEMENT_TYPE_COMPOUND_STMT
		struct mcc_ast_compound_statement *compound_statement;
	};
};

struct mcc_ast_statement *mcc_ast_new_statement_if_stmt(struct mcc_ast_expression *condition,
                                                        struct mcc_ast_statement *on_true);

struct mcc_ast_statement *mcc_ast_new_statement_if_else_stmt(struct mcc_ast_expression *condition,
                                                             struct mcc_ast_statement *on_true,
                                                             struct mcc_ast_statement *on_false);

struct mcc_ast_statement *mcc_ast_new_statement_expression(struct mcc_ast_expression *expression);

struct mcc_ast_statement *mcc_ast_new_statement_while(struct mcc_ast_expression *condition,
                                                      struct mcc_ast_statement *on_true);

struct mcc_ast_statement *mcc_ast_new_statement_declaration(struct mcc_ast_declaration *declaration);

struct mcc_ast_statement *mcc_ast_new_statement_assignment(struct mcc_ast_assignment *assignment);

struct mcc_ast_statement *mcc_ast_new_statement_return(bool is_empty_return, struct mcc_ast_expression *expression);

struct mcc_ast_statement *mcc_ast_new_statement_compound_stmt(struct mcc_ast_compound_statement *compound_statement);

void mcc_ast_delete_statement(struct mcc_ast_statement *statement);

// ------------------------------------------------------------------- Compound Statement

struct mcc_ast_compound_statement {
	struct mcc_ast_node node;
	bool is_empty;
	bool has_next_statement;
	struct mcc_ast_compound_statement *next_compound_statement;
	struct mcc_ast_statement *statement;
};

struct mcc_ast_compound_statement *mcc_ast_new_compound_stmt(bool is_empty,
                                                             struct mcc_ast_statement *statement,
                                                             struct mcc_ast_compound_statement *next_compound_stmt);

void mcc_ast_delete_compound_statement(struct mcc_ast_compound_statement *compound_statement);

// ------------------------------------------------------------------- Literals

enum mcc_ast_literal_type {
	MCC_AST_LITERAL_TYPE_INT,
	MCC_AST_LITERAL_TYPE_FLOAT,
	MCC_AST_LITERAL_TYPE_BOOL,
	MCC_AST_LITERAL_TYPE_STRING,
};

struct mcc_ast_literal {
	struct mcc_ast_node node;

	enum mcc_ast_literal_type type;
	union {
		// MCC_AST_LITERAL_TYPE_INT
		long i_value;

		// MCC_AST_LITERAL_TYPE_FLOAT
		double f_value;

		// MCC_AST_LITERAL_TYPE_BOOL
		bool bool_value;

		// MCC_AST_LITERAL_TYPE_STRING
		char *string_value;
	};
};

struct mcc_ast_literal *mcc_ast_new_literal_int(long value);

struct mcc_ast_literal *mcc_ast_new_literal_float(double value);

struct mcc_ast_literal *mcc_ast_new_literal_string(char *value);

char *mcc_remove_quotes_from_string(char *string);

struct mcc_ast_literal *mcc_ast_new_literal_bool(bool value);

void mcc_ast_delete_literal(struct mcc_ast_literal *literal);

// ------------------------------------------------------------------- Function Definition

struct mcc_ast_function_definition {
	struct mcc_ast_node node;

	enum mcc_ast_types type;
	struct mcc_ast_identifier *identifier;
	struct mcc_ast_parameters *parameters;
	struct mcc_ast_compound_statement *compound_stmt;
};

void mcc_ast_delete_function_definition(struct mcc_ast_function_definition *function_definition);

struct mcc_ast_function_definition *
mcc_ast_new_void_function_def(struct mcc_ast_identifier *identifier,
                              struct mcc_ast_parameters *parameters,
                              struct mcc_ast_compound_statement *compound_statement);

struct mcc_ast_function_definition *
mcc_ast_new_type_function_def(enum mcc_ast_types type,
                              struct mcc_ast_identifier *identifier,
                              struct mcc_ast_parameters *parameters,
                              struct mcc_ast_compound_statement *compound_statement);

// ------------------------------------------------------------------- Program

struct mcc_ast_program {
	struct mcc_ast_node node;
	bool has_next_function;
	struct mcc_ast_function_definition *function;
	struct mcc_ast_program *next_function;
};

struct mcc_ast_program *mcc_ast_new_program(struct mcc_ast_function_definition *function_definition,
                                            struct mcc_ast_program *next_program);

void mcc_ast_delete_program(struct mcc_ast_program *program);

// ------------------------------------------------------------------- Parameters

struct mcc_ast_parameters {
	struct mcc_ast_node node;
	bool has_next_parameter;
	bool is_empty;
	struct mcc_ast_parameters *next_parameters;
	struct mcc_ast_declaration *declaration;
};

void mcc_ast_delete_parameters(struct mcc_ast_parameters *parameters);

struct mcc_ast_parameters *mcc_ast_new_parameters(bool is_empty,
                                                  struct mcc_ast_declaration *declaration,
                                                  struct mcc_ast_parameters *next_parameters);

// ------------------------------------------------------------------- Arguments

struct mcc_ast_arguments {
	struct mcc_ast_node node;
	bool has_next_expression;
	bool is_empty;
	struct mcc_ast_arguments *next_arguments;
	struct mcc_ast_expression *expression;
};

struct mcc_ast_arguments *
mcc_ast_new_arguments(bool is_empty, struct mcc_ast_expression *expression, struct mcc_ast_arguments *next_arguments);

void mcc_ast_delete_arguments(struct mcc_ast_arguments *arguments);

// ------------------------------------------------------------------- Add built_ins

int mcc_ast_add_built_ins(struct mcc_ast_program *program);

struct mcc_ast_program *mcc_ast_remove_built_ins(struct mcc_ast_program *program);

// ------------------------------------------------------------------- Transforming the complete AST

// Remove everything but one function from the AST
struct mcc_parser_result *mcc_ast_limit_result_to_function(struct mcc_parser_result *result,
                                                           char *wanted_function_name);

// Merge an array of parser results into one single AST (e.g. when combining files)
struct mcc_parser_result *mcc_ast_merge_results(struct mcc_parser_result *array, int size);

// clang-format off

#define mcc_ast_delete(x) _Generic((x), \
		struct mcc_ast_expression *: mcc_ast_delete_expression, \
		struct mcc_ast_declaration *: mcc_ast_delete_declaration, \
		struct mcc_ast_assignment*:	 mcc_ast_delete_assignment, \
		struct mcc_ast_statement *: mcc_ast_delete_statement, \
		struct mcc_ast_literal *:    mcc_ast_delete_literal, \
		struct mcc_ast_compound_statement *: mcc_ast_delete_compound_statement, \
		struct mcc_ast_program *: mcc_ast_delete_program, \
		struct mcc_ast_function_definition *: mcc_ast_delete_function_definition, \
		struct mcc_ast_parameters *: mcc_ast_delete_parameters, \
		struct mcc_ast_arguments *: mcc_ast_delete_arguments, \
		struct mcc_ast_identifier *: mcc_ast_delete_identifier  \
	)(x)

// clang-format on

#endif // MCC_AST_H
