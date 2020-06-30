#ifndef MC_GET_AST_INC
#define MC_GET_AST_INC

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "mcc/ast.h"

#include "mc_cl_parser.inc"

// Parse file and return the result of the parser
struct mcc_parser_result parse_file(char *filename);

// Parse all given files and return a single AST
struct mcc_parser_result get_ast_from_files(struct mc_cl_parser_command_line_parser *command_line);

struct mcc_parser_result get_ast_from_stdin(bool quiet);

struct mcc_parser_result get_ast_from_files(struct mc_cl_parser_command_line_parser *command_line)
{

	struct mcc_parser_result parse_results[command_line->arguments->size];

	// Iterate over all files and hand them to parser
	for (int i = 0; i < command_line->arguments->size; i++) {

		parse_results[i] = parse_file(*(command_line->arguments->args + i));

		if (parse_results[i].status != MCC_PARSER_STATUS_OK) {

			// Free all other returned ASTs
			for (int j = 0; j < i; j++) {
				mcc_ast_delete_result(parse_results + j);
				free(parse_results + j);
			}
			return parse_results[i];
		}
	}
	return *(mcc_ast_merge_results(parse_results, command_line->arguments->size));
}

struct mcc_parser_result get_ast_from_stdin(bool quiet)
{
	struct mcc_parser_result result;
	char *input = mc_cl_parser_stdin_to_string(quiet);
	if (!input) {
		// mc_cl_parser_stdin_to_string() prints error message to stderr itself
		result.status = MCC_PARSER_STATUS_UNABLE_TO_OPEN_STREAM;
		result.error_buffer = NULL;
		return result;
	}
	result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
	free(input);
	return result;
}

struct mcc_parser_result parse_file(char *filename)
{
	FILE *f = fopen(filename, "rt");
	if (!f) {
		struct mcc_parser_result result = {
		    .status = MCC_PARSER_STATUS_UNKNOWN_ERROR,
		};
		char *string = malloc(sizeof(char) * (1 + strlen("unable to open file\n")));
		if (!string) {
			result.error_buffer = NULL;
		} else {
			strcpy(string, "unable to open file\n");
			result.error_buffer = string;
		}
		return result;
	}
	struct mcc_parser_result return_value;
	return_value = mcc_parse_file(f, MCC_PARSER_ENTRY_POINT_PROGRAM, filename);
	fclose(f);
	return return_value;
}

#endif // MC_GET_AST_INC
