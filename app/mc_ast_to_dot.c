#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>

#include "mcc/ast.h"
#include "mcc/ast_print.h"
#include "mcc/parser.h"

#include "mc_cl_parser.inc"
#include "mc_get_ast.inc"

// register datastructures with register_cleanup and they will be deleted on exit
#include "mc_cleanup.inc"

int main(int argc, char *argv[])
{

	// ---------------------------------------------------------------------- Parsing and checking command line

	// Get all options and arguments from command line
	char *usage_string = "Utility for printing an abstract syntax tree in the DOT format. The output\n"
	                     "can be visualised using Graphviz. Errors are reported on invalid inputs.\n";
	struct mc_cl_parser_command_line_parser *command_line =
	    mc_cl_parser_parse(argc, argv, usage_string, MC_AST_TO_DOT);
	register_cleanup(command_line);

	// Check if command line parser returned any errors or if "-h" was passed. If so, help was already printed,
	// return.
	if (!command_line)
		return EXIT_FAILURE;
	if (command_line->options->print_help || command_line->argument_status == MC_CL_PARSER_ARGSTAT_ERROR ||
	    command_line->argument_status == MC_CL_PARSER_ARGSTAT_FILE_NOT_FOUND) {
		return EXIT_FAILURE;
	}

	// ---------------------------------------------------------------------- Parsing provided input and create AST

	// Declare struct that will hold the result of the parser and corresponding pointer
	struct mcc_parser_result result;

	switch (command_line->argument_status) {
	case MC_CL_PARSER_ARGSTAT_STDIN:
		result = get_ast_from_stdin(command_line->options->quiet);
		break;
	case MC_CL_PARSER_ARGSTAT_FILES:
		result = get_ast_from_files(command_line);
		break;
	default:
		return EXIT_FAILURE;
	}
	register_cleanup(result.error_buffer);
	register_cleanup(result.program);

	if (result.status != MCC_PARSER_STATUS_OK) {
		if (result.error_buffer) {
			fprintf(stderr, "%s", result.error_buffer);
		} else {
			fprintf(stderr, "Parsing failed. Unknwon error.\n");
		}
		return EXIT_FAILURE;
	}

	// ---------------------------------------------------------------------- Print AST in dot format

	// Print to file or stdout
	if (command_line->options->write_to_file == true) {
		FILE *out = fopen(command_line->options->output_file, "w");
		if (!out) {
			return EXIT_FAILURE;
		}
		mcc_ast_print_dot_result(out, &result);
		fclose(out);
	} else {
		mcc_ast_print_dot_result(stdout, &result);
	}

	return EXIT_SUCCESS;
}
