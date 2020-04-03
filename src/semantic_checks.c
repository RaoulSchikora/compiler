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
	if(check){
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

// getter for data type
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

static bool is_int(struct mcc_semantic_check_data_type *type)
{
	assert(type);
	if((type->type == MCC_SEMANTIC_CHECK_INT) && !type->is_array){
		return true;
	}
	return false;
}

// returns true if type is BOOL
static bool is_bool(struct mcc_semantic_check_data_type *type)
{
	assert(type);
	if((type->type == MCC_SEMANTIC_CHECK_BOOL) && !type->is_array){
		return true;
	}
	return false;
}

// returns true is type is string
static bool is_string(struct mcc_semantic_check_data_type *type)
{
	assert(type);
	if((type->type == MCC_SEMANTIC_CHECK_STRING) && !type->is_array){
		return true;
	}
	return false;
}

static bool types_equal(struct mcc_semantic_check_data_type *first, struct mcc_semantic_check_data_type *second)
{
	assert(first);
	assert(second);

	if((first->type == second->type) && (first->array_size == second->array_size)){
		return true;
	}
	return false;
}

// check and get type of binary expression. Returns MCC_SEMANTIC_CHECK_UNKNOWN if error occurs
static struct mcc_semantic_check_data_type *check_and_get_type_binary_expression(struct mcc_ast_expression *expression, 
	struct mcc_semantic_check *check)
{
	assert(expression->lhs);
	assert(expression->rhs);
	assert(check);

	bool success = false;
	struct mcc_semantic_check_data_type *lhs = check_and_get_type(expression->lhs, check);
	struct mcc_semantic_check_data_type *rhs = check_and_get_type(expression->rhs, check);
	enum mcc_ast_binary_op op = expression->op;

	switch (op)
	{
	case MCC_AST_BINARY_OP_ADD:
		success = types_equal(lhs, rhs) && !is_bool(lhs);
		break;
	case MCC_AST_BINARY_OP_SUB:
		success = types_equal(lhs, rhs) && !is_bool(lhs);
		break;
	case MCC_AST_BINARY_OP_MUL:
		success = types_equal(lhs, rhs) && !is_bool(lhs);
		break;
	case MCC_AST_BINARY_OP_DIV:
		success = types_equal(lhs, rhs) && !is_bool(lhs);
		break;
	case MCC_AST_BINARY_OP_SMALLER:
		success = types_equal(lhs, rhs) && !is_bool(lhs);
		break;
	case MCC_AST_BINARY_OP_GREATER:
		success = types_equal(lhs, rhs) && !is_bool(lhs);
		break;
	case MCC_AST_BINARY_OP_SMALLEREQ:
		success = types_equal(lhs, rhs) && !is_bool(lhs);
		break;
	case MCC_AST_BINARY_OP_GREATEREQ:
		success = types_equal(lhs, rhs) && !is_bool(lhs);
		break;
	case MCC_AST_BINARY_OP_CONJ:
		success = is_bool(lhs) && is_bool(rhs);
		break;
	case MCC_AST_BINARY_OP_DISJ:
		success = is_bool(lhs) && is_bool(rhs);
		break;
	case MCC_AST_BINARY_OP_EQUAL:
		success = types_equal(lhs, rhs);
		break;
	case MCC_AST_BINARY_OP_NOTEQUAL:
		success = types_equal(lhs, rhs);
		break;	
	default:
		break;
	}

	if(!success || lhs->is_array || rhs->is_array || is_string(lhs) || is_string(rhs)){
		//TODO error handling: differentiate between logical connectives and implict type conversion: Implicite type conversion from 'lhs' to 'rhs'
		check->status = MCC_SEMANTIC_CHECK_FAIL;
		char *buffer = malloc(49);
		snprintf(buffer, 49, "operation on incompatible types 'lhs' and 'rhs'.");
		if(check->error_buffer == NULL){
			check->error_buffer = buffer;
		}
		lhs->type = MCC_SEMANTIC_CHECK_UNKNOWN;
	}
	if(success && (lhs->type == MCC_SEMANTIC_CHECK_UNKNOWN)){
		//TODO error handling (is error possible ?!)
		check->status = MCC_SEMANTIC_CHECK_FAIL;
		char *buffer = malloc(50);
		snprintf(buffer, 50, "error: operation on unknown type");
		if(check->error_buffer == NULL){
			check->error_buffer = buffer;
		}
	}
	if (!(op == MCC_AST_BINARY_OP_ADD || op == MCC_AST_BINARY_OP_SUB || op == MCC_AST_BINARY_OP_MUL || op == MCC_AST_BINARY_OP_DIV)){
		lhs->type = MCC_SEMANTIC_CHECK_BOOL;
	}
	return lhs;

}

struct mcc_semantic_check_data_type *check_and_get_type_unary_expression(struct mcc_ast_expression *expression, struct mcc_semantic_check *check)
{
	assert(expression->type == MCC_AST_EXPRESSION_TYPE_UNARY_OP);
	assert(expression->child);
	assert(check);

	struct mcc_semantic_check_data_type *child = check_and_get_type(expression->child, check);
	enum mcc_ast_unary_op u_op = expression->u_op;

	if(child->is_array || is_string(child) || ((u_op == MCC_AST_UNARY_OP_NEGATIV) && is_bool(child)) || ((u_op == MCC_AST_UNARY_OP_NOT) && !is_bool(child)) ){
		// TODO error Handling 
		check->status = MCC_SEMANTIC_CHECK_FAIL;
		char *buffer = malloc(70);
		snprintf(buffer, 70, "unary operation not compatible with 'child'.");
		if(check->error_buffer == NULL){
			check->error_buffer = buffer;
		}
		child->type = MCC_SEMANTIC_CHECK_UNKNOWN;
	}
	return child;
}

// get and check the type of an array element, including the index to be of type 'INT'
static struct mcc_semantic_check_data_type *check_and_get_type_array_element(struct mcc_ast_expression *array_element, 
	struct mcc_semantic_check *check)
{
	assert(array_element->type == MCC_AST_EXPRESSION_TYPE_ARRAY_ELEMENT);
	assert(check);

	struct mcc_semantic_check_data_type *index = check_and_get_type(array_element->index, check);
	struct mcc_semantic_check_data_type *identifier = check_and_get_type(array_element->array_identifier, check, array_element->array_row);
	if(!is_int(index)){
		// TODO error Handling 
		check->status = MCC_SEMANTIC_CHECK_FAIL;
		char *buffer = malloc(37);
		snprintf(buffer, 37, "expected type 'INT' but was 'index'.");
		if(check->error_buffer == NULL){
			check->error_buffer = buffer;
		}
		identifier->type = MCC_SEMANTIC_CHECK_UNKNOWN;
	}
	if(!identifier->is_array){
		// TODO error Handling 
		check->status = MCC_SEMANTIC_CHECK_FAIL;
		char *buffer = malloc(42);
		snprintf(buffer, 42, "subscripted value 'name' is not an array.");
		if(check->error_buffer == NULL){
			check->error_buffer = buffer;
		}
		identifier->type = MCC_SEMANTIC_CHECK_UNKNOWN;
	}
	identifier->is_array = false;
	identifier->array_size = -1;
	return identifier;
}

// get the type of a literal, placeholder unused but needed due to macro
struct mcc_semantic_check_data_type *check_and_get_type_literal(struct mcc_ast_literal *literal, void *placeholder)
{
	assert(literal);
	UNUSED(placeholder);

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

// get and check the type of an array element, including the index to be of type 'INT'
static struct mcc_semantic_check_data_type *check_and_get_type_function_call(struct mcc_ast_expression *function_call, 
	struct mcc_semantic_check *check)
{
	// TODO implement
	assert(function_call->type == MCC_AST_EXPRESSION_TYPE_FUNCTION_CALL);
	assert(check);

	return get_new_data_type();
}

// check and get data type of expression. Returns MCC_SEMANTIC_CHECK_UNKNOWN if error occurs
struct mcc_semantic_check_data_type *check_and_get_type_expression(struct mcc_ast_expression *expression, 
    struct mcc_semantic_check *check)
{
	assert(expression);
	assert(check);

	switch (expression->type)
	{
	case MCC_AST_EXPRESSION_TYPE_LITERAL:
		return check_and_get_type(expression->literal, NULL);
	case MCC_AST_EXPRESSION_TYPE_BINARY_OP:
		return check_and_get_type_binary_expression(expression, check);
	case MCC_AST_EXPRESSION_TYPE_PARENTH:
		return check_and_get_type(expression->expression, check);
	case MCC_AST_EXPRESSION_TYPE_UNARY_OP:
		return check_and_get_type_unary_expression(expression, check);
	case MCC_AST_EXPRESSION_TYPE_VARIABLE:
		return check_and_get_type(expression->identifier, check, expression->variable_row);
	case MCC_AST_EXPRESSION_TYPE_ARRAY_ELEMENT:
		return check_and_get_type_array_element(expression, check);
	case MCC_AST_EXPRESSION_TYPE_FUNCTION_CALL:
		//TODO
		return check_and_get_type_function_call(expression, check);
	default:
		return NULL;
	}
}

// check and get data type of identifier. Returns MCC_SEMANTIC_CHECK_UNKNOWN if identifier 
// has not been found in the symbol table
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
		//TODO error: 'name' undeclared (first use in this function)."
		check->status = MCC_SEMANTIC_CHECK_FAIL;
		char *buffer = malloc(50);
		snprintf(buffer, 50, "'name' undeclared (first use in this function).");
		if(check->error_buffer == NULL){
			check->error_buffer = buffer;
		}
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

	type->array_size = row->array_size;
	if(row->array_size != -1){
		type->is_array = true;
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

	struct mcc_semantic_check_data_type *lhs_type = NULL;
	struct mcc_semantic_check_data_type *rhs_type = NULL;
	struct mcc_semantic_check_data_type *index = NULL;

	switch (assignment->assignment_type)
	{
	case MCC_AST_ASSIGNMENT_TYPE_VARIABLE:
		lhs_type = check_and_get_type(assignment->variable_identifier, check, assignment->row);
		rhs_type = check_and_get_type(assignment->variable_assigned_value, check);
		break;
	case MCC_AST_ASSIGNMENT_TYPE_ARRAY:
		lhs_type = check_and_get_type(assignment->array_identifier, check, assignment->row);
		rhs_type = check_and_get_type(assignment->array_assigned_value, check);
		index = check_and_get_type(assignment->array_index, check);
		lhs_type->is_array = false;
		lhs_type->array_size = -1;
		break;
	default:
		break;
	}

	if(index && !is_int(index)){
		//TODO use elaborate error msg'.
		check->status = MCC_SEMANTIC_CHECK_FAIL;
		char *buffer = malloc(35);
		snprintf(buffer, 35, "array subscript is not an integer.");
		if(check->error_buffer == NULL){
			check->error_buffer = buffer;
		}
	}
	if(!types_equal(lhs_type, rhs_type)){
		//TODO use elaborate error msg: implicit type conversion. Expected 'lhs_type' but was 'rhs_type'.
		check->status = MCC_SEMANTIC_CHECK_FAIL;
		char *buffer = malloc(65);
		snprintf(buffer, 65, "implicit type conversion. Expected 'lhs_type' but was 'rhs_type'");
		if(check->error_buffer == NULL){
			check->error_buffer = buffer;
		}
	}
	if(lhs_type->is_array){
		//TODO use elaborate error msg: Assignment to Variable of array type not possible.
		check->status = MCC_SEMANTIC_CHECK_FAIL;
		char *buffer = malloc(50);
		snprintf(buffer, 50, "Assignment to Variable of array type not possible");
		if(check->error_buffer == NULL){
			check->error_buffer = buffer;
		}
	}
	free(lhs_type);
	free(rhs_type);
}

// Setup an AST Visitor for type checking
static struct mcc_ast_visitor type_checking_visitor(struct mcc_semantic_check *check)
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

	struct mcc_ast_visitor visitor = type_checking_visitor(check);
	mcc_ast_visit(ast, &visitor);
	return MCC_SEMANTIC_CHECK_ERROR_OK;
}

// Each execution path of non-void function returns a value
enum mcc_semantic_check_error_code mcc_semantic_check_run_nonvoid_check(struct mcc_ast_program* ast,
                                                                        struct mcc_symbol_table *symbol_table,
                                                                        struct mcc_semantic_check *check){
	UNUSED(ast);
	UNUSED(symbol_table);
	check->status = MCC_SEMANTIC_CHECK_OK;
	check->error_buffer = NULL;
	return MCC_SEMANTIC_CHECK_ERROR_OK;
}

// Main function exists and has correct signature
enum mcc_semantic_check_error_code mcc_semantic_check_run_main_function(struct mcc_ast_program* ast,
                                                                        struct mcc_symbol_table *symbol_table,
                                                                        struct mcc_semantic_check *check){
	UNUSED(ast);
	UNUSED(symbol_table);
	check->status = MCC_SEMANTIC_CHECK_OK;
	check->error_buffer = NULL;
	return MCC_SEMANTIC_CHECK_ERROR_OK;
}

// No multiple definitions of the same function
enum mcc_semantic_check_error_code mcc_semantic_check_run_multiple_function_definitions(struct mcc_ast_program* ast,
                                                                                        struct mcc_symbol_table *symbol_table,
                                                                                        struct mcc_semantic_check *check){
	UNUSED(ast);
	UNUSED(symbol_table);
	check->status = MCC_SEMANTIC_CHECK_OK;
	check->error_buffer = NULL;
	return MCC_SEMANTIC_CHECK_ERROR_OK;
}

// No multiple declarations of a variable in the same scope
enum mcc_semantic_check_error_code mcc_semantic_check_run_multiple_variable_declarations(struct mcc_ast_program* ast,
                                                                                        struct mcc_symbol_table *symbol_table,
                                                                                        struct mcc_semantic_check *check){
	UNUSED(ast);
	UNUSED(symbol_table);
	check->status = MCC_SEMANTIC_CHECK_OK;
	check->error_buffer = NULL;
	return MCC_SEMANTIC_CHECK_ERROR_OK;
}

// ------------------------------------------------------------- Functions: Cleanup

// Delete single checks
void mcc_semantic_check_delete_single_check(struct mcc_semantic_check *check){
	UNUSED(check);
	return;
}
