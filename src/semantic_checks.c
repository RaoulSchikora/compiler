#include "mcc/semantic_checks.h"

#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <math.h>

#include "mcc/ast.h"
#include "mcc/ast_visit.h"
#include "mcc/symbol_table.h"
#include "utils/unused.h"

#define not_zero(x) (x>0 ? x : 1)

// ------------------------------------------------------------- Functions: Error handling

// Compute string length of source code location
static int get_sloc_string_size(struct mcc_ast_node node){
	// Hard coded 6 due to rounding and colons
	return floor(log10(not_zero(node.sloc.start_col)) + log10(not_zero(node.sloc.start_line))) 
	       + strlen(node.sloc.filename) + 6;
}

static enum mcc_semantic_check_error_code write_error_message_to_check_with_sloc(struct mcc_semantic_check *check,
																				 struct mcc_ast_node node,
																				 const char *string)
{
	assert(check);
	assert(check->error_buffer == NULL);
	assert(string);

	// +1 for terminating character
	int size = sizeof(char) * (strlen(string) + get_sloc_string_size(node) + 1);
	char *buffer = malloc(size);
	if (!buffer) {
		return MCC_SEMANTIC_CHECK_ERROR_MALLOC_FAILED;
	}
	if( 0 > snprintf(buffer, size, "%s:%d:%d: %s\n", node.sloc.filename, 
					 node.sloc.start_line, node.sloc.start_col, string)){
		return MCC_SEMANTIC_CHECK_ERROR_SNPRINTF_FAILED;
	}
	check->error_buffer = buffer;
	return MCC_SEMANTIC_CHECK_ERROR_OK;
}

enum mcc_semantic_check_error_code raise_error(int num, struct mcc_semantic_check *check,
														struct mcc_ast_node node,
														const char *format_string, ...)
{
	assert(format_string);

	if(check->status == MCC_SEMANTIC_CHECK_FAIL){
		return MCC_SEMANTIC_CHECK_ERROR_OK;
	}

	check->status = MCC_SEMANTIC_CHECK_FAIL;

	// Get all the args & determine string length
	size_t args_size = 0;
	va_list args;
	va_start(args, format_string);
	for (int i = 0; i < num; i++){
		args_size += strlen(va_arg(args, const char *));
	}
	va_end(args);

	// Malloc buffer string
	int size = sizeof(char) * (strlen(format_string) + args_size + 1);
	char *buffer = malloc(size);
	if (!buffer) {
		return MCC_SEMANTIC_CHECK_ERROR_MALLOC_FAILED;
	}

	// Get args again and print string into buffer
	va_start(args, format_string);
	if( 0 > vsnprintf(buffer,size,format_string, args)){
		va_end(args);
		free(buffer);
		return MCC_SEMANTIC_CHECK_ERROR_SNPRINTF_FAILED;
	}
	va_end(args);
	enum mcc_semantic_check_error_code error = MCC_SEMANTIC_CHECK_ERROR_OK;

	// Write buffer string into check
	error = write_error_message_to_check_with_sloc(check,node,buffer);
	free(buffer);
	return error;
}

// ------------------------------------------------------------- Functions: Set up and run all checks

// Generate struct for semantic check
struct mcc_semantic_check *mcc_semantic_check_initialize_check(){
	struct mcc_semantic_check *check = malloc(sizeof(*check));
	if(check){
		check->status = MCC_SEMANTIC_CHECK_OK;
		check->error_buffer = NULL;
	}
	return check;
}


// Run all semantic checks, returns NULL if library functions fail
struct mcc_semantic_check* mcc_semantic_check_run_all(struct mcc_ast_program* ast,
													  struct mcc_symbol_table* symbol_table){
	assert(ast);
	assert(symbol_table);

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
	error =
	    mcc_semantic_check_early_abort_wrapper(mcc_semantic_check_run_function_arguments, ast, symbol_table, check, error);

	if (error != MCC_SEMANTIC_CHECK_ERROR_OK){
		mcc_semantic_check_delete_single_check(check);
		return NULL;
	}
	return check;
}

