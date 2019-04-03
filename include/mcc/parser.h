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
    MCC_PARSER_ENTRY_EXPRESSION,
    MCC_PARSER_ENRTY_VARIABLE_DECLARATION,
};

struct mcc_parser_result {
	enum mcc_parser_status status;

	union{
	    // MCC_PARSER_ENTRY_EXPRESSION
		struct mcc_ast_expression *expression;
		// MCC_PARSER_ENTRY_VARIABLE_EXPRESSION
		struct mcc_ast_variable_declaration *variable_declaration;
	};
};

struct mcc_parser_result mcc_parse_string(const char *input);

struct mcc_parser_result mcc_parse_file(FILE *input);

#endif // MCC_PARSER_H
