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
// ------------------------------------------------------------------- AST Node

struct mcc_ast_source_location {
	int start_line;
	int start_col;
	int end_line;
	int end_col;
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
		};

		// MCC AST_EXPRESSION_TYPE_ARRAY_ELEMENT
		struct {
			struct mcc_ast_identifier* array_identifier;
			struct mcc_ast_expression *index;
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

struct mcc_ast_expression *mcc_ast_new_expression_variable(char *identifier);

struct mcc_ast_identifier *mcc_ast_new_identifier(char *identifier);

struct mcc_ast_expression *mcc_ast_new_expression_array_element(char* identifier, struct mcc_ast_expression* index);

void mcc_ast_delete_identifier(struct mcc_ast_identifier *identifier);

void mcc_ast_delete_expression(struct mcc_ast_expression *expression);

// ------------------------------------------------------------------- Types



enum mcc_ast_types{
    INT,
    FLOAT,
    BOOL,
    STRING,
};

struct mcc_ast_type{

	struct mcc_ast_node *node;
    enum mcc_ast_types type_value;

};

void mcc_ast_delete_type(struct mcc_ast_type *type);


// ------------------------------------------------------------------- Declarations

struct mcc_ast_variable_declaration{

	struct mcc_ast_node node;

	struct{
		struct mcc_ast_type *type;
		struct mcc_ast_identifier *identifier;
	};
};

struct mcc_ast_variable_declaration *mcc_ast_new_variable_declaration(enum mcc_ast_types, char* identifier);

void mcc_ast_delete_variable_declaration(struct mcc_ast_variable_declaration* decl);

struct mcc_ast_array_declaration{

	struct mcc_ast_node node;

	struct{
	    struct mcc_ast_literal *size;
		struct mcc_ast_type *type;
		struct mcc_ast_identifier *identifier;
	};

};

struct mcc_ast_array_declaration *mcc_ast_new_array_declaration(enum mcc_ast_types type, struct mcc_ast_literal* size, char* identifier);

void mcc_ast_delete_array_declaration(struct mcc_ast_array_declaration* array_decl);

// ------------------------------------------------------------------ Assignments

enum mcc_ast_assignment_type{
    MCC_AST_ASSIGNMENT_TYPE_VARIABLE,
    MCC_AST_ASSIGNMENT_TYPE_ARRAY,
};

struct mcc_ast_assignment{
    struct mcc_ast_node node;
    enum mcc_ast_assignment_type assignment_type;
    union{
        // MCC_AST_ASSIGNMENT_TYPE_VARIABLE
        struct{
        	struct mcc_ast_identifier *variable_identifier;
        	struct mcc_ast_expression *variable_assigned_value;
        };
        // MCC_AST_ASSIGNMENT_TYPE_ARRAY
		struct{
			struct mcc_ast_identifier *array_identifier;
			struct mcc_ast_expression *array_index;
			struct mcc_ast_expression *array_assigned_value;
		};

    };
};

struct mcc_ast_assignment *mcc_ast_new_variable_assignment (char *identifier, struct mcc_ast_expression *assigned_value);

struct mcc_ast_assignment *mcc_ast_new_array_assignment (char *identifier, struct mcc_ast_expression *index, struct mcc_ast_expression *assigned_value);


//-------------------------------------------------------------------- Identifier

struct mcc_ast_identifier{

	struct mcc_ast_node node;
	char* identifier_name;
};

//-------------------------------------------------------------------- Statements

enum mcc_ast_statement_type {
	MCC_AST_STATEMENT_TYPE_IF_STMT,
	MCC_AST_STATEMENT_TYPE_IF_ELSE_STMT,
	MCC_AST_STATEMENT_TYPE_EXPRESSION,
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
	};
};

struct mcc_ast_statement *mcc_ast_new_statement_if_stmt( struct mcc_ast_expression *condition,
														struct mcc_ast_statement *on_true);

struct mcc_ast_statement *mcc_ast_new_statement_if_else_stmt( struct mcc_ast_expression *condition,
															 struct mcc_ast_statement *on_true,
															 struct mcc_ast_statement *on_false);

struct mcc_ast_statement *mcc_ast_new_statement_expression( struct mcc_ast_expression *expression);

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
		char* string_value;
	};
};

struct mcc_ast_literal *mcc_ast_new_literal_int(long value);

struct mcc_ast_literal *mcc_ast_new_literal_float(double value);

struct mcc_ast_literal *mcc_ast_new_literal_string(char* value);

char* mcc_remove_quotes_from_string(char* string);

struct mcc_ast_literal *mcc_ast_new_literal_bool(bool value);

void mcc_ast_delete_literal(struct mcc_ast_literal *literal);

// -------------------------------------------------------------------- Utility

// clang-format off

#define mcc_ast_delete(x) _Generic((x), \
		struct mcc_ast_expression *: mcc_ast_delete_expression, \
		struct mcc_ast_literal *:    mcc_ast_delete_literal \
	)(x)

// clang-format on

#endif // MCC_AST_H
