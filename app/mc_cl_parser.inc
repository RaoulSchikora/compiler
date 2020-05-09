#ifndef MC_CL_PARSER_INC
#define MC_CL_PARSER_INC

#define _GNU_SOURCE

#include <assert.h>
#include <errno.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUF_SIZE 1024

// ----------------------------------------------------------------------- Data structures

enum mc_apps {
	MCC,
	MC_ASM,
	MC_AST_TO_DOT,
	MC_IR,
	MC_CFG_TO_DOT,
	MC_SYMBOL_TABLE,
};

enum mc_cl_parser_mode {
	MC_CL_PARSER_MODE_FUNCTION,
	MC_CL_PARSER_MODE_PROGRAM,
};

enum mc_cl_parser_argument_status {
	MC_CL_PARSER_ARGSTAT_STDIN,
	MC_CL_PARSER_ARGSTAT_FILES,
	MC_CL_PARSER_ARGSTAT_ERROR,
	MC_CL_PARSER_ARGSTAT_FILE_NOT_FOUND,
	MC_CL_PARSER_ARGSTAT_PRINT_HELP,

};

struct mc_cl_parser_options {
	bool write_to_file;
	char *output_file;
	bool print_help;
	bool limited_scope;
	bool quiet;
	char *function;
	bool print_dot;
	enum mc_cl_parser_mode mode;
};

struct mc_cl_parser_command_line_parser {
	struct mc_cl_parser_options *options;
	struct mc_cl_parser_program_arguments *arguments;
	enum mc_cl_parser_argument_status argument_status;
};

struct mc_cl_parser_program_arguments {
	int size;
	char **args;
};

// ----------------------------------------------------------------------- Functions

// Main function. Parses command line and returns struct
struct mc_cl_parser_command_line_parser *
mc_cl_parser_parse(int argc, char *argv[], char *usage_string, enum mc_apps app);

// Read from stdin and write into string
char *mc_cl_parser_stdin_to_string(bool quiet);

// Clean up command line parsing results
void mc_cl_parser_delete_command_line_parser(struct mc_cl_parser_command_line_parser *command_line);

static void print_usage(enum mc_apps app, const char *usage_string);

static struct mc_cl_parser_options *parse_options(int argc, char *argv[], enum mc_apps app);

static struct mc_cl_parser_program_arguments *parse_arguments(int argc, char *argv[]);

static struct mc_cl_parser_command_line_parser *parse_command_line(int argc, char *argv[], enum mc_apps app);

static enum mc_cl_parser_argument_status check_args(struct mc_cl_parser_command_line_parser *command_line);

struct mc_cl_parser_command_line_parser *
mc_cl_parser_parse(int argc, char *argv[], char *usage_string, enum mc_apps app)
{
	// ------------------------------------------------------------ Parsing and checking command line

	// Get all options and arguments from command line
	struct mc_cl_parser_command_line_parser *command_line = parse_command_line(argc, argv, app);
	if (command_line == NULL) {
		return NULL;
	}

	// print usage if "-h" or "--help" was specified
	if (command_line->options->print_help == true) {
		if (!command_line->options->quiet) {
			print_usage(app, usage_string);
		}
		command_line->argument_status = MC_CL_PARSER_ARGSTAT_PRINT_HELP;
		return command_line;
	}

	// Get info, if stdin or files are used as input
	command_line->argument_status = check_args(command_line);

	// Args were malformed
	if (command_line->argument_status == MC_CL_PARSER_ARGSTAT_ERROR) {
		if (!command_line->options->quiet) {
			print_usage(app, usage_string);
		}
		return command_line;
	}

	if (command_line->argument_status == MC_CL_PARSER_ARGSTAT_FILE_NOT_FOUND) {
		if (!command_line->options->quiet) {
			fprintf(stderr, "-------------------------------------------\n");
			fprintf(stderr, "File not found, please provide valid input.\n");
			fprintf(stderr, "-------------------------------------------\n");
			fprintf(stderr, "\n");
			print_usage(app, usage_string);
		}
		return command_line;
	}

	return command_line;
}

void mc_cl_parser_delete_command_line_parser(struct mc_cl_parser_command_line_parser *command_line)
{
	if (!command_line) {
		return;
	}
	if (command_line->arguments != NULL) {
		free(command_line->arguments->args);
	}

	free(command_line->options);
	free(command_line->arguments);
	free(command_line);
}

char *get_prg_name(enum mc_apps app)
{
	switch (app) {
	case MCC:
		return "mcc";
	case MC_AST_TO_DOT:
		return "mc_ast_to_dot";
	case MC_CFG_TO_DOT:
		return "mc_cfg_to_dot";
	case MC_IR:
		return "mc_ir";
	case MC_SYMBOL_TABLE:
		return "mc_symbol_table";
	case MC_ASM:
		return "mc_asm";
	default:
		return "program";
	}
}

static void print_usage(enum mc_apps app, const char *usage_string)
{
	char *prg = get_prg_name(app);

	fprintf(stderr, "usage: %s [OPTIONS] <file>...\n\n", prg);
	fprintf(stderr, "%s\n", usage_string);
	fprintf(stderr, "Use '-' as input file to read from stdin.\n\n");
	fprintf(stderr, "OPTIONS:\n");
	fprintf(stderr, "  -h, --help                display this help message\n");
	if (app == MC_SYMBOL_TABLE) {
		fprintf(stderr, "  -d, --dot                 print in dot-format\n");
	}
	if (app == MCC) {

		fprintf(stderr, "  -q, --quiet               suppress error output\n");
		fprintf(stderr, "  -o, --output <out-file>   write the output to <out-file> (defaults to 'a.out')\n");
	} else {
		fprintf(stderr, "  -o, --output <out-file>   write the output to <out-file> (defaults to stdout)\n");
	}
	if (app == MC_CFG_TO_DOT) {
		fprintf(stderr,
		        "  -f, --function <name>     print the CFG of the given function (defaults to 'main')\n");
	}
}

