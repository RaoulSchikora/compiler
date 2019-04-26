#include "mc_ast_to_dot/mc_ast_to_dot.h"

int main(int argc, char *argv[])
{

	struct mcc_ast_to_dot_command_line_parser *command_line = parse_command_line(argc, argv);
	if (command_line == NULL) {
		mc_ast_to_dot_delete_command_line_parser(command_line);
		return EXIT_FAILURE;
	}

	char *input = mc_ast_to_dot_generate_input(command_line);

	if (command_line->options->print_help == true) {
		print_usage(argv[0]);
		mc_ast_to_dot_delete_command_line_parser(command_line);
		free(input);
		return EXIT_FAILURE;
	}

	if (input == NULL) {
		mc_ast_to_dot_delete_command_line_parser(command_line);
		free(input);
		return EXIT_FAILURE;
	}

	struct mcc_parser_result result;
	struct mcc_parser_result limited_result;
	struct mcc_parser_result *ptr_result;

	// handle entry point dependent on wether "-t" was passed
	switch (command_line->options->mode) {
	case MCC_AST_TO_DOT_MODE_TEST:;

		// parsing phase - entry point set as expression. Actual entry point is set
		// while parsing
		result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_EXPRESSION);
		break;

	case MCC_AST_TO_DOT_MODE_FUNCTION:

        //parsing phase
        result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
        limited_result = limit_result_to_function_scope(&result, command_line->options->function);
        break;

	case MCC_AST_TO_DOT_MODE_PROGRAM:;

		// parsing phase
		result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
		break;
	}

	if (command_line->options->mode != MCC_AST_TO_DOT_MODE_FUNCTION){
        ptr_result = &result;
    } else {
        ptr_result = &limited_result;
    }
    // Check if Parser returned correctly
    if (ptr_result->status != MCC_PARSER_STATUS_OK) {
		mc_ast_to_dot_delete_command_line_parser(command_line);
		free(input);
		return EXIT_FAILURE;
	}

	// Print to file or stdout
	if (command_line->options->write_to_file == true) {
		FILE *out = fopen(command_line->options->output_file, "a");
		if (out == NULL) {
			mc_ast_to_dot_delete_command_line_parser(command_line);
			free(input);
			return EXIT_FAILURE;
		}
		mcc_ast_print_dot_result(out, ptr_result);
		fclose(out);
	} else {
		mcc_ast_print_dot_result(stdout, ptr_result);
	}

	// Cleanup
    if (command_line->options->mode != MCC_AST_TO_DOT_MODE_FUNCTION){
        mcc_ast_delete_result(ptr_result);
    } else {
        mcc_ast_delete_result(&result);
    }
	mc_ast_to_dot_delete_command_line_parser(command_line);
	free(input);

	return EXIT_SUCCESS;
}

void mc_ast_to_dot_delete_command_line_parser(struct mcc_ast_to_dot_command_line_parser *command_line)
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
	printf("  -t, --test                parse, even if input is not a program or function\n");
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
	// TODO error handling fread
	fread(buffer, 1, length, f);
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

struct mcc_ast_to_dot_options *parse_options(int argc, char *argv[])
{
	struct mcc_ast_to_dot_options *options = malloc(sizeof(*options));
	if (options == NULL) {
		perror("Error:Allocation in parse_options");
		return NULL;
	}

	options->write_to_file = false;
	options->output_file = NULL;
	options->print_help = false;
	options->limited_scope = false;
	options->function = NULL;
	options->mode = MCC_AST_TO_DOT_MODE_PROGRAM;
	if (argc == 1) {
		options->print_help = true;
		return options;
	}

	int c;
	while ((c = getopt(argc, argv, "o:h:f:t")) != -1) {
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
			options->mode = MCC_AST_TO_DOT_MODE_FUNCTION;
			options->function = optarg;
			break;
		case 't':
			options->mode = MCC_AST_TO_DOT_MODE_TEST;
			break;
		default:
			options->print_help = true;
			break;
		}
	}

	return options;
}

struct mcc_ast_to_dot_program_arguments *parse_arguments(int argc, char *argv[])
{

	int i = optind;