// ------------------------------------------------------------- Functions: Running single semantic checks with early abort

// Define function pointer to a single sematic check
enum mcc_semantic_check_error_code (*fctptr)(struct mcc_ast_program *ast, struct mcc_symbol_table *table, 
											 struct mcc_semantic_check *check);

// Wrapper for running one of the checks with 2 early aborts:
// Error code of the previous check is not OK: Error code is handed back immediately
// Status code of the previous check is not OK: Abort with error code OK, but don't change check
enum mcc_semantic_check_error_code mcc_semantic_check_early_abort_wrapper(
    enum mcc_semantic_check_error_code (*fctptr)(struct mcc_ast_program *ast, struct mcc_symbol_table *table, 
	struct mcc_semantic_check *check), struct mcc_ast_program *ast, 
	struct mcc_symbol_table *table, struct mcc_semantic_check* check,
	enum mcc_semantic_check_error_code previous_return) {

	assert(ast);
	assert(table);
	assert(check);
	assert(fctptr);

	// Early abort. Return error code of the previous check
	if(previous_return != MCC_SEMANTIC_CHECK_ERROR_OK){
		return previous_return;
	}

	// Early abort. Return without doing anything.
	if(check->status != MCC_SEMANTIC_CHECK_OK){
			return MCC_SEMANTIC_CHECK_ERROR_OK;
	}

	// Run the semantic check and return its return value
	return (*fctptr)(ast, table, check);
	}

// ------------------------------------------------------------- check_and_get_type functionalities

// getter for default data type
static struct mcc_semantic_check_data_type *get_new_data_type()
{
	struct mcc_semantic_check_data_type *type = malloc(sizeof(*type));
	if(!type){
		return NULL;
	}
	type->type = MCC_SEMANTIC_CHECK_UNKNOWN;
	type->is_array = false;
	type->array_size = -1;
	return type;
}

// get data type of given symbol tabel row
static struct mcc_semantic_check_data_type *get_data_type_from_row(struct mcc_symbol_table_row *row)
{
	assert(row);

	struct mcc_semantic_check_data_type *type = get_new_data_type();
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

	if(row->array_size != -1){
		type->is_array = true;
	}
	type->array_size = row->array_size;
	return type;
}

// Convert Type from AST into Type from semantic checks
static enum mcc_semantic_check_data_types ast_to_semantic_check_type(struct mcc_ast_type *type){
	switch(type->type_value){
		case INT:
			return MCC_SEMANTIC_CHECK_INT;
		case FLOAT:
			return MCC_SEMANTIC_CHECK_FLOAT;
		case BOOL:
			return MCC_SEMANTIC_CHECK_BOOL;
		case STRING:
			return MCC_SEMANTIC_CHECK_STRING;
		default:
			return MCC_SEMANTIC_CHECK_UNKNOWN;
	}
}

// get data type of declaration
static struct mcc_semantic_check_data_type *get_data_type_declaration(struct mcc_ast_declaration* decl,
																	  struct mcc_semantic_check *check){
	assert(decl);
	UNUSED(check);
	struct mcc_semantic_check_data_type *type = get_new_data_type();
	if(!type)
		return NULL;
	if(decl->declaration_type == MCC_AST_DECLARATION_TYPE_VARIABLE){
		type->is_array = false;
		type->type = ast_to_semantic_check_type(decl->variable_type);
	} else {
		type->is_array = true;
		type->array_size = decl->array_size->i_value;
		type->type = ast_to_semantic_check_type(decl->array_type);
	}
	return type;
}

// returns true if type is INT
static bool is_int(struct mcc_semantic_check_data_type *type)
{
	assert(type);
	return ((type->type == MCC_SEMANTIC_CHECK_INT) && !type->is_array);
}

// returns true if type is BOOL
static bool is_bool(struct mcc_semantic_check_data_type *type)
{
	assert(type);
	return ((type->type == MCC_SEMANTIC_CHECK_BOOL) && !type->is_array);
}

