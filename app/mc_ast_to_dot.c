#include <stdio.h>
#include <stdlib.h>

#include "mcc/ast.h"
#include "mcc/ast_print.h"
#include "mcc/parser.h"

int main(void)
{
	struct mcc_ast_expression *expr = NULL;

	// parsing phase
	{
		struct mcc_parser_result result = mcc_parse_file(stdin);
		if (result.status != MCC_PARSER_STATUS_OK) {
			return EXIT_FAILURE;
		}
		expr = result.expression;
	}

	mcc_ast_print_dot(stdout, expr);

	// cleanup
	mcc_ast_delete(expr);

	return EXIT_SUCCESS;
}
