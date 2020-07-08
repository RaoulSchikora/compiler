#include "mcc/semantic_checks.h"

#include <assert.h>
#include <math.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "mcc/ast_visit.h"
#include "utils/unused.h"

#define not_zero(x) (x > 0 ? x : 1)

// clang-format off

// Generics for check and get type
#define check_and_get_type(x, ...) _Generic((x), \
		struct mcc_ast_expression *:          check_and_get_type_expression, \
		struct mcc_ast_identifier *:          check_and_get_type_identifier, \
		struct mcc_ast_declaration *:         get_data_type_declaration, \
		struct mcc_ast_literal *:             check_and_get_type_literal \
		)(x, __VA_ARGS__)

#define mcc_semantic_check_raise_error(x,y,...) _Generic((y), \
        int :                                 raise_type_error, \
        struct mcc_semantic_check *:          raise_non_type_error \
	)(x,y,__VA_ARGS__)

// clang-format on

// ------------------------------------------------------------- Forward declaration

// Check and get type functions
struct mcc_semantic_check_data_type *check_and_get_type_expression(struct mcc_ast_expression *expression,
                                                                   struct mcc_semantic_check *check);

struct mcc_semantic_check_data_type *check_and_get_type_identifier(struct mcc_ast_identifier *identifier,
                                                                   struct mcc_semantic_check *check,
                                                                   struct mcc_symbol_table_row *row);

struct mcc_semantic_check_data_type *check_and_get_type_literal(struct mcc_ast_literal *literal, void *placeholder);

// ------------------------------------------------------------- Functions: Error handling

// Compute string length of source code location
static int get_sloc_string_size(struct mcc_ast_node node)
{
	// Hard coded 6 due to rounding and colons
	return floor(log10(not_zero(node.sloc.start_col)) + log10(not_zero(node.sloc.start_line))) +
	       strlen(node.sloc.filename) + 6;
}

static enum mcc_semantic_check_error_code
err_to_check_with_sloc(struct mcc_semantic_check *check, struct mcc_ast_node node, const char *string)
{
	assert(check);
	assert(check->error_buffer == NULL);
	assert(string);

	// +1 for terminating character
	size_t size = sizeof(char) * (strlen(string) + get_sloc_string_size(node) + 1);
	char *buffer = malloc(size);
	if (!buffer) {
		return MCC_SEMANTIC_CHECK_ERROR_MALLOC_FAILED;
	}
	if (0 > snprintf(buffer, size, "%s:%d:%d: %s", node.sloc.filename, node.sloc.start_line, node.sloc.start_col,
	                 string)) {
		free(buffer);
		return MCC_SEMANTIC_CHECK_ERROR_SNPRINTF_FAILED;
	}
	check->error_buffer = buffer;
	return MCC_SEMANTIC_CHECK_ERROR_OK;
}

static enum mcc_semantic_check_error_code v_raise_error(int num,
                                                        struct mcc_semantic_check *check,
                                                        struct mcc_ast_node node,
                                                        const char *format_string,
                                                        bool is_from_heap,
                                                        va_list args)
{
	assert(format_string);

	// Early abort
	if (check->status == MCC_SEMANTIC_CHECK_FAIL) {
		// If strings are from heap, free them
		if (is_from_heap) {
			char *temp;
			for (int i = 0; i < num; i++) {
				temp = va_arg(args, char *);
				free(temp);
			}
		}
		return MCC_SEMANTIC_CHECK_ERROR_OK;
	}

	check->status = MCC_SEMANTIC_CHECK_FAIL;

	// Store a copy of the va_list for later access
	va_list args_cp;
	va_list args_cp2;
	va_list args_cp3;
	va_copy(args_cp, args);
	va_copy(args_cp2, args);
	va_copy(args_cp3, args);

	// Get all the args & determine string length
	size_t args_size = 0;
	for (int i = 0; i < num; i++) {
		char *temp;
		temp = va_arg(args_cp, char *);
		if (!temp)
			// If NULL is handed as string, then to_string must have failed to malloc
			return MCC_SEMANTIC_CHECK_ERROR_UNKNOWN;
		args_size += strlen(temp);
	}

	// Malloc buffer string
	size_t size = sizeof(char) * (strlen(format_string) + args_size + 1);
	char *buffer = malloc(size);
	if (!buffer) {
		return MCC_SEMANTIC_CHECK_ERROR_MALLOC_FAILED;
	}

	// Get args again and print string into buffer
	if (0 > vsnprintf(buffer, size, format_string, args_cp2)) {
		va_end(args_cp2);
		free(buffer);
		return MCC_SEMANTIC_CHECK_ERROR_SNPRINTF_FAILED;
	}
	enum mcc_semantic_check_error_code error = MCC_SEMANTIC_CHECK_ERROR_OK;

	// Write buffer string into check
	error = err_to_check_with_sloc(check, node, buffer);
	free(buffer);

	// If strings are from heap, free them
	if (is_from_heap) {
		char *temp;
		for (int i = 0; i < num; i++) {
			temp = va_arg(args, char *);
			free(temp);
		}
	}

	va_end(args_cp);
	va_end(args_cp2);
	va_end(args_cp3);

	return error;
}

