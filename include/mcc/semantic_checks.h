#ifndef PROJECT_SEMANTIC_CHECKS_H
#define PROJECT_SEMANTIC_CHECKS_H

#include <stdbool.h>
#include <string.h>

#include "mcc/ast.h"
#include "mcc/symbol_table.h"

// ------------------------------------------------------------ Data structure: A single semantic check

enum mcc_semantic_check_status{
    MCC_SEMANTIC_CHECK_OK,
    MCC_SEMANTIC_CHECK_FAIL,
};

enum mcc_semantic_check_error_code {
	MCC_SEMANTIC_CHECK_ERROR_OK,
	MCC_SEMANTIC_CHECK_ERROR_MALLOC_FAILED,
	MCC_SEMANTIC_CHECK_ERROR_SNPRINTF_FAILED,
};

struct mcc_semantic_check {
    enum mcc_semantic_check_status status;
    // error_buffer is set to NULL if status is OK
    char* error_buffer;
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

// ------------------------------------------------------------ Function: Run all semantic checks

// Generate struct for semantic check
struct mcc_semantic_check *mcc_semantic_check_initialize_check();

// Run all semantic checks
struct mcc_semantic_check* mcc_semantic_check_run_all(struct mcc_ast_program* ast,
                                                      struct mcc_symbol_table* symbol_table);


// ------------------------------------------------------------- Functions: Running single semantic checks with early abort

// Define function pointer to a single sematic check
enum mcc_semantic_check_error_code (*fctptr)(struct mcc_ast_program *ast, struct mcc_symbol_table *table, struct mcc_semantic_check *check);

// Wrapper for running one of the checks with early abort
enum mcc_semantic_check_error_code mcc_semantic_check_early_abort_wrapper(
    enum mcc_semantic_check_error_code (*fctptr)(struct mcc_ast_program *ast, struct mcc_symbol_table *table, struct mcc_semantic_check *check),
    struct mcc_ast_program *ast,
    struct mcc_symbol_table *table,
    struct mcc_semantic_check *check,
    enum mcc_semantic_check_error_code);

// ------------------------------------------------------------- Functions: Implementation of the individual semantic checks

// Check and get type functions

struct mcc_semantic_check_data_type *check_and_get_type_expression(struct mcc_ast_expression *expression, 
    struct mcc_semantic_check *check);
struct mcc_semantic_check_data_type *check_and_get_type_identifier(struct mcc_ast_identifier *identifier, 
    struct mcc_semantic_check *check, struct mcc_symbol_table_row *row);
struct mcc_semantic_check_data_type *check_and_get_type_literal(struct mcc_ast_literal *literal);

// No Type conversions in expressions
enum mcc_semantic_check_error_code mcc_semantic_check_run_type_check(struct mcc_ast_program *ast,
                                                                     struct mcc_symbol_table *symbol_table,
                                                                     struct mcc_semantic_check *check);

// Each execution path of non-void function returns a value
enum mcc_semantic_check_error_code mcc_semantic_check_run_nonvoid_check(struct mcc_ast_program* ast,
                                                                        struct mcc_symbol_table *symbol_table,
                                                                        struct mcc_semantic_check *check);

// Main function exists and has correct signature
enum mcc_semantic_check_error_code mcc_semantic_check_run_main_function(struct mcc_ast_program* ast,
                                                                        struct mcc_symbol_table *symbol_table,
                                                                        struct mcc_semantic_check *check);

// No multiple definitions of the same function
enum mcc_semantic_check_error_code mcc_semantic_check_run_multiple_function_definitions(struct mcc_ast_program* ast,
                                                                                        struct mcc_symbol_table *symbol_table,
                                                                                        struct mcc_semantic_check *check);

// No multiple declarations of a variable in the same scope
enum mcc_semantic_check_error_code mcc_semantic_check_run_multiple_variable_declarations(struct mcc_ast_program* ast,
                                                                                        struct mcc_symbol_table *symbol_table,
                                                                                        struct mcc_semantic_check *check);

// ------------------------------------------------------------- Functions: Cleanup

// Delete single checks
void mcc_semantic_check_delete_single_check(struct mcc_semantic_check *check);

// ------------------------------------------------------------- Generic for check_and_get_type

// clang-format off

#define check_and_get_type(x, ...) _Generic((x), \
		struct mcc_ast_expression *:          check_and_get_type_expression, \
		struct mcc_ast_identifier *:          check_and_get_type_identifier, \
        struct mcc_ast_literal *:             check_and_get_type_literal \
	)(x, __VA_ARGS__)

// clang-format on

#endif //PROJECT_SEMANTIC_CHECKS_H
