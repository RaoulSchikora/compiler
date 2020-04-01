#include "mcc/semantic_checks_old.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mcc/ast.h"
#include "mcc/ast_visit.h"
#include "utils/unused.h"
// unused.h contains macro to suppress warnings of unused variables

// ------------------------------------------------------------- Forward declaration

static bool recursively_check_nonvoid_property(struct mcc_ast_compound_statement *compound_statement);
static enum mcc_semantic_check_expression_type get_type(struct mcc_ast_expression *expression);
static char *semantic_check_expression_type_to_string(enum mcc_semantic_check_expression_type type);

// ------------------------------------------------------------- High level semantic check: Runs all and returns error

// Returns error_message on fail (allocated on the heap) and NULL otherwise
char *mcc_check_semantics(struct mcc_ast_program *ast, struct mcc_symbol_table *st)
{
	struct mcc_semantic_check_all_checks *check = mcc_semantic_check_run_all(ast, st);
	if (check->status == MCC_SEMANTIC_CHECK_OK) {
		mcc_semantic_check_delete_all_checks(check);
		return NULL;
	}
	int size = sizeof(char) * (strlen(check->error_buffer) + 1);
	char *string = malloc(size);
	if (!string) {
		mcc_semantic_check_delete_all_checks(check);
		return "malloc failed.";
	}
	if (0 > snprintf(string, size, "%s", check->error_buffer)) {
		mcc_semantic_check_delete_all_checks(check);
		return "snprintf failed.";
	}
	mcc_semantic_check_delete_all_checks(check);
	return string;
}

// ------------------------------------------------------------- Convert enum types

static enum mcc_semantic_check_expression_type convert_enum_symbol_table(enum mcc_symbol_table_row_type type)
{
	switch (type) {
	case MCC_SYMBOL_TABLE_ROW_TYPE_INT:
		return MCC_SEMANTIC_CHECK_EXPRESSION_TYPE_INT;
	case MCC_SYMBOL_TABLE_ROW_TYPE_FLOAT:
		return MCC_SEMANTIC_CHECK_EXPRESSION_TYPE_FLOAT;
	case MCC_SYMBOL_TABLE_ROW_TYPE_BOOL:
		return MCC_SEMANTIC_CHECK_EXPRESSION_TYPE_BOOL;
	case MCC_SYMBOL_TABLE_ROW_TYPE_STRING:
		return MCC_SEMANTIC_CHECK_EXPRESSION_TYPE_STRING;
	case MCC_SYMBOL_TABLE_ROW_TYPE_VOID:
		return MCC_SEMANTIC_CHECK_EXPRESSION_TYPE_VOID;
	case MCC_SYMBOL_TABLE_ROW_TYPE_PSEUDO:
		return MCC_SEMANTIC_CHECK_EXPRESSION_TYPE_UNKNOWN;
	}
	return MCC_SEMANTIC_CHECK_EXPRESSION_TYPE_UNKNOWN;
}

static enum mcc_semantic_check_expression_type convert_enum_ast_literal(enum mcc_ast_literal_type type)
{
	switch (type) {
	case MCC_AST_LITERAL_TYPE_INT:
		return MCC_SEMANTIC_CHECK_EXPRESSION_TYPE_INT;
	case MCC_AST_LITERAL_TYPE_FLOAT:
		return MCC_SEMANTIC_CHECK_EXPRESSION_TYPE_FLOAT;
	case MCC_AST_LITERAL_TYPE_BOOL:
		return MCC_SEMANTIC_CHECK_EXPRESSION_TYPE_BOOL;
	case MCC_AST_LITERAL_TYPE_STRING:
		return MCC_SEMANTIC_CHECK_EXPRESSION_TYPE_STRING;
	}

	return MCC_SEMANTIC_CHECK_EXPRESSION_TYPE_UNKNOWN;
}

// ------------------------------------------------------------- Functions: Running all semantic checks

// Write error message into existing mcc_semantic_check struct
static void write_error_message_to_all_checks(struct mcc_semantic_check_all_checks *checks, const char *string)
{

	int size = sizeof(char) * (strlen(string) + 2);
	char *buffer = malloc(size);
	if (buffer == NULL) {
		perror("write_error_message_to_check: malloc");
	}
	snprintf(buffer, size, "%s\n", string);
	checks->error_buffer = buffer;
}

static void handle_returned_null_pointer(struct mcc_semantic_check_all_checks *checks, const char *check_name)
{
    assert(checks);

    checks->status = MCC_SEMANTIC_CHECK_FAIL;
    if (checks->error_buffer == NULL) {
        int size = sizeof(char) * (strlen(check_name) + 60);
        char *msg = malloc(size);
        snprintf(msg, size, "mcc_semantic_check_run_%s returned NULL pointer", check_name);
        write_error_message_to_all_checks(checks, msg);
        free(msg);
    }
}

static void handle_failed_check(struct mcc_semantic_check_all_checks *checks, const char* error_msg)
{
    assert(checks);

    checks->status = MCC_SEMANTIC_CHECK_FAIL;
    if (checks->error_buffer == NULL) {
        write_error_message_to_all_checks(checks, error_msg);
    }
}

// Run all semantic checks. First encountered error is written into error buffer
struct mcc_semantic_check_all_checks *mcc_semantic_check_run_all(struct mcc_ast_program *ast,
                                                                 struct mcc_symbol_table *symbol_table)
{
	struct mcc_semantic_check_all_checks *checks = malloc(sizeof(*checks));
	if (checks == NULL) {
		return NULL;
	}
	checks->status = MCC_SEMANTIC_CHECK_OK;
	checks->error_buffer = NULL;

	// Initialize all tests to NULL. Some will stay NULL if a previous test has failed and the function returns
	// early
	checks->type_conversion = NULL;
	checks->function_arguments = NULL;
	checks->function_return_value = NULL;
	checks->nonvoid_check = NULL;
	checks->main_function = NULL;
	checks->unknown_function_call = NULL;
	checks->multiple_function_definitions = NULL;
	checks->multiple_variable_declarations = NULL;
	checks->use_undeclared_variable = NULL;
	checks->define_built_in = NULL;

	// Each execution path of non-void function returns a value
	checks->nonvoid_check = mcc_semantic_check_run_nonvoid_check(ast, symbol_table);
	if (checks->nonvoid_check == NULL) {
		handle_returned_null_pointer(checks, "nonvoid_check");
	} else if (checks->nonvoid_check->status != MCC_SEMANTIC_CHECK_OK) {
	    handle_failed_check(checks, checks->nonvoid_check->error_buffer);
	    return checks;
	}

	// Main function exists and has correct signature
	checks->main_function = mcc_semantic_check_run_main_function(ast, symbol_table);
	if (checks->main_function == NULL) {
        handle_returned_null_pointer(checks, "main_function");
	} else if (checks->main_function->status != MCC_SEMANTIC_CHECK_OK) {
	    handle_failed_check(checks, checks->main_function->error_buffer);
	    return checks;
	}

	// No Calls to unknown functions
	checks->unknown_function_call = mcc_semantic_check_run_unknown_function_call(ast, symbol_table);
	if (checks->unknown_function_call == NULL) {
        handle_returned_null_pointer(checks, "unknown_function");
	} else if (checks->unknown_function_call->status != MCC_SEMANTIC_CHECK_OK) {
	    handle_failed_check(checks, checks->unknown_function_call->error_buffer);
        return checks;
	}

	// No multiple definitions of the same function
	checks->multiple_function_definitions = mcc_semantic_check_run_multiple_function_definitions(ast, symbol_table);
	if (checks->multiple_function_definitions == NULL) {
        handle_returned_null_pointer(checks, "multiple_function_definitions");
	} else if (checks->multiple_function_definitions->status != MCC_SEMANTIC_CHECK_OK) {
	    handle_failed_check(checks, checks->multiple_function_definitions->error_buffer);
	    return checks;
	}

	// No multiple declarations of a variable in the same scope
	checks->multiple_variable_declarations =
	    mcc_semantic_check_run_multiple_variable_declarations(ast, symbol_table);
	if (checks->multiple_variable_declarations == NULL) {
        handle_returned_null_pointer(checks, "multiple_variable_declaration");
	} else if (checks->multiple_variable_declarations->status != MCC_SEMANTIC_CHECK_OK) {
	    handle_failed_check(checks, checks->multiple_variable_declarations->error_buffer);
        return checks;
	}

	// No use of undeclared variables
	checks->use_undeclared_variable = mcc_semantic_check_run_use_undeclared_variable(ast, symbol_table);
	if (checks->use_undeclared_variable == NULL) {
        handle_returned_null_pointer(checks, "use_undeclared_variable");
	} else if (checks->use_undeclared_variable->status != MCC_SEMANTIC_CHECK_OK) {
	    handle_failed_check(checks, checks->use_undeclared_variable->error_buffer);
	    return checks;
	}

	// No use of the names of the built_in functions in function definitions
	checks->define_built_in = mcc_semantic_check_run_define_built_in(ast, symbol_table);
	if (checks->define_built_in == NULL) {
        handle_returned_null_pointer(checks, "define_built_in");
	} else if (checks->define_built_in->status != MCC_SEMANTIC_CHECK_OK) {
	    handle_failed_check(checks, checks->define_built_in->error_buffer);
	    return checks;
	}

	// No type conversion
	checks->type_conversion = mcc_semantic_check_run_type_conversion(ast, symbol_table);
	if (checks->type_conversion == NULL) {
        handle_returned_null_pointer(checks, "type_conversion");
	} else if (checks->type_conversion->status != MCC_SEMANTIC_CHECK_OK) {
	    handle_failed_check(checks, checks->type_conversion->error_buffer);
	    return checks;
	}

