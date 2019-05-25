#include <assert.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "mcc/ast.h"
#include "mcc/ast_print.h"
#include "mcc/ast_visit.h"
#include "mcc/parser.h"

#define BUF_SIZE 1024

// Forward declarations:

enum mc_ast_to_dot_mode {
	MC_AST_TO_DOT_MODE_TEST,
	MC_AST_TO_DOT_MODE_FUNCTION,
	MC_AST_TO_DOT_MODE_PROGRAM,
};

enum mc_ast_to_dot_argument_status {
	MC_AST_TO_DOT_ARGSTAT_STDIN,
	MC_AST_TO_DOT_ARGSTAT_FILES,
	MC_AST_TO_DOT_ARGSTAT_ERROR,
};

struct mc_ast_to_dot_options {
	bool write_to_file;
	char *output_file;
	bool print_help;
	bool limited_scope;
	char *function;
	enum mc_ast_to_dot_mode mode;
};

struct mc_ast_to_dot_command_line_parser {
	struct mc_ast_to_dot_options *options;
	struct mc_ast_to_dot_program_arguments *arguments;
};

struct mc_ast_to_dot_program_arguments {
	int size;
	char **args;
};

// Parse file and return pointer to allocated struct
struct mcc_parser_result parse_file(char *filename);

// Print usage of mc_ast_to_dot
void print_usage(const char *prg);

// Transform a file from hard drive into a string
char *fileToString(char *filename);

// Read from stdin and write into string
char *stdinToString();

// Parse the command line from mc_ast_to_dot
struct mc_ast_to_dot_command_line_parser *parse_command_line(int argc, char *argv[]);

// Parse the command line options from mc_ast_to_dot
struct mc_ast_to_dot_options *parse_options(int argc, char *argv[]);

// Parse the command line arguments from mc_ast_to_dot
struct mc_ast_to_dot_program_arguments *parse_arguments(int argc, char *argv[]);

// Clean up command line parsing results
void mc_ast_to_dot_delete_command_line_parser(struct mc_ast_to_dot_command_line_parser *command_line);

// Check if stdin or files was supplied
enum mc_ast_to_dot_argument_status mc_ast_to_dot_check_args(struct mc_ast_to_dot_command_line_parser *command_line);

// get a function out of a mcc_parser_result
struct mcc_parser_result limit_result_to_function_scope(struct mcc_parser_result *result, char *wanted_function_name);

// take an array of mcc_parser_results and merge them into one
struct mcc_parser_result mc_ast_to_dot_merge_results(struct mcc_parser_result* array, int size);

