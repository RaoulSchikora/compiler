#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mcc/ast.h"
#include "mcc/ast_print.h"
#include "mcc/ast_visit.h"
#include "mcc/parser.h"
#include "mcc/symbol_table.h"
#include "mcc/semantic_checks.h"

#include "mc_cl_parser.inc"
#include "mc_get_ast.inc"


// clang-format off

#define clean_up(x)  _Generic((x), \
			struct mcc_symbol_table * : mcc_symbol_table_delete_table, \
			struct mc_cl_parser_command_line_parser * : mc_cl_parser_delete_command_line_parser, \
			struct mcc_parser_result * : mcc_ast_delete_result, \
			struct mcc_semantic_check * : mcc_semantic_check_delete_single_check\
			)(x)

// clang-format on

int main(int argc, char *argv[])
{

	// ---------------------------------------------------------------------- Parsing and checking command line

	// Get all options and arguments from command line
	char *usage_string = "Utility for printing an abstract syntax tree in the DOT format. The output\n"
	                     "can be visualised using Graphviz. Errors are reported on invalid inputs.\n";
	struct mc_cl_parser_command_line_parser *command_line = mc_cl_parser_parse(argc, argv, usage_string);

	// Check if command line parser returned any errors or if "-h" was passed. If so, help was already printed, return.
	if (!command_line || command_line->options->print_help ||
	    command_line->argument_status == MC_CL_PARSER_ARGSTAT_ERROR ||
	    command_line->argument_status == MC_CL_PARSER_ARGSTAT_FILE_NOT_FOUND) {
		clean_up(command_line);
		return EXIT_FAILURE;
	}


	// ---------------------------------------------------------------------- Parsing provided input and create AST

	// Declare struct that will hold the result of the parser and corresponding pointer
	struct mcc_parser_result result;

	// Invoke parser on input from Stdin
	char *input = NULL;
	if (command_line->argument_status == MC_CL_PARSER_ARGSTAT_STDIN) {
		input = mc_cl_parser_stdin_to_string();
		if(!input){
			// mc_cl_parser_stdin_to_string() prints error message to stderr itself
			clean_up(command_line);
			return EXIT_FAILURE;
		}
		result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
		free(input);
	}

	if(command_line->argument_status == MC_CL_PARSER_ARGSTAT_FILES){
		result = get_ast_from_files(command_line);
	}

	if (result.status != MCC_PARSER_STATUS_OK) {
		fprintf(stderr, "%s", result.error_buffer);
		free(result.error_buffer);
		clean_up(command_line);
		return EXIT_FAILURE;
	}
	// ---------------------------------------------------------------------- Executing option "-f"

	// find correct node of ast tree, depending on wether "-f" was passed, result will then be set to it
	struct mcc_parser_result *intermediate = NULL;
	if (command_line->options->mode == MC_CL_PARSER_MODE_FUNCTION) {
		intermediate = mcc_ast_limit_result_to_function(&result, command_line->options->function);
		if (intermediate == NULL) {
			mc_cl_parser_delete_command_line_parser(command_line);
			mcc_ast_delete_program(result.program);
			return EXIT_FAILURE;
		}
		result = *intermediate;
		if (result.status != MCC_PARSER_STATUS_OK) {
			mcc_ast_delete_program(result.program);
			mc_cl_parser_delete_command_line_parser(command_line);
			if (intermediate != NULL)
				free(intermediate);
			return EXIT_FAILURE;
		}
	}

	// ---------------------------------------------------------------------- Print ast in dot format

	// Print to file or stdout
	if (command_line->options->write_to_file == true) {
		FILE *out = fopen(command_line->options->output_file, "a");
		if (!out) {
			clean_up(&result);
			clean_up(command_line);
			if (intermediate != NULL)
				free(intermediate);
			return EXIT_FAILURE;
		}
		mcc_ast_print_dot_result(out, &result);
		fclose(out);
	} else {
		mcc_ast_print_dot_result(stdout, &result);
	}

	// ---------------------------------------------------------------------- Clean up

	clean_up(command_line);
	clean_up(&result);
	if (intermediate != NULL)
		free(intermediate);

	return EXIT_SUCCESS;
}