	// No invalid function calls
	checks->function_arguments = mcc_semantic_check_run_function_arguments(ast, symbol_table);
	if (checks->function_arguments == NULL) {
        handle_returned_null_pointer(checks, "function_arguments");
	} else if (checks->function_arguments->status != MCC_SEMANTIC_CHECK_OK) {
	    handle_failed_check(checks, checks->function_arguments->error_buffer);
	    return checks;
	}

	// No functions with invalid returns
	checks->function_return_value = mcc_semantic_check_run_function_return_value(ast, symbol_table);
	if (checks->function_return_value == NULL) {
        handle_returned_null_pointer(checks, "function_return_value");
	} else if (checks->function_return_value->status != MCC_SEMANTIC_CHECK_OK) {
	    handle_failed_check(checks, checks->function_return_value->error_buffer);
	}

	return checks;
}

// ------------------------------------------------------------- Functions: Running single semantic checks

// Write error message into existing mcc_semantic_check struct
static void write_error_message_to_check(struct mcc_semantic_check *check, struct mcc_ast_node node, const char *string)
{
	int size = sizeof(char) * (strlen(string) + strlen(node.sloc.filename) + 50);
	char *buffer = malloc(size);
	if (buffer == NULL) {
		perror("write_error_message_to_check: malloc");
	}
	snprintf(buffer, size, "%s:%d:%d: %s\n", node.sloc.filename, node.sloc.start_line, node.sloc.start_col, string);
	check->error_buffer = buffer;
}

// generate error
static void generate_error(struct mcc_ast_node node, struct mcc_semantic_check *check, const char *string)
{
    if(check->error_buffer){
        return;
    }
    write_error_message_to_check(check, node, string);
    check->status = MCC_SEMANTIC_CHECK_FAIL;
}

// ------------------------------------------------------------- No Type conversions in expressions

// look up the type of the expression in the symbol table, only possible if expression is of type variable, array
// element or function call
static enum mcc_semantic_check_expression_type look_up_type_in_symbol_table(struct mcc_ast_expression *expression)
{
	assert((expression->type == MCC_AST_EXPRESSION_TYPE_VARIABLE) ||
	       (expression->type == MCC_AST_EXPRESSION_TYPE_ARRAY_ELEMENT) ||
	       (expression->type == MCC_AST_EXPRESSION_TYPE_FUNCTION_CALL));

	struct mcc_symbol_table_row *row;

	if (expression->type == MCC_AST_EXPRESSION_TYPE_VARIABLE) {
		row = mcc_symbol_table_check_upwards_for_declaration(expression->identifier->identifier_name,
		                                                     expression->variable_row);
	} else if (expression->type == MCC_AST_EXPRESSION_TYPE_ARRAY_ELEMENT) {
		row = mcc_symbol_table_check_upwards_for_declaration(expression->array_identifier->identifier_name,
		                                                     expression->array_row);
	} else if (expression->type == MCC_AST_EXPRESSION_TYPE_FUNCTION_CALL) {
		row = mcc_symbol_table_check_for_function_declaration(expression->function_identifier->identifier_name,
		                                                      expression->function_row);
	} else {
		return MCC_SEMANTIC_CHECK_EXPRESSION_TYPE_UNKNOWN;
	}

	if (row) {
		return convert_enum_symbol_table(row->row_type);
	} else {
		return MCC_SEMANTIC_CHECK_EXPRESSION_TYPE_UNKNOWN;
	}
}

// get type for binary operations. If SMALLER, GREATER, SMALLEREQ, GREATEREQ, EQUAL, NOTEQUAL expression becomes bool
static enum mcc_semantic_check_expression_type get_type_binary_op(struct mcc_ast_expression *expression)
{
	assert(expression);
	assert(expression->lhs);
	assert(expression->rhs);

	struct mcc_ast_expression *lhs = expression->lhs;
	struct mcc_ast_expression *rhs = expression->rhs;

	enum mcc_semantic_check_expression_type type_lhs = get_type(lhs);
	enum mcc_semantic_check_expression_type type_rhs = get_type(rhs);

	if (type_lhs != type_rhs && type_lhs != MCC_SEMANTIC_CHECK_EXPRESSION_TYPE_UNKNOWN) {
		return MCC_SEMANTIC_CHECK_EXPRESSION_TYPE_UNKNOWN;
	}

	switch (expression->op) {
	case MCC_AST_BINARY_OP_ADD:
		return type_lhs;
	case MCC_AST_BINARY_OP_SUB:
		return type_lhs;
	case MCC_AST_BINARY_OP_MUL:
		return type_lhs;
	case MCC_AST_BINARY_OP_DIV:
		return type_lhs;
	case MCC_AST_BINARY_OP_SMALLER:
		return MCC_SEMANTIC_CHECK_EXPRESSION_TYPE_BOOL;
	case MCC_AST_BINARY_OP_GREATER:
		return MCC_SEMANTIC_CHECK_EXPRESSION_TYPE_BOOL;
	case MCC_AST_BINARY_OP_SMALLEREQ:
		return MCC_SEMANTIC_CHECK_EXPRESSION_TYPE_BOOL;
	case MCC_AST_BINARY_OP_GREATEREQ:
		return MCC_SEMANTIC_CHECK_EXPRESSION_TYPE_BOOL;
	case MCC_AST_BINARY_OP_CONJ:
		return MCC_SEMANTIC_CHECK_EXPRESSION_TYPE_BOOL;
	case MCC_AST_BINARY_OP_DISJ:
		return MCC_SEMANTIC_CHECK_EXPRESSION_TYPE_BOOL;
	case MCC_AST_BINARY_OP_EQUAL:
		return MCC_SEMANTIC_CHECK_EXPRESSION_TYPE_BOOL;
	case MCC_AST_BINARY_OP_NOTEQUAL:
		return MCC_SEMANTIC_CHECK_EXPRESSION_TYPE_BOOL;
	default:
		return MCC_SEMANTIC_CHECK_EXPRESSION_TYPE_UNKNOWN;
	}

	return MCC_SEMANTIC_CHECK_EXPRESSION_TYPE_UNKNOWN;
}

// recursively generate type of expression, returns _TYPE_UNKNOWN if not of valid type, i.e. subexpressions are not
// compatible
static enum mcc_semantic_check_expression_type get_type(struct mcc_ast_expression *expression)
{
	assert(expression);

	switch (expression->type) {
	case MCC_AST_EXPRESSION_TYPE_LITERAL:
		return convert_enum_ast_literal(expression->literal->type);
	case MCC_AST_EXPRESSION_TYPE_BINARY_OP:
		return get_type_binary_op(expression);
	case MCC_AST_EXPRESSION_TYPE_PARENTH:
		return get_type(expression->expression);
	case MCC_AST_EXPRESSION_TYPE_UNARY_OP:
		return get_type(expression->child);
	case MCC_AST_EXPRESSION_TYPE_VARIABLE:
		return look_up_type_in_symbol_table(expression);
	case MCC_AST_EXPRESSION_TYPE_ARRAY_ELEMENT:
		return look_up_type_in_symbol_table(expression);
	case MCC_AST_EXPRESSION_TYPE_FUNCTION_CALL:
		return look_up_type_in_symbol_table(expression);
	}

	return MCC_SEMANTIC_CHECK_EXPRESSION_TYPE_UNKNOWN;
}

// check if given expression is of type bool
static bool is_bool(struct mcc_ast_expression *expression)
{
	assert(expression);

	enum mcc_semantic_check_expression_type type = get_type(expression);

	return (type == MCC_SEMANTIC_CHECK_EXPRESSION_TYPE_BOOL);
}

// check if given expression is variable of type string
static bool is_string(struct mcc_ast_expression *expression)
{
	assert(expression);

	if ((expression->type == MCC_AST_EXPRESSION_TYPE_VARIABLE) &&
	    (get_type(expression) == MCC_SEMANTIC_CHECK_EXPRESSION_TYPE_STRING)) {
		return true;
	} else {
		return false;
	}
}

// check if given expression is whole array (not array element)
static bool is_whole_array(struct mcc_ast_expression *expression)
{
	assert(expression);

	if ((expression->type == MCC_AST_EXPRESSION_TYPE_VARIABLE)) {
		char *name = expression->identifier->identifier_name;
		struct mcc_symbol_table_row *row = expression->variable_row;
		row = mcc_symbol_table_check_upwards_for_declaration(name, row);
		if (row->row_structure == MCC_SYMBOL_TABLE_ROW_STRUCTURE_ARRAY) {
			return true;
		}
	}

	return false;
}

// check if given expressions are of same type
static bool is_of_same_type(struct mcc_ast_expression *expression1, struct mcc_ast_expression *expression2)
{
	assert(expression1);
	assert(expression2);

	enum mcc_semantic_check_expression_type type_expr1 = get_type(expression1);
	enum mcc_semantic_check_expression_type type_expr2 = get_type(expression2);

	if ((type_expr1 == MCC_SEMANTIC_CHECK_EXPRESSION_TYPE_UNKNOWN) ||
	    (type_expr2 == MCC_SEMANTIC_CHECK_EXPRESSION_TYPE_UNKNOWN)) {
		return false;
	}
	return (type_expr1 == type_expr2);
}