// returns true if type is string
static bool is_string(struct mcc_semantic_check_data_type *type)
{
	assert(type);
	return ((type->type == MCC_SEMANTIC_CHECK_STRING) && !type->is_array);
}

// returns true if given types equal
static bool types_equal(struct mcc_semantic_check_data_type *first, struct mcc_semantic_check_data_type *second)
{
	assert(first);
	assert(second);
	return ((first->type == second->type) && (first->array_size == second->array_size));
}

// converts a data type into a string
static char* to_string(struct mcc_semantic_check_data_type *type)
{
	assert(type);

	char buffer[12 + (int) floor(log10(not_zero(type->array_size)))];
	switch (type->type)
	{
	case MCC_SEMANTIC_CHECK_INT:
		(type->is_array) ? sprintf(buffer, "INT[%d]", type->array_size) : sprintf(buffer, "INT");
		break;
	case MCC_SEMANTIC_CHECK_FLOAT:
		(type->is_array) ? sprintf(buffer, "FLOAT[%d]", type->array_size) : sprintf(buffer, "FLOAT");
		break;
	case MCC_SEMANTIC_CHECK_BOOL:
		(type->is_array) ? sprintf(buffer, "BOOL[%d]", type->array_size) : sprintf(buffer, "BOOL");
		break;
	case MCC_SEMANTIC_CHECK_STRING:
		(type->is_array) ? sprintf(buffer, "STRING[%d]", type->array_size) : sprintf(buffer, "STRING");
		break;
	case MCC_SEMANTIC_CHECK_VOID:
		sprintf(buffer, "VOID");
		break;	
	default:
		sprintf(buffer, "UNKNOWN");
		break;
	}
	char *string = malloc(sizeof(char) * strlen(buffer) + 1);
	snprintf(string, strlen(buffer) + 1, "%s", buffer);
	return string;
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
		raise_error(2, check, expression->node, "operation on incompatible types '%s' and '%s'.", to_string(lhs), to_string(rhs));
		lhs->type = MCC_SEMANTIC_CHECK_UNKNOWN;
	}
	if(success && (lhs->type == MCC_SEMANTIC_CHECK_UNKNOWN)){
		raise_error(0, check, expression->node, "unknown type.");
	}
	if (!(op == MCC_AST_BINARY_OP_ADD || op == MCC_AST_BINARY_OP_SUB || op == MCC_AST_BINARY_OP_MUL || op == MCC_AST_BINARY_OP_DIV)){
		lhs->type = MCC_SEMANTIC_CHECK_BOOL;
	}
	free(rhs);
	return lhs;
}

// check and get type of a unary expression
struct mcc_semantic_check_data_type *check_and_get_type_unary_expression(struct mcc_ast_expression *expression, struct mcc_semantic_check *check)
{
	assert(expression->type == MCC_AST_EXPRESSION_TYPE_UNARY_OP);
	assert(expression->child);
	assert(check);

	struct mcc_semantic_check_data_type *child = check_and_get_type(expression->child, check);
	enum mcc_ast_unary_op u_op = expression->u_op;

	if(child->is_array || is_string(child) || ((u_op == MCC_AST_UNARY_OP_NEGATIV) && is_bool(child)) || ((u_op == MCC_AST_UNARY_OP_NOT) && !is_bool(child)) ){
		raise_error(1, check, expression->node, "unary operation not compatible with '%s'.", to_string(child));
		child->type = MCC_SEMANTIC_CHECK_UNKNOWN;
	}
	return child;
}

// get and check the type of an array element. Includes ensuring index to be of type 'INT'
static struct mcc_semantic_check_data_type *check_and_get_type_array_element(struct mcc_ast_expression *array_element, 
	struct mcc_semantic_check *check)
{
	assert(array_element->type == MCC_AST_EXPRESSION_TYPE_ARRAY_ELEMENT);
	assert(check);

