#include <assert.h>
#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "mcc/symbol_table.h"
#include "mcc/ast.h"
#include "mcc/ast_print.h"
#include "mcc/ast_visit.h"
#include "mcc/parser.h"
#include "mcc/symbol_table_print.h"
#include "mcc/symbol_table_print_dot.h"

#include "mc_cl_parser.inc"

#define BUF_SIZE 1024

// ----------------------------------------------------------------------- Forward declaration: functions

// get a function out of a mcc_parser_result
struct mcc_parser_result *mcc_ast_limit_result_to_function(struct mcc_parser_result *result, char *wanted_function_name);

// take an array of mcc_parser_results and merge them into one
struct mcc_parser_result *mcc_ast_merge_results(struct mcc_parser_result *array, int size);

// Hand file to the parser
struct mcc_parser_result parse_file(char *filename);

// ----------------------------------------------------------------------- Main

int main(int argc, char *argv[])
{
	// ---------------------------------------------------------------------- Parsing and checking command line

	char *usage_string = "Utility for displaying the generated symbol tables. \n"
	                     "Errors are reported on invalid inputs.\n";
	// Get all options and arguments from command line
	struct mc_cl_parser_command_line_parser *command_line = mc_cl_parser_parse(argc, argv, usage_string);
	if (command_line == NULL) {
		mc_cl_parser_delete_command_line_parser(command_line);
		return EXIT_FAILURE;
	}

	// print usage if "-h" or "--help" was specified
	if (command_line->options->print_help == true) {
		mc_cl_parser_delete_command_line_parser(command_line);
		return EXIT_FAILURE;
	}

	// Args were malformed
	if (command_line->argument_status == MC_CL_PARSER_ARGSTAT_ERROR) {
		mc_cl_parser_delete_command_line_parser(command_line);
		return EXIT_FAILURE;
	}

	// Given file does not exist
	if (command_line->argument_status == MC_CL_PARSER_ARGSTAT_FILE_NOT_FOUND) {
		mc_cl_parser_delete_command_line_parser(command_line);
		return EXIT_FAILURE;
	}

	// Read from Stdin
	char *input = NULL;
	if (command_line->argument_status == MC_CL_PARSER_ARGSTAT_STDIN) {
		input = mc_cl_parser_stdin_to_string();
	}

	// ---------------------------------------------------------------------- Parsing provided input

	// Declare struct that will hold the result of the parser and corresponding pointer
	struct mcc_parser_result result;

	// Invoke parser on input from Stdin
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
				mc_cl_parser_delete_command_line_parser(command_line);
				return EXIT_FAILURE;
			}

			// Continue Loop
			i++;
		}
		result = *(mcc_ast_merge_results(parse_results, command_line->arguments->size));
	}

	// ---------------------------------------------------------------------- Print symbol table

	struct mcc_symbol_table *table = mcc_symbol_table_create((&result)->program);

	// Print to file or stdout
	if(!command_line->options->print_dot){
		if (command_line->options->write_to_file == true) {
			FILE *out = fopen(command_line->options->output_file, "a");
			if (out == NULL) {
				mcc_ast_delete_program(result.program);
				mcc_symbol_table_delete_table(table);
				mc_cl_parser_delete_command_line_parser(command_line);
				return EXIT_FAILURE;
			}
			mcc_symbol_table_print(table, out);
			fclose(out);
		} else {
			mcc_symbol_table_print(table, stdout);
		}
	}

	// ---------------------------------------------------------------------- Print symbol table with flag -dot set

	// Print in dot format to file or stdout
	if(command_line->options->print_dot){
		if (command_line->options->write_to_file == true) {
			FILE *out = fopen(command_line->options->output_file, "a");
			if (out == NULL) {
				mcc_ast_delete_program(result.program);
				mcc_symbol_table_delete_table(table);
				mc_cl_parser_delete_command_line_parser(command_line);
				return EXIT_FAILURE;
			}
			mcc_symbol_table_print_dot(table, out);
			fclose(out);
		} else {
			mcc_symbol_table_print_dot(table, stdout);
		}
	}

	// ---------------------------------------------------------------------- Clean up

	mc_cl_parser_delete_command_line_parser(command_line);
	mcc_ast_delete_result(&result);
	mcc_symbol_table_delete_table(table);
	return EXIT_SUCCESS;
}

// ----------------------------------------------------------------------- Function definitions

struct mcc_parser_result parse_file(char *filename)
{
	FILE *f = fopen(filename, "rt");
	if (f == NULL) {
		struct mcc_parser_result result = {
		    .status = MCC_PARSER_STATUS_UNKNOWN_ERROR,
		    .error_buffer = "unable to open file\n",
		};
		fclose(f);
		return result;
	}
	struct mcc_parser_result return_value;
	return_value = mcc_parse_file(f, MCC_PARSER_ENTRY_POINT_PROGRAM, filename);
	fclose(f);
	return return_value;
}
