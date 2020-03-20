#include "mcc/semantic_checks.h"
#include "utils/unused.h"

// unused.h contains macro to suppress warnings of unused variables

// TODO: Implementation

// ------------------------------------------------------------- Functions: Running all semantic checks

// Run all semantic checks
struct mcc_semantic_check_all_checks* mcc_semantic_check_run_all(struct mcc_ast_program* ast,
                                                                 struct mcc_symbol_table* symbol_table){
    UNUSED(ast);
    UNUSED(symbol_table);
    return NULL;
}

// ------------------------------------------------------------- Functions: Running single semantic checks

// Types of used variables
struct mcc_semantic_check* mcc_semantic_check_run_type_check(struct mcc_ast_program* ast,
                                                             struct mcc_symbol_table* symbol_table){
    UNUSED(ast);
    UNUSED(symbol_table);
    return NULL;
}

// Each execution path of non-void function returns a value
struct mcc_semantic_check* mcc_semantic_check_run_nonvoid_check(struct mcc_ast_program* ast,
                                                                struct mcc_symbol_table* symbol_table){
    UNUSED(ast);
    UNUSED(symbol_table);
    return NULL;
}

// Main function exists and has correct signature
struct mcc_semantic_check* mcc_semantic_check_run_main_function(struct mcc_ast_program* ast,
                                                                struct mcc_symbol_table* symbol_table){
    UNUSED(ast);
    UNUSED(symbol_table);
    return NULL;
}

// No Calls to unknown functions
struct mcc_semantic_check* mcc_semantic_check_run_unknown_function_call(struct mcc_ast_program* ast,
                                                                        struct mcc_symbol_table* symbol_table){
    UNUSED(ast);
    UNUSED(symbol_table);
    return NULL;
}

// No multiple definitions of the same function
struct mcc_semantic_check* mcc_semantic_check_run_multiple_function_definitions(struct mcc_ast_program* ast,
                                                                                struct mcc_symbol_table* symbol_table){
    UNUSED(ast);
    UNUSED(symbol_table);
    return NULL;
}

// No multiple declarations of a variable in the same scope
struct mcc_semantic_check* mcc_semantic_check_run_multiple_variable_declarations(struct mcc_ast_program* ast,
                                                                                 struct mcc_symbol_table* symbol_table){
    UNUSED(ast);
    UNUSED(symbol_table);
    return NULL;
}

// No use of undeclared variables
struct mcc_semantic_check* mcc_semantic_check_run_use_undeclared_variable(struct mcc_ast_program* ast,
                                                                          struct mcc_symbol_table* symbol_table){
    UNUSED(ast);
    UNUSED(symbol_table);
    return NULL;
}

// ------------------------------------------------------------- Functions: Cleanup

// Delete all checks
void mcc_semantic_check_delete_all_checks(struct mcc_semantic_check_all_checks *checks){
    UNUSED(checks);
    return;
}

// Delete single checks
void mcc_semantic_check_delete_single_check(struct mcc_semantic_check *check){
    UNUSED(check);
    return;
}