	struct mcc_semantic_check_data_type *index = check_and_get_type(array_element->index, check);
	struct mcc_semantic_check_data_type *identifier = check_and_get_type(array_element->array_identifier, check, array_element->array_row);
	char *name = array_element->array_identifier->identifier_name;
	if(!is_int(index)){
		raise_error(1, check, array_element->node, "expected type 'INT' but was '%s'.", to_string(index));
		identifier->type = MCC_SEMANTIC_CHECK_UNKNOWN;
	}
	if(!identifier->is_array){
		raise_error(1, check, array_element->node, "subscripted value '%s' is not an array.", name);
		identifier->type = MCC_SEMANTIC_CHECK_UNKNOWN;
	}
	identifier->is_array = false;
	identifier->array_size = -1;
	free(index);
	return identifier;
}

// gets the type of an function call expression. Arguments are checked seperatly.
static struct mcc_semantic_check_data_type *check_and_get_type_function_call(struct mcc_ast_expression *function_call, 
	struct mcc_semantic_check *check)
{
	assert(function_call->type == MCC_AST_EXPRESSION_TYPE_FUNCTION_CALL);
	assert(check);

	char *name = function_call->function_identifier->identifier_name;
	struct mcc_symbol_table_row *row = function_call->function_row;
	row = mcc_symbol_table_check_for_function_declaration(name, row);

	if(!row){
		raise_error(1, check, function_call->node, "'%s' undeclared (first use in this function).", name);
		return get_new_data_type();
	} else {
		return get_data_type_from_row(row);
	}
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

	char *name = identifier->identifier_name;
	row = mcc_symbol_table_check_upwards_for_declaration(name, row);
	if(!row){
		//raise_error(1, check, identifier->node, "'%s' undeclared (first use in this function).", name);
		return get_new_data_type();
	}

	return get_data_type_from_row(row);
}

// ------------------------------------------------------------- type checker

// struct for user data concerning the type checker
struct type_checking_userdata{
	struct mcc_semantic_check *check;
	enum mcc_semantic_check_error_code error;
};

// callback for checking type conversion in an assignment
static void cb_type_conversion_assignment(struct mcc_ast_statement *statement, void *data)
{
	assert(statement);
	assert(data);

	struct type_checking_userdata *userdata = data;
	struct mcc_semantic_check *check = userdata->check;
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
		break;
	default:
		break;
	}

	if(!lhs_type || !rhs_type || (!index && assignment->assignment_type == MCC_AST_ASSIGNMENT_TYPE_ARRAY)){
		userdata->error = MCC_SEMANTIC_CHECK_ERROR_MALLOC_FAILED; 
	} else {
		if(assignment->assignment_type == MCC_AST_ASSIGNMENT_TYPE_ARRAY){
			lhs_type->is_array = false;
			lhs_type->array_size = -1;
		}
		if(index && !is_int(index)){
			userdata->error = raise_error(0, check, assignment->node, "array subscript is not an integer.");
		} else if(!types_equal(lhs_type, rhs_type)){
			userdata->error = raise_error(2, check, assignment->node, "implicit type conversion. Expected '%s' but was '%s'", to_string(lhs_type), to_string(rhs_type));
		} else if(lhs_type->is_array){
			userdata->error = raise_error(0, check, assignment->node, "assignment to Variable of array type not possible.");
		}
	}
	free(lhs_type);
	free(rhs_type);
	free(index);
}

// callback ensuring the condition of an if-statement to be of type BOOL
static void cb_type_check_if_stmt(struct mcc_ast_statement *statement, void *data)
{
	assert(statement->if_condition);
	assert(data);

	struct type_checking_userdata *userdata = data;
	struct mcc_semantic_check *check = userdata->check;
	struct mcc_ast_expression *if_condition = statement->if_condition;
	struct mcc_semantic_check_data_type *type = check_and_get_type(if_condition, check);

	if(!type){
		userdata->error = MCC_SEMANTIC_CHECK_ERROR_MALLOC_FAILED; 
	} else if(!is_bool(type)){
		userdata->error = raise_error(1, check, if_condition->node, "condition of if-statement of type '%s', expected type 'BOOL'.", to_string(type));
	}
	free(type);
}

