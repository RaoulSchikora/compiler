#include "mcc/semantic_checks.h"
#include "utils/unused.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>


// unused.h contains macro to suppress warnings of unused variables

// TODO: Implementation

// ------------------------------------------------------------- Functions: Running all semantic checks

// Write error message into existing mcc_semantic_check struct
static void write_error_message_to_all_checks(struct mcc_semantic_check_all_checks* checks, const char* string){

    int size = sizeof(char)*(strlen(string)+2);
    char* buffer = malloc(size);
    if(buffer == NULL){
        perror("write_error_message_to_check: malloc");
    }
    snprintf(buffer,size,"%s\n",string);
    checks->error_buffer = buffer;
}

// Run all semantic checks. First encountered error is written into error buffer
struct mcc_semantic_check_all_checks* mcc_semantic_check_run_all(struct mcc_ast_program* ast,
                                                                 struct mcc_symbol_table* symbol_table){
    struct mcc_semantic_check_all_checks* checks = malloc(sizeof(*checks));
    if (checks == NULL){
        return NULL;
    }
    checks->status = MCC_SEMANTIC_CHECK_OK;
    checks->error_buffer = NULL;

    // Types of used variables
    /*checks->type_check = mcc_semantic_check_run_type_check(ast, symbol_table);
    if(checks->type_check == NULL){
        checks->status = MCC_SEMANTIC_CHECK_FAIL;
        if(checks->error_buffer == NULL){
            write_error_message_to_all_checks(checks,"mcc_semantic_check_run_type_check returned NULL pointer.");
        }
    } else {
        if(checks->type_check->status != MCC_SEMANTIC_CHECK_OK){
            checks->status = MCC_SEMANTIC_CHECK_FAIL;
            if(checks->error_buffer == NULL){
                write_error_message_to_all_checks(checks,checks->type_check->error_buffer);
            }
        }
    }*/

    // Each execution path of non-void function returns a value
    /*checks->nonvoid_check = mcc_semantic_check_run_nonvoid_check(ast, symbol_table);
    if(checks->nonvoid_check == NULL){
        checks->status = MCC_SEMANTIC_CHECK_FAIL;
        if(checks->error_buffer == NULL){
            write_error_message_to_all_checks(checks,"mcc_semantic_check_run_nonvoid_check returned NULL pointer.");
        }
    } else {
        if(checks->nonvoid_check->status != MCC_SEMANTIC_CHECK_OK){
            checks->status = MCC_SEMANTIC_CHECK_FAIL;
            if(checks->error_buffer == NULL){
                write_error_message_to_all_checks(checks,checks->nonvoid_check->error_buffer);
            }
        }
    }*/

    // Main function exists and has correct signature
    checks->main_function = mcc_semantic_check_run_main_function(ast, symbol_table);
    if(checks->main_function == NULL){
        checks->status = MCC_SEMANTIC_CHECK_FAIL;
        if(checks->error_buffer == NULL){
            write_error_message_to_all_checks(checks,"mcc_semantic_check_run_main_function returned NULL pointer.");
        }
    } else {
        if(checks->main_function->status != MCC_SEMANTIC_CHECK_OK){
            checks->status = MCC_SEMANTIC_CHECK_FAIL;
            if(checks->error_buffer == NULL){
                write_error_message_to_all_checks(checks,checks->main_function->error_buffer);
            }
        }
    }

    // No Calls to unknown functions
    /*checks->unknown_function_call = mcc_semantic_check_run_unknown_function_call(ast, symbol_table);
    if(checks->unknown_function_call == NULL){
        checks->status = MCC_SEMANTIC_CHECK_FAIL;
        if(checks->error_buffer == NULL){
            write_error_message_to_all_checks(checks,"mcc_semantic_check_run_unknown_function_call returned NULL pointer.");
        }
    } else {
        if(checks->unknown_function_call->status != MCC_SEMANTIC_CHECK_OK){
            checks->status = MCC_SEMANTIC_CHECK_FAIL;
            if(checks->error_buffer == NULL){
                write_error_message_to_all_checks(checks,checks->unknown_function_call->error_buffer);
            }
        }
    }*/

    // No multiple definitions of the same function
    checks->multiple_function_definitions = mcc_semantic_check_run_multiple_function_definitions(ast, symbol_table);
    if(checks->multiple_function_definitions == NULL){
        checks->status = MCC_SEMANTIC_CHECK_FAIL;
        if(checks->error_buffer == NULL){
            write_error_message_to_all_checks(checks,"mcc_semantic_check_run_multiple_function_definitions returned NULL pointer.");
        }
    } else {
        if(checks->multiple_function_definitions->status != MCC_SEMANTIC_CHECK_OK){
            checks->status = MCC_SEMANTIC_CHECK_FAIL;
            if(checks->error_buffer == NULL){
                write_error_message_to_all_checks(checks,checks->multiple_function_definitions->error_buffer);
            }
        }
    }

    // No multiple declarations of a variable in the same scope
    /*checks->multiple_variable_declarations = mcc_semantic_check_run_multiple_variable_declarations(ast, symbol_table);
    if(checks->multiple_variable_declarations == NULL){
        checks->status = MCC_SEMANTIC_CHECK_FAIL;
        if(checks->error_buffer == NULL){
            write_error_message_to_all_checks(checks,"mcc_semantic_check_run_multiple_variable_declarations returned NULL pointer.");
        }
    } else {
        if(checks->multiple_variable_declarations->status != MCC_SEMANTIC_CHECK_OK){
            checks->status = MCC_SEMANTIC_CHECK_FAIL;
            if(checks->error_buffer == NULL){
                write_error_message_to_all_checks(checks,checks->multiple_variable_declarations->error_buffer);
            }
        }
    }*/

    // No use of undeclared variables
    /*checks->use_undeclared_variable = mcc_semantic_check_run_use_undeclared_variable(ast, symbol_table);
    if(checks->use_undeclared_variable == NULL){
        checks->status = MCC_SEMANTIC_CHECK_FAIL;
        if(checks->error_buffer == NULL){
            write_error_message_to_all_checks(checks,"mcc_semantic_check_run_use_undeclared_variable returned NULL pointer.");
        }
    } else {
        if(checks->use_undeclared_variable->status != MCC_SEMANTIC_CHECK_OK){
            checks->status = MCC_SEMANTIC_CHECK_FAIL;
            if(checks->error_buffer == NULL){
                write_error_message_to_all_checks(checks,checks->use_undeclared_variable->error_buffer);
            }
        }
    }*/
    return checks;
}