int main(int argc, char *argv[])
{
	// ---------------------------------------------------------------------- Parsing and checking command line

	// Get all options and arguments from command line
	struct mc_ast_to_dot_command_line_parser *command_line = parse_command_line(argc, argv);
	if (command_line == NULL) {
		mc_ast_to_dot_delete_command_line_parser(command_line);
		return EXIT_FAILURE;
	}

	// print usage if "-h" or "--help" was specified
	if (command_line->options->print_help == true) {
		print_usage(argv[0]);
		mc_ast_to_dot_delete_command_line_parser(command_line);
		return EXIT_FAILURE;
	}

	// Get info, if stdin or files are used as input
	enum mc_ast_to_dot_argument_status argument_status = mc_ast_to_dot_check_args(command_line);

	// Args were malformed
	if (argument_status == MC_AST_TO_DOT_ARGSTAT_ERROR){
		print_usage(argv[0]);
		mc_ast_to_dot_delete_command_line_parser(command_line);
		return EXIT_FAILURE;
	}

	// ---------------------------------------------------------------------- Parsing provided input

	// Declare struct that will hold the result of the parser and corresponding pointer
	struct mcc_parser_result result;
	struct mcc_parser_result *ptr_result;

	// Invoke parser on stdin
	if (argument_status == MC_AST_TO_DOT_ARGSTAT_STDIN){
		char* input = stdinToString();
		result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
		free(input);
		if (result.status != MCC_PARSER_STATUS_OK){
			fprintf(stderr, "%s", result.error_buffer);
			free(result.error_buffer);
			mc_ast_to_dot_delete_command_line_parser(command_line);
			return EXIT_FAILURE;
		}
	}


	// Invoke parser on input files, merge resulting trees into one
	struct mcc_parser_result parse_results [command_line->arguments->size];
	if (argument_status == MC_AST_TO_DOT_ARGSTAT_FILES){

		// Iterate over all files and hand them to parser
		int i = 0;
		while (i<command_line->arguments->size){
			parse_results[i] = parse_file(*(command_line->arguments->args+i));

			// If error of parser: print error and clean up
			if (parse_results[i].status != MCC_PARSER_STATUS_OK){
				fprintf(stderr, "%s", parse_results[i].error_buffer);
				// only invalid inputs will be destroyed by parser: manually delete parser_results of other files:
				int j = 0;
				while (j<i){
					mcc_ast_delete_result(parse_results+j);
					j++;
				}
				mc_ast_to_dot_delete_command_line_parser(command_line);
				return EXIT_FAILURE;
			}

			// Continue Loop
			i++;
		}
		result = mc_ast_to_dot_merge_results(parse_results, command_line->arguments->size);
	}

	// ---------------------------------------------------------------------- Executing option "-f"

	// find correct node of ast tree, depending on wether "-f" was passed, ptr_result will then be set to it
	struct mcc_parser_result limited_result;

	if (command_line->options->mode == MC_AST_TO_DOT_MODE_FUNCTION) {
    	limited_result = limit_result_to_function_scope(&result, command_line->options->function);
		if(limited_result.status != MCC_PARSER_STATUS_OK){
		    mc_ast_to_dot_delete_command_line_parser(command_line);
		    return EXIT_FAILURE;
		}
		ptr_result = &limited_result;
	} else {
        ptr_result = &result;
    }

	// ---------------------------------------------------------------------- Print ast in dot format

	// Print to file or stdout
	if (command_line->options->write_to_file == true) {
		FILE *out = fopen(command_line->options->output_file, "a");
		if (out == NULL) {
			mc_ast_to_dot_delete_command_line_parser(command_line);
			return EXIT_FAILURE;
		}
		mcc_ast_print_dot_result(out, ptr_result);
		fclose(out);
	} else {
		mcc_ast_print_dot_result(stdout, ptr_result);
	}

	// ---------------------------------------------------------------------- Clean up

	// Cleanup
	mc_ast_to_dot_delete_command_line_parser(command_line);
	mcc_ast_delete_result(&result);

	return EXIT_SUCCESS;
}

struct mcc_parser_result parse_file(char *filename)
{
	FILE *f = fopen(filename,"rt");
	if (f == NULL){
		struct mcc_parser_result result = {
				.status = MCC_PARSER_STATUS_UNKNOWN_ERROR,
				.error_buffer = "unable to open file\n",
		};
		return result;
	}
	struct mcc_parser_result *result = malloc(sizeof(struct mcc_parser_result));
	if (result == NULL){
		struct mcc_parser_result result = {
				.status = MCC_PARSER_STATUS_UNKNOWN_ERROR,
				.error_buffer = "unable to allocate memory for parser result\n",
		};
		return result;
	}
	return mcc_parse_file(f,MCC_PARSER_ENTRY_POINT_PROGRAM,filename);
}

void print_usage(const char *prg)
{
	printf("usage: %s [OPTIONS] file...\n\n", prg);
	printf("Utility for printing an abstract syntax tree in the DOT format. The output\n");
	printf("can be visualised using graphviz. Errors are reported on invalid inputs.\n\n");
	printf("Use '-' as input file to read from stdin.\n\n");
	printf("OPTIONS:\n");
	printf("  -h, --help                displays this help message\n");
	printf("  -o, --output <file>       write the output to <file> (defaults to stdout)\n");
	printf("  -f, --function <name>     limit scope to the given function\n");
	printf("  -t, --test                parse rules of the grammar that are not a program\n");
}

