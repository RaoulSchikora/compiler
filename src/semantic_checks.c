#include "mcc/semantic_checks.h"

#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "mcc/ast.h"
#include "mcc/ast_visit.h"
#include "mcc/symbol_table.h"
#include "utils/unused.h"

// Generate struct for semantic check
struct mcc_semantic_check *mcc_semantic_check_initialize_check(){
	struct mcc_semantic_check *check = malloc(sizeof(check));
	if(!check){
		check->status = MCC_SEMANTIC_CHECK_OK;
		check->error_buffer = NULL;
	}
	return check;
}

// Run all semantic checks
struct mcc_semantic_check* mcc_semantic_check_run_all(struct mcc_ast_program* ast,
													  struct mcc_symbol_table* symbol_table){

	enum mcc_semantic_check_error_code error = MCC_SEMANTIC_CHECK_ERROR_OK;

	struct mcc_semantic_check *check = mcc_semantic_check_initialize_check();
	if(!check){
		return NULL;
	}

	error =
	    mcc_semantic_check_early_abort_wrapper(mcc_semantic_check_run_type_check, ast, symbol_table, check, error);
	error =
	    mcc_semantic_check_early_abort_wrapper(mcc_semantic_check_run_nonvoid_check, ast, symbol_table, check, error);
	error =
	    mcc_semantic_check_early_abort_wrapper(mcc_semantic_check_run_main_function, ast, symbol_table, check, error);
	error =
	    mcc_semantic_check_early_abort_wrapper(mcc_semantic_check_run_multiple_function_definitions, ast, symbol_table, check, error);
	error =
	    mcc_semantic_check_early_abort_wrapper(mcc_semantic_check_run_multiple_variable_declarations, ast, symbol_table, check, error);

	if (error != MCC_SEMANTIC_CHECK_ERROR_OK){
		return NULL;
	}
	return check;
}

// ------------------------------------------------------------- Functions: Running single semantic checks

// Wrapper for running one of the checks  
// If the error code of the previous check is not OK the error code is handed back immediately
// If the status code of the previous check is not OK, we abort with error code OK
enum mcc_semantic_check_error_code mcc_semantic_check_early_abort_wrapper(
    enum mcc_semantic_check_error_code (*fctptr)(struct mcc_ast_program *ast, struct mcc_symbol_table *table, struct mcc_semantic_check *check),
	struct mcc_ast_program *ast, 
	struct mcc_symbol_table *table,
	struct mcc_semantic_check* check,
	enum mcc_semantic_check_error_code previous_ok)
	{
		// Early abort. Return error code of the previous check
		if(previous_ok != MCC_SEMANTIC_CHECK_ERROR_OK){
		        return previous_ok;
		}

		// Early abort. Just return without doing anything.
		if(check->status != MCC_SEMANTIC_CHECK_OK){
		        return MCC_SEMANTIC_CHECK_ERROR_OK;
		}

		// Run the semantic check and return its return value
		return (*fctptr)(ast, table, check);
	}

// ------------------------------------------------------------- Type conversion

static struct mcc_semantic_check_data_type *get_new_data_type()
{
	struct mcc_semantic_check_data_type *type = malloc(sizeof(type));
	if(!type){
		return NULL;
	}
	type->type = MCC_SEMANTIC_CHECK_UNKNOWN;
	type->is_array = false;
	type->array_size = -1;
	return type;
}

struct mcc_semantic_check_data_type *check_and_get_type_literal(struct mcc_ast_literal *literal)
{
	assert(literal);

	struct mcc_semantic_check_data_type *type = get_new_data_type();

	switch (literal->type)
	{
	case MCC_AST_LITERAL_TYPE_INT:
		type->type = MCC_SEMANTIC_CHECK_INT;
		break;
	case MCC_AST_LITERAL_TYPE_FLOAT:
		type->type = MCC_SEMANTIC_CHECK_FLOAT;
		break;
	case MCC_AST_LITERAL_TYPE_BOOL:
		type->type = MCC_SEMANTIC_CHECK_BOOL;
		break;
	case MCC_AST_LITERAL_TYPE_STRING:
		type->type = MCC_SEMANTIC_CHECK_STRING;
		break;
	default:
		type->type = MCC_SEMANTIC_CHECK_UNKNOWN;
		break;
	}

	return type;
}

// check and get data type of expression
struct mcc_semantic_check_data_type *check_and_get_type_expression(struct mcc_ast_expression *expression, 
    struct mcc_semantic_check *check)
{
	assert(expression);
	assert(check);

	struct mcc_semantic_check_data_type *type = get_new_data_type();

	switch (expression->type)
	{
	case MCC_AST_EXPRESSION_TYPE_LITERAL:
		return check_and_get_type_literal(expression->literal);
	case MCC_AST_EXPRESSION_TYPE_BINARY_OP:
		break;
	case MCC_AST_EXPRESSION_TYPE_PARENTH:
		return check_and_get_type(expression->expression, check);
	case MCC_AST_EXPRESSION_TYPE_UNARY_OP:
		break;
	case MCC_AST_EXPRESSION_TYPE_VARIABLE:
		return check_and_get_type(expression->identifier, check, expression->variable_row);
	case MCC_AST_EXPRESSION_TYPE_ARRAY_ELEMENT:
		break;
	case MCC_AST_EXPRESSION_TYPE_FUNCTION_CALL:
		break;
	default:
		break;
	}

	return type;
}

// check and get data type of identifier
struct mcc_semantic_check_data_type *check_and_get_type_identifier(struct mcc_ast_identifier *identifier, 
    struct mcc_semantic_check *check, struct mcc_symbol_table_row *row)
{
	assert(identifier);
	assert(check);
	assert(row);

