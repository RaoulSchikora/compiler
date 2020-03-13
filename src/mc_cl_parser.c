#include "mc_cl_parser.h"

#define BUF_SIZE 1024

// ------------------------------------------------------------- Forward declarations

static void print_usage(const char *prg, const char *usage_string);

static struct mc_cl_parser_options *parse_options(int argc, char *argv[]);

static struct mc_cl_parser_program_arguments *parse_arguments(int argc, char *argv[]);

static struct mc_cl_parser_command_line_parser *parse_command_line(int argc, char *argv[]);

static enum mc_cl_parser_argument_status check_args(struct mc_cl_parser_command_line_parser *command_line);


// ------------------------------------------------------------- Definitions

struct mc_cl_parser_command_line_parser* mc_cl_parser_parse (int argc, char *argv[], char* usage_string)
{
    // ------------------------------------------------------------ Parsing and checking command line

    // Get all options and arguments from command line
    struct mc_cl_parser_command_line_parser *command_line = parse_command_line(argc, argv);
    if (command_line == NULL) {
        mc_cl_parser_delete_command_line_parser(command_line);
        return NULL;
    }

    // print usage if "-h" or "--help" was specified
    if (command_line->options->print_help == true) {
        print_usage(argv[0],usage_string);
        command_line->argument_status=MC_CL_PARSER_ARGSTAT_PRINT_HELP;
        return command_line;
    }

    // Get info, if stdin or files are used as input
    command_line->argument_status = check_args(command_line);

    // Args were malformed
    if (command_line->argument_status == MC_CL_PARSER_ARGSTAT_ERROR){
        print_usage(argv[0],usage_string);
        return command_line;
    }

    if (command_line->argument_status == MC_CL_PARSER_ARGSTAT_FILE_NOT_FOUND){
        printf("-------------------------------------------\n");
        printf("File not found, please provide valid input.\n");
        printf("-------------------------------------------\n");
        printf("\n");
        print_usage(argv[0],usage_string);
        return command_line;
    }

    return command_line;
}

void mc_cl_parser_delete_command_line_parser(struct mc_cl_parser_command_line_parser *command_line)
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

static void print_usage(const char *prg, const char *usage_string)
{
    printf("usage: %s [OPTIONS] file...\n\n", prg);
    printf("%s\n",usage_string);
    printf("Use '-' as input file to read from stdin.\n\n");
    printf("OPTIONS:\n");
    printf("  -h, --help                displays this help message\n");
    printf("  -o, --output <file>       write the output to <file> (defaults to stdout)\n");
    printf("  -f, --function <name>     limit scope to the given function\n");
    printf("  -t, --test                parse rules of the grammar that are not a program\n");
}

// from: https://stackoverflow.com/questions/2496668
char *mc_cl_parser_stdin_to_string()
{
    char buffer[BUF_SIZE];
    size_t contentSize = 1; // includes NULL
    /* Preallocate space. */
    char *content = malloc(sizeof(char) * BUF_SIZE);
    if (content == NULL) {
        perror("mc_cl_stdin_to_string:Failed to allocate content");
        exit(1);
    }
    content[0] = '\0'; // make null-terminated
    while (fgets(buffer, BUF_SIZE, stdin)) {
        char *old = content;
        contentSize += strlen(buffer);
        content = realloc(content, contentSize);
        if (content == NULL) {
            perror("mc_cl_stdin_to_string:Failed to reallocate content");
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

static struct mc_cl_parser_options *parse_options(int argc, char *argv[])
{
    struct mc_cl_parser_options *options = malloc(sizeof(*options));
    if (options == NULL) {
        perror("parse_options:malloc");
        return NULL;
    }

    options->write_to_file = false;
    options->output_file = NULL;
    options->print_help = false;
    options->limited_scope = false;
    options->function = NULL;
    options->mode = MC_CL_PARSER_MODE_PROGRAM;
    if (argc == 1) {
        options->print_help = true;
        return options;
    }

    static struct option long_options[] =
            {
                    {"help", no_argument, NULL, 'h'},
                    {"output", required_argument, NULL, 'o'},
                    {"function", required_argument, NULL, 'f'},
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
                options->mode = MC_CL_PARSER_MODE_FUNCTION;
                options->function = optarg;
                break;
            default:
                options->print_help = true;
                break;
        }
    }

    return options;
}

static struct mc_cl_parser_program_arguments *parse_arguments(int argc, char *argv[])
{

    int i = optind;

    struct mc_cl_parser_program_arguments *arguments = malloc(sizeof(*arguments));
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

static struct mc_cl_parser_command_line_parser *parse_command_line(int argc, char *argv[])
{

    struct mc_cl_parser_options *options = parse_options(argc, argv);
    if (options == NULL) {
        perror("parse_command_line: parse_options");
        return NULL;
    }

    struct mc_cl_parser_command_line_parser *parser = malloc(sizeof(*parser));
    if (parser == NULL) {
        perror("parse_command_line: malloc");
        return NULL;
    }

    struct mc_cl_parser_program_arguments *arguments = parse_arguments(argc, argv);

    if (arguments->size == 0) {
        options->print_help = true;
    }

    parser->options = options;
    parser->arguments = arguments;

    return parser;
}

static enum mc_cl_parser_argument_status check_args(struct mc_cl_parser_command_line_parser *command_line)
{
    // 0 arguments
    if (command_line->arguments->size == 0){
        return MC_CL_PARSER_ARGSTAT_ERROR;
    }

    // 1 argument -> stdin ?
    if (command_line->arguments->size == 1 && strcmp(*(command_line->arguments->args), "-") == 0) {
        return MC_CL_PARSER_ARGSTAT_STDIN;

        // 1+ arguments -> does "-" appear among arguments?
    } else {

        int i = 0;

        // Check if one of the specified files is stdin
        while (i < command_line->arguments->size) {
            if (strcmp(*(command_line->arguments->args + i), "-") == 0) {
                command_line->options->print_help = true;
                return MC_CL_PARSER_ARGSTAT_ERROR;
            } else {
                // Check if file exists
                if (access(*(command_line->arguments->args + i), F_OK) == -1){
                    return MC_CL_PARSER_ARGSTAT_FILE_NOT_FOUND;
                }
            }
            i++;
        }
        // 1+ arguments, all of which are files
        return MC_CL_PARSER_ARGSTAT_FILES;

    }
}

