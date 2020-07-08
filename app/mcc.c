#define _GNU_SOURCE

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#include "mcc/asm.h"
#include "mcc/asm_print.h"
#include "mcc/ast.h"
#include "mcc/ir.h"
#include "mcc/parser.h"
#include "mcc/semantic_checks.h"
#include "mcc/symbol_table.h"

#include "mc_cl_parser.inc"
#include "mc_get_ast.inc"

// register datastructures with register_cleanup and they will be deleted on exit
#include "mc_cleanup.inc"

bool assemble_and_link(char *binary_filename, bool quiet);

int main(int argc, char *argv[])
{

	// ---------------------------------------------------------------------- Parsing and checking command line

	// Get all options and arguments from command line
	char *usage_string = "The mC compiler. It takes one or more mC input files and produces an executable.\n"
	                     "Errors are reported on invalid inputs.\n";
	struct mc_cl_parser_command_line_parser *command_line = mc_cl_parser_parse(argc, argv, usage_string, MCC);
	register_cleanup(command_line);

	// Check if command line parser returned any errors or if "-h" was passed. If so, help was already printed,
	// return.
	if (!command_line)
		return EXIT_FAILURE;
	if (command_line->options->print_help || command_line->argument_status == MC_CL_PARSER_ARGSTAT_ERROR ||
	    command_line->argument_status == MC_CL_PARSER_ARGSTAT_FILE_NOT_FOUND) {
		return EXIT_FAILURE;
	}

	// ---------------------------------------------------------------------- Parsing provided input and create AST

	// Declare struct that will hold the result of the parser and corresponding pointer
	struct mcc_parser_result result;

	switch (command_line->argument_status) {
	case MC_CL_PARSER_ARGSTAT_STDIN:
		result = get_ast_from_stdin(command_line->options->quiet);
		break;
	case MC_CL_PARSER_ARGSTAT_FILES:
		result = get_ast_from_files(command_line);
		break;
	default:
		return EXIT_FAILURE;
	}
	register_cleanup(result.error_buffer);
	register_cleanup(result.program);

	if (result.status != MCC_PARSER_STATUS_OK) {
		if (result.error_buffer) {
			if (!command_line->options->quiet) {
				fprintf(stderr, "%s", result.error_buffer);
			}
		} else {
			if (!command_line->options->quiet) {
				fprintf(stderr, "Parsing failed. Unknwon error.\n");
			}
		}
		return EXIT_FAILURE;
	}

	// ---------------------------------------------------------------------- Create Symbol Table

	struct mcc_symbol_table *table = mcc_symbol_table_create((&result)->program);
	if (!table) {
		if (!command_line->options->quiet) {
			fprintf(stderr, "Symbol table generation failed. Unknown error.\n");
		}
		return EXIT_FAILURE;
	}
	register_cleanup(table);

	// ---------------------------------------------------------------------- Run semantic checks

	struct mcc_semantic_check *semantic_check = mcc_semantic_check_run_all((&result)->program, table);
	if (!semantic_check) {
		if (!command_line->options->quiet) {
			fprintf(stderr, "Process of semantic checks failed. Unknwon error.\n");
		}
		return EXIT_FAILURE;
	}
	register_cleanup(semantic_check);

	if (semantic_check->error_buffer) {
		if (!command_line->options->quiet) {
			fprintf(stderr, "%s\n", semantic_check->error_buffer);
		}
		return EXIT_FAILURE;
	}

	// ---------------------------------------------------------------------- Generate IR

	struct mcc_ir_row *ir = mcc_ir_generate((&result)->program);
	if (!ir) {
		if (!command_line->options->quiet) {
			fprintf(stderr, "IR generation failed. Unknwon error.\n");
		}
		return EXIT_FAILURE;
	}
	register_cleanup(ir);

	// ---------------------------------------------------------------------- Generate Assembly

	struct mcc_asm *code = mcc_asm_generate(ir);
	if (!code) {
		if (!command_line->options->quiet) {
			fprintf(stderr, "Assembly code generation failed. Unknown error.\n");
		}
		return EXIT_FAILURE;
	}
	register_cleanup(code);

	// ---------------------------------------------------------------------- Save assembly to file

	// Print assembly to file
	FILE *assembly_out = fopen("a.s", "w");
	if (!assembly_out) {
		if (!command_line->options->quiet) {
			fprintf(stderr, "Failed to open file a.s for writing.\n");
		}
		return EXIT_FAILURE;
	}
	mcc_asm_print_asm(assembly_out, code);
	fclose(assembly_out);

	// ---------------------------------------------------------------------- Call gcc to assemble and link

	bool success = true;
	if (command_line->options->write_to_file) {
		success = assemble_and_link(command_line->options->output_file, command_line->options->quiet);
	} else {
		success = assemble_and_link("a.out", command_line->options->quiet);
	}
	if (!success) {
		if (!command_line->options->quiet) {
			fprintf(stderr, "Backend compiler failed.\n");
		}
		return EXIT_FAILURE;
	}

	// ---------------------------------------------------------------------- Print results

	return EXIT_SUCCESS;
}

bool assemble_and_link(char *binary_filename, bool quiet)
{
	// Create string of command
	char *cc = NULL;
	char builtins[] = "mc_builtins.c ";
	char gcc[] = "gcc";
	char *env_cc = getenv("MCC_BACKEND");
	if (!env_cc) {
		cc = gcc;
	} else {
		cc = env_cc;
	}
	int length =
	    strlen(cc) + strlen(" -m32 -o ") + strlen(binary_filename) + strlen(" a.s ") + strlen(builtins) + 1;
	if (quiet)
		length += strlen(" &>/dev/null");
	char callstring[length];
	if (quiet) {
		snprintf(callstring, length, "%s -m32 -o %s a.s %s &>/dev/null", cc, binary_filename, builtins);
	} else {
		snprintf(callstring, length, "%s -m32 -o %s a.s %s", cc, binary_filename, builtins);
	}

	// Call backend compiler
	int wstatus = system(callstring);

	// Check return value of gcc
	if (WIFEXITED(wstatus)) {
		if (WEXITSTATUS(wstatus) == 0) {
			return true;
		}
	}

	return false;
}