// callback for checking implicit type conversions in binary operation expressions, since we visit post order the
// innermost expression is visited first
static void cb_type_conversion_expression_binary_op(struct mcc_ast_expression *expression, void *data)
{
	assert(expression);
	assert(data);

	struct mcc_semantic_check *check = data;

	// Early abort if already failed
	if (check->status == MCC_SEMANTIC_CHECK_FAIL) {
		return;
	}
	struct mcc_ast_expression *lhs = expression->lhs;
	struct mcc_ast_expression *rhs = expression->rhs;
	// operations on strings and whole arrays are not supported
	if (is_string(lhs) || is_string(rhs) || is_whole_array(lhs) || is_whole_array(rhs)) {
		generate_error(expression->node, check, "invalid operands to binary operation.");
		return;
	}

	bool is_permitted_op = true;

	switch (expression->op) {
	case MCC_AST_BINARY_OP_ADD:
		is_permitted_op = is_of_same_type(lhs, rhs) && !is_bool(lhs) && !is_bool(rhs);
		break;
	case MCC_AST_BINARY_OP_SUB:
		is_permitted_op = is_of_same_type(lhs, rhs) && !is_bool(lhs) && !is_bool(rhs);
		break;
	case MCC_AST_BINARY_OP_MUL:
		is_permitted_op = is_of_same_type(lhs, rhs) && !is_bool(lhs) && !is_bool(rhs);
		break;
	case MCC_AST_BINARY_OP_DIV:
		is_permitted_op = is_of_same_type(lhs, rhs) && !is_bool(lhs) && !is_bool(rhs);
		break;
	case MCC_AST_BINARY_OP_SMALLER:
		is_permitted_op = is_of_same_type(lhs, rhs) && !is_bool(lhs) && !is_bool(rhs);
		break;
	case MCC_AST_BINARY_OP_GREATER:
		is_permitted_op = is_of_same_type(lhs, rhs) && !is_bool(lhs) && !is_bool(rhs);
		break;
	case MCC_AST_BINARY_OP_SMALLEREQ:
		is_permitted_op = is_of_same_type(lhs, rhs) && !is_bool(lhs) && !is_bool(rhs);
		break;
	case MCC_AST_BINARY_OP_GREATEREQ:
		is_permitted_op = is_of_same_type(lhs, rhs) && !is_bool(lhs) && !is_bool(rhs);
		break;
	case MCC_AST_BINARY_OP_CONJ:
		is_permitted_op = is_bool(lhs) && is_bool(rhs); // Conjunction can only be used with bool
		break;
	case MCC_AST_BINARY_OP_DISJ:
		is_permitted_op = is_bool(lhs) && is_bool(rhs); // Disjunction can only be used with bool
		break;
	case MCC_AST_BINARY_OP_EQUAL:
		is_permitted_op = is_of_same_type(lhs, rhs);
		break;
	case MCC_AST_BINARY_OP_NOTEQUAL:
		is_permitted_op = is_of_same_type(lhs, rhs);
		break;
	}

	// since we visit post order the innermost expression is visited first
	if ((!is_permitted_op) &&
	    ((expression->op == MCC_AST_BINARY_OP_CONJ) || (expression->op == MCC_AST_BINARY_OP_DISJ))) {
		generate_error(expression->node, check, "using non-boolean variable or expression with logical connective.");
		return;
	}
	if (!is_permitted_op) {
		generate_error(expression->node, check, "type conversion not possible.");
	}
}

// callback for checking implicit type conversions in unary operation expressions, since we visit post order the
// innermost expression is visited first
static void cb_type_conversion_expression_unary_op(struct mcc_ast_expression *expression, void *data)
{
	assert(expression);
	assert(data);

	struct mcc_semantic_check *check = data;
	// Early abort if already failed
	if (check->status == MCC_SEMANTIC_CHECK_FAIL) {
		return;
	}
	struct mcc_ast_expression *child = expression->child;
	bool child_is_bool = is_bool(child);

	if (is_string(child) || is_whole_array(child)) {
		generate_error(expression->node, check, "invalid operand to unary operation.");
		return;
	}

	bool is_permitted_op = true;

	switch (expression->u_op) {
	case MCC_AST_UNARY_OP_NEGATIV:
		is_permitted_op = !child_is_bool;
		break;
	case MCC_AST_UNARY_OP_NOT:
		is_permitted_op = child_is_bool;
		break;
	}

	// since we visit post order the innermost expression is visited first
	if ((!is_permitted_op) && (expression->u_op == MCC_AST_UNARY_OP_NOT)) {
		generate_error(expression->node, check, "using non-boolean variable or expression with logical connective.");
		return;
	}
	if (!is_permitted_op) {
		generate_error(expression->node, check, "type conversion not possible.");
	}
}

// callback for checking if the condition in an if statement is of type bool
static void cb_type_conversion_statement_if_stmt(struct mcc_ast_statement *statement, void *data)
{
	assert(statement);
	assert(data);

	struct mcc_semantic_check *check = data;
	// Early abort if already failed
	if (check->status == MCC_SEMANTIC_CHECK_FAIL) {
		return;
	}

	if (get_type(statement->if_condition) != MCC_SEMANTIC_CHECK_EXPRESSION_TYPE_BOOL) {
		generate_error(statement->if_condition->node, check,
		        "condition of if statement expected to be of type 'BOOL'.");
	}
}

// callback for checking if the condition in an if_else statement is of type bool
static void cb_type_conversion_statement_if_else_stmt(struct mcc_ast_statement *statement, void *data)
{
	assert(statement);
	assert(data);

	struct mcc_semantic_check *check = data;
	// Early abort if already failed
	if (check->status == MCC_SEMANTIC_CHECK_FAIL) {
		return;
	}

	if (get_type(statement->if_else_condition) != MCC_SEMANTIC_CHECK_EXPRESSION_TYPE_BOOL) {
		generate_error(statement->if_else_condition->node, check,
		        "condition of if statement expected to be of type 'BOOL'.");
	}
}

// callback for checking if the condition in a while loop is of type bool
static void cb_type_conversion_statement_while(struct mcc_ast_statement *statement, void *data)
{
	assert(statement);
	assert(data);

	struct mcc_semantic_check *check = data;
	// Early abort if already failed
	if (check->status == MCC_SEMANTIC_CHECK_FAIL) {
		return;
	}

	if (get_type(statement->while_condition) != MCC_SEMANTIC_CHECK_EXPRESSION_TYPE_BOOL) {
		generate_error(statement->while_condition->node, check,
		        "condition of while loop expected to be of type 'BOOL'.");
	}
}

// generate error message for type conversion in assignment
static void generate_error_msg_type_conversion_assignment(struct mcc_ast_assignment *assignment,
                                                          struct mcc_semantic_check *check)
{
	assert(assignment);
	assert(check);

	if (check->error_buffer) {
		return;
	}

	char *name;
	char *var_or_array;

	switch (assignment->assignment_type) {
	case MCC_AST_ASSIGNMENT_TYPE_VARIABLE:
		name = assignment->variable_identifier->identifier_name;
		var_or_array = "variable";
		break;
	case MCC_AST_ASSIGNMENT_TYPE_ARRAY:
		name = assignment->array_identifier->identifier_name;
		var_or_array = "array";
		break;
	}

	int size = 50 + strlen(name);
	char *error_msg = (char *)malloc(sizeof(char) * size);
	snprintf(error_msg, size, "implicit type conversion of %s '%s'.", var_or_array, name);
	write_error_message_to_check(check, assignment->node, error_msg);
	check->status = MCC_SEMANTIC_CHECK_FAIL;
	free(error_msg);
}

// check an assignment for type conversion
static void check_type_conversion_assignment(struct mcc_ast_assignment *assignment, struct mcc_semantic_check *check)
{
	assert(assignment);
	assert(check);

	// Early abort if already failed
	if (check->status == MCC_SEMANTIC_CHECK_FAIL) {
		return;
	}

	struct mcc_symbol_table_row *row = assignment->row;
	enum mcc_semantic_check_expression_type value_type = MCC_SEMANTIC_CHECK_EXPRESSION_TYPE_UNKNOWN;

	switch (assignment->assignment_type) {
	case MCC_AST_ASSIGNMENT_TYPE_VARIABLE:
		row = mcc_symbol_table_check_upwards_for_declaration(assignment->variable_identifier->identifier_name,
		                                                     row);
		value_type = get_type(assignment->variable_assigned_value);
		break;
	case MCC_AST_ASSIGNMENT_TYPE_ARRAY:
		row =
		    mcc_symbol_table_check_upwards_for_declaration(assignment->array_identifier->identifier_name, row);
		value_type = get_type(assignment->array_assigned_value);
		break;
	}

	bool is_permitted = false;
	if (row && (value_type != MCC_SEMANTIC_CHECK_EXPRESSION_TYPE_UNKNOWN)) {
		is_permitted = (convert_enum_symbol_table(row->row_type) == value_type);
	}

	if (!is_permitted) {
		generate_error_msg_type_conversion_assignment(assignment, check);
	}
}

// generate error msg if index of array in expression is non int
static void generate_error_msg_array_index_non_int(struct mcc_ast_expression *expression,
                                                   enum mcc_semantic_check_expression_type type,
                                                   struct mcc_semantic_check *check)
{
	if (check->error_buffer) {
		return;
	}
	char *str = semantic_check_expression_type_to_string(type);
	int size = 56 + strlen(str);
	char *error_msg = (char *)malloc(sizeof(char) * size);
	snprintf(error_msg, size, "array index of type '%s', expected to be 'INT'.", str);
	write_error_message_to_check(check, expression->node, error_msg);
	check->status = MCC_SEMANTIC_CHECK_FAIL;
	free(error_msg);
}

// check the index of an array element during assignment to be of type INT
static void check_assignment_array_index(struct mcc_ast_assignment *assignment, struct mcc_semantic_check *check)
{
	assert(assignment);
	assert(check);

	// Early abort if already failed
	if (check->status == MCC_SEMANTIC_CHECK_FAIL) {
		return;
	}

	struct mcc_ast_expression *index = assignment->array_index;
	enum mcc_semantic_check_expression_type type = get_type(index);

	if (type != MCC_SEMANTIC_CHECK_EXPRESSION_TYPE_INT) {
		generate_error_msg_array_index_non_int(index, type, check);
	}
}

// generate error msg if a variable of array type is assigned
static void generate_error_msg_variable_assignment_of_array(struct mcc_ast_assignment *assignment,
                                                            struct mcc_semantic_check *check)
{
	if (check->error_buffer) {
		return;
	}
	char *name = assignment->variable_identifier->identifier_name;
	int size = 60 + strlen(name);
	char *error_msg = (char *)malloc(sizeof(char) * size);
	snprintf(error_msg, size, "assignment to variable '%s' of array type not possible.", name);
	write_error_message_to_check(check, assignment->node, error_msg);
	check->status = MCC_SEMANTIC_CHECK_FAIL;
	free(error_msg);
}