// callback ensuring the condition of an if_else-statement to be of type BOOL
static void cb_type_check_if_else_stmt(struct mcc_ast_statement *statement, void *data)
{
	assert(statement->if_else_condition);
	assert(data);

	struct type_checking_userdata *userdata = data;
	struct mcc_semantic_check *check = userdata->check;
	struct mcc_ast_expression *if_condition = statement->if_else_condition;
	struct mcc_semantic_check_data_type *type = check_and_get_type(if_condition, check);
	
	if(!type){
		userdata->error = MCC_SEMANTIC_CHECK_ERROR_MALLOC_FAILED; 
	}else if(!is_bool(type)){
		userdata->error = raise_error(1, check, if_condition->node, "condition of if-statement of type '%s', expected type 'BOOL'.", to_string(type));
	}
	free(type);
}

// callback ensuring the condition of a while-statement to be of type BOOL
static void cb_type_check_while_stmt(struct mcc_ast_statement *statement, void *data)
{
	assert(statement->while_condition);
	assert(data);

	struct type_checking_userdata *userdata = data;
	struct mcc_semantic_check *check = userdata->check;
	struct mcc_ast_expression *while_condition = statement->while_condition;
	struct mcc_semantic_check_data_type *type = check_and_get_type(while_condition, check);
	
	if(!type){
		userdata->error = MCC_SEMANTIC_CHECK_ERROR_MALLOC_FAILED;
	} else if(!is_bool(type)){
		userdata->error = raise_error(1, check, while_condition->node, "condition of while-loop of type '%s', expected type 'BOOL'.", to_string(type));
	}
	free(type);
}

// callback for type checking statements consisting of a single expression
static void cb_type_check_expression_stmt(struct mcc_ast_statement *statement, void *data)
{
	assert(statement->stmt_expression);
	assert(data);

	struct type_checking_userdata *userdata = data;
	struct mcc_semantic_check *check = userdata->check;
	struct mcc_ast_expression *expression = statement->stmt_expression;
	// check the expression. No Error handling needed
	struct mcc_semantic_check_data_type *type = check_and_get_type(expression, check);

	if(!type){
		userdata->error = MCC_SEMANTIC_CHECK_ERROR_MALLOC_FAILED;
	}
	free(type);
}

// callback for type checking return statements
static void cb_type_conversion_return_stmt(struct mcc_ast_statement *statement, void *data)
{
	assert(statement->return_value);
	assert(data);

	struct type_checking_userdata *userdata = data;
	struct mcc_semantic_check *check = userdata->check;
	struct mcc_ast_expression *expression = statement->return_value;
	// check the expression. No Error handling needed
	struct mcc_semantic_check_data_type *type = check_and_get_type(expression, check);

	if(!type){
		userdata->error = MCC_SEMANTIC_CHECK_ERROR_MALLOC_FAILED;
	}
	free(type);
}

// Setup an AST Visitor for type checking
static struct mcc_ast_visitor type_checking_visitor(struct type_checking_userdata *userdata)
{

	return (struct mcc_ast_visitor){
	    .traversal = MCC_AST_VISIT_DEPTH_FIRST,
	    .order = MCC_AST_VISIT_PRE_ORDER,

	    .userdata = userdata,

		// TODO checking correct call to a function.
	    .statement_assignment = cb_type_conversion_assignment,
		.statement_if_stmt = cb_type_check_if_stmt,
		.statement_if_else_stmt = cb_type_check_if_else_stmt,
		.statement_while = cb_type_check_while_stmt,
		.statement_expression_stmt = cb_type_check_expression_stmt,
		.statement_return = cb_type_conversion_return_stmt,
	};
}

// run the type checker
enum mcc_semantic_check_error_code mcc_semantic_check_run_type_check(struct mcc_ast_program *ast,
                                                                     struct mcc_symbol_table *symbol_table,
                                                                     struct mcc_semantic_check *check)
{
	UNUSED(symbol_table);

