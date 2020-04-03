#include "mcc/semantic_checks.h"

#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "mcc/ast.h"
#include "mcc/ast_visit.h"
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
struct mcc_semantic_check* mcc_semantic_check_run_type_check(struct mcc_ast_program* ast,
                                                                  struct mcc_symbol_table* symbol_table)
{
	UNUSED(ast);
	UNUSED(symbol_table);
	struct mcc_semantic_check *check;
	check = malloc(sizeof(check));
	if(!check){
		return NULL;
	}
	check->status = MCC_SEMANTIC_CHECK_OK;
	check->type = MCC_SEMANTIC_CHECK_TYPE_CHECK;
	check->error_buffer = NULL;

	struct mcc_ast_visitor visitor = type_conversion_visitor(check);
	mcc_ast_visit(ast, &visitor);
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