// from: https://stackoverflow.com/questions/2496668
char *mc_cl_parser_stdin_to_string(bool quiet)
{
	char buffer[BUF_SIZE];
	size_t contentSize = 1; // includes NULL
	/* Preallocate space. */
	char *content = malloc(sizeof(char) * BUF_SIZE);
	if (!content) {
		if (!quiet) {
			perror("mc_cl_stdin_to_string:Failed to allocate content");
		}
		return NULL;
	}
	content[0] = '\0'; // make null-terminated
	while (fgets(buffer, BUF_SIZE, stdin)) {
		char *old = content;
		contentSize += strlen(buffer);
		content = realloc(content, contentSize);
		if (!content) {
			if (!quiet) {
				perror("mc_cl_stdin_to_string:Failed to reallocate content");
			}
			free(old);
			return NULL;
		}
		strcat(content, buffer);
	}

	if (ferror(stdin)) {
		if (!quiet) {
			perror("Error reading from stdin.");
		}
		free(content);
		return NULL;
	}

	return content;
}

static struct mc_cl_parser_options *parse_options(int argc, char *argv[], enum mc_apps app)
{
	struct mc_cl_parser_options *options = malloc(sizeof(*options));
	if (!options) {
		perror("parse_options:malloc");
		return NULL;
	}

	options->write_to_file = false;
	options->output_file = NULL;
	options->print_help = false;
	options->limited_scope = false;
	options->quiet = false;
	options->function = NULL;
	options->print_dot = false;
	options->mode = MC_CL_PARSER_MODE_PROGRAM;
	if (argc == 1) {
		options->print_help = true;
		return options;
	}

	static struct option long_options[] = {
	    {"help", no_argument, NULL, 'h'},           {"output", required_argument, NULL, 'o'},
	    {"function", required_argument, NULL, 'f'}, {"dot", no_argument, NULL, 'd'},
	    {"quiet", no_argument, NULL, 'q'},          {NULL, 0, NULL, 0}};

	int c;
	while ((c = getopt_long(argc, argv, "o:hf:tdq", long_options, NULL)) != -1) {
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
			options->mode = MC_CL_PARSER_MODE_FUNCTION;
			options->function = optarg;
			break;
		case 'd':
			options->print_dot = true;
			break;
		case 'q':
			options->quiet = true;
			break;
		default:
			options->print_help = true;
			break;
		}
	}
	if (app != MC_SYMBOL_TABLE && options->print_dot) {
		options->print_dot = false;
		options->print_help = true;
	}
	if (app != MC_CFG_TO_DOT && options->limited_scope) {
		options->limited_scope = false;
		options->print_help = true;
	}
	if (app != MCC && options->quiet) {
		options->quiet = false;
		options->print_help = true;
	}

	return options;
}

static struct mc_cl_parser_program_arguments *parse_arguments(int argc, char *argv[])
{

	int i = optind;

	struct mc_cl_parser_program_arguments *arguments = malloc(sizeof(*arguments));
	if (!arguments) {
		perror("parse_arguments: malloc");
		return NULL;
	}

	if (argc == 1) {
		arguments->size = 0;
		arguments->args = malloc(1);
		return arguments;
	}

	char **args = malloc(sizeof(char *) * (argc - optind));
	if (!args) {
		perror("parse_arguments: malloc");
		free(arguments);
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

static struct mc_cl_parser_command_line_parser *parse_command_line(int argc, char *argv[], enum mc_apps app)
{

	struct mc_cl_parser_options *options = parse_options(argc, argv, app);
	if (!options) {
		return NULL;
	}

	struct mc_cl_parser_command_line_parser *parser = malloc(sizeof(*parser));
	if (!parser) {
		if (!options->quiet) {
			perror("parse_command_line: malloc");
		}
		return NULL;
	}

	struct mc_cl_parser_program_arguments *arguments = parse_arguments(argc, argv);
	if (!arguments) {
		free(parser);
		return NULL;
	}

	if (arguments->size == 0) {
		options->print_help = true;
	}

	parser->options = options;
	parser->arguments = arguments;

	return parser;
}

static enum mc_cl_parser_argument_status check_args(struct mc_cl_parser_command_line_parser *command_line)
{
	assert(command_line);
	// 0 arguments
	if (command_line->arguments->size == 0) {
		return MC_CL_PARSER_ARGSTAT_ERROR;
	}

	// 1 argument -> stdin ?
	if (command_line->arguments->size == 1 && strcmp(*(command_line->arguments->args), "-") == 0) {
		return MC_CL_PARSER_ARGSTAT_STDIN;

		// 1+ arguments -> does "-" appear among arguments?
	} else {

		// Check if one of the specified files is stdin
		for (int i = 0; i < command_line->arguments->size; i++) {
			if (strcmp(*(command_line->arguments->args + i), "-") == 0) {
				command_line->options->print_help = true;
				return MC_CL_PARSER_ARGSTAT_ERROR;
			} else {
				// Check if file exists
				if (access(*(command_line->arguments->args + i), F_OK) == -1) {
					return MC_CL_PARSER_ARGSTAT_FILE_NOT_FOUND;
				}
			}
		}
		// 1+ arguments, all of which are files
		return MC_CL_PARSER_ARGSTAT_FILES;
	}
}

#endif // MC_CL_PARSER_INC
