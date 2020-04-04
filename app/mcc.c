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

// get a function out of a mcc_parser_result
struct mcc_parser_result *limit_result_to_function_scope(struct mcc_parser_result *result, char *wanted_function_name);

// take an array of mcc_parser_results and merge them into one
struct mcc_parser_result *mc_ast_to_dot_merge_results(struct mcc_parser_result *array, int size);

// Hand file to the parser
struct mcc_parser_result parse_file(char *filename);

int main(int argc, char *argv[])
{

	// ---------------------------------------------------------------------- Parsing and checking command line

	// Get all options and arguments from command line
	char *usage_string = "The mC compiler. It takes an mC input file and produces an executable.\n"
	                     "Errors are reported on invalid inputs.\n";
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
		result = *(mc_ast_to_dot_merge_results(parse_results, command_line->arguments->size));
	}

	// Print to file or stdout
	if (command_line->options->write_to_file == true) {
		FILE *out = fopen(command_line->options->output_file, "a");
		if (out == NULL) {
			mcc_ast_delete_result(&result);
			mc_cl_parser_delete_command_line_parser(command_line);
			return EXIT_FAILURE;
		}
		fprintf(out, "Teststring for integration testing\n");
		fclose(out);
	} else {
		printf("Teststring for integration testing\n");
	}

	// ---------------------------------------------------------------------- Create Symbol Table

	struct mcc_symbol_table *table = mcc_symbol_table_create((&result)->program);
	if (table == NULL) {
		perror("mcc_symbol_table_create: returned NULL pointer.");
	}

	// ---------------------------------------------------------------------- Run semantic checks

	struct mcc_semantic_check *semantic_check = mcc_semantic_check_run_all((&result)->program, table);
	if (semantic_check->error_buffer) {
		fprintf(stderr, "Semantic check failed:\n%s", semantic_check->error_buffer);
		mc_cl_parser_delete_command_line_parser(command_line);
		mcc_ast_delete_result(&result);
		mcc_symbol_table_delete_table(table);
		mcc_semantic_check_delete_single_check(semantic_check);
		return EXIT_FAILURE;
	}

	// ---------------------------------------------------------------------- Clean up

	mc_cl_parser_delete_command_line_parser(command_line);
	mcc_ast_delete_result(&result);
	mcc_symbol_table_delete_table(table);

	// TODO:
	// - run semantic checks
	// - create three-address code
	// - output assembly code
	// - invoke backend compiler

	return EXIT_SUCCESS;
}

struct mcc_parser_result *limit_result_to_function_scope(struct mcc_parser_result *result, char *wanted_function_name)
{

	// INPUT: 	- pointer to struct mcc_parser_result that is allocated on the heap
	//			- string that contains the name of the function that the result should be limited to
	// RETURN: 	- pointer to struct mcc_parser_result that is allocated on the heap and contains only
	//			  the function that was specified by the input string
	//			- returns NULL in case of runtime errors (in that case the input result won't have been
	// deleted)
	// RECOMMENDED USE: call it like this: 		ptr = limit_result_to_function_scope(ptr,function_name)

	// New struct mcc_parser_result that will be returned
	struct mcc_parser_result *new_result = malloc(sizeof(struct mcc_parser_result));
	if (new_result == NULL) {
		perror("limit_result_to_function_scope: malloc");
		return NULL;
	}

	// Set default values of the mcc_parser_result struct
	new_result->status = MCC_PARSER_STATUS_OK;
	new_result->entry_point = MCC_PARSER_ENTRY_POINT_PROGRAM;

	// Decleare pointer that will be iterated over and initialize it with the first function of the input
	struct mcc_ast_program *cur_program = result->program;

	// Set up two pointers to keep track of the found function and its predecessor
	struct mcc_ast_program *right_function = NULL;
	struct mcc_ast_program *predecessor = NULL;

	// Set up boolean to keep track of wether the function was encountered yet
	bool found_function = false;

	// Check if wanted function is the toplevel function
	if (strcmp(wanted_function_name, cur_program->function->identifier->identifier_name) == 0) {
		right_function = cur_program;
		found_function = true;
	}

	// Iterate over the rest of the tree
	struct mcc_ast_program *intermediate = cur_program;
	while (cur_program->has_next_function) {
		intermediate = cur_program;
		cur_program = cur_program->next_function;

		// check if wanted function is found
		if (strcmp(wanted_function_name, cur_program->function->identifier->identifier_name) == 0) {
			if (found_function == true) {
				fprintf(stderr, "error: function with name %s appears multiple times\n",
				        wanted_function_name);
				free(new_result);
				return NULL;
			} else {
				right_function = cur_program;
				predecessor = intermediate;
				found_function = true;
			}
		}
	}

	// Check if the function was found
	if (found_function == true) {
		// Delete all nodes followed by the found function
		if (right_function->has_next_function) {
			mcc_ast_delete_program(right_function->next_function);
		}
		// Tell the predecessor of the found function to forget about its successor
		if (predecessor != NULL) {
			predecessor->has_next_function = false;
			predecessor->next_function = NULL;
		}
		// Set up the new result with the found function
		new_result->program = right_function;
		right_function->has_next_function = false;
		right_function->next_function = NULL;
		// Delete the previous AST up to including the predecessor
		if (predecessor != NULL) {
			mcc_ast_delete_result(result);
		}

		return new_result;
	} else {
		fprintf(stderr, "error: no function with function name %s\n", wanted_function_name);
		free(new_result);
		return NULL;
	}
}

struct mcc_parser_result *mc_ast_to_dot_merge_results(struct mcc_parser_result *array, int size)
{
	/*
	 * In the outer loop we iterate over the array of mcc_parser_result structs.
	 * In the inner loop we iterate over the programs within one mcc_parser_result struct, in order to find
	 * the last program and set its "next_function" pointer to the first program of the next mcc_parser_result
	 * struct.
	 */

	struct mcc_ast_program *cur_prog = NULL;

	// Iterate over array
	for (int i = 0; i < size - 1; i++) {
		// Iterate over programs within result
		cur_prog = array[i].program;
		while (cur_prog->has_next_function) {
			cur_prog = cur_prog->next_function;
		}
		cur_prog->has_next_function = true;
		cur_prog->next_function = array[i + 1].program;
	}

	return array;
}

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
