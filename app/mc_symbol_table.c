#include <assert.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "mcc/ast.h"
#include "mcc/ast_print.h"
#include "mcc/ast_visit.h"
#include "mcc/parser.h"
#include "mcc/symbol_table.h"
#include "mc_cl_parser.h"

#define BUF_SIZE 1024

// Forward declarations:

// ----------------------------------------------------------------------- Data structues

// ----------------------------------------------------------------------- Functions

// ----------------------------------------------------------------------- Main

int main(int argc, char *argv[]) {
    // ---------------------------------------------------------------------- Parsing and checking command line

    // Get all options and arguments from command line
    struct mc_cl_parser_command_line_parser *command_line = mc_cl_parser_parse(argc, argv);
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

    if (command_line->argument_status == MC_CL_PARSER_ARGSTAT_FILE_NOT_FOUND) {
        mc_cl_parser_delete_command_line_parser(command_line);
        return EXIT_FAILURE;
    }

    // Read from Stdin
    char* input = NULL;
    if (command_line->argument_status == MC_CL_PARSER_ARGSTAT_STDIN){
        input = stdinToString();
        free(input);
    }

    return EXIT_SUCCESS;
}