static enum mcc_semantic_check_error_code raise_non_type_error(int num,
                                                               struct mcc_semantic_check *check,
                                                               struct mcc_ast_node node,
                                                               const char *format_string,
                                                               bool is_from_heap,
                                                               ...)
{
	enum mcc_semantic_check_error_code error;

	va_list args;
	va_start(args, is_from_heap);
	error = v_raise_error(num, check, node, format_string, is_from_heap, args);
	va_end(args);

	return error;
}

// This function implements early abort depending on previously raised errors, so that the type checker doesn't have to
static enum mcc_semantic_check_error_code raise_type_error(enum mcc_semantic_check_error_code error,
                                                           int num,
                                                           struct mcc_semantic_check *check,
                                                           struct mcc_ast_node node,
                                                           const char *format_string,
                                                           bool is_from_heap,
                                                           ...)
{
	if (error == MCC_SEMANTIC_CHECK_ERROR_OK) {
		va_list args;
		va_start(args, is_from_heap);
		error = v_raise_error(num, check, node, format_string, is_from_heap, args);
		va_end(args);
	}
	return error;
}

// ------------------------------------------------------------- Functions: Set up and run all checks

// Generate struct for semantic check
struct mcc_semantic_check *mcc_semantic_check_initialize_check()
{
	struct mcc_semantic_check *check = malloc(sizeof(*check));
	if (check) {
		check->status = MCC_SEMANTIC_CHECK_OK;
		check->error_buffer = NULL;
	}
	return check;
}

// Wrapper for running one of the checks with 2 early aborts:
// Error code of the previous check is not OK: Error code is handed back immediately
// Status code of the previous check is not OK: Abort with error code OK, but don't change check
enum mcc_semantic_check_error_code
run_check_early_abrt(enum mcc_semantic_check_error_code (*fctptr)(struct mcc_ast_program *ast,
                                                                  struct mcc_symbol_table *table,
                                                                  struct mcc_semantic_check *check),
                     struct mcc_ast_program *ast,
                     struct mcc_symbol_table *table,
                     struct mcc_semantic_check *check,
                     enum mcc_semantic_check_error_code previous_return)
{

	assert(ast);
	assert(table);
	assert(check);
	assert(fctptr);

	if (previous_return != MCC_SEMANTIC_CHECK_ERROR_OK) {
		return previous_return;
	}

	if (check->status != MCC_SEMANTIC_CHECK_OK) {
		return MCC_SEMANTIC_CHECK_ERROR_OK;
	}

	// Run the semantic check and return its return value
	return (*fctptr)(ast, table, check);
}

// Run all semantic checks, returns NULL if library functions fail
struct mcc_semantic_check *mcc_semantic_check_run_all(struct mcc_ast_program *ast,
                                                      struct mcc_symbol_table *symbol_table)
{
	assert(ast);
	assert(symbol_table);

	enum mcc_semantic_check_error_code error = MCC_SEMANTIC_CHECK_ERROR_OK;

	struct mcc_semantic_check *check = mcc_semantic_check_initialize_check();
	if (!check)
		return NULL;

	error = run_check_early_abrt(mcc_semantic_check_run_type_check, ast, symbol_table, check, error);
	error = run_check_early_abrt(mcc_semantic_check_run_nonvoid_check, ast, symbol_table, check, error);
	error = run_check_early_abrt(mcc_semantic_check_run_main_function, ast, symbol_table, check, error);
	error =
	    run_check_early_abrt(mcc_semantic_check_run_multiple_function_definitions, ast, symbol_table, check, error);
	error = run_check_early_abrt(mcc_semantic_check_run_multiple_variable_declarations, ast, symbol_table, check,
	                             error);
	error = run_check_early_abrt(mcc_semantic_check_run_function_arguments, ast, symbol_table, check, error);

	if (error != MCC_SEMANTIC_CHECK_ERROR_OK) {
		mcc_semantic_check_delete_single_check(check);
		return NULL;
	}
	return check;
}

// ------------------------------------------------------------- check_and_get_type functionalities