	struct type_checking_userdata *userdata = malloc(sizeof userdata);
	userdata->check = check;
	userdata->error = MCC_SEMANTIC_CHECK_ERROR_OK;
	enum mcc_semantic_check_error_code error;

	struct mcc_ast_visitor visitor = type_checking_visitor(userdata);
	mcc_ast_visit(ast, &visitor);
	error = userdata->error;
	free(userdata);
	return error;
}

// ------------------------------------------------------------- check execution paths of non-void functions

// Forward declarations:

static bool recursively_check_nonvoid_property(struct mcc_ast_compound_statement *compound_statement);
static bool check_nonvoid_property(struct mcc_ast_statement *statement);

// check a single statement on non-void property
static bool check_nonvoid_property(struct mcc_ast_statement *statement)
{
	if(!statement)
		return false;

	switch (statement->type) {
	case MCC_AST_STATEMENT_TYPE_IF_STMT:
		break;
	case MCC_AST_STATEMENT_TYPE_IF_ELSE_STMT:
		return (check_nonvoid_property(statement->if_else_on_true) && 
				check_nonvoid_property(statement->if_else_on_false));
	case MCC_AST_STATEMENT_TYPE_EXPRESSION:
		break;
	case MCC_AST_STATEMENT_TYPE_WHILE:
		break;
	case MCC_AST_STATEMENT_TYPE_DECLARATION:
		break;
	case MCC_AST_STATEMENT_TYPE_ASSIGNMENT:
		break;
	case MCC_AST_STATEMENT_TYPE_RETURN:
		return true;
	case MCC_AST_STATEMENT_TYPE_COMPOUND_STMT:
		return recursively_check_nonvoid_property(statement->compound_statement);
	}

	return false;
}

// recursively check non-void property, i.e. all execution paths end in a return
static bool recursively_check_nonvoid_property(struct mcc_ast_compound_statement *compound_statement)
{
	if(!compound_statement)
		return false;

	return (check_nonvoid_property(compound_statement->statement) ||
	        recursively_check_nonvoid_property(compound_statement->next_compound_statement));
}

static enum mcc_semantic_check_error_code run_nonvoid_check(struct mcc_ast_function_definition *function, struct mcc_semantic_check *check)
{
	assert(function);
	assert(check);
	// Early abort if already failed
	if (check->status == MCC_SEMANTIC_CHECK_FAIL)
		return MCC_SEMANTIC_CHECK_ERROR_OK;

	if (function->type == MCC_AST_FUNCTION_TYPE_VOID)
		return MCC_SEMANTIC_CHECK_ERROR_OK;

	if(recursively_check_nonvoid_property(function->compound_stmt) == false){
		return raise_error(1, check, function->node,
		            "%s:Function is non-void, but doesn't return value on every execution path.",
		            function->identifier->identifier_name);
	}
	return MCC_SEMANTIC_CHECK_ERROR_OK;
}

// run non-void check
enum mcc_semantic_check_error_code mcc_semantic_check_run_nonvoid_check(struct mcc_ast_program* ast,
                                                                        struct mcc_symbol_table *symbol_table,
                                                                        struct mcc_semantic_check *check)
{
	UNUSED(symbol_table);
	assert(ast);
	assert(check);
	assert(!check->error_buffer);
	assert(check->status == MCC_SEMANTIC_CHECK_OK);

	enum mcc_semantic_check_error_code error = MCC_SEMANTIC_CHECK_ERROR_OK;

	do {
		error = run_nonvoid_check(ast->function, check);
		ast = ast->next_function;
	} while (ast);

	return error;
}

// ------------------------------------------------------------- checking for main function