// check that the assignment of a variable is not of array type
static void check_assignment_var_not_equal_array(struct mcc_ast_assignment *assignment,
                                                 struct mcc_semantic_check *check)
{
	assert(assignment);
	assert(check);
	assert(assignment->assignment_type == MCC_AST_ASSIGNMENT_TYPE_VARIABLE);

	// Early abort if already failed
	if (check->status == MCC_SEMANTIC_CHECK_FAIL) {
		return;
	}

	struct mcc_symbol_table_row *row = assignment->row;
	char *name = assignment->variable_identifier->identifier_name;
	row = mcc_symbol_table_check_upwards_for_declaration(name, row);

	if (row && (row->row_structure == MCC_SYMBOL_TABLE_ROW_STRUCTURE_ARRAY)) {
		generate_error_msg_variable_assignment_of_array(assignment, check);
	}
}

// callback for checking type conversion in an assignment and ensure, that index is of type INT
static void cb_type_conversion_assignment(struct mcc_ast_statement *statement, void *data)
{
	assert(statement);
	assert(data);

	struct mcc_semantic_check *check = data;
	// Early abort if already failed
	if (check->status == MCC_SEMANTIC_CHECK_FAIL) {
		return;
	}
	struct mcc_ast_assignment *assignment = statement->assignment;

	switch (assignment->assignment_type) {
	case MCC_AST_ASSIGNMENT_TYPE_VARIABLE:
		check_assignment_var_not_equal_array(assignment, check);
		break;
	case MCC_AST_ASSIGNMENT_TYPE_ARRAY:
		check_assignment_array_index(assignment, check);
		break;
	}
	check_type_conversion_assignment(assignment, check);
}

// callback for an array element as an expression of type int
static void cb_type_conversion_expression_array_element(struct mcc_ast_expression *expression, void *data)
{
	assert(expression);
	assert(data);

	struct mcc_semantic_check *check = data;
	// Early abort if already failed
	if (check->status == MCC_SEMANTIC_CHECK_FAIL) {
		return;
	}

	struct mcc_ast_expression *index = expression->index;
	enum mcc_semantic_check_expression_type type = get_type(index);

	if (type != MCC_SEMANTIC_CHECK_EXPRESSION_TYPE_INT) {
		generate_error_msg_array_index_non_int(expression, type, check);
	}
}

// Setup an AST Visitor for checking types within expressions and if conditions are of type bool
static struct mcc_ast_visitor type_conversion_visitor(struct mcc_semantic_check *check)
{

	return (struct mcc_ast_visitor){
	    .traversal = MCC_AST_VISIT_DEPTH_FIRST,
	    .order = MCC_AST_VISIT_POST_ORDER,

	    .userdata = check,

	    .expression_binary_op = cb_type_conversion_expression_binary_op,
	    .expression_unary_op = cb_type_conversion_expression_unary_op,
	    .expression_array_element = cb_type_conversion_expression_array_element,
	    .statement_if_stmt = cb_type_conversion_statement_if_stmt,
	    .statement_if_else_stmt = cb_type_conversion_statement_if_else_stmt,
	    .statement_while = cb_type_conversion_statement_while,
	    .statement_assignment = cb_type_conversion_assignment,
	};
}

// check for type conversion in expressions and if expressions used as conditions are of type bool
struct mcc_semantic_check *mcc_semantic_check_run_type_conversion(struct mcc_ast_program *ast,
                                                                  struct mcc_symbol_table *symbol_table)
{
	UNUSED(symbol_table);

	struct mcc_semantic_check *check = malloc(sizeof(*check));
	if (!check) {
		return NULL;
	}

	check->status = MCC_SEMANTIC_CHECK_OK;
	check->type = MCC_SEMANTIC_CHECK_TYPE_CONVERSION;
	check->error_buffer = NULL;

	struct mcc_ast_visitor visitor = type_conversion_visitor(check);
	mcc_ast_visit(ast, &visitor);
	return check;
}

// ------------------------------------------------------------- No invalid function calls

enum function_call_error {
	EX_ARRAY_BUT_VAR_GIVEN,
	EX_VAR_BUT_ARRAY_GIVEN,
	ARRAY_SIZES_NOT_MATCHING,
	ARRAY_TYPES_NOT_MATCHING,
	VAR_TYPES_NOT_MATCHING,
	NO_ARGUMENTS_NEEDED,
	TOO_MANY_ARGUMENTS,
	TOO_FEW_ARGUMENTS,
	UNKNOWN_FUNCTION,
	NO_ERROR,
};

struct function_arguments_userdata {
	struct mcc_semantic_check *check;
	struct mcc_ast_program *program;
};

static char *semantic_check_expression_type_to_string(enum mcc_semantic_check_expression_type type)
{

	switch (type) {
	case MCC_SEMANTIC_CHECK_EXPRESSION_TYPE_INT:
		return "INT";
	case MCC_SEMANTIC_CHECK_EXPRESSION_TYPE_FLOAT:
		return "FLOAT";
	case MCC_SEMANTIC_CHECK_EXPRESSION_TYPE_BOOL:
		return "BOOL";
	case MCC_SEMANTIC_CHECK_EXPRESSION_TYPE_STRING:
		return "STRING";
    case MCC_SEMANTIC_CHECK_EXPRESSION_TYPE_VOID:
        return "VOID";
	default:
		return "UNKNOWN";
	}
}

static char *ast_type_to_string(enum mcc_ast_types type)
{
	switch (type) {
	case INT:
		return "INT";
	case FLOAT:
		return "FLOAT";
	case BOOL:
		return "BOOL";
	case STRING:
		return "STRING";
	default:
		return NULL;
	}
}

static char *get_given_var_type(struct mcc_ast_arguments *args)
{
	enum mcc_semantic_check_expression_type type = get_type(args->expression);
	return semantic_check_expression_type_to_string(type);
}

static char *get_expected_var_type(struct mcc_ast_parameters *pars)
{
	return ast_type_to_string(pars->declaration->variable_type->type_value);
}

static char *get_given_array_type(struct mcc_ast_arguments *args)
{
	enum mcc_semantic_check_expression_type type = get_type(args->expression);
	return semantic_check_expression_type_to_string(type);
}

static char *get_expected_array_type(struct mcc_ast_parameters *pars)
{
	return ast_type_to_string(pars->declaration->array_type->type_value);
}

// String is allocated on the heap
static char *generate_error_string_function_arguments(struct mcc_ast_parameters *pars,
                                                      struct mcc_ast_arguments *args,
                                                      enum function_call_error error_status)
{
	char *string;
	switch (error_status) {
	case EX_ARRAY_BUT_VAR_GIVEN:
		string = "Expected array, but variable was given.";
		break;
	case EX_VAR_BUT_ARRAY_GIVEN:
		string = "Expected variable, but array was given.";
		break;
	case ARRAY_SIZES_NOT_MATCHING:
		string = "Expected array of size";
		break;
	case ARRAY_TYPES_NOT_MATCHING:
		string = "Expected array of type , but  was given.";
		break;
	case VAR_TYPES_NOT_MATCHING:
		string = "Expected variable of type , but  was given.";
		break;
	case NO_ARGUMENTS_NEEDED:
		string = "Too many arguments.";
		break;
	case TOO_FEW_ARGUMENTS:
		string = "Too few arguments provided.";
		break;
	case UNKNOWN_FUNCTION:
		string = "Function unknown, no definition found.";
		break;
	default:
		string = "Unknown error.";
		break;
	}

	int size = sizeof(char) * strlen(string) + 14;
	char *buffer = malloc(size);
	if (!buffer) {
		return NULL;
	}

	switch (error_status) {
	case VAR_TYPES_NOT_MATCHING:
		if (0 > snprintf(buffer, size, "Expected variable of type %s, but %s was given",
		                 get_expected_var_type(pars), get_given_var_type(args))) {
			return NULL;
		} else {
			return buffer;
		}
	case ARRAY_TYPES_NOT_MATCHING:
		if (0 > snprintf(buffer, size, "Expected array of type %s, but %s was given",
		                 get_expected_array_type(pars), get_given_array_type(args))) {
			return NULL;
		} else {
			return buffer;
		}
	default:
		if (0 > snprintf(buffer, size, "%s", string)) {
			return NULL;
		} else {
			return buffer;
		}
	}
}

// Generate error message for invalid function call
static void generate_error_msg_function_arguments(struct mcc_semantic_check *check,
                                                  struct mcc_ast_parameters *pars,
                                                  struct mcc_ast_arguments *args,
                                                  struct mcc_ast_expression *expression,
                                                  enum function_call_error error_status)
{
	char *string = generate_error_string_function_arguments(pars, args, error_status);
	if (!string) {
		write_error_message_to_check(check, args->node,
		                             "generate_error_string_function_arguments: malloc failed.");
		check->status = MCC_SEMANTIC_CHECK_FAIL;
		return;
	}
	int size = sizeof(char) * (strlen(string) + strlen(expression->function_identifier->identifier_name) + 30);
	char *buffer = malloc(size);
	if (!buffer) {
		write_error_message_to_check(check, args->node,
		                             "generate_error_msg_function_arguments: malloc failed.");
		check->status = MCC_SEMANTIC_CHECK_FAIL;
		free(string);
		return;
	}
	if (0 > snprintf(buffer, size, "%s, Invalid function call, %s\n",
	                 expression->function_identifier->identifier_name, string)) {
		write_error_message_to_check(check, args->node,
		                             "generate_error_msg_function_arguments: snprintf failed.");
		check->status = MCC_SEMANTIC_CHECK_FAIL;
		free(string);
		free(buffer);
		return;
	} else {
		write_error_message_to_check(check, args->node, buffer);
		free(buffer);
		free(string);
		check->status = MCC_SEMANTIC_CHECK_FAIL;
		return;
	}
}

