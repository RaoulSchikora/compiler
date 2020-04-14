#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mcc/ast.h"
#include "mcc/ast_print.h"
#include "mcc/ast_visit.h"
#include "mcc/parser.h"
#include "mcc/semantic_checks.h"

#include "mc_cl_parser.inc"

#define BUF_SIZE 1024


// clang-format off

#define clean_up(x)  _Generic((x), \
			struct mcc_symbol_table * : mcc_symbol_table_delete_table, \
			struct mc_cl_parser_command_line_parser * : mc_cl_parser_delete_command_line_parser, \
			struct mcc_parser_result * : mcc_ast_delete_result, \
			struct mcc_semantic_check * : mcc_semantic_check_delete_single_check\
			)(x)

// clang-format on

// Hand file to the parser
struct mcc_parser_result parse_file(char *filename);

int main(int argc, char *argv[])
{

	// ---------------------------------------------------------------------- Parsing and checking command line

	// Get all options and arguments from command line
	char *usage_string = "The mC compiler. It takes an mC input file and produces an executable.\n"
	                     "Errors are reported on invalid inputs.\n";
	struct mc_cl_parser_command_line_parser *command_line = mc_cl_parser_parse(argc, argv, usage_string);

	// Check if command line parser returned any errors or if "-h" was passed. If so, return.
	if (!command_line || command_line->options->print_help ||
	    command_line->argument_status == MC_CL_PARSER_ARGSTAT_ERROR ||
	    command_line->argument_status == MC_CL_PARSER_ARGSTAT_FILE_NOT_FOUND) {
		clean_up(command_line);
		return EXIT_FAILURE;
	}

	// Read from Stdin
	char *input = NULL;
	if (command_line->argument_status == MC_CL_PARSER_ARGSTAT_STDIN) {
		input = mc_cl_parser_stdin_to_string();
	}

	// ---------------------------------------------------------------------- Parsing provided input and create AST

	// Declare struct that will hold the result of the parser and corresponding pointer
	struct mcc_parser_result result;

	// Invoke parser on input from Stdin
	if (command_line->argument_status == MC_CL_PARSER_ARGSTAT_STDIN) {
		result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
		free(input);
		if (result.status != MCC_PARSER_STATUS_OK) {
			fprintf(stderr, "%s", result.error_buffer);
			free(result.error_buffer);
			clean_up(command_line);
			return EXIT_FAILURE;
		}
	}

	// Invoke parser on input files, merge resulting trees into one
	struct mcc_parser_result parse_results[command_line->arguments->size];
	if (command_line->argument_status == MC_CL_PARSER_ARGSTAT_FILES) {

		// Iterate over all files and hand them to parser
		int i = 0;
		while (i < command_line->arguments->size) {
			parse_results[i] = parse_file(*(command_line->arguments->args + i));
			// If error of parser: print error and clean up
			if (parse_results[i].status != MCC_PARSER_STATUS_OK) {
				fprintf(stderr, "%s", parse_results[i].error_buffer);
				free(parse_results[i].error_buffer);
				// only invalid inputs will be destroyed by parser: manually delete parser_results of
				// other files:
				int j = 0;
				while (j < i) {
					mcc_ast_delete_result(parse_results + j);
					free(parse_results + j);
					j++;
				}
				clean_up(command_line);
				return EXIT_FAILURE;
			}

			// Continue Loop
			i++;
		}
		result = *(mcc_ast_merge_results(parse_results, command_line->arguments->size));
	}

	// Print to file or stdout
	if (command_line->options->write_to_file == true) {
		FILE *out = fopen(command_line->options->output_file, "a");
		if (out == NULL) {
			clean_up(&result);
			clean_up(command_line);
			return EXIT_FAILURE;
		}
		fprintf(out, "Teststring for integration testing\n");
		fclose(out);
	} else {
		printf("Teststring for integration testing\n");
	}

	// ---------------------------------------------------------------------- Create Symbol Table

	struct mcc_symbol_table *table = mcc_symbol_table_create((&result)->program);
	if (!table) {
		clean_up(&result);
		clean_up(command_line);
		fprintf(stderr, "mcc_symbol_table_create: returned NULL pointer.");
		return EXIT_FAILURE;
	}

	// ---------------------------------------------------------------------- Run semantic checks

	struct mcc_semantic_check *semantic_check = mcc_semantic_check_run_all((&result)->program, table);
	if (!semantic_check) {
		printf("Library error: mcc_semantic_check_run_all returned with NULL");
		clean_up(command_line);
		clean_up(&result);
		clean_up(table);
		return EXIT_FAILURE;
	}
	if (semantic_check->error_buffer) {
		fprintf(stderr, "Semantic check failed:\n%s\n", semantic_check->error_buffer);
		clean_up(command_line);
		clean_up(&result);
		clean_up(table);
		clean_up(semantic_check);
		return EXIT_FAILURE;
	}

	// ---------------------------------------------------------------------- Clean up

	clean_up(command_line);
	clean_up(&result);
	clean_up(table);
	clean_up(semantic_check);

	// TODO:
	// - run semantic checks
	// - create three-address code
	// - output assembly code
	// - invoke backend compiler

	return EXIT_SUCCESS;
}



struct mcc_parser_result parse_file(char *filename)
{
	FILE *f = fopen(filename, "rt");
	if (f == NULL) {
		struct mcc_parser_result result = {
		    .status = MCC_PARSER_STATUS_UNKNOWN_ERROR,
		    .error_buffer = "unable to open file\n",
		};
		return result;
	}
	struct mcc_parser_result return_value;
	return_value = mcc_parse_file(f, MCC_PARSER_ENTRY_POINT_PROGRAM, filename);
	fclose(f);
	return return_value;
}
