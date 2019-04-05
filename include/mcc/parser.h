// Parser Infrastructure
//
// This defines the interface to the parser component of the compiler.
//
// It tries to convert a given text input to an AST. On success, ownership of
// the AST is transferred to the caller via the `mcc_parser_result` struct.

#ifndef MCC_PARSER_H
#define MCC_PARSER_H

#include <stdio.h>
#include <stdbool.h>

#include "mcc/ast.h"

enum mcc_parser_status {
	MCC_PARSER_STATUS_OK,
	MCC_PARSER_STATUS_UNABLE_TO_OPEN_STREAM,
	MCC_PARSER_STATUS_UNKNOWN_ERROR,
};

enum mcc_parser_entry_point{
	// entry point which are set while parsing
    MCC_PARSER_ENTRY_POINT_EXPRESSION,
    MCC_PARSER_ENTRY_POINT_VARIABLE_DECLARATION,
    MCC_PARSER_ENTRY_POINT_PROGRAM,
	MCC_PARSER_ENTRY_POINT_DECLARATION,
	MCC_PARSER_ENTRY_POINT_ASSIGNMENT,
	MCC_PARSER_ENTRY_POINT_STATEMENT,
};

struct mcc_parser_result {
	enum mcc_parser_status status;
	enum mcc_parser_entry_point entry_point;

	union{
	    // MCC_PARSER_ENTRY_POINT_EXPRESSION
		struct mcc_ast_expression *expression;
		// MCC_PARSER_ENTRY_POINT_DECLARATION
		struct mcc_ast_declaration *declaration;
		// MCC_PARSER_ENTRY_POINT_ASSIGNMENT
		struct mcc_ast_assignment *assignment;
		// MCC_PARSER_ENTRY_POINT_STATEMENT
		struct mcc_ast_statement *statement;
	};
};

struct mcc_parser_result mcc_parse_string(const char *input, enum mcc_parser_entry_point entry_point);

struct mcc_parser_result mcc_parse_file(FILE *input);

void mcc_transform_into_unit_test (const char* in, char* out );

#endif // MCC_PARSER_H