// Returns -1 if not array. Array must be declared correctly
static int get_array_size_from_par(struct mcc_ast_parameters *par)
{
	if (par->declaration->declaration_type == MCC_AST_DECLARATION_TYPE_VARIABLE) {
		return -1;
	} else {
		return par->declaration->array_size->i_value;
	}
}

// Array must be given as expression (variable)
static int get_array_size_from_arg(struct mcc_ast_arguments *arg)
{
	char *array_name = arg->expression->identifier->identifier_name;
	struct mcc_symbol_table_row *array_declaration_row =
	    mcc_symbol_table_check_upwards_for_declaration(array_name, arg->expression->variable_row);
	return array_declaration_row->array_size;
}

static bool parameter_is_array(struct mcc_ast_parameters *par)
{
	return (par->declaration->declaration_type == MCC_AST_DECLARATION_TYPE_ARRAY);
}

static bool arguments_is_array(struct mcc_ast_arguments *arg)
{

	struct mcc_ast_expression *arg_exp = arg->expression;
	// Remove paranthesis
	if (arg_exp->type == MCC_AST_EXPRESSION_TYPE_PARENTH) {
		while (arg_exp->type == MCC_AST_EXPRESSION_TYPE_PARENTH) {
			arg_exp = arg_exp->expression;
		}
	}
	if (arg_exp->type != MCC_AST_EXPRESSION_TYPE_VARIABLE) {
		return false;
	}
	char *name = arg_exp->identifier->identifier_name;
	struct mcc_symbol_table_row *var_declaration_row =
	    mcc_symbol_table_check_upwards_for_declaration(name, arg->expression->variable_row);
	return (var_declaration_row->row_structure == MCC_SYMBOL_TABLE_ROW_STRUCTURE_ARRAY);
}

static bool par_and_arg_array_sizes_match(struct mcc_ast_arguments *arg, struct mcc_ast_parameters *par)
{
	return (get_array_size_from_arg(arg) == get_array_size_from_par(par));
}

static bool ast_and_semantic_check_type_are_equal(enum mcc_semantic_check_expression_type extype,
                                                  enum mcc_ast_types ast_type)
{
	switch (ast_type) {
	case INT:
		return (extype == MCC_SEMANTIC_CHECK_EXPRESSION_TYPE_INT);
	case FLOAT:
		return (extype == MCC_SEMANTIC_CHECK_EXPRESSION_TYPE_FLOAT);
	case BOOL:
		return (extype == MCC_SEMANTIC_CHECK_EXPRESSION_TYPE_BOOL);
	case STRING:
		return (extype == MCC_SEMANTIC_CHECK_EXPRESSION_TYPE_STRING);
	default:
		return false;
	}
}

static bool par_and_arg_array_types_match(struct mcc_ast_arguments *arg, struct mcc_ast_parameters *par)
{
	enum mcc_ast_types ast_type = par->declaration->array_type->type_value;
	enum mcc_semantic_check_expression_type extype = get_type(arg->expression);
	return (ast_and_semantic_check_type_are_equal(extype, ast_type));
}

static bool par_and_arg_var_types_match(struct mcc_ast_arguments *arg, struct mcc_ast_parameters *par)
{
	enum mcc_ast_types ast_type = par->declaration->variable_type->type_value;
	enum mcc_semantic_check_expression_type extype = get_type(arg->expression);
	return (ast_and_semantic_check_type_are_equal(extype, ast_type));
}

static enum function_call_error par_and_arg_equal_type(struct mcc_ast_parameters *par, struct mcc_ast_arguments *arg)
{

	if (parameter_is_array(par)) {
		// Function expects array
		if (!arguments_is_array(arg)) {
			// No array was given
			return EX_ARRAY_BUT_VAR_GIVEN;
		} else {
			// Check if array sizes match
			if (!par_and_arg_array_sizes_match(arg, par)) {
				return ARRAY_SIZES_NOT_MATCHING;
			}
			// Check if array types match
			if (!par_and_arg_array_types_match(arg, par)) {
				return ARRAY_TYPES_NOT_MATCHING;
			}
		}
	} else {
		// Function expects non-array
		if (arguments_is_array(arg)) {
			// Array was given
			return EX_VAR_BUT_ARRAY_GIVEN;
		} else {
			// Check if variable types compatible
			if (!par_and_arg_var_types_match(arg, par)) {
				return VAR_TYPES_NOT_MATCHING;
			}
		}
	}
	return NO_ERROR;
}

struct mcc_ast_parameters *get_pars_print()
{
	char *name = strdup("a");
	struct mcc_ast_declaration *dec = mcc_ast_new_variable_declaration(STRING, name);
	return mcc_ast_new_parameters(false, dec, NULL);
}

struct mcc_ast_parameters *get_pars_print_nl()
{
	return mcc_ast_new_parameters(true, NULL, NULL);
}

struct mcc_ast_parameters *get_pars_print_int()
{
	char *name = strdup("a");
	struct mcc_ast_declaration *dec = mcc_ast_new_variable_declaration(INT, name);
	return mcc_ast_new_parameters(false, dec, NULL);
}

struct mcc_ast_parameters *get_pars_print_float()
{
	char *name = strdup("a");
	struct mcc_ast_declaration *dec = mcc_ast_new_variable_declaration(FLOAT, name);
	return mcc_ast_new_parameters(false, dec, NULL);
}

struct mcc_ast_parameters *get_pars_read_int()
{
	return mcc_ast_new_parameters(true, NULL, NULL);
}

struct mcc_ast_parameters *get_pars_read_float()
{
	return mcc_ast_new_parameters(true, NULL, NULL);
}

struct mcc_ast_parameters *get_built_in_pars(struct mcc_ast_expression *expr)
{
	char *func_name = expr->function_identifier->identifier_name;
	if (strcmp(func_name, "print") == 0) {
		return get_pars_print();
	} else if (strcmp(func_name, "print_nl") == 0) {
		return get_pars_print_nl();
	} else if (strcmp(func_name, "print_int") == 0) {
		return get_pars_print_int();
	} else if (strcmp(func_name, "print_float") == 0) {
		return get_pars_print_float();
	} else if (strcmp(func_name, "read_int") == 0) {
		return get_pars_read_int();
	} else if (strcmp(func_name, "read_float") == 0) {
		return get_pars_read_float();
	} else {
		return NULL;
	}
}