	struct mcc_semantic_check_data_type *type = get_new_data_type();

	char *name = identifier->identifier_name;
	row = mcc_symbol_table_check_upwards_for_declaration(name, row);
	if(!row){
		type->type = MCC_SEMANTIC_CHECK_UNKNOWN;
		//TODO error for use of unknown varibale
		return type;
	}

	switch (row->row_type)
	{
	case MCC_SYMBOL_TABLE_ROW_TYPE_INT:
		type->type = MCC_SEMANTIC_CHECK_INT;
		break;
	case MCC_SYMBOL_TABLE_ROW_TYPE_FLOAT:
		type->type = MCC_SEMANTIC_CHECK_FLOAT;
		break;
	case MCC_SYMBOL_TABLE_ROW_TYPE_BOOL:
		type->type = MCC_SEMANTIC_CHECK_BOOL;
		break;
	case MCC_SYMBOL_TABLE_ROW_TYPE_STRING:
		type->type = MCC_SEMANTIC_CHECK_STRING;
		break;
	case MCC_SYMBOL_TABLE_ROW_TYPE_VOID:
		type->type = MCC_SEMANTIC_CHECK_VOID;
		break;
	default:
		type->type = MCC_SEMANTIC_CHECK_UNKNOWN;
		break;
	}

	return type;
}

// callback for checking type conversion in an assignment
static void cb_type_conversion_assignment(struct mcc_ast_statement *statement, void *data)
{
	assert(statement);
	assert(data);

	struct mcc_semantic_check *check = data;
	struct mcc_ast_assignment *assignment = statement->assignment;

	//TODO think about special cases with arrays
	if(assignment->assignment_type == MCC_AST_ASSIGNMENT_TYPE_ARRAY){
		return;
	}
	struct mcc_semantic_check_data_type *lhs_type = check_and_get_type(assignment->variable_identifier, check, assignment->row);
	struct mcc_semantic_check_data_type *rhs_type = check_and_get_type(assignment->variable_assigned_value, check);

	//TODO use compare_type
	if(lhs_type->type != rhs_type->type){
		check->status = MCC_SEMANTIC_CHECK_FAIL;
		//TODO use elaborate error msg 
		char *buffer = malloc(50);
		snprintf(buffer, 50, "error");
		check->error_buffer = buffer;
	}
}

// Setup an AST Visitor for type checking
static struct mcc_ast_visitor type_conversion_visitor(struct mcc_semantic_check *check)
{

	return (struct mcc_ast_visitor){
	    .traversal = MCC_AST_VISIT_DEPTH_FIRST,
	    .order = MCC_AST_VISIT_POST_ORDER,

	    .userdata = check,

	    .statement_assignment = cb_type_conversion_assignment,
	};
}

// run the type checker
enum mcc_semantic_check_error_code mcc_semantic_check_run_type_check(struct mcc_ast_program *ast,
                                                                     struct mcc_symbol_table *symbol_table,
                                                                     struct mcc_semantic_check *check)
{
	UNUSED(symbol_table);
	check->status = MCC_SEMANTIC_CHECK_OK;
	check->error_buffer = NULL;

	struct mcc_ast_visitor visitor = type_conversion_visitor(check);
	mcc_ast_visit(ast, &visitor);
	return MCC_SEMANTIC_CHECK_ERROR_OK;
}

// Each execution path of non-void function returns a value
enum mcc_semantic_check_error_code mcc_semantic_check_run_nonvoid_check(struct mcc_ast_program* ast,
                                                                        struct mcc_symbol_table *symbol_table,
                                                                        struct mcc_semantic_check *check){
	UNUSED(ast);
	UNUSED(symbol_table);
	check->status = MCC_SEMANTIC_CHECK_FAIL;
	check->error_buffer = NULL;
	return MCC_SEMANTIC_CHECK_ERROR_OK;
}

// Main function exists and has correct signature
enum mcc_semantic_check_error_code mcc_semantic_check_run_main_function(struct mcc_ast_program* ast,
                                                                        struct mcc_symbol_table *symbol_table,
                                                                        struct mcc_semantic_check *check){
	UNUSED(ast);
	UNUSED(symbol_table);
	check->status = MCC_SEMANTIC_CHECK_FAIL;
	check->error_buffer = NULL;
	return MCC_SEMANTIC_CHECK_ERROR_OK;
}

// No multiple definitions of the same function
enum mcc_semantic_check_error_code mcc_semantic_check_run_multiple_function_definitions(struct mcc_ast_program* ast,
                                                                                        struct mcc_symbol_table *symbol_table,
                                                                                        struct mcc_semantic_check *check){
	UNUSED(ast);
	UNUSED(symbol_table);
	check->status = MCC_SEMANTIC_CHECK_FAIL;
	check->error_buffer = NULL;
	return MCC_SEMANTIC_CHECK_ERROR_OK;
}

// No multiple declarations of a variable in the same scope
enum mcc_semantic_check_error_code mcc_semantic_check_run_multiple_variable_declarations(struct mcc_ast_program* ast,
                                                                                        struct mcc_symbol_table *symbol_table,
                                                                                        struct mcc_semantic_check *check){
	UNUSED(ast);
	UNUSED(symbol_table);
	check->status = MCC_SEMANTIC_CHECK_FAIL;
	check->error_buffer = NULL;
	return MCC_SEMANTIC_CHECK_ERROR_OK;
}

// ------------------------------------------------------------- Functions: Cleanup

// Delete single checks
void mcc_semantic_check_delete_single_check(struct mcc_semantic_check *check){
	UNUSED(check);
	return;
}
