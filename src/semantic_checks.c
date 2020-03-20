#include "../include/mcc/semantic_checks.h"

// TODO: Implementation

// ------------------------------------------------------------- Functions: Running all semantic checks

// Run all semantic checks
struct mcc_semantic_check_all_checks* mcc_semantic_check_run_all(struct mcc_ast_program* ast,
                                                                 struct mcc_symbol_table* symbol_table){
    return NULL;
}

// ------------------------------------------------------------- Functions: Running single semantic checks

// Types of used variables
struct mcc_semantic_check* mcc_semantic_check_run_type_check(struct mcc_ast_program* ast,
                                                             struct mcc_symbol_table* symbol_table){
    return NULL;
}

// Each execution path of non-void function returns a value
struct mcc_semantic_check* mcc_semantic_check_run_nonvoid_check(struct mcc_ast_program* ast,
                                                                struct mcc_symbol_table* symbol_table){
    return NULL;
}

// ------------------------------------------------------------- Functions: Cleanup

// Delete all checks
void mcc_semantic_check_delete_all_checks(struct mcc_semantic_check_all_checks *checks){
    return;
}

// Delete type check
void mcc_semantic_check_delete_type_check(struct mcc_semantic_check *check){
    return;
}

// Delete nonvoid check
void mcc_semantic_check_delete_nonvoid_check(struct mcc_semantic_check *check){
    return;
}
