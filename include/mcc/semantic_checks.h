// Semantic Checks
//
// This module defines infrastructure for the semantic checks.
// Semantic checks can be run individually or composed in a single function.

#ifndef PROJECT_SEMANTIC_CHECKS_H
#define PROJECT_SEMANTIC_CHECKS_H

#include <stdbool.h>

#include "mcc/ast.h"
#include "mcc/symbol_table.h"

// ------------------------------------------------------------ Data structure: A single semantic check

enum mcc_semantic_check_status {
	MCC_SEMANTIC_CHECK_OK,
	MCC_SEMANTIC_CHECK_FAIL,
};

enum mcc_semantic_check_error_code {
	MCC_SEMANTIC_CHECK_ERROR_OK,
	MCC_SEMANTIC_CHECK_ERROR_MALLOC_FAILED,
	MCC_SEMANTIC_CHECK_ERROR_SNPRINTF_FAILED,
	MCC_SEMANTIC_CHECK_ERROR_UNKNOWN,
};

struct mcc_semantic_check {
	enum mcc_semantic_check_status status;
	// error_buffer is set to NULL if status is OK
	char *error_buffer;
};

// ------------------------------------------------------------ Data structure: Data type for type checking

enum mcc_semantic_check_data_types {
	MCC_SEMANTIC_CHECK_INT,
	MCC_SEMANTIC_CHECK_FLOAT,
	MCC_SEMANTIC_CHECK_BOOL,
	MCC_SEMANTIC_CHECK_STRING,
	MCC_SEMANTIC_CHECK_VOID,
	MCC_SEMANTIC_CHECK_UNKNOWN,
};

struct mcc_semantic_check_data_type {
	enum mcc_semantic_check_data_types type;
	// -1 if not array
	int array_size;
	bool is_array;
};

// ------------------------------------------------------------ Function: Error handling

/*  Writes error message (format_string) into an existing struct (check).
    Use format_string and variable arguments like printf.
    If "is_from_heap" is set, the variable arguments will be freed.
    Set num to the number of variable arguments
*/
enum mcc_semantic_check_error_code mcc_semantic_check_raise_error(int num,
                                                                  struct mcc_semantic_check *check,
                                                                  struct mcc_ast_node node,
                                                                  const char *format_string,
                                                                  bool is_from_heap,
                                                                  ...);

// ------------------------------------------------------------ Function: Run all semantic checks

// Generate struct for semantic check
struct mcc_semantic_check *mcc_semantic_check_initialize_check();

// Run all semantic checks
struct mcc_semantic_check *mcc_semantic_check_run_all(struct mcc_ast_program *ast,
                                                      struct mcc_symbol_table *symbol_table);

// ------------------------------------------------------------- Functions: Implementation of the individual semantic
// checks

// Struct for user data concerning the type check visitor
struct type_checking_userdata {
	struct mcc_semantic_check *check;
	enum mcc_semantic_check_error_code error;
};

// No Type conversions in expressions
enum mcc_semantic_check_error_code mcc_semantic_check_run_type_check(struct mcc_ast_program *ast,
                                                                     struct mcc_symbol_table *symbol_table,
                                                                     struct mcc_semantic_check *check);

// Each execution path of non-void function returns a value
enum mcc_semantic_check_error_code mcc_semantic_check_run_nonvoid_check(struct mcc_ast_program *ast,
                                                                        struct mcc_symbol_table *symbol_table,
                                                                        struct mcc_semantic_check *check);

// Main function exists and has correct signature
enum mcc_semantic_check_error_code mcc_semantic_check_run_main_function(struct mcc_ast_program *ast,
                                                                        struct mcc_symbol_table *symbol_table,
                                                                        struct mcc_semantic_check *check);

// No multiple definitions of the same function
enum mcc_semantic_check_error_code mcc_semantic_check_run_multiple_function_definitions(
    struct mcc_ast_program *ast, struct mcc_symbol_table *symbol_table, struct mcc_semantic_check *check);

// No multiple declarations of a variable in the same scope
enum mcc_semantic_check_error_code mcc_semantic_check_run_multiple_variable_declarations(
    struct mcc_ast_program *ast, struct mcc_symbol_table *symbol_table, struct mcc_semantic_check *check);

// No incorrect function calls
enum mcc_semantic_check_error_code mcc_semantic_check_run_function_arguments(struct mcc_ast_program *ast,
                                                                             struct mcc_symbol_table *symbol_table,
                                                                             struct mcc_semantic_check *check);

// ------------------------------------------------------------- Functions: Cleanup

// Delete single checks
void mcc_semantic_check_delete_single_check(struct mcc_semantic_check *check);

#endif // PROJECT_SEMANTIC_CHECKS_H
