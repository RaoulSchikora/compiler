#ifndef PROJECT_MC_CL_PARSER_H
#define PROJECT_MC_CL_PARSER_H



#include <assert.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

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

// Parse file and return pointer to allocated struct
struct mcc_parser_result parse_file(char *filename);

// Print usage of mc_cl_parser
void print_usage(const char *prg, const char *usage_string);

// Transform a file from hard drive into a string
char *fileToString(char *filename);

// Read from stdin and write into string
char *stdinToString();

// Parse the command line from mc_cl_parser
struct mc_cl_parser_command_line_parser *parse_command_line(int argc, char *argv[]);

// Parse the command line options from mc_cl_parser
struct mc_cl_parser_options *parse_options(int argc, char *argv[]);

// Parse the command line arguments from mc_cl_parser
struct mc_cl_parser_program_arguments *parse_arguments(int argc, char *argv[]);

// Clean up command line parsing results
void mc_cl_parser_delete_command_line_parser(struct mc_cl_parser_command_line_parser *command_line);

// Check if stdin or files was supplied
enum mc_cl_parser_argument_status mc_cl_parser_check_args(struct mc_cl_parser_command_line_parser *command_line);

#endif //PROJECT_MC_CL_PARSER_H