// getter for default data type
static struct mcc_semantic_check_data_type *get_new_data_type()
{
	struct mcc_semantic_check_data_type *type = malloc(sizeof(*type));
	if (!type)
		return NULL;
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
	if (!type)
		return NULL;
	switch (row->row_type) {
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

	if (row->array_size != -1) {
		type->is_array = true;
	}
	type->array_size = row->array_size;
	return type;
}

static enum mcc_semantic_check_data_types ast_to_semantic_check_type(enum mcc_ast_types type)
{
	switch (type) {
	case INT:
		return MCC_SEMANTIC_CHECK_INT;
	case FLOAT:
		return MCC_SEMANTIC_CHECK_FLOAT;
	case BOOL:
		return MCC_SEMANTIC_CHECK_BOOL;
	case STRING:
		return MCC_SEMANTIC_CHECK_STRING;
	case VOID:
		return MCC_SEMANTIC_CHECK_VOID;
	default:
		return MCC_SEMANTIC_CHECK_UNKNOWN;
	}
}

static struct mcc_semantic_check_data_type *get_data_type_declaration(struct mcc_ast_declaration *decl,
                                                                      struct mcc_semantic_check *check)
{
	assert(decl);
	UNUSED(check);
	struct mcc_semantic_check_data_type *type = get_new_data_type();
	if (!type)
		return NULL;
	if (decl->declaration_type == MCC_AST_DECLARATION_TYPE_VARIABLE) {
		type->is_array = false;
		type->type = ast_to_semantic_check_type(decl->variable_type->type_value);
	} else {
		type->is_array = true;
		type->array_size = decl->array_size->i_value;
		type->type = ast_to_semantic_check_type(decl->array_type->type_value);
	}
	return type;
}

static bool is_int(struct mcc_semantic_check_data_type *type)
{
	assert(type);
	return ((type->type == MCC_SEMANTIC_CHECK_INT) && !type->is_array);
}

static bool is_bool(struct mcc_semantic_check_data_type *type)
{
	assert(type);
	return ((type->type == MCC_SEMANTIC_CHECK_BOOL) && !type->is_array);
}

static bool is_string(struct mcc_semantic_check_data_type *type)
{
	assert(type);
	return ((type->type == MCC_SEMANTIC_CHECK_STRING) && !type->is_array);
}

static bool types_equal(struct mcc_semantic_check_data_type *first, struct mcc_semantic_check_data_type *second)
{
	assert(first);
	assert(second);
	return ((first->type == second->type) && (first->array_size == second->array_size));
}

// Converts a data type into a string. Returns NULL if malloc or snprintf fail
static char *to_string(struct mcc_semantic_check_data_type *type)
{
	assert(type);

	// Longest type string has 7 characters
	char type_string[8];

	switch (type->type) {
	case MCC_SEMANTIC_CHECK_INT:
		strncpy(type_string, "INT", 8);
		break;
	case MCC_SEMANTIC_CHECK_FLOAT:
		strncpy(type_string, "FLOAT", 8);
		break;
	case MCC_SEMANTIC_CHECK_BOOL:
		strncpy(type_string, "BOOL", 8);
		break;
	case MCC_SEMANTIC_CHECK_STRING:
		strncpy(type_string, "STRING", 8);
		break;
	case MCC_SEMANTIC_CHECK_VOID:
		strncpy(type_string, "VOID", 8);
		break;
	default:
		strncpy(type_string, "UNKNOWN", 8);
		break;
	}
	size_t size = 11 + (size_t)floor(log10(not_zero(type->array_size)));
	char *buffer = malloc(size);
	if (!buffer) {
		return NULL;
	}

	if (type->is_array) {
		if (0 > snprintf(buffer, size, "%s[%d]", type_string, type->array_size)) {
			free(buffer);
			return NULL;
		}
	} else {
		if (0 > snprintf(buffer, size, "%s", type_string)) {
			free(buffer);
			return NULL;
		}
	}

	return buffer;
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
	if (!lhs)
		return NULL;
	struct mcc_semantic_check_data_type *rhs = check_and_get_type(expression->rhs, check);
	if (!rhs) {
		free(lhs);
		return NULL;
	}
	enum mcc_ast_binary_op op = expression->op;

	switch (op) {
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

	if (!success || lhs->is_array || rhs->is_array || is_string(lhs) || is_string(rhs)) {
		if (mcc_semantic_check_raise_error(2, check, expression->node,
		                                   "operation on incompatible types '%s' and '%s'.", true,
		                                   to_string(lhs), to_string(rhs)) != MCC_SEMANTIC_CHECK_ERROR_OK) {
			free(rhs);
			free(lhs);
			return NULL;
		}
		lhs->type = MCC_SEMANTIC_CHECK_UNKNOWN;
	}
	if (success && (lhs->type == MCC_SEMANTIC_CHECK_UNKNOWN)) {
		if (mcc_semantic_check_raise_error(0, check, expression->node, "unknown type.", false) !=
		    MCC_SEMANTIC_CHECK_ERROR_OK) {
			free(rhs);
			free(lhs);
			return NULL;
		}
	}
	if (!(op == MCC_AST_BINARY_OP_ADD || op == MCC_AST_BINARY_OP_SUB || op == MCC_AST_BINARY_OP_MUL ||
	      op == MCC_AST_BINARY_OP_DIV)) {
		lhs->type = MCC_SEMANTIC_CHECK_BOOL;
	}
	free(rhs);
	return lhs;
}

struct mcc_semantic_check_data_type *check_and_get_type_unary_expression(struct mcc_ast_expression *expression,
                                                                         struct mcc_semantic_check *check)
{
	assert(expression->type == MCC_AST_EXPRESSION_TYPE_UNARY_OP);
	assert(expression->child);
	assert(check);

	struct mcc_semantic_check_data_type *child = check_and_get_type(expression->child, check);
	if (!child)
		return NULL;
	enum mcc_ast_unary_op u_op = expression->u_op;

	if (child->is_array || is_string(child) || ((u_op == MCC_AST_UNARY_OP_NEGATIV) && is_bool(child)) ||
	    ((u_op == MCC_AST_UNARY_OP_NOT) && !is_bool(child))) {
		if (mcc_semantic_check_raise_error(1, check, expression->node,
		                                   "unary operation not compatible with '%s'.", true,
		                                   to_string(child)) != MCC_SEMANTIC_CHECK_ERROR_OK) {
			free(child);
			return NULL;
		}
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
	if (!index)
		return NULL;
	struct mcc_semantic_check_data_type *identifier =
	    check_and_get_type(array_element->array_identifier, check, array_element->array_row);
	if (!identifier) {
		free(index);
		return NULL;
	}
	char *name = array_element->array_identifier->identifier_name;
	if (!is_int(index)) {
		if (mcc_semantic_check_raise_error(1, check, array_element->node, "expected type 'INT' but was '%s'.",
		                                   true, to_string(index)) != MCC_SEMANTIC_CHECK_ERROR_OK) {
			free(index);
			free(identifier);
			return NULL;
		}
		identifier->type = MCC_SEMANTIC_CHECK_UNKNOWN;
	}
	if (!identifier->is_array) {
		if (mcc_semantic_check_raise_error(1, check, array_element->node,
		                                   "subscripted value '%s' is not an array.", false,
		                                   name) != MCC_SEMANTIC_CHECK_ERROR_OK) {
			free(index);
			free(identifier);
			return NULL;
		}
		identifier->type = MCC_SEMANTIC_CHECK_UNKNOWN;
	}
	identifier->is_array = false;
	identifier->array_size = -1;
	free(index);
	return identifier;
}

// gets the type of a function call expression. Arguments are checked seperatly.
static struct mcc_semantic_check_data_type *check_and_get_type_function_call(struct mcc_ast_expression *function_call,
                                                                             struct mcc_semantic_check *check)
{
	assert(function_call->type == MCC_AST_EXPRESSION_TYPE_FUNCTION_CALL);
	assert(check);

	char *name = function_call->function_identifier->identifier_name;
	struct mcc_symbol_table_row *row = function_call->function_row;
	row = mcc_symbol_table_check_for_function_declaration(name, row);

	if (!row) {
		if (mcc_semantic_check_raise_error(1, check, function_call->node,
		                                   "'%s' undeclared (first use in this function).", false,
		                                   name) != MCC_SEMANTIC_CHECK_ERROR_OK) {
			return NULL;
		}
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
	if (!type)
		return NULL;

	switch (literal->type) {
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

struct mcc_semantic_check_data_type *check_and_get_type_expression(struct mcc_ast_expression *expression,
                                                                   struct mcc_semantic_check *check)
{
	assert(expression);
	assert(check);

	switch (expression->type) {
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

struct mcc_semantic_check_data_type *check_and_get_type_identifier(struct mcc_ast_identifier *identifier,
                                                                   struct mcc_semantic_check *check,
                                                                   struct mcc_symbol_table_row *row)
{
	assert(identifier);
	assert(check);
	assert(row);

	char *name = identifier->identifier_name;
	row = mcc_symbol_table_check_upwards_for_declaration(name, row);
	if (!row) {
		if (mcc_semantic_check_raise_error(1, check, identifier->node,
		                                   "'%s' undeclared (first use in this function).", false,
		                                   name) != MCC_SEMANTIC_CHECK_ERROR_OK) {
			return NULL;
		}
		return get_new_data_type();
	}

	return get_data_type_from_row(row);
}

// ------------------------------------------------------------- type checker

// struct for user data concerning the type checker
struct return_value_userdata {
	struct mcc_semantic_check *check;
	struct mcc_semantic_check_data_type *function_type;
	enum mcc_semantic_check_error_code error;
};

static void cb_return_value(struct mcc_ast_statement *statement, void *r_v_userdata)
{
	assert(statement);
	assert(r_v_userdata);

	struct return_value_userdata *userdata = r_v_userdata;
	struct mcc_semantic_check *check = userdata->check;
	struct mcc_semantic_check_data_type *function_type = userdata->function_type;
	struct mcc_semantic_check_data_type *return_type = NULL;
	if (statement->return_value) {
		return_type = check_and_get_type(statement->return_value, check);
		if (!return_type) {
			userdata->error = MCC_SEMANTIC_CHECK_ERROR_UNKNOWN;
			return;
		}
	} else {
		return_type = get_new_data_type();
		if (!return_type) {
			userdata->error = MCC_SEMANTIC_CHECK_ERROR_UNKNOWN;
			return;
		}
		return_type->type = MCC_SEMANTIC_CHECK_VOID;
	}
	if (!types_equal(function_type, return_type)) {
		userdata->error =
		    mcc_semantic_check_raise_error(userdata->error, 2, check, statement->return_value->node,
		                                   "return value of type '%s', expected '%s'.", true,
		                                   to_string(return_type), to_string(function_type));
	}
	free(return_type);
}

// Setup an AST Visitor for checking that return values have the correct type
static struct mcc_ast_visitor return_value_visitor(struct return_value_userdata *r_v_userdata)
{
	return (struct mcc_ast_visitor){
	    .order = MCC_AST_VISIT_PRE_ORDER,

	    .userdata = r_v_userdata,

	    .statement_return = cb_return_value,
	};
}

static void cb_type_check_return_value(struct mcc_ast_function_definition *function, void *data)
{
	assert(function);
	assert(data);

	struct type_checking_userdata *t_c_userdata = data;
	struct return_value_userdata *r_v_userdata = malloc(sizeof(*r_v_userdata));
	if (!r_v_userdata) {
		t_c_userdata->error = MCC_SEMANTIC_CHECK_ERROR_MALLOC_FAILED;
		return;
	}
	r_v_userdata->check = t_c_userdata->check;
	r_v_userdata->error = t_c_userdata->error;
	r_v_userdata->function_type = get_new_data_type();
	if (!r_v_userdata->function_type) {
		t_c_userdata->error = MCC_SEMANTIC_CHECK_ERROR_MALLOC_FAILED;
		return;
	}
	r_v_userdata->function_type->type = ast_to_semantic_check_type(function->type);

	struct mcc_ast_visitor visitor = return_value_visitor(r_v_userdata);
	mcc_ast_visit(function, &visitor);
	t_c_userdata->error = r_v_userdata->error;
	r_v_userdata->error = 0;
	r_v_userdata->check = NULL;
	free(r_v_userdata->function_type);
	free(r_v_userdata);
}

static void cb_type_conversion_assignment(struct mcc_ast_statement *statement, void *data)
{
	assert(statement);
	assert(data);

	struct type_checking_userdata *userdata = data;
	struct mcc_semantic_check *check = userdata->check;
	struct mcc_ast_assignment *assignment = statement->assignment;
	struct mcc_semantic_check_data_type *lhs_type = NULL, *rhs_type = NULL, *index = NULL;

	switch (assignment->assignment_type) {
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
		return;
	}

	if (!lhs_type || !rhs_type || (!index && assignment->assignment_type == MCC_AST_ASSIGNMENT_TYPE_ARRAY)) {
		userdata->error = MCC_SEMANTIC_CHECK_ERROR_UNKNOWN;
		free(lhs_type);
		free(rhs_type);
		free(index);
		return;
	} else {
		if (assignment->assignment_type == MCC_AST_ASSIGNMENT_TYPE_ARRAY) {
			lhs_type->is_array = false;
			lhs_type->array_size = -1;
		}
		if (index && !is_int(index)) {
			userdata->error = mcc_semantic_check_raise_error(userdata->error, 0, check, assignment->node,
			                                                 "array subscript is not an integer.", false);
		} else if (!types_equal(lhs_type, rhs_type)) {
			userdata->error =
			    mcc_semantic_check_raise_error(userdata->error, 2, check, assignment->node,
			                                   "implicit type conversion. Expected '%s' but was '%s'", true,
			                                   to_string(lhs_type), to_string(rhs_type));
		} else if (lhs_type->is_array) {
			userdata->error =
			    mcc_semantic_check_raise_error(userdata->error, 0, check, assignment->node,
			                                   "assignment to Variable of array type not possible.", false);
		}
	}
	free(lhs_type);
	free(rhs_type);
	free(index);
}

static void cb_type_check_if_stmt(struct mcc_ast_statement *statement, void *data)
{
	assert(statement->if_condition);
	assert(data);

	struct type_checking_userdata *userdata = data;
	struct mcc_semantic_check *check = userdata->check;
	struct mcc_ast_expression *if_condition = statement->if_condition;
	struct mcc_semantic_check_data_type *type = check_and_get_type(if_condition, check);

	if (!type) {
		userdata->error = MCC_SEMANTIC_CHECK_ERROR_UNKNOWN;
	} else if (!is_bool(type)) {
		userdata->error = mcc_semantic_check_raise_error(
		    userdata->error, 1, check, if_condition->node,
		    "condition of if-statement of type '%s', expected type 'BOOL'.", true, to_string(type));
	}
	free(type);
}

static void cb_type_check_if_else_stmt(struct mcc_ast_statement *statement, void *data)
{
	assert(statement->if_else_condition);
	assert(data);

	struct type_checking_userdata *userdata = data;
	struct mcc_semantic_check *check = userdata->check;
	struct mcc_ast_expression *if_condition = statement->if_else_condition;
	struct mcc_semantic_check_data_type *type = check_and_get_type(if_condition, check);

	if (!type) {
		userdata->error = MCC_SEMANTIC_CHECK_ERROR_UNKNOWN;
	} else if (!is_bool(type)) {
		userdata->error = mcc_semantic_check_raise_error(
		    userdata->error, 1, check, if_condition->node,
		    "condition of if-statement of type '%s', expected type 'BOOL'.", true, to_string(type));
	}
	free(type);
}

static void cb_type_check_while_stmt(struct mcc_ast_statement *statement, void *data)
{
	assert(statement->while_condition);
	assert(data);

	struct type_checking_userdata *userdata = data;
	struct mcc_semantic_check *check = userdata->check;
	struct mcc_ast_expression *while_condition = statement->while_condition;
	struct mcc_semantic_check_data_type *type = check_and_get_type(while_condition, check);

	if (!type) {
		userdata->error = MCC_SEMANTIC_CHECK_ERROR_UNKNOWN;
	} else if (!is_bool(type)) {
		userdata->error = mcc_semantic_check_raise_error(
		    userdata->error, 1, check, while_condition->node,
		    "condition of while-loop of type '%s', expected type 'BOOL'.", true, to_string(type));
	}
	free(type);
}

static void cb_type_check_expression_stmt(struct mcc_ast_statement *statement, void *data)
{
	assert(statement->stmt_expression);
	assert(data);

	struct type_checking_userdata *userdata = data;
	struct mcc_semantic_check *check = userdata->check;
	struct mcc_ast_expression *expression = statement->stmt_expression;
	// check the expression. No Error handling needed
	struct mcc_semantic_check_data_type *type = check_and_get_type(expression, check);

	if (!type) {
		userdata->error = MCC_SEMANTIC_CHECK_ERROR_UNKNOWN;
	}
	free(type);
}

static struct mcc_ast_visitor type_checking_visitor(struct type_checking_userdata *userdata)
{
	return (struct mcc_ast_visitor){
	    .order = MCC_AST_VISIT_PRE_ORDER,

	    .userdata = userdata,

	    .statement_assignment = cb_type_conversion_assignment,
	    .statement_if_stmt = cb_type_check_if_stmt,
	    .statement_if_else_stmt = cb_type_check_if_else_stmt,
	    .statement_while = cb_type_check_while_stmt,
	    .statement_expression_stmt = cb_type_check_expression_stmt,
	    .function_definition = cb_type_check_return_value,
	};
}

enum mcc_semantic_check_error_code mcc_semantic_check_run_type_check(struct mcc_ast_program *ast,
                                                                     struct mcc_symbol_table *symbol_table,
                                                                     struct mcc_semantic_check *check)
{
	UNUSED(symbol_table);

	struct type_checking_userdata *userdata = malloc(sizeof(*userdata));
	if (!userdata) {
		return MCC_SEMANTIC_CHECK_ERROR_MALLOC_FAILED;
	}
	userdata->check = check;
	userdata->error = MCC_SEMANTIC_CHECK_ERROR_OK;
	enum mcc_semantic_check_error_code error;

	struct mcc_ast_visitor visitor = type_checking_visitor(userdata);
	mcc_ast_visit(ast, &visitor);
	error = userdata->error;
	userdata->check = NULL;
	free(userdata);
	return error;
}

// ------------------------------------------------------------- check execution paths of non-void functions

static bool recursively_check_nonvoid_property(struct mcc_ast_compound_statement *compound_statement);
static bool check_nonvoid_property(struct mcc_ast_statement *statement);

static bool check_nonvoid_property(struct mcc_ast_statement *statement)
{
	if (!statement)
		return false;

	switch (statement->type) {
	case MCC_AST_STATEMENT_TYPE_IF_ELSE_STMT:
		return (check_nonvoid_property(statement->if_else_on_true) &&
		        check_nonvoid_property(statement->if_else_on_false));
	case MCC_AST_STATEMENT_TYPE_RETURN:
		return true;
	case MCC_AST_STATEMENT_TYPE_COMPOUND_STMT:
		return recursively_check_nonvoid_property(statement->compound_statement);
	default:
		return false;
	}
}

static bool recursively_check_nonvoid_property(struct mcc_ast_compound_statement *compound_statement)
{
	if (!compound_statement)
		return false;

	return (check_nonvoid_property(compound_statement->statement) ||
	        recursively_check_nonvoid_property(compound_statement->next_compound_statement));
}

static enum mcc_semantic_check_error_code run_nonvoid_check(struct mcc_ast_function_definition *function,
                                                            struct mcc_semantic_check *check)
{
	assert(function);
	assert(check);

	if (function->type == VOID)
		return MCC_SEMANTIC_CHECK_ERROR_OK;

	if (recursively_check_nonvoid_property(function->compound_stmt) == false) {
		return mcc_semantic_check_raise_error(1, check, function->node,
		                                      "control reaches end of non-void function '%s'.", false,
		                                      function->identifier->identifier_name);
	}
	return MCC_SEMANTIC_CHECK_ERROR_OK;
}

enum mcc_semantic_check_error_code mcc_semantic_check_run_nonvoid_check(struct mcc_ast_program *ast,
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
		if (error != MCC_SEMANTIC_CHECK_ERROR_OK)
			break;
		ast = ast->next_function;
	} while (ast);

	return error;
}

// ------------------------------------------------------------- checking for correct main function

enum mcc_semantic_check_error_code mcc_semantic_check_run_main_function(struct mcc_ast_program *ast,
                                                                        struct mcc_symbol_table *symbol_table,
                                                                        struct mcc_semantic_check *check)
{
	UNUSED(symbol_table);
	assert(ast);
	assert(symbol_table);
	assert(check);
	assert(check->status == MCC_SEMANTIC_CHECK_OK);
	assert(!check->error_buffer);

	// Memorize AST entry point to get the correct filename later
	struct mcc_ast_program *original_ast = ast;
	int number_of_mains = 0;

	do {
		if (strcmp(ast->function->identifier->identifier_name, "main") == 0) {
			number_of_mains += 1;
			if (number_of_mains > 1) {
				return mcc_semantic_check_raise_error(0, check, original_ast->node,
				                                      "Too many main functions defined.", false);
			}
			if (!(ast->function->parameters->is_empty)) {
				return mcc_semantic_check_raise_error(0, check, original_ast->node,
				                                      "Main has wrong signature. "
				                                      "Must be `int main()`.",
				                                      false);
			}
		}
		ast = ast->next_function;
	} while (ast);

	if (number_of_mains == 0) {
		return mcc_semantic_check_raise_error(0, check, original_ast->node, "No main function defined.", false);
	}

	return MCC_SEMANTIC_CHECK_ERROR_OK;
}

// ------------------------------------------------------------- check for multiple function definitions

enum mcc_semantic_check_error_code mcc_semantic_check_run_multiple_function_definitions(
    struct mcc_ast_program *ast, struct mcc_symbol_table *symbol_table, struct mcc_semantic_check *check)
{
	UNUSED(symbol_table);
	assert(check);
	assert(!check->error_buffer);
	assert(ast);

	struct mcc_ast_program *program_to_check = ast;

	// Program has only one function
	if (!program_to_check->next_function) {
		return MCC_SEMANTIC_CHECK_ERROR_OK;
	}

	while (program_to_check->next_function) {
		struct mcc_ast_program *program_to_compare = program_to_check->next_function;
		char *name_of_check = program_to_check->function->identifier->identifier_name;
		char *name_of_compare = program_to_compare->function->identifier->identifier_name;

		if (strcmp(name_of_check, name_of_compare) == 0) {
			return mcc_semantic_check_raise_error(1, check, program_to_check->node, "redefinition of '%s'.",
			                                      false, name_of_compare);
		}
		// compare all next_functions
		while (program_to_compare->next_function) {
			program_to_compare = program_to_compare->next_function;
			char *name_of_compare = program_to_compare->function->identifier->identifier_name;
			if (strcmp(name_of_check, name_of_compare) == 0) {
				return mcc_semantic_check_raise_error(1, check, program_to_check->node,
				                                      "redefinition of '%s'.", false, name_of_compare);
			}
		}

		program_to_check = program_to_check->next_function;
	}

	return MCC_SEMANTIC_CHECK_ERROR_OK;
}

// ------------------------------------------------------------- check for multiple variable declarations

static enum mcc_semantic_check_error_code
check_scope_for_multiple_variable_declaration(struct mcc_symbol_table_scope *scope, struct mcc_semantic_check *check)
{
	assert(scope);
	assert(check);

	if (!scope->head) {
		return MCC_SEMANTIC_CHECK_ERROR_OK;
	}

	enum mcc_semantic_check_error_code error = MCC_SEMANTIC_CHECK_ERROR_OK;

	if (check->status == MCC_SEMANTIC_CHECK_FAIL) {
		return error;
	}

	struct mcc_symbol_table_row *row_to_check = scope->head;

	// Recursively check child scope of current row
	if (row_to_check->child_scope) {
		error = check_scope_for_multiple_variable_declaration(row_to_check->child_scope, check);
		if (error != MCC_SEMANTIC_CHECK_ERROR_OK) {
			return error;
		}
	}

	struct mcc_symbol_table_row *row_to_compare = NULL;
	while (row_to_check->next_row) {

		row_to_compare = row_to_check->next_row;

		if (strcmp(row_to_check->name, row_to_compare->name) == 0) {
			return mcc_semantic_check_raise_error(1, check, *(row_to_compare->node),
			                                      "redefinition of '%s'.", false, row_to_check->name);
		}

		while (row_to_compare->next_row) {
			row_to_compare = row_to_compare->next_row;
			if (strcmp(row_to_check->name, row_to_compare->name) == 0) {
				return mcc_semantic_check_raise_error(1, check, *(row_to_compare->node),
				                                      "redefinition of '%s'.", false,
				                                      row_to_check->name);
			}
		}

		row_to_check = row_to_check->next_row;
	}

	if (row_to_check->child_scope) {
		error = check_scope_for_multiple_variable_declaration(row_to_check->child_scope, check);
	}
	return error;
}

enum mcc_semantic_check_error_code mcc_semantic_check_run_multiple_variable_declarations(
    struct mcc_ast_program *ast, struct mcc_symbol_table *symbol_table, struct mcc_semantic_check *check)
{
	UNUSED(ast);
	assert(ast);
	assert(check);
	assert(!check->error_buffer);
	assert(check->status == MCC_SEMANTIC_CHECK_OK);

	struct mcc_symbol_table_scope *scope = symbol_table->head;
	struct mcc_symbol_table_row *function_row = scope->head;
	enum mcc_semantic_check_error_code error = MCC_SEMANTIC_CHECK_ERROR_OK;

	do {
		if (function_row->child_scope) {
			error = check_scope_for_multiple_variable_declaration(function_row->child_scope, check);
		}
		if (check->status != MCC_SEMANTIC_CHECK_OK || error != MCC_SEMANTIC_CHECK_ERROR_OK) {
			return error;
		}
		function_row = function_row->next_row;
	} while (function_row);

	return error;
}

// ------------------------------------------------------------- No invalid function calls

struct function_arguments_userdata {
	struct mcc_semantic_check *check;
	struct mcc_ast_program *program;
	enum mcc_semantic_check_error_code error;
};

static int get_number_of_params(struct mcc_ast_parameters *parameters)
{
	assert(parameters);

	if (parameters->is_empty) {
		return 0;
	}
	int num = 1;
	while (parameters->next_parameters) {
		num += 1;
		parameters = parameters->next_parameters;
	}
	return num;
}

static int get_number_of_args(struct mcc_ast_arguments *arguments)
{
	assert(arguments);

	if (arguments->is_empty) {
		return 0;
	}
	int num = 1;
	while (arguments->next_arguments) {
		num += 1;
		arguments = arguments->next_arguments;
	}
	return num;
}

static struct mcc_ast_parameters *get_params_from_ast(struct mcc_ast_program *ast, const char *name)
{
	assert(ast);
	assert(name);
	struct mcc_ast_parameters *params = NULL;
	do {
		if (strcmp(ast->function->identifier->identifier_name, name) == 0) {
			params = ast->function->parameters;
		}
		ast = ast->next_function;
	} while (ast);

	return params;
}

static void cb_function_arguments_expression_function_call(struct mcc_ast_expression *expression, void *userdata)
{
	assert(expression);
	assert(userdata);

	struct function_arguments_userdata *data = userdata;
	struct mcc_ast_program *ast = data->program;
	struct mcc_semantic_check *check = data->check;

	// Get the used arguments from the AST:
	struct mcc_ast_arguments *args = expression->arguments;
	// Get the required parameters from the function declaration
	struct mcc_ast_parameters *params = get_params_from_ast(ast, expression->function_identifier->identifier_name);
	// No parameters found -> unkown function
	if (!params) {
		data->error = mcc_semantic_check_raise_error(1, check, expression->node, "Undefined reference to '%s'",
		                                             false, expression->function_identifier->identifier_name);
		return;
	}

	int num_params = get_number_of_params(params);
	int num_args = get_number_of_args(args);
	if (num_params == 0 && num_args == 0) {
		return;
	} else if (num_args - num_params > 0) {
		data->error =
		    mcc_semantic_check_raise_error(1, check, expression->node, "Too many arguments to function '%s'",
		                                   false, expression->function_identifier->identifier_name);
		return;
	} else if (num_args - num_params < 0) {
		data->error =
		    mcc_semantic_check_raise_error(1, check, expression->node, "Too few arguments to function '%s'",
		                                   false, expression->function_identifier->identifier_name);
		return;
	}

	struct mcc_semantic_check_data_type *type_expr = NULL, *type_decl = NULL;
	do {
		// Check for type error
		type_expr = check_and_get_type(args->expression, check);
		type_decl = check_and_get_type(params->declaration, check);
		if (!type_expr || !type_decl) {
			data->error = MCC_SEMANTIC_CHECK_ERROR_UNKNOWN;
			free(type_decl);
			free(type_expr);
			return;
		}
		if (!types_equal(type_expr, type_decl)) {
			data->error = mcc_semantic_check_raise_error(2, check, expression->node,
			                                             "Expected '%s' but argument is of type '%s'", true,
			                                             to_string(type_decl), to_string(type_expr));
			free(type_expr);
			free(type_decl);
			return;
		}
		free(type_expr);
		free(type_decl);

		params = params->next_parameters;
		args = args->next_arguments;
	} while (params && args);
	return;
}

// Setup an AST Visitor for checking if function calls are correct
static struct mcc_ast_visitor function_arguments_visitor(struct function_arguments_userdata *data)
{
	return (struct mcc_ast_visitor){
	    .order = MCC_AST_VISIT_POST_ORDER,

	    .userdata = data,

	    .expression_function_call = cb_function_arguments_expression_function_call,
	};
}

enum mcc_semantic_check_error_code mcc_semantic_check_run_function_arguments(struct mcc_ast_program *ast,
                                                                             struct mcc_symbol_table *symbol_table,
                                                                             struct mcc_semantic_check *check)
{
	UNUSED(symbol_table);
	assert(ast);
	assert(check);
	assert(!check->error_buffer);
	assert(check->status == MCC_SEMANTIC_CHECK_OK);

	struct function_arguments_userdata *userdata = malloc(sizeof(*userdata));
	if (!userdata)
		return MCC_SEMANTIC_CHECK_ERROR_MALLOC_FAILED;
	userdata->check = check;
	userdata->program = ast;
	userdata->error = MCC_SEMANTIC_CHECK_ERROR_OK;

	struct mcc_ast_visitor visitor = function_arguments_visitor(userdata);
	mcc_ast_visit(ast, &visitor);
	enum mcc_semantic_check_error_code error = userdata->error;
	free(userdata);
	return error;
}

// ------------------------------------------------------------- Functions: Cleanup

void mcc_semantic_check_delete_single_check(struct mcc_semantic_check *check)
{
	if (check == NULL)
		return;
	free(check->error_buffer);
	free(check);
}

