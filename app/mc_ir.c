#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>

#include "mcc/ast.h"
#include "mcc/ir.h"
#include "mcc/ir_print.h"
#include "mcc/parser.h"
#include "mcc/semantic_checks.h"
#include "mcc/symbol_table.h"

#include "mc_cl_parser.inc"
#include "mc_get_ast.inc"

// register datastructures with register_cleanup and they will be deleted on exit
#include "mc_cleanup.inc"

int main(int argc, char *argv[])
{

	// ---------------------------------------------------------------------- Parsing and checking command line

	// Get all options and arguments from command line
	char *usage_string = "Utility for viewing the generated intermediate representation. \nErrors are "
	                     "reported on invalid inputs.\n";

	struct mc_cl_parser_command_line_parser *command_line = mc_cl_parser_parse(argc, argv, usage_string, MC_IR);
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
			fprintf(stderr, "Unknown error from parser. Error buffer is NULL.\n");
		}
		return EXIT_FAILURE;
	}

	// ---------------------------------------------------------------------- Create Symbol Table

	struct mcc_symbol_table *table = mcc_symbol_table_create((&result)->program);
	if (!table) {
		fprintf(stderr, "mcc_symbol_table_create: returned NULL pointer\n");
		return EXIT_FAILURE;
	}
	register_cleanup(table);

	// ---------------------------------------------------------------------- Run semantic checks

	struct mcc_semantic_check *semantic_check = mcc_semantic_check_run_all((&result)->program, table);
	if (!semantic_check) {
		fprintf(stderr, "library error: mcc_semantic_check_run_all returned with NULL\n");
		return EXIT_FAILURE;
	}
	register_cleanup(semantic_check);

	if (semantic_check->error_buffer) {
		fprintf(stderr, "%s\n", semantic_check->error_buffer);
		return EXIT_FAILURE;
	}

	// ---------------------------------------------------------------------- Generate IR

	struct mcc_ir_row *ir = mcc_ir_generate((&result)->program, table);
	if (!ir) {
		fprintf(stderr, "IR generation failed\n");
		return EXIT_FAILURE;
	}
	register_cleanup(ir);

	// ---------------------------------------------------------------------- Print IR

	// Print to file or stdout
	if (command_line->options->write_to_file == true) {
		FILE *out = fopen(command_line->options->output_file, "w");
		if (!out) {
			return EXIT_FAILURE;
		}
		// Print IR, don't escape quotes
		mcc_ir_print_ir(out, ir, false);
		fclose(out);
	} else {
		// Print IR, don't escape quotes
		mcc_ir_print_ir(stdout, ir, false);
	}

	return EXIT_SUCCESS;
}
