#include "mcc/semantic_checks.h"

#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "mcc/ast.h"
#include "mcc/symbol_table.h"
#include "utils/unused.h"

// Run all semantic checks
struct mcc_semantic_check* mcc_semantic_check_run_all(struct mcc_ast_program* ast,
                                                                 struct mcc_symbol_table* symbol_table){
	UNUSED(ast);
	UNUSED(symbol_table);
	struct mcc_semantic_check *check;
	check = malloc(sizeof(check));
	if(!check){
		return NULL;
	}
	check->status = MCC_SEMANTIC_CHECK_FAIL;
	check->error_buffer = NULL;
	return check;
}

// ------------------------------------------------------------- Functions: Running single semantic checks

// No Type conversions in expressions
struct mcc_semantic_check* mcc_semantic_check_run_type_check(struct mcc_ast_program* ast,
                                                                  struct mcc_symbol_table* symbol_table){

	UNUSED(ast);
	UNUSED(symbol_table);
	struct mcc_semantic_check *check;
	check = malloc(sizeof(check));
	if(!check){
		return NULL;
	}
	check->status = MCC_SEMANTIC_CHECK_FAIL;
	check->error_buffer = NULL;
	return check;
}

// Each execution path of non-void function returns a value
struct mcc_semantic_check* mcc_semantic_check_run_nonvoid_check(struct mcc_ast_program* ast,
                                                                struct mcc_symbol_table* symbol_table){
	UNUSED(ast);
	UNUSED(symbol_table);
	struct mcc_semantic_check *check;
	check = malloc(sizeof(check));
	if(!check){
		return NULL;
	}
	check->status = MCC_SEMANTIC_CHECK_FAIL;
	check->error_buffer = NULL;
	return check;
}

// Main function exists and has correct signature
struct mcc_semantic_check* mcc_semantic_check_run_main_function(struct mcc_ast_program* ast,
                                                                struct mcc_symbol_table* symbol_table){
	UNUSED(ast);
	UNUSED(symbol_table);
	struct mcc_semantic_check *check;
	check = malloc(sizeof(check));
	if(!check){
		return NULL;
	}
	check->status = MCC_SEMANTIC_CHECK_FAIL;
	check->error_buffer = NULL;
	return check;
}

// No multiple definitions of the same function
struct mcc_semantic_check* mcc_semantic_check_run_multiple_function_definitions(struct mcc_ast_program* ast,
                                                                struct mcc_symbol_table* symbol_table){
	UNUSED(ast);
	UNUSED(symbol_table);
	struct mcc_semantic_check *check;
	check = malloc(sizeof(check));
	if(!check){
		return NULL;
	}
	check->status = MCC_SEMANTIC_CHECK_FAIL;
	check->error_buffer = NULL;
	return check;
}

// No multiple declarations of a variable in the same scope
struct mcc_semantic_check* mcc_semantic_check_run_multiple_variable_declarations(struct mcc_ast_program* ast,
                                                                struct mcc_symbol_table* symbol_table){
	UNUSED(ast);
	UNUSED(symbol_table);
	struct mcc_semantic_check *check;
	check = malloc(sizeof(check));
	if(!check){
		return NULL;
	}
	check->status = MCC_SEMANTIC_CHECK_FAIL;
	check->error_buffer = NULL;
	return check;
}

// No use of the names of the built_in functions in function definitions
struct mcc_semantic_check* mcc_semantic_check_run_define_built_in(struct mcc_ast_program* ast,
                                                                          struct mcc_symbol_table* symbol_table){
	UNUSED(ast);
	UNUSED(symbol_table);
	struct mcc_semantic_check *check;
	check = malloc(sizeof(check));
	if(!check){
		return NULL;
	}
	check->status = MCC_SEMANTIC_CHECK_FAIL;
	check->error_buffer = NULL;
	return check;
}


// ------------------------------------------------------------- Functions: Cleanup

// Delete single checks
void mcc_semantic_check_delete_single_check(struct mcc_semantic_check *check){
	UNUSED(check);
	return;
}
