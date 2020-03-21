#include "mcc/semantic_checks.h"
#include "utils/unused.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>


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

// Write error message into existing mcc_semantic_check struct
static void write_error_message_to_check(struct mcc_semantic_check* check, const char* string){

    int size = sizeof(char)*strlen(string);
    char* buffer = malloc(size);
    if(buffer == NULL){
        perror("write_error_message_to_check: malloc");
    }
    strncpy(buffer,string,size);
    check->error_buffer = buffer;
}

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
    UNUSED(symbol_table);

    struct mcc_semantic_check* check = malloc(sizeof(*check));
    if (!check){
        return NULL;
    }

    check->status = MCC_SEMANTIC_CHECK_OK;
    check->type = MCC_SEMANTIC_CHECK_MAIN_FUNCTION;
    check->error_buffer = NULL;

    int number_of_mains = 0;

    if (strcmp(ast->function->identifier->identifier_name,"main")==0){
        number_of_mains += 1;
        if(!(ast->function->parameters->is_empty)){
            check->status = MCC_SEMANTIC_CHECK_FAIL;
            return check;
        }
    }
    while (ast->has_next_function){
        ast = ast->next_function;
        if (strcmp(ast->function->identifier->identifier_name,"main")==0){
            number_of_mains += 1;
            if(!(ast->function->parameters->is_empty)){
                write_error_message_to_check(check,"Main has wrong signature. Must be `int main()`");
                check->status = MCC_SEMANTIC_CHECK_FAIL;
                return check;
            }
        }
    }

    if (number_of_mains > 1){
        write_error_message_to_check(check,"Too many main functions defined.");
        check->status = MCC_SEMANTIC_CHECK_FAIL;
    }

    if (number_of_mains == 0){
        write_error_message_to_check(check,"No main function defined.");
        check->status = MCC_SEMANTIC_CHECK_FAIL;
    }

    return check;
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