// callback for checking correctness of function calls
static void cb_function_arguments_expression_function_call(struct mcc_ast_expression *expression, void *userdata)
{
	assert(expression);
	assert(userdata);
	struct function_arguments_userdata *data = userdata;
	struct mcc_ast_program *ast = data->program;
	struct mcc_semantic_check *check = data->check;

	enum function_call_error status = NO_ERROR;
	bool pars_from_heap = false;

	// Early abort if check already failed
	if (check->status == MCC_SEMANTIC_CHECK_FAIL) {
		return;
	}

	// Get the parameters of the function from the AST
	struct mcc_ast_parameters *pars = NULL;
	struct mcc_ast_parameters *pars_head = NULL;

	// Get the used arguments from the AST:
	struct mcc_ast_arguments *args = expression->arguments;

	do {
		if (strcmp(ast->function->identifier->identifier_name,
		           expression->function_identifier->identifier_name) == 0) {
			// Found the corresponding function declaration, now memorize the parameters
			pars = ast->function->parameters;
		}
		ast = ast->next_function;
	} while (ast);

	if (!pars) {
		// No parameters found. Must be built in or unkown function
		// Get the required parameters for the built in that was called
		pars = get_built_in_pars(expression);
		pars_head = pars;
		pars_from_heap = true;

		// No paramters found for the given call
		if (!pars) {
			status = UNKNOWN_FUNCTION;
			generate_error_msg_function_arguments(check, pars, args, expression, status);
			return;
		}
	}

	if (pars->is_empty) {
		// Expression is set to NULL during parsing, if no args are given
		if (expression->arguments->expression) {
			status = TOO_MANY_ARGUMENTS;
			generate_error_msg_function_arguments(check, pars, args, expression, status);
			if (pars_from_heap) {
				mcc_ast_delete(pars_head);
			}
			return;
		}
		// No arguments needed and none given: return
		if (pars_from_heap) {
			mcc_ast_delete(pars_head);
		}
		return;
	}

	do {
		// Too little arguments (if no arguments are given, args exists, but args->expr is set to NULL)
		if (!args->expression) {
			status = TOO_FEW_ARGUMENTS;
			generate_error_msg_function_arguments(check, pars, args, expression, status);
			if (pars_from_heap) {
				mcc_ast_delete(pars_head);
			}
			return;
		}

		// Type Error
		status = par_and_arg_equal_type(pars, args);
		if (status != NO_ERROR) {
			generate_error_msg_function_arguments(check, pars, args, expression, status);
			if (pars_from_heap) {
				mcc_ast_delete(pars_head);
			}
			return;
		}

		pars = pars->next_parameters;
		args = args->next_arguments;

	} while (pars);

	if (args) {
		// Too many arguments
		status = TOO_MANY_ARGUMENTS;
		generate_error_msg_function_arguments(check, pars, args, expression, status);
		if (pars_from_heap) {
			mcc_ast_delete(pars_head);
		}
		return;
	}
	if (pars_from_heap) {
		mcc_ast_delete(pars_head);
	}
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

struct mcc_semantic_check *mcc_semantic_check_run_function_arguments(struct mcc_ast_program *ast,
                                                                     struct mcc_symbol_table *symbol_table)
{
	UNUSED(symbol_table);
	struct mcc_semantic_check *check = malloc(sizeof(*check));
	if (!check) {
		return NULL;
	}

	check->status = MCC_SEMANTIC_CHECK_OK;
	check->type = MCC_SEMANTIC_CHECK_FUNCTION_ARGUMENTS;
	check->error_buffer = NULL;

	// Set up userdata
	struct function_arguments_userdata *userdata = malloc(sizeof(*userdata));
	if (!userdata) {
		check->status = MCC_SEMANTIC_CHECK_FAIL;
		write_error_message_to_check(check, ast->node, "function_arguments: malloc failed.");
		return check;
	}
	userdata->check = check;
	userdata->program = ast;

	struct mcc_ast_visitor visitor = function_arguments_visitor(userdata);
	mcc_ast_visit(ast, &visitor);
	free(userdata);
	return check;
}

// -------------------------------------------------------------- Function doesn't return wrong type

struct function_return_value_userdata {
	struct mcc_semantic_check *check;
	enum mcc_semantic_check_expression_type declared_function_type;
};

static void generate_error_msg_function_return_value(struct mcc_ast_statement *statement,
                                                     struct mcc_semantic_check *check,
                                                     enum mcc_semantic_check_expression_type actual_return_type,
                                                     enum mcc_semantic_check_expression_type declared_function_type)
{

	char *str_act_type = semantic_check_expression_type_to_string(actual_return_type);
	char *str_exp_type = semantic_check_expression_type_to_string(declared_function_type);

	if (check->error_buffer) {
		return;
	}

	int size = sizeof(char) * (strlen(str_act_type) + strlen(str_exp_type) + 60);
	char *buffer = malloc(size);

	if (!buffer) {
		write_error_message_to_check(check, statement->node,
		                             "generate_error_msg_function_return_value: malloc failed.");
		check->status = MCC_SEMANTIC_CHECK_FAIL;
		return;
	}
	if (0 >
	    snprintf(buffer, size, "Invalid return type, expected type %s but was %s\n", str_exp_type, str_act_type)) {
		write_error_message_to_check(check, statement->node,
		                             "generate_error_msg_function_return_value_conflicting_type: "
		                             "snprintf failed.");
		check->status = MCC_SEMANTIC_CHECK_FAIL;
		return;
	} else {
		write_error_message_to_check(check, statement->node, buffer);
		free(buffer);
		check->status = MCC_SEMANTIC_CHECK_FAIL;
		return;
	}
}

// generate error msg if return value is a variable of array typ
static void generate_error_msg_return_value_array(struct mcc_ast_expression *expression,
                                                  const char *name,
                                                  struct mcc_semantic_check *check)
{
	if (check->error_buffer) {
		return;
	}
	int size = 65 + strlen(name);
	char *error_msg = (char *)malloc(sizeof(char) * size);
	snprintf(error_msg, size, "unexpected array-type of variable '%s' in return statement.", name);
	write_error_message_to_check(check, expression->node, error_msg);
	check->status = MCC_SEMANTIC_CHECK_FAIL;
	free(error_msg);
}

// Callback for visitor
static void cb_function_return_value_statement_return(struct mcc_ast_statement *statement, void *data)
{
    assert(statement);
    assert(data);

	struct function_return_value_userdata *userdata = data;

	// check if return value is a variable
	if (statement->return_value && (statement->return_value->type == MCC_AST_EXPRESSION_TYPE_VARIABLE)) {

		struct mcc_symbol_table_row *row = statement->return_value->variable_row;
		char *name = statement->return_value->identifier->identifier_name;
		row = mcc_symbol_table_check_upwards_for_declaration(name, row);

		// error if row indicates array type.
		if (row && (row->row_structure == MCC_SYMBOL_TABLE_ROW_STRUCTURE_ARRAY)) {
			generate_error_msg_return_value_array(statement->return_value, name, userdata->check);
			return;
		}
	}

    enum mcc_semantic_check_expression_type act_type = MCC_SEMANTIC_CHECK_EXPRESSION_TYPE_UNKNOWN;
	if(statement->return_value){
        act_type = get_type(statement->return_value);
    } else {
        act_type = MCC_SEMANTIC_CHECK_EXPRESSION_TYPE_VOID;
	}

	if (act_type != userdata->declared_function_type) {
		generate_error_msg_function_return_value(statement, userdata->check, act_type,
		                                         userdata->declared_function_type);
	}
}

// Set up visitor
static struct mcc_ast_visitor function_return_value_visitor(struct function_return_value_userdata *data)
{
	return (struct mcc_ast_visitor){
	    .traversal = MCC_AST_VISIT_DEPTH_FIRST,
	    .order = MCC_AST_VISIT_POST_ORDER,

	    .userdata = data,

	    .statement_return = cb_function_return_value_statement_return,
	};
}

// Check a single function for correct return types with visitor
static void check_function_return_value_ok(struct mcc_ast_program *ast, struct mcc_semantic_check *check)
{

	struct mcc_ast_function_definition *function = ast->function;
	// Set up custom userdata struct
	struct function_return_value_userdata *userdata = malloc(sizeof(*userdata));
	if (!userdata) {
		write_error_message_to_check(check, ast->node, "malloc failed");
		check->status = MCC_SEMANTIC_CHECK_FAIL;
		return;
	}
	userdata->check = check;
	switch (function->type) {
	case MCC_AST_FUNCTION_TYPE_INT:
		userdata->declared_function_type = MCC_SEMANTIC_CHECK_EXPRESSION_TYPE_INT;
		break;
	case MCC_AST_FUNCTION_TYPE_FLOAT:
		userdata->declared_function_type = MCC_SEMANTIC_CHECK_EXPRESSION_TYPE_FLOAT;
		break;
	case MCC_AST_FUNCTION_TYPE_STRING:
		userdata->declared_function_type = MCC_SEMANTIC_CHECK_EXPRESSION_TYPE_STRING;
		break;
	case MCC_AST_FUNCTION_TYPE_BOOL:
		userdata->declared_function_type = MCC_SEMANTIC_CHECK_EXPRESSION_TYPE_BOOL;
		break;
	case MCC_AST_FUNCTION_TYPE_VOID:
		userdata->declared_function_type = MCC_SEMANTIC_CHECK_EXPRESSION_TYPE_VOID;
		break;
	default:
		userdata->declared_function_type = MCC_SEMANTIC_CHECK_EXPRESSION_TYPE_UNKNOWN;
		break;
	}

	struct mcc_ast_visitor visitor = function_return_value_visitor(userdata);

	// Manually start the visitor only at the function level
	mcc_ast_visit(function, &visitor);
	free(userdata);
}

struct mcc_semantic_check *mcc_semantic_check_run_function_return_value(struct mcc_ast_program *ast,
                                                                        struct mcc_symbol_table *symbol_table)
{
	UNUSED(symbol_table);
	struct mcc_semantic_check *check = malloc(sizeof(*check));
	if (!check) {
		return NULL;
	}

	check->status = MCC_SEMANTIC_CHECK_OK;
	check->type = MCC_SEMANTIC_CHECK_FUNCTION_RETURN_VALUE;
	check->error_buffer = NULL;

	do {
		check_function_return_value_ok(ast, check);
		if (check->status == MCC_SEMANTIC_CHECK_FAIL) {
			return check;
		}
		ast = ast->next_function;
	} while (ast);

	return check;
}

// -------------------------------------------------------------- Each execution path of non-void function returns a
//                                                                value

// generate error msg for nonvoid check
static void
generate_error_msg_failed_nonvoid_check(const char *name, struct mcc_ast_node node, struct mcc_semantic_check *check)
{
	if (check->error_buffer) {
		return;
	}
	int size = 50 + strlen(name);
	char *error_msg = (char *)malloc(sizeof(char) * size);
	snprintf(error_msg, size, "control reaches end of non-void function '%s'.", name);
	write_error_message_to_check(check, node, error_msg);
	check->status = MCC_SEMANTIC_CHECK_FAIL;
	free(error_msg);
}

// check a single statement on non-void property, i.e. either success if return or invoke recursive check for if_else
static bool check_nonvoid_property(struct mcc_ast_statement *statement)
{
	assert(statement);

	bool is_successful = false;

	switch (statement->type) {
	case MCC_AST_STATEMENT_TYPE_IF_STMT:
		// do nothing
		break;
	case MCC_AST_STATEMENT_TYPE_IF_ELSE_STMT:
		is_successful = check_nonvoid_property(statement->if_else_on_true) &&
		                check_nonvoid_property(statement->if_else_on_false);
		break;
	case MCC_AST_STATEMENT_TYPE_EXPRESSION:
		// do nothing
		break;
	case MCC_AST_STATEMENT_TYPE_WHILE:
		// do nothing
		break;
	case MCC_AST_STATEMENT_TYPE_DECLARATION:
		// do nothing
		break;
	case MCC_AST_STATEMENT_TYPE_ASSIGNMENT:
		// do nothing
		break;
	case MCC_AST_STATEMENT_TYPE_RETURN:
		is_successful = true;
		break;
	case MCC_AST_STATEMENT_TYPE_COMPOUND_STMT:
		is_successful = recursively_check_nonvoid_property(statement->compound_statement);
		break;
	}

	return is_successful;
}

// recursively check non-void property, i.e. all execution paths end in a return
static bool recursively_check_nonvoid_property(struct mcc_ast_compound_statement *compound_statement)
{
	assert(compound_statement);

	bool is_successful = false;

	// check recursively statements, start with last compound_statement
	if (compound_statement->next_compound_statement) {
		is_successful = recursively_check_nonvoid_property(compound_statement->next_compound_statement);
	}
	// if not successfully found any return on all execution paths
	if (is_successful == false && compound_statement->statement) {
		struct mcc_ast_statement *statement = compound_statement->statement;
		is_successful = check_nonvoid_property(statement);
	}

	return is_successful;
}

static void run_nonvoid_check(struct mcc_ast_function_definition *function, struct mcc_semantic_check *check)
{
	assert(function);
	assert(check);
	// Early abort if already failed
	if (check->status == MCC_SEMANTIC_CHECK_FAIL) {
		return;
	}

	bool is_successful = false;

	if (function->type != MCC_AST_FUNCTION_TYPE_VOID) {
		is_successful = recursively_check_nonvoid_property(function->compound_stmt);
	} else {
		is_successful = true;
	}

	if (is_successful == false) {
		generate_error_msg_failed_nonvoid_check(function->identifier->identifier_name, function->node, check);
	}
}

// run non-void check
struct mcc_semantic_check *mcc_semantic_check_run_nonvoid_check(struct mcc_ast_program *ast,
                                                                struct mcc_symbol_table *symbol_table)
{
	UNUSED(symbol_table);
	assert(ast);

	struct mcc_semantic_check *check = malloc(sizeof(*check));
	if (!check) {
		return NULL;
	}

	check->status = MCC_SEMANTIC_CHECK_OK;
	check->type = MCC_SEMANTIC_CHECK_NONVOID_CHECK;
	check->error_buffer = NULL;

	do {
		run_nonvoid_check(ast->function, check);

		ast = ast->next_function;
	} while (ast);

	return check;
}

// -------------------------------------------------------------- Main function exists and has correct signature
struct mcc_semantic_check *mcc_semantic_check_run_main_function(struct mcc_ast_program *ast,
                                                                struct mcc_symbol_table *symbol_table)
{
	UNUSED(symbol_table);

	struct mcc_semantic_check *check = malloc(sizeof(*check));
	if (!check) {
		return NULL;
	}

	check->status = MCC_SEMANTIC_CHECK_OK;
	check->type = MCC_SEMANTIC_CHECK_MAIN_FUNCTION;
	check->error_buffer = NULL;

	int number_of_mains = 0;

	if (strcmp(ast->function->identifier->identifier_name, "main") == 0) {
		number_of_mains += 1;
		if (!(ast->function->parameters->is_empty)) {
			write_error_message_to_check(check, ast->function->node,
			                             "Main has wrong signature. Must be `int main()`");
			check->status = MCC_SEMANTIC_CHECK_FAIL;
			return check;
		}
	}
	while (ast->has_next_function) {
		ast = ast->next_function;
		if (strcmp(ast->function->identifier->identifier_name, "main") == 0) {
			number_of_mains += 1;
			if (number_of_mains > 1) {
				write_error_message_to_check(check, ast->function->node,
				                             "Too many main functions defined.");
				check->status = MCC_SEMANTIC_CHECK_FAIL;
			}
			if (!(ast->function->parameters->is_empty)) {
				write_error_message_to_check(check, ast->function->node,
				                             "Main has wrong signature. Must be `int main()`");
				check->status = MCC_SEMANTIC_CHECK_FAIL;
				return check;
			}
		}
	}
	if (number_of_mains == 0) {
		write_error_message_to_check(check, ast->node, "No main function defined.");
		check->status = MCC_SEMANTIC_CHECK_FAIL;
		return check;
	}

	return check;
}

// -------------------------------------------------------------- No Calls to unknown functions

// generate error message
static void
generate_error_msg_unknown_function_call(const char *name, struct mcc_ast_node node, struct mcc_semantic_check *check)
{
	if (check->error_buffer) {
		return;
	}
	int size = 60 + strlen(name);
	char *error_msg = (char *)malloc(sizeof(char) * size);
	snprintf(error_msg, size, "function  '%s' undeclared (first use in this function).", name);
	write_error_message_to_check(check, node, error_msg);
	check->status = MCC_SEMANTIC_CHECK_FAIL;
	free(error_msg);
}

// callback for check of call to unknown function
static void cb_unknown_function_call(struct mcc_ast_expression *expression, void *data)
{
	assert(expression);
	assert(data);

	struct mcc_semantic_check *check = data;
	// Early abort if already failed
	if (check->status == MCC_SEMANTIC_CHECK_FAIL) {
		return;
	}
	struct mcc_symbol_table_row *row = expression->function_row;
	char *name = expression->function_identifier->identifier_name;

	struct mcc_symbol_table_row *function_declaration = mcc_symbol_table_check_for_function_declaration(name, row);

	if (!function_declaration) {
		generate_error_msg_unknown_function_call(name, expression->node, check);
	}
}

// Setup an AST Visitor for checking calls to unknown functions.
static struct mcc_ast_visitor unknown_function_call_visitor(struct mcc_semantic_check *check)
{

	return (struct mcc_ast_visitor){
	    .traversal = MCC_AST_VISIT_DEPTH_FIRST,
	    .order = MCC_AST_VISIT_PRE_ORDER,

	    .userdata = check,

	    .expression_function_call = cb_unknown_function_call,
	};
}

// check for calls to unknown functions
struct mcc_semantic_check *mcc_semantic_check_run_unknown_function_call(struct mcc_ast_program *ast,
                                                                        struct mcc_symbol_table *symbol_table)
{

	UNUSED(symbol_table);

	assert(ast);
	struct mcc_semantic_check *check = malloc(sizeof(*check));
	if (!check) {
		return NULL;
	}

	check->status = MCC_SEMANTIC_CHECK_OK;
	check->type = MCC_SEMANTIC_CHECK_UNKNOWN_FUNCTION_CALL;
	check->error_buffer = NULL;

	struct mcc_ast_visitor visitor = unknown_function_call_visitor(check);
	mcc_ast_visit(ast, &visitor);
	return check;
}

// -------------------------------------------------------------- No multiple definitions of the same function
// generate error message
static void generate_error_msg_multiple_function_defintion(const char *name,
                                                           struct mcc_ast_program *program,
                                                           struct mcc_semantic_check *check)
{
	assert(program);
	assert(check);

	if (check->error_buffer) {
		return;
	}
	int size = 20 + strlen(name);
	char *error_msg = (char *)malloc(sizeof(char) * size);
	snprintf(error_msg, size, "redefinition of '%s'", name);
	write_error_message_to_check(check, program->node, error_msg);
	check->status = MCC_SEMANTIC_CHECK_FAIL;
	free(error_msg);
}

// check for no multiple definitions of the same function
struct mcc_semantic_check *mcc_semantic_check_run_multiple_function_definitions(struct mcc_ast_program *ast,
                                                                                struct mcc_symbol_table *symbol_table)
{
	UNUSED(symbol_table);

	assert(ast);
	struct mcc_semantic_check *check = malloc(sizeof(*check));
	if (!check) {
		return NULL;
	}

	check->status = MCC_SEMANTIC_CHECK_OK;
	check->type = MCC_SEMANTIC_CHECK_MULTIPLE_FUNCTION_DEFINITIONS;
	check->error_buffer = NULL;

	struct mcc_ast_program *program_to_check = ast;

	// Program has only one function
	if (!program_to_check->next_function) {
		return check;
	}

	while (program_to_check->next_function) {
		struct mcc_ast_program *program_to_compare = program_to_check->next_function;
		char *name_of_check = program_to_check->function->identifier->identifier_name;
		char *name_of_compare = program_to_compare->function->identifier->identifier_name;

		// if name of program_to_check and name of program_to_compare equals
		if (strcmp(name_of_check, name_of_compare) == 0) {
			generate_error_msg_multiple_function_defintion(name_of_compare, program_to_compare, check);
			return check;
		}
		// compare all next_functions
		while (program_to_compare->next_function) {
			program_to_compare = program_to_compare->next_function;
			char *name_of_compare = program_to_compare->function->identifier->identifier_name;
			// if name of program_to_check and name of program_to_compare equals
			if (strcmp(name_of_check, name_of_compare) == 0) {
				generate_error_msg_multiple_function_defintion(name_of_compare, program_to_compare,
				                                               check);
				return check;
			}
		}

		program_to_check = program_to_check->next_function;
	}

	return check;
}

// -------------------------------------------------------------- No multiple declarations of a variable in the same
//                                                                scope
// generate error meassage
static void generate_error_msg_multiple_variable_declaration(struct mcc_symbol_table_row *row,
                                                             struct mcc_semantic_check *check)
{
	assert(row);
	assert(check);

	if (check->error_buffer) {
		return;
	}

	int size = 20 + strlen(row->name);
	char *error_msg = (char *)malloc(sizeof(char) * size);
	snprintf(error_msg, size, "redefinition of '%s'", row->name);
	write_error_message_to_check(check, *(row->node), error_msg);
	check->status = MCC_SEMANTIC_CHECK_FAIL;
	free(error_msg);
}

// check scopes individually
static void check_scope_for_multiple_variable_declaration(struct mcc_symbol_table_scope *scope,
                                                          struct mcc_semantic_check *check)
{
	assert(scope);
	assert(check);

	// Early abort if already failed
	if (check->status == MCC_SEMANTIC_CHECK_FAIL) {
		return;
	}

	if (!scope->head) {
		return;
	}

	struct mcc_symbol_table_row *row_to_check = scope->head;

	if (row_to_check->child_scope) {
		check_scope_for_multiple_variable_declaration(row_to_check->child_scope, check);
	}

	while (row_to_check->next_row) {
		struct mcc_symbol_table_row *row_to_compare = row_to_check->next_row;

		if (strcmp(row_to_check->name, row_to_compare->name) == 0) {
			generate_error_msg_multiple_variable_declaration(row_to_compare, check);
			return;
		}

		while (row_to_compare->next_row) {
			row_to_compare = row_to_compare->next_row;
			if (strcmp(row_to_check->name, row_to_compare->name) == 0) {
				generate_error_msg_multiple_variable_declaration(row_to_compare, check);
				return;
			}
		}

		row_to_check = row_to_check->next_row;
	}

	if (row_to_check->child_scope) {
		check_scope_for_multiple_variable_declaration(row_to_check->child_scope, check);
	}
}

// check for no multiple declarations of a variable in the same scope
struct mcc_semantic_check *mcc_semantic_check_run_multiple_variable_declarations(struct mcc_ast_program *ast,
                                                                                 struct mcc_symbol_table *symbol_table)
{
	UNUSED(ast);

	assert(symbol_table);
	struct mcc_semantic_check *check = malloc(sizeof(*check));
	if (!check) {
		return NULL;
	}

	check->status = MCC_SEMANTIC_CHECK_OK;
	check->type = MCC_SEMANTIC_CHECK_MULTIPLE_VARIABLE_DECLARATIONS;
	check->error_buffer = NULL;

	struct mcc_symbol_table_scope *scope = symbol_table->head;
	struct mcc_symbol_table_row *function_row = scope->head;

	if (function_row->child_scope) {
		check_scope_for_multiple_variable_declaration(function_row->child_scope, check);
	}

	while (function_row->next_row) {
		function_row = function_row->next_row;
		if (function_row->child_scope) {
			check_scope_for_multiple_variable_declaration(function_row->child_scope, check);
		}
	}

	return check;
}
// -------------------------------------------------------------- No use of undeclared variables

// generate messaage
static void
generate_error_msg_undeclared_variable(const char *name, struct mcc_ast_node node, struct mcc_semantic_check *check)
{
	assert(check);

	if (check->error_buffer) {
		return;
	}
	int size = 50 + strlen(name);
	char *error_msg = (char *)malloc(sizeof(char) * size);
	snprintf(error_msg, size, "'%s' undeclared (first use in this function).", name);
	write_error_message_to_check(check, node, error_msg);
	check->status = MCC_SEMANTIC_CHECK_FAIL;
	free(error_msg);
}

// callback for expression of variable type concerning the check of undeclared variables
static void cb_use_undeclared_variable(struct mcc_ast_expression *expression, void *data)
{
	assert(expression);
	assert(data);

	struct mcc_semantic_check *check = data;
	// Early abort if already failed
	if (check->status == MCC_SEMANTIC_CHECK_FAIL) {
		return;
	}
	struct mcc_symbol_table_row *row = expression->variable_row;
	char *name = expression->identifier->identifier_name;

	struct mcc_symbol_table_row *upward_declaration = mcc_symbol_table_check_upwards_for_declaration(name, row);

	if (!upward_declaration) {
		generate_error_msg_undeclared_variable(name, expression->node, check);
	}
}

// callback for expression of array type concerning the check of undeclared variables
static void cb_use_undeclared_array(struct mcc_ast_expression *expression, void *data)
{
	assert(expression);
	assert(data);

	struct mcc_semantic_check *check = data;
	// Early abort if already failed
	if (check->status == MCC_SEMANTIC_CHECK_FAIL) {
		return;
	}
	struct mcc_symbol_table_row *row = expression->array_row;
	char *name = expression->array_identifier->identifier_name;

	struct mcc_symbol_table_row *upward_declaration = mcc_symbol_table_check_upwards_for_declaration(name, row);

	if (!upward_declaration) {
		generate_error_msg_undeclared_variable(name, expression->node, check);
	}
}

// callback for assignment of variable type concerning the check of undeclared variables
static void cb_use_undeclared_variable_assignment(struct mcc_ast_assignment *assignment, void *data)
{
	assert(assignment);
	assert(data);

	struct mcc_semantic_check *check = data;
	// Early abort if already failed
	if (check->status == MCC_SEMANTIC_CHECK_FAIL) {
		return;
	}
	struct mcc_symbol_table_row *row = assignment->row;
	char *name = assignment->variable_identifier->identifier_name;

	struct mcc_symbol_table_row *upward_declaration = mcc_symbol_table_check_upwards_for_declaration(name, row);

	if (!upward_declaration) {
		generate_error_msg_undeclared_variable(name, assignment->node, check);
	}
}

// callback for assignment of array type concerning the check of undeclared variables
static void cb_use_undeclared_array_assignment(struct mcc_ast_assignment *assignment, void *data)
{
	assert(assignment);
	assert(data);

	struct mcc_semantic_check *check = data;
	// Early abort if already failed
	if (check->status == MCC_SEMANTIC_CHECK_FAIL) {
		return;
	}
	struct mcc_symbol_table_row *row = assignment->row;
	char *name = assignment->array_identifier->identifier_name;

	struct mcc_symbol_table_row *upward_declaration = mcc_symbol_table_check_upwards_for_declaration(name, row);

	if (!upward_declaration) {
		generate_error_msg_undeclared_variable(name, assignment->node, check);
	}
}

// Setup an AST Visitor for checking undeclared variables.
static struct mcc_ast_visitor use_undeclared_variable_visitor(struct mcc_semantic_check *check)
{

	return (struct mcc_ast_visitor){
	    .traversal = MCC_AST_VISIT_DEPTH_FIRST,
	    .order = MCC_AST_VISIT_PRE_ORDER,

	    .userdata = check,

	    .expression_variable = cb_use_undeclared_variable,
	    .expression_array_element = cb_use_undeclared_array,

	    .variable_assignment = cb_use_undeclared_variable_assignment,
	    .array_assignment = cb_use_undeclared_array_assignment,
	};
}

// No use of undeclared variables - the actual check
struct mcc_semantic_check *mcc_semantic_check_run_use_undeclared_variable(struct mcc_ast_program *ast,
                                                                          struct mcc_symbol_table *symbol_table)
{
	UNUSED(symbol_table);

	assert(ast);
	struct mcc_semantic_check *check = malloc(sizeof(*check));
	if (!check) {
		return NULL;
	}

	check->status = MCC_SEMANTIC_CHECK_OK;
	check->type = MCC_SEMANTIC_CHECK_USE_UNDECLARED_VARIABLE;
	check->error_buffer = NULL;

	struct mcc_ast_visitor visitor = use_undeclared_variable_visitor(check);
	mcc_ast_visit(ast, &visitor);
	return check;
}

// -------------------------------------------------------------- No use of the names of the built_in functions in
//                                                                function definitions
struct mcc_semantic_check *mcc_semantic_check_run_define_built_in(struct mcc_ast_program *ast,
                                                                  struct mcc_symbol_table *symbol_table)
{
	UNUSED(symbol_table);

	struct mcc_semantic_check *check = malloc(sizeof(*check));
	if (!check) {
		return NULL;
	}

	check->status = MCC_SEMANTIC_CHECK_OK;
	check->type = MCC_SEMANTIC_CHECK_DEFINE_BUILT_IN;
	check->error_buffer = NULL;

	if (!(ast->function)) {
		return check;
	}

	// Check if we encounter built_ins as userdefined functions
	// Since we walk the AST, the first encounter already is an error, the built_ins
	// are only later added to the symbol table

	do {

		if (strcmp(ast->function->identifier->identifier_name, "print") == 0) {
			write_error_message_to_check(check, ast->function->node,
			                             "Multiple definitions of function `print` found."
			                             "`print` is reserved for the built_in function.");
			check->status = MCC_SEMANTIC_CHECK_FAIL;
			return check;
		}
		if (strcmp(ast->function->identifier->identifier_name, "print_nl") == 0) {
			write_error_message_to_check(check, ast->function->node,
			                             "Multiple definitions of function `print_nl` found."
			                             "`print_nl` is reserved for the built_in function.");
			check->status = MCC_SEMANTIC_CHECK_FAIL;
			return check;
		}
		if (strcmp(ast->function->identifier->identifier_name, "print_int") == 0) {
			write_error_message_to_check(check, ast->function->node,
			                             "Multiple definitions of function `print_int` found."
			                             "`print_int` is reserved for the built_in function.");
			check->status = MCC_SEMANTIC_CHECK_FAIL;
			return check;
		}
		if (strcmp(ast->function->identifier->identifier_name, "print_float") == 0) {
			write_error_message_to_check(check, ast->function->node,
			                             "Multiple definitions of function `print_float` found."
			                             "`print_float` is reserved for the built_in function.");
			check->status = MCC_SEMANTIC_CHECK_FAIL;
			return check;
		}
		if (strcmp(ast->function->identifier->identifier_name, "read_int") == 0) {
			write_error_message_to_check(check, ast->function->node,
			                             "Multiple definitions of function `read_int` found."
			                             "`read_int` is reserved for the built_in function.");
			check->status = MCC_SEMANTIC_CHECK_FAIL;
			return check;
		}
		if (strcmp(ast->function->identifier->identifier_name, "read_float") == 0) {
			write_error_message_to_check(check, ast->function->node,
			                             "Multiple definitions of function `read_float` found."
			                             "`read_float` is reserved for the built_in function.");
			check->status = MCC_SEMANTIC_CHECK_FAIL;
			return check;
		}

		ast = ast->next_function;

	} while (ast);

	return check;
}

// ------------------------------------------------------------- Functions: Cleanup

// TODO: Delete all checks
void mcc_semantic_check_delete_all_checks(struct mcc_semantic_check_all_checks *checks)
{

	if (!checks) {
		return;
	}
	mcc_semantic_check_delete_single_check(checks->type_conversion);
	mcc_semantic_check_delete_single_check(checks->function_arguments);
	mcc_semantic_check_delete_single_check(checks->function_return_value);
	mcc_semantic_check_delete_single_check(checks->nonvoid_check);
	mcc_semantic_check_delete_single_check(checks->main_function);
	mcc_semantic_check_delete_single_check(checks->unknown_function_call);
	mcc_semantic_check_delete_single_check(checks->multiple_function_definitions);
	mcc_semantic_check_delete_single_check(checks->multiple_variable_declarations);
	mcc_semantic_check_delete_single_check(checks->use_undeclared_variable);
	mcc_semantic_check_delete_single_check(checks->define_built_in);

	if (checks->error_buffer != NULL) {
		free(checks->error_buffer);
	}
	free(checks);
	return;
}

// Delete single check
void mcc_semantic_check_delete_single_check(struct mcc_semantic_check *check)
{

	if (check == NULL) {
		return;
	}

	if (check->error_buffer != NULL) {
		free(check->error_buffer);
	}

	free(check);
}