// ------------------------------------------------------------- Functions: Running single semantic checks

// Write error message into existing mcc_semantic_check struct
static void write_error_message_to_check(struct mcc_semantic_check* check, struct mcc_ast_node node, const char* string){

    int size = sizeof(char)*(strlen(string)+50);
    char* buffer = malloc(size);
    if(buffer == NULL){
        perror("write_error_message_to_check: malloc");
    }
    snprintf(buffer,size,"%d:%d: %s\n",node.sloc.start_line,node.sloc.start_col,string);
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
            write_error_message_to_check(check,ast->function->node,"Main has wrong signature. Must be `int main()`");
            check->status = MCC_SEMANTIC_CHECK_FAIL;
            return check;
        }
    }
    while (ast->has_next_function){
        ast = ast->next_function;
        if (strcmp(ast->function->identifier->identifier_name,"main")==0){
            number_of_mains += 1;
            if (number_of_mains > 1){
                write_error_message_to_check(check,ast->function->node,"Too many main functions defined.");
                check->status = MCC_SEMANTIC_CHECK_FAIL;
            }
            if(!(ast->function->parameters->is_empty)){
                write_error_message_to_check(check,ast->function->node,"Main has wrong signature. Must be `int main()`");
                check->status = MCC_SEMANTIC_CHECK_FAIL;
                return check;
            }
        }
    }
    if (number_of_mains == 0){
        write_error_message_to_check(check,ast->node,"No main function defined.");
        check->status = MCC_SEMANTIC_CHECK_FAIL;
        return check;
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
    UNUSED(symbol_table);

    assert(ast);
    struct mcc_semantic_check *check = malloc(sizeof(*check));
    if (!check){
        return NULL;
    }

    check->status = MCC_SEMANTIC_CHECK_OK;
    check->type = MCC_SEMANTIC_CHECK_MULTIPLE_FUNCTION_DEFINITIONS;
    check->error_buffer = NULL;

    struct mcc_ast_program *program_to_check = ast;

    // Program has only one function
    if(!program_to_check->next_function){
        return check;
    }

    while(program_to_check->next_function){
        struct mcc_ast_program *program_to_compare = program_to_check->next_function;
        char *name_of_check = program_to_check->function->identifier->identifier_name;
        char *name_of_compare = program_to_compare->function->identifier->identifier_name;

        // if name of program_to_check and name of program_to_compare equals
        if(strcmp(name_of_check, name_of_compare)==0){
            char* error_msg = (char *)malloc( sizeof(char) * (20 + strlen(name_of_check)));
            sprintf(error_msg, "redefinintion of %s", name_of_check);
            write_error_message_to_check(check,program_to_compare->node, error_msg);
            check->status = MCC_SEMANTIC_CHECK_FAIL;
            free(error_msg);
            return check;
        }

        // compare all next_functions
        while(program_to_compare->next_function){
            program_to_compare = program_to_compare->next_function;
            char *name_of_compare = program_to_compare->function->identifier->identifier_name;

            // if name of program_to_check and name of program_to_compare equals
            if(strcmp(name_of_check, name_of_compare)==0){
                char* error_msg = (char *)malloc( sizeof(char) * (20 + strlen(name_of_check)));
                sprintf(error_msg, "redefinintion of %s", name_of_check);
                write_error_message_to_check(check,program_to_compare->node, error_msg);
                check->status = MCC_SEMANTIC_CHECK_FAIL;
                free(error_msg);
                return check;
            }
        }

        program_to_check = program_to_check->next_function;
    }

    return check;
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


    /*if (checks->error_buffer != NULL){
        free(checks->error_buffer);
    }*/
    /*if (checks->type_check != NULL)
    {
        mcc_semantic_check_delete_single_check(checks->type_check);
    }*/
    /*if (checks->nonvoid_check != NULL)
    {
        mcc_semantic_check_delete_single_check(checks->nonvoid_check);
    }*/
    if (checks->main_function != NULL)
    {
        mcc_semantic_check_delete_single_check(checks->main_function);
    }
    /*if (checks->unknown_function_call != NULL)
    {
        mcc_semantic_check_delete_single_check(checks->unknown_function_call);
    }*/
    if (checks->multiple_function_definitions != NULL)
    {
        mcc_semantic_check_delete_single_check(checks->multiple_function_definitions);
    }
    /*if (checks->multiple_variable_declarations != NULL)
    {
        mcc_semantic_check_delete_single_check(checks->multiple_variable_declarations);
    }*/
    /*if (checks->use_undeclared_variable != NULL)
    {
        mcc_semantic_check_delete_single_check(checks->use_undeclared_variable);
    }*/
    if (checks->error_buffer != NULL){
        free(checks->error_buffer);
    }
    free(checks);
    return;
}

// Delete single checks
void mcc_semantic_check_delete_single_check(struct mcc_semantic_check *check){

    if (check == NULL){
        return;
    }

    if (check->error_buffer != NULL){
        free(check->error_buffer);
    }

    free(check);
}