// Main function exists and has correct signature
enum mcc_semantic_check_error_code mcc_semantic_check_run_main_function(struct mcc_ast_program* ast,
                                                                        struct mcc_symbol_table *symbol_table,
                                                                        struct mcc_semantic_check *check){
	UNUSED(symbol_table);
	assert(ast);
	assert(symbol_table);
	assert(check);
	assert(check->status == MCC_SEMANTIC_CHECK_OK);
	assert(!check->error_buffer);

	// Memorize AST entry point to get the correct filename later
	struct mcc_ast_program *original_ast = ast;
	enum mcc_semantic_check_error_code error_code = MCC_SEMANTIC_CHECK_OK;

	int number_of_mains = 0;

	do {
		if (strcmp(ast->function->identifier->identifier_name, "main") == 0) {
			number_of_mains += 1;
			if (number_of_mains > 1) {
				error_code = raise_error(0, check, original_ast->node, "Too many main functions defined.");
				return error_code;
			}
			if (!(ast->function->parameters->is_empty)) {
				error_code = raise_error(0, check, original_ast->node, "Main has wrong signature. "
																		"Must be `int main()`.");
				return error_code;
			}
		}
		ast = ast->next_function;
	} while (ast) ;

	if (number_of_mains == 0) {
		error_code = raise_error(0, check, original_ast->node, "No main function defined.");
		return error_code;
	}

	return MCC_SEMANTIC_CHECK_ERROR_OK;
}

// ------------------------------------------------------------- check for multiple function definitions

// No multiple definitions of the same function
enum mcc_semantic_check_error_code mcc_semantic_check_run_multiple_function_definitions(struct mcc_ast_program* ast,
                                                                                        struct mcc_symbol_table *symbol_table,
                                                                                        struct mcc_semantic_check *check){
	UNUSED(symbol_table);
	assert(check);
	assert(!check->error_buffer);
	assert(ast);

	struct mcc_ast_program *program_to_check = ast;
	enum mcc_semantic_check_error_code error = MCC_SEMANTIC_CHECK_ERROR_OK;

	// Program has only one function
	if (!program_to_check->next_function) {
		return MCC_SEMANTIC_CHECK_ERROR_OK;
	}

	while (program_to_check->next_function) {
		struct mcc_ast_program *program_to_compare = program_to_check->next_function;
		char *name_of_check = program_to_check->function->identifier->identifier_name;
		char *name_of_compare = program_to_compare->function->identifier->identifier_name;

		// if name of program_to_check and name of program_to_compare equals
		if (strcmp(name_of_check, name_of_compare) == 0) {
			error = raise_error(1, check, program_to_check->node, "%s: Previous function declaration was here", name_of_compare);
			return error;
		}
		// compare all next_functions
		while (program_to_compare->next_function) {
			program_to_compare = program_to_compare->next_function;
			char *name_of_compare = program_to_compare->function->identifier->identifier_name;
			// if name of program_to_check and name of program_to_compare equals
			if (strcmp(name_of_check, name_of_compare) == 0) {
				error = raise_error(1, check, program_to_check->node, "%s: Previous function declaration was here", name_of_compare);
				return error;
			}
		}

		program_to_check = program_to_check->next_function;
	}

	return MCC_SEMANTIC_CHECK_ERROR_OK;
}

// ------------------------------------------------------------- check for multiple variable declarations

// No multiple declarations of a variable in the same scope
enum mcc_semantic_check_error_code mcc_semantic_check_run_multiple_variable_declarations(struct mcc_ast_program* ast,
                                                                                        struct mcc_symbol_table *symbol_table,
                                                                                        struct mcc_semantic_check *check){
	UNUSED(check);
	UNUSED(ast);
	UNUSED(symbol_table);
	UNUSED(check);
	return MCC_SEMANTIC_CHECK_ERROR_OK;
}

// ------------------------------------------------------------- No invalid function calls

struct function_arguments_userdata {
	struct mcc_semantic_check *check;
	struct mcc_ast_program *program;
};


