#ifndef PROJECT_MC_CL_PARSER_H
#define PROJECT_MC_CL_PARSER_H

#include <stdbool.h>

// ----------------------------------------------------------------------- Data structues

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
    char *function;
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
struct mc_cl_parser_command_line_parser* mc_cl_parser_parse (int argc, char *argv[], char* usage_string);

// Read from stdin and write into string
char *mc_cl_parser_stdin_to_string();

// Clean up command line parsing results
void mc_cl_parser_delete_command_line_parser(struct mc_cl_parser_command_line_parser *command_line);

#endif //PROJECT_MC_CL_PARSER_H