// modified from: https://stackoverflow.com/questions/174531
char *fileToString(char *filename)
{
	FILE *f = fopen(filename, "rt");
	if (f == NULL) {
		return NULL;
	}
	fseek(f, 0, SEEK_END);
	long length = ftell(f);
	fseek(f, 0, SEEK_SET);
	char *buffer = (char *)malloc(length + 1);
	buffer[length] = '\0';
	size_t ret = fread(buffer, 1, length, f);
	if (ret == 0){
		perror("fileToString: fread");
		return NULL;
	}
	fclose(f);
	return buffer;
}

// from: https://stackoverflow.com/questions/2496668
char *stdinToString()
{
	char buffer[BUF_SIZE];
	size_t contentSize = 1; // includes NULL
	/* Preallocate space. */
	char *content = malloc(sizeof(char) * BUF_SIZE);
	if (content == NULL) {
		perror("stdinToString:Failed to allocate content");
		exit(1);
	}
	content[0] = '\0'; // make null-terminated
	while (fgets(buffer, BUF_SIZE, stdin)) {
		char *old = content;
		contentSize += strlen(buffer);
		content = realloc(content, contentSize);
		if (content == NULL) {
			perror("stdinToString:Failed to reallocate content");
			free(old);
			exit(2);
		}
		strcat(content, buffer);
	}

	if (ferror(stdin)) {
		free(content);
		perror("Error reading from stdin.");
		exit(3);
	}

	return content;
}

struct mc_ast_to_dot_options *parse_options(int argc, char *argv[])
{
	struct mc_ast_to_dot_options *options = malloc(sizeof(*options));
	if (options == NULL) {
		perror("parse_options:malloc");
		return NULL;
	}

	options->write_to_file = false;
	options->output_file = NULL;
	options->print_help = false;
	options->limited_scope = false;
	options->function = NULL;
	options->mode = MC_AST_TO_DOT_MODE_PROGRAM;
	if (argc == 1) {
		options->print_help = true;
		return options;
	}

	static struct option long_options[] =
			{
					{"help", no_argument, NULL, 'h'},
					{"output", required_argument, NULL, 'o'},
					{"function", required_argument, NULL, 'f'},
					{"test", no_argument, NULL, 't'},
					{NULL,0,NULL,0}
			};

	int c;
	while ((c = getopt_long(argc, argv, "o:hf:t",long_options,NULL)) != -1) {
		switch (c) {
		case 'o':
			options->write_to_file = true;
			options->output_file = optarg;
			break;
		case 'h':
			options->print_help = true;
			break;
		case 'f':
			options->limited_scope = true;
			options->mode = MC_AST_TO_DOT_MODE_FUNCTION;
			options->function = optarg;
			break;
		case 't':
			options->mode = MC_AST_TO_DOT_MODE_TEST;
			break;
		default:
			options->print_help = true;
			break;
		}
	}

	return options;
}

struct mc_ast_to_dot_program_arguments *parse_arguments(int argc, char *argv[])
{

	int i = optind;

	struct mc_ast_to_dot_program_arguments *arguments = malloc(sizeof(*arguments));
	if (arguments == NULL) {
		perror("parse_arguments: malloc");
	}

	if (argc == 1) {
		arguments->size = 0;
		arguments->args = malloc(1);
		return arguments;
	}

	char **args = malloc(sizeof(char *) * (argc - optind));
	if (args == NULL) {
		perror("parse_arguments: malloc");
		return NULL;
	}

	while (i < argc) {
		*(args + i - optind) = argv[i];
		i++;
	}
	arguments->args = args;
	arguments->size = argc - optind;
	return arguments;
}

struct mc_ast_to_dot_command_line_parser *parse_command_line(int argc, char *argv[])
{

