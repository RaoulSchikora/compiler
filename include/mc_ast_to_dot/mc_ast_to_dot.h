#ifndef COMPILER_MC_AST_TO_DOT_H
#define COMPILER_MC_AST_TO_DOT_H

#include <assert.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mcc/ast.h"
#include "mcc/ast_print.h"
#include "mcc/ast_visit.h"
#include "mcc/parser.h"

#define BUF_SIZE 1024

enum mcc_ast_to_dot_mode {
	MCC_AST_TO_DOT_MODE_TEST,
	MCC_AST_TO_DOT_MODE_PROGRAM,
};

struct mcc_ast_to_dot_options {
	bool write_to_file;
	char *output_file;
	bool print_help;
	bool limited_scope;
	char *function;
	enum mcc_ast_to_dot_mode mode;
};

struct mcc_ast_to_dot_command_line_parser {
	struct mcc_ast_to_dot_options *options;
	struct mcc_ast_to_dot_program_arguments *arguments;
};

struct mcc_ast_to_dot_program_arguments {
	int size;
	char **args;
};


// Print usage of mc_ast_to_dot
void print_usage(const char *prg);

// Transform a file from hard drive into a string
char *fileToString(char *filename);

// Read from stdin and write into string
char *stdinToString();

// Parse the command line from mc_ast_to_dot
struct mcc_ast_to_dot_command_line_parser *parse_command_line(int argc, char *argv[]);

// Parse the command line options from mc_ast_to_dot
struct mcc_ast_to_dot_options *parse_options(int argc, char *argv[]);

// Parse the arguments from mc_ast_to_dot
struct mcc_ast_to_dot_program_arguments *parse_arguments(int argc, char *argv[]);

// Clean up command line parsing results
void mc_ast_to_dot_delete_command_line_parser(struct mcc_ast_to_dot_command_line_parser *command_line);

// Generate string from command line inputs
char *mc_ast_to_dot_generate_input(struct mcc_ast_to_dot_command_line_parser *command_line);

// Obsolete
// static int readInputAndSetMode(int argc, char *argv[]);

#endif // COMPILER_MC_AST_TO_DOT_H
