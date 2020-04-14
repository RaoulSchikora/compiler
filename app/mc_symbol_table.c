#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mcc/ast.h"
#include "mcc/ast_print.h"
#include "mcc/ast_visit.h"
#include "mcc/parser.h"
#include "mcc/symbol_table.h"
#include "mcc/symbol_table_print.h"
#include "mcc/symbol_table_print_dot.h"
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

// ----------------------------------------------------------------------- Main

int main(int argc, char *argv[])
{
	// ---------------------------------------------------------------------- Parsing and checking command line

	char *usage_string = "Utility for displaying the generated symbol tables. \n"
	                     "Errors are reported on invalid inputs.\n";
	struct mc_cl_parser_command_line_parser *command_line = mc_cl_parser_parse(argc, argv, usage_string);

	// Check if command line parser returned any errors or if "-h" was passed. If so, help was already printed, return.
	if (!command_line || command_line->options->print_help ||
	    command_line->argument_status == MC_CL_PARSER_ARGSTAT_ERROR ||
	    command_line->argument_status == MC_CL_PARSER_ARGSTAT_FILE_NOT_FOUND) {
		clean_up(command_line);
		return EXIT_FAILURE;
	}


	// ---------------------------------------------------------------------- Parsing provided input

	// Declare struct that will hold the result of the parser and corresponding pointer
	struct mcc_parser_result result;

	// Invoke parser on input from Stdin
	char *input = NULL;
	if (command_line->argument_status == MC_CL_PARSER_ARGSTAT_STDIN) {
		result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
		free(input);
		if (result.status != MCC_PARSER_STATUS_OK) {
			fprintf(stderr, "%s", result.error_buffer);
			free(result.error_buffer);
			mc_cl_parser_delete_command_line_parser(command_line);
			return EXIT_FAILURE;
		}
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

	// ---------------------------------------------------------------------- Generate symbol table

	struct mcc_symbol_table *table = mcc_symbol_table_create((&result)->program);
	if (!table) {
		clean_up(&result);
		clean_up(command_line);
		fprintf(stderr, "mcc_symbol_table_create: returned NULL pointer\n");
		return EXIT_FAILURE;
	}

	// ---------------------------------------------------------------------- Print symbol table

	// Print to file or stdout
	if(!command_line->options->print_dot){
		if (command_line->options->write_to_file == true) {
			FILE *out = fopen(command_line->options->output_file, "a");
			if (out == NULL) {
				clean_up(command_line);
				clean_up(&result);
				clean_up(table);
				return EXIT_FAILURE;
			}
			mcc_symbol_table_print(table, out);
			fclose(out);
		} else {
			mcc_symbol_table_print(table, stdout);
		}
	}

	// ---------------------------------------------------------------------- Print symbol table in dot-Format 

	// Print in dot format to file or stdout
	if(command_line->options->print_dot){
		if (command_line->options->write_to_file == true) {
			FILE *out = fopen(command_line->options->output_file, "a");
			if (!out) {
				clean_up(command_line);
				clean_up(&result);
				clean_up(table);
				return EXIT_FAILURE;
			}
			mcc_symbol_table_print_dot(table, out);
			fclose(out);
		} else {
			mcc_symbol_table_print_dot(table, stdout);
		}
	}

	// ---------------------------------------------------------------------- Clean up

	clean_up(command_line);
	clean_up(&result);
	clean_up(table);
	return EXIT_SUCCESS;
}
