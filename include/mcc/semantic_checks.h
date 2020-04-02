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

enum mcc_semantic_check_type{
    MCC_SEMANTIC_CHECK_TYPE_CHECK,
    MCC_SEMANTIC_CHECK_NONVOID_CHECK,
    MCC_SEMANTIC_CHECK_MAIN_FUNCTION,
    MCC_SEMANTIC_CHECK_MULTIPLE_FUNCTION_DEFINITIONS,
    MCC_SEMANTIC_CHECK_MULTIPLE_VARIABLE_DECLARATIONS,
    MCC_SEMANTIC_CHECK_DEFINE_BUILT_IN,
};

struct mcc_semantic_check {
    enum mcc_semantic_check_status status;
    enum mcc_semantic_check_type type;
    // error_buffer is set to NULL if status is OK
    char* error_buffer;
};

// ------------------------------------------------------------ Data structure: Type for type checking

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

// Run all semantic checks
struct mcc_semantic_check* mcc_semantic_check_run_all(struct mcc_ast_program* ast,
                                                                 struct mcc_symbol_table* symbol_table);

// ------------------------------------------------------------- Functions: Running single semantic checks

// No Type conversions in expressions
struct mcc_semantic_check* mcc_semantic_check_run_type_check(struct mcc_ast_program* ast,
                                                                  struct mcc_symbol_table* symbol_table);

// Each execution path of non-void function returns a value
struct mcc_semantic_check* mcc_semantic_check_run_nonvoid_check(struct mcc_ast_program* ast,
                                                                struct mcc_symbol_table* symbol_table);

// Main function exists and has correct signature
struct mcc_semantic_check* mcc_semantic_check_run_main_function(struct mcc_ast_program* ast,
                                                                struct mcc_symbol_table* symbol_table);

// No multiple definitions of the same function
struct mcc_semantic_check* mcc_semantic_check_run_multiple_function_definitions(struct mcc_ast_program* ast,
                                                                struct mcc_symbol_table* symbol_table);

// No multiple declarations of a variable in the same scope
struct mcc_semantic_check* mcc_semantic_check_run_multiple_variable_declarations(struct mcc_ast_program* ast,
                                                                struct mcc_symbol_table* symbol_table);

// No use of the names of the built_in functions in function definitions
struct mcc_semantic_check* mcc_semantic_check_run_define_built_in(struct mcc_ast_program* ast,
                                                                          struct mcc_symbol_table* symbol_table);


// ------------------------------------------------------------- Functions: Cleanup

// Delete single checks
void mcc_semantic_check_delete_single_check(struct mcc_semantic_check *check);


#endif //PROJECT_SEMANTIC_CHECKS_H