	struct mcc_ast_to_dot_program_arguments *arguments = malloc(sizeof(arguments));
	if (arguments == NULL) {
		perror("malloc");
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

struct mcc_ast_to_dot_command_line_parser *parse_command_line(int argc, char *argv[])
{

	struct mcc_ast_to_dot_options *options = parse_options(argc, argv);
	if (options == NULL) {
		return NULL;
	}

	struct mcc_ast_to_dot_command_line_parser *parser = malloc(sizeof(*parser));
	if (parser == NULL) {
		return NULL;
	}

	struct mcc_ast_to_dot_program_arguments *arguments = parse_arguments(argc, argv);
	if (arguments == NULL) {
		parser->options = options;
		parser->arguments = NULL;
		return parser;
	}

	if (arguments->size == 0) {
		options->print_help = true;
		parser->options = options;
		parser->arguments = arguments;
		return parser;
	}

	parser->options = options;
	parser->arguments = arguments;

	return parser;
}

char *mc_ast_to_dot_generate_input(struct mcc_ast_to_dot_command_line_parser *command_line)
{

	if (command_line->options->print_help == true) {
		return NULL;
	}

	// check if stdin is input
	if (command_line->arguments->size == 1 && strcmp(*(command_line->arguments->args), "-") == 0) {
		return stdinToString();

		// read from files into input string
	} else {
		int i = 0;
		int length = 0;

		// Check if one of the specified files is stdin
		while (i < command_line->arguments->size) {
			if (strcmp(*(command_line->arguments->args + i), "-") == 0) {
				command_line->options->print_help = true;
				return NULL;
			}
			i++;
		}

		i = 0;

		// calculate accumulated length of input file contents
		while (i < command_line->arguments->size) {
			char *content = fileToString(*(command_line->arguments->args + i));
			if (content == NULL) {
				printf("Error opening file \"%s\"\n", *(command_line->arguments->args + i));
				command_line->options->print_help = true;
				return NULL;
			}
			length += strlen(content);
			free(content);
			i++;
		}

		// allocate memory for input string
		char *input = malloc(sizeof(char) * (length + command_line->arguments->size));
		if (input == NULL) {
			perror("mc_ast_to_dot_generate_input: malloc");
			return NULL;
		}

		// Concatenate input files into input string
		snprintf(input, 1, "\0");
		i = 0;

		while (i < command_line->arguments->size) {
			char *file_content = fileToString(*(command_line->arguments->args + i));
			int arg_length = strlen(file_content) + 1;
			char *intermediate = malloc(arg_length);
			if (intermediate == NULL) {
				perror("mc_ast_to_dot_generate_input: malloc");
				return NULL;
			}
			snprintf(intermediate, arg_length + 1, "\n%s", file_content);
			free(file_content);
			strncat(input, intermediate, arg_length);
			i++;
		}
		return input;
	}
}

struct mcc_parser_result limit_result_to_function_scope(struct mcc_parser_result *result, char *wanted_function_name)
{
    // current program
    struct mcc_ast_program *cur_program = result->program;

    // new result with limited scope
    struct mcc_parser_result limited_result;
    limited_result.status = MCC_PARSER_STATUS_OK;
    limited_result.entry_point = MCC_PARSER_ENTRY_POINT_FUNCTION_DEFINITION;

    // look for wanted function
    if (strcmp(wanted_function_name, cur_program->function->identifier->identifier_name) == 0)
    {// wanted function is in top-level program
        limited_result.function_definition = cur_program->function;
        return limited_result;
    } else {
        while (cur_program->has_next_function) {
            cur_program = cur_program->next_function;

            // if wanted function is found
            if (strcmp(wanted_function_name, cur_program->function->identifier->identifier_name) == 0) {
                limited_result.function_definition = cur_program->function;
                return limited_result;
            }
        }

    }
    // if not returned till this point parser status not okay
    limited_result.status = MCC_PARSER_STATUS_UNKNOWN_ERROR;
    perror("error while printing: no function with given function name\n");
    return limited_result;
}

//void limit_result_to_function_scope(struct mcc_parser_result *result, char *wanted_function_name)
//{
//    assert(result);
//
//    printf("1: %s\n", result->program->has_next_function ? "true" : "false");
//    printf("2: %s\n", wanted_function_name);
//
//    // preceding program
//    struct mcc_ast_program *pre_program;
//    // current program
//    struct mcc_ast_program *cur_program = result->program;
//
//    // wanted function
//    struct mcc_ast_function_definition *wanted_function;
//
//    // look for wanted function
//    if (strcmp(wanted_function_name, cur_program->function->identifier->identifier_name) == 0)
//    {// wanted function is in top-level program
//
//        wanted_function = cur_program->function;
//        cur_program->function = NULL;
//        // delete everything except wanted_function and its program
//        if (cur_program->has_next_function){
//            mcc_ast_delete_program(cur_program->next_function);
//        }
//        // free remaining program of old result
//        free(result->program);
//        result->program = NULL;
//        // set new result and return
//        result->function_definition = wanted_function;
//        return;
//    } else { // wanted function is not in top-level program
//
//        while (cur_program->has_next_function) {
//            pre_program = cur_program;
//            cur_program = cur_program->next_function;
//
//            // if wanted function is found
//            if (strcmp(wanted_function_name, cur_program->function->identifier->identifier_name) == 0){
//                wanted_function = cur_program->function;
//                // reorganize pointers
//                if (cur_program->has_next_function){
//                    pre_program->next_function = cur_program->next_function;
//                    cur_program->function = NULL;
//                    cur_program->has_next_function = false;
//                    cur_program->next_function = NULL;
//                    free(cur_program);
//                } else {
//                    pre_program->has_next_function = false;
//                    pre_program->next_function = NULL;
//                }
//                // delete everything except wanted function
//                mcc_ast_delete_program(result->program);
//                // set new result and return
//                result->function_definition = wanted_function;
//                printf(result->program->has_next_function ? "program exists\n" : "program does not exist\n");
//                printf("%s\n", result->function_definition->identifier->identifier_name);
//                return;
//            }
//        }
//    }
//    // if not returned until here parser status not okay
//    result->status = MCC_PARSER_STATUS_UNKNOWN_ERROR;
//    perror("error while printing: no function with given function name\n");
//    return;
//}