// callback for checking correctness of function calls
static void cb_function_arguments_expression_function_call(struct mcc_ast_expression *expression, void *userdata)
{
	assert(expression);
	assert(userdata);

	struct function_arguments_userdata *data = userdata;
	struct mcc_ast_program *ast = data->program;
	struct mcc_semantic_check *check = data->check;

	// Early abort if check already failed
	if (check->status == MCC_SEMANTIC_CHECK_FAIL) {
		return;
	}


	// Get the used arguments from the AST:
	struct mcc_ast_arguments *args = expression->arguments;

	// Get the required parameters from the function declaration
	struct mcc_ast_parameters *pars = NULL;
	do {
		if (strcmp(ast->function->identifier->identifier_name,
		           expression->function_identifier->identifier_name) == 0) {
			pars = ast->function->parameters;
		}
		ast = ast->next_function;
	} while (ast);

	// No parameters found -> unkown function
	if (!pars) {
		if (!pars) {
			raise_error(1, check, expression->node, "Undefined reference to `%s`",
			            expression->function_identifier->identifier_name);
			return;
		}
	}

	if (pars->is_empty) {
		// Expression is set to NULL during parsing, if no args are given
		if (expression->arguments->expression) {
			raise_error(1, check, expression->node, "Too many arguments to function `%s`",
			            expression->function_identifier->identifier_name);
			return;
		}
		// No arguments needed and none given: return
		return;
	}

	struct mcc_semantic_check_data_type *type_expr = NULL;
	struct mcc_semantic_check_data_type *type_decl = NULL;
	do {
		// Too little arguments (if no arguments are given, args exists, but args->expr is set to NULL)
		if (!args->expression) {
			raise_error(1, check, expression->node, "Too few arguments to function `%s`",
			            expression->function_identifier->identifier_name);
			return;
		}

		// Check for type error
		type_expr = check_and_get_type(args->expression,check);
		type_decl = check_and_get_type(pars->declaration,check);
		if (!types_equal(type_expr,type_decl)) {
			char *decl_string = to_string(type_decl);
			char *expr_string = to_string(type_expr);
			raise_error(2, check, expression->node, "Expected %s but argument is of type %s", decl_string,
			            expr_string);
			free(expr_string);
			free(decl_string);
			free(type_expr);
			free(type_decl);
			return;
		}
		free(type_expr);
		free(type_decl);

		pars = pars->next_parameters;
		args = args->next_arguments;
	} while (pars);

	if (args) {
		// Too many arguments
		raise_error(1, check, expression->node, "Too many arguments to function `%s`",
					expression->function_identifier->identifier_name);
	}
	free(type_expr);
	free(type_decl);
	return;
}

// Setup an AST Visitor for checking if function calls are correct
static struct mcc_ast_visitor function_arguments_visitor(struct function_arguments_userdata *data)
{
	return (struct mcc_ast_visitor){
	    .traversal = MCC_AST_VISIT_DEPTH_FIRST,
	    .order = MCC_AST_VISIT_POST_ORDER,

	    .userdata = data,

	    .expression_function_call = cb_function_arguments_expression_function_call,
	};
}

enum mcc_semantic_check_error_code mcc_semantic_check_run_function_arguments(struct mcc_ast_program* ast,
                                                                             struct mcc_symbol_table *symbol_table,
                                                                             struct mcc_semantic_check *check)
{
	UNUSED(symbol_table);
	assert(ast);
	assert(check);
	assert(!check->error_buffer);
	assert(check->status == MCC_SEMANTIC_CHECK_OK);

	// Set up userdata
	struct function_arguments_userdata *userdata = malloc(sizeof(*userdata));
	if (!userdata) {
		return MCC_SEMANTIC_CHECK_ERROR_MALLOC_FAILED;
	}
	userdata->check = check;
	userdata->program = ast;

	struct mcc_ast_visitor visitor = function_arguments_visitor(userdata);
	mcc_ast_visit(ast, &visitor);
	free(userdata);
	return MCC_SEMANTIC_CHECK_ERROR_OK;
}



// ------------------------------------------------------------- Functions: Cleanup

// Delete single checks
void mcc_semantic_check_delete_single_check(struct mcc_semantic_check *check){
	if (check == NULL)
		return;

	if (check->error_buffer != NULL)
		free(check->error_buffer);

	free(check);
}