	struct mc_ast_to_dot_options *options = parse_options(argc, argv);
	if (options == NULL) {
	    perror("parse_command_line: parse_options");
		return NULL;
	}

	struct mc_ast_to_dot_command_line_parser *parser = malloc(sizeof(*parser));
	if (parser == NULL) {
		perror("parse_command_line: malloc");
		return NULL;
	}

	struct mc_ast_to_dot_program_arguments *arguments = parse_arguments(argc, argv);

	if (arguments->size == 0) {
		options->print_help = true;
	}

	parser->options = options;
	parser->arguments = arguments;

	return parser;
}

void mc_ast_to_dot_delete_command_line_parser(struct mc_ast_to_dot_command_line_parser *command_line)
{
	if (command_line->arguments->args != NULL) {
		free(command_line->arguments->args);
	}
	if (command_line->arguments != NULL) {
		free(command_line->arguments);
	}
	if (command_line->options != NULL) {
		free(command_line->options);
	}
	if (command_line != NULL) {
		free(command_line);
	}
}

enum mc_ast_to_dot_argument_status mc_ast_to_dot_check_args(struct mc_ast_to_dot_command_line_parser *command_line)
{
	// 0 arguments
	if (command_line->arguments->size == 0){
		return MC_AST_TO_DOT_ARGSTAT_ERROR;
	}

	// 1 argument -> stdin ?
	if (command_line->arguments->size == 1 && strcmp(*(command_line->arguments->args), "-") == 0) {
		return MC_AST_TO_DOT_ARGSTAT_STDIN;

	// 1+ arguments -> does "-" appear among arguments?
	} else {

		int i = 0;

		// Check if one of the specified files is stdin
		while (i < command_line->arguments->size) {
			if (strcmp(*(command_line->arguments->args + i), "-") == 0) {
				command_line->options->print_help = true;
				return MC_AST_TO_DOT_ARGSTAT_ERROR;
			}
			i++;
		}
		// 1+ arguments, all of which are files
		return MC_AST_TO_DOT_ARGSTAT_FILES;

	}
}

struct mcc_parser_result limit_result_to_function_scope(struct mcc_parser_result *result, char *wanted_function_name)
{
    // new result that will be returned (only containing wanted function)
	struct mcc_parser_result limited_result;
	limited_result.status = MCC_PARSER_STATUS_OK;
	limited_result.entry_point = MCC_PARSER_ENTRY_POINT_PROGRAM;

	// set cur_program to current toplevel program
	struct mcc_ast_program *cur_program = result->program;

	// look for wanted function
	if (strcmp(wanted_function_name, cur_program->function->identifier->identifier_name) == 0)
	// wanted function is in top-level program
	{
		limited_result.program = cur_program;
		limited_result.program->has_next_function = false;
		limited_result.program->next_function = NULL;
		return limited_result;
	} else {
	// look for wanted function, following the tree structure
		while (cur_program->has_next_function) {
			cur_program = cur_program->next_function;

			// if wanted function is found
			if (strcmp(wanted_function_name, cur_program->function->identifier->identifier_name) == 0) {
				limited_result.program = cur_program;
				limited_result.program->has_next_function = false;
				limited_result.program->next_function = NULL;
				return limited_result;
			}
		}
	}
	// if not returned till this point, function can't be found
	limited_result.status = MCC_PARSER_STATUS_UNKNOWN_ERROR;
	fprintf(stderr,"error: no function with function name %s\n",wanted_function_name);
	return limited_result;
}

struct mcc_parser_result mc_ast_to_dot_merge_results(struct mcc_parser_result* array, int size)
{
	// Starting from the last element, iteratively set "next_function" of the previous element to the current one
	int i = size-1;
    while (i>0){
        (*(array+i-1)).program->has_next_function = true;
        (*(array+i-1)).program->next_function = (*(array+i)).program;
        i--;
	}
    return *array;
}
