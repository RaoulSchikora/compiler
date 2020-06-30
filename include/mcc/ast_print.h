// AST Print Infrastructure
//
// This module provides basic printing infrastructure for the AST data
// structure. The DOT printer enables easy visualisation of an AST.

#ifndef MCC_AST_PRINT_H
#define MCC_AST_PRINT_H

#include "mcc/ast.h"

const char *mcc_ast_print_binary_op(enum mcc_ast_binary_op op);

// ---------------------------------------------------------------- DOT Printer

void mcc_ast_print_dot_expression(FILE *out, struct mcc_ast_expression *expression);

void mcc_ast_print_dot_statement(FILE *out, struct mcc_ast_statement *statement);

void mcc_ast_print_dot_literal(FILE *out, struct mcc_ast_literal *literal);

void mcc_ast_print_dot_declaration(FILE *out, struct mcc_ast_declaration *declaration);

void mcc_ast_print_dot_assignment(FILE *out, struct mcc_ast_assignment *assignment);

void mcc_ast_print_dot_type(FILE *out, struct mcc_ast_type *type);

void mcc_ast_print_dot_identifier(FILE *out, struct mcc_ast_identifier *identifier);

void mcc_ast_print_dot_result(FILE *out, struct mcc_parser_result *result);

void mcc_ast_print_dot_compound_statement(FILE *out, struct mcc_ast_compound_statement *compound_statement);

void mcc_ast_print_dot_program(FILE *out, struct mcc_ast_program *program);

void mcc_ast_print_dot_function_definition(FILE *out, struct mcc_ast_function_definition *function_definition);

void mcc_ast_print_dot_parameters(FILE *out, struct mcc_ast_parameters *parameters);

void mcc_ast_print_dot_arguments(FILE *out, struct mcc_ast_arguments *arguments);

// clang-format off

#define mcc_ast_print_dot(out, x) _Generic((x), \
		struct mcc_ast_expression *: mcc_ast_print_dot_expression, \
		struct mcc_ast_statement *: mcc_ast_print_dot_statement, \
		struct mcc_ast_literal *:    mcc_ast_print_dot_literal, \
		struct mcc_ast_declaration *: mcc_ast_print_dot_declaration, \
		struct mcc_ast_assignment *: mcc_ast_print_dot_assignment, \
		struct mcc_ast_type *: mcc_ast_print_dot_type, \
		struct mcc_ast_identifier *: mcc_ast_print_dot_identifier, \
		struct mcc_ast_compound_statement *: mcc_ast_print_dot_compound_statement, \
		struct mcc_ast_program *: mcc_ast_print_dot_program, \
		struct mcc_ast_function_definition *: mcc_ast_print_dot_function_definition, \
		struct mcc_ast_parameters *: mcc_ast_print_dot_parameters, \
		struct mcc_ast_arguments *: mcc_ast_print_dot_arguments  \
	)(out, x)

// clang-format on

#endif // MCC_AST_PRINT_H
