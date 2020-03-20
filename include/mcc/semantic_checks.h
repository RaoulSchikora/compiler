//
// Created by oliver on 20.03.20.
//

#ifndef PROJECT_SEMANTIC_CHECKS_H
#define PROJECT_SEMANTIC_CHECKS_H

#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "mcc/ast.h"
#include "mcc/symbol_table.h"

enum mcc_semantic_check_status{
    MCC_SEMANTIC_CHECK_OK,
    MCC_SEMANTIC_CHECK_FAIL,
};

// ------------------------------------------------------------ Data structure: A suite of semantic checks

struct mcc_semantic_check_all_checks{
   enum mcc_semantic_check_status status;
   struct mcc_semantic_check *type_check;
   struct mcc_semantic_check *nonvoid_check;
};

// ------------------------------------------------------------ Data structure: A single semantic check

enum mcc_semantic_check_type{
    // TODO: Add more checks
    MCC_SEMANTIC_CHECK_TYPE_CHECK,
    MCC_SEMANTIC_CHECK_NONVOID_CHECK,
};

struct mcc_semantic_check {
    enum mcc_semantic_check_status status;
    enum mcc_semantic_check_type type;
    union{
        // TODO: Add more checks
        struct mcc_semantic_check_type_check *type_check;
        struct mcc_semantic_check_nonvoid_check *nonvoid_check;
    };
};

// ------------------------------------------------------------- Functions: Running all semantic checks

// Run all semantic checks
struct mcc_semantic_check_all_checks* mcc_semantic_check_run_all(struct mcc_ast_program* ast,
        struct mcc_symbol_table* symbol_table);

// ------------------------------------------------------------- Functions: Running single semantic checks

// Types of used variables
struct mcc_semantic_check* mcc_semantic_check_run_type_check(struct mcc_ast_program* ast,
        struct mcc_symbol_table* symbol_table);

// Each execution path of non-void function returns a value
struct mcc_semantic_check* mcc_semantic_check_run_nonvoid_check(struct mcc_ast_program* ast,
        struct mcc_symbol_table* symbol_table);

// ------------------------------------------------------------- Functions: Cleanup

// Delete all checks
void mcc_semantic_check_delete_all_checks(struct mcc_semantic_check_all_checks *checks);

// Delete type check
void mcc_semantic_check_delete_type_check(struct mcc_semantic_check *check);

// Delete nonvoid check
void mcc_semantic_check_delete_nonvoid_check(struct mcc_semantic_check *check);


#endif //PROJECT_SEMANTIC_CHECKS_H
