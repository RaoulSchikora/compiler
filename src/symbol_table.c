#include "mcc/symbol_table.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "mcc/ast_visit.h"

// ------------------------------------------------------- Forward declaration

static int create_rows_statement(struct mcc_ast_statement *statement, struct mcc_symbol_table_scope *scope);
static int link_pointer_expression(struct mcc_ast_expression *expression, struct mcc_symbol_table_scope *scope);

// ------------------------------------------------------- converting enum types

static enum mcc_symbol_table_row_type convert_enum(enum mcc_ast_types type)
{
	switch (type) {
	case INT:
		return MCC_SYMBOL_TABLE_ROW_TYPE_INT;
	case FLOAT:
		return MCC_SYMBOL_TABLE_ROW_TYPE_FLOAT;
	case BOOL:
		return MCC_SYMBOL_TABLE_ROW_TYPE_BOOL;
	case STRING:
		return MCC_SYMBOL_TABLE_ROW_TYPE_STRING;
	case VOID:
		return MCC_SYMBOL_TABLE_ROW_TYPE_VOID;
	default:
		return MCC_SYMBOL_TABLE_ROW_TYPE_PSEUDO;
	}
}

// ------------------------------------------------------- Symbol Table row

struct mcc_symbol_table_row *
mcc_symbol_table_new_row_variable(char *name, enum mcc_symbol_table_row_type type, struct mcc_ast_node *node)
{
	struct mcc_symbol_table_row *row = malloc(sizeof(*row));
	if (!row) {
		return NULL;
	}

	row->row_structure = MCC_SYMBOL_TABLE_ROW_STRUCTURE_VARIABLE;
	row->array_size = -1;
	row->row_type = type;
	row->name = strdup(name);
	if (!row->name) {
		free(row);
		return NULL;
	}
	row->node = node;
	row->prev_row = NULL;
	row->next_row = NULL;
	row->scope = NULL;
	row->child_scope = NULL;

	return row;
}

struct mcc_symbol_table_row *
mcc_symbol_table_new_row_function(char *name, enum mcc_symbol_table_row_type type, struct mcc_ast_node *node)
{
	struct mcc_symbol_table_row *row = malloc(sizeof(*row));
	if (!row) {
		return NULL;
	}

	row->row_structure = MCC_SYMBOL_TABLE_ROW_STRUCTURE_FUNCTION;
	row->array_size = -1;
	row->row_type = type;
	row->name = strdup(name);
	if (!row->name) {
		free(row);
		return NULL;
	}
	row->node = node;
	row->prev_row = NULL;
	row->next_row = NULL;
	row->scope = NULL;
	row->child_scope = NULL;

	return row;
}

struct mcc_symbol_table_row *mcc_symbol_table_new_row_array(char *name,
                                                            long array_size,
                                                            enum mcc_symbol_table_row_type type,
                                                            struct mcc_ast_node *node)
{
	struct mcc_symbol_table_row *row = malloc(sizeof(*row));
	if (!row) {
		return NULL;
	}

	row->row_structure = MCC_SYMBOL_TABLE_ROW_STRUCTURE_ARRAY;
	row->array_size = array_size;
	row->row_type = type;
	row->name = strdup(name);
	if (!row->name) {
		free(row);
		return NULL;
	}
	row->node = node;
	row->prev_row = NULL;
	row->next_row = NULL;
	row->scope = NULL;
	row->child_scope = NULL;

	return row;
}

void mcc_symbol_table_delete_row(struct mcc_symbol_table_row *row)
{
	assert(row);

	// if row has a child scope delete that scope
	if (row->child_scope) {
		mcc_symbol_table_delete_all_scopes(row->child_scope);
	}

	// rearrange pointer structure
	if (!row->prev_row && row->next_row) {
		row->next_row->prev_row = NULL;
	}
	if (row->prev_row && !row->next_row) {
		row->prev_row->next_row = NULL;
	}
	if (row->prev_row && row->next_row) {
		row->next_row->prev_row = row->prev_row;
		row->prev_row->next_row = row->next_row;
	}

	// free row
	free(row->name);
	free(row);
}

void mcc_symbol_table_delete_all_rows(struct mcc_symbol_table_row *head)
{
	assert(head);

	if (head) {
		while (head->next_row) {
			mcc_symbol_table_delete_row(head->next_row);
		}
		mcc_symbol_table_delete_row(head);
	}

	return;
}

void mcc_symbol_table_row_append_child_scope(struct mcc_symbol_table_row *row, struct mcc_symbol_table_scope *child)
{
	assert(row);
	assert(child);

	if (!row->child_scope) {
		row->child_scope = child;
		child->parent_row = row;

		return;
	} else {
		struct mcc_symbol_table_scope *last_child = row->child_scope;

		while (last_child->next_scope) {
			last_child = last_child->next_scope;
		}
		last_child->next_scope = child;
		child->parent_row = row;
	}

	return;
}

// --------------------------------------------------------- Symbol Table scope

struct mcc_symbol_table_scope *mcc_symbol_table_new_scope()
{
	struct mcc_symbol_table_scope *scope = malloc(sizeof(*scope));
	if (!scope) {
		return NULL;
	}

	scope->head = NULL;
	scope->parent_row = NULL;
	scope->next_scope = NULL;
	scope->prev_scope = NULL;

	return scope;
}

struct mcc_symbol_table_row *mcc_symbol_table_scope_get_last_row(struct mcc_symbol_table_scope *scope)
{
	assert(scope);

	if (!scope->head) {
		return NULL;
	}

	struct mcc_symbol_table_row *row = scope->head;
	while (row->next_row) {
		row = row->next_row;
	}
	return row;
}

void mcc_symbol_table_scope_append_row(struct mcc_symbol_table_scope *scope, struct mcc_symbol_table_row *row)
{
	assert(scope);
	assert(row);

	struct mcc_symbol_table_row **head_ref = &scope->head;
	struct mcc_symbol_table_row *last_row = *head_ref;
	row->scope = scope;

	if (!*head_ref) {
		*head_ref = row;
		return;
	}
	while (last_row->next_row) {
		last_row = last_row->next_row;
	}
	last_row->next_row = row;
	row->prev_row = last_row;
	return;
}

void mcc_symbol_table_delete_scope(struct mcc_symbol_table_scope *scope)
{
	assert(scope);

	// delete rows
	if (scope->head) {
		mcc_symbol_table_delete_all_rows(scope->head);
	}

	free(scope);
}

void mcc_symbol_table_delete_all_scopes(struct mcc_symbol_table_scope *head)
{
	assert(head);

	struct mcc_symbol_table_scope *tmp = head;

	if (head) {
		while (head) {
			tmp = head->next_scope;
			mcc_symbol_table_delete_scope(head);
			head = tmp;
		}
	}

	return;
}

// ------------------------------------------------------- Symbol Table

struct mcc_symbol_table *mcc_symbol_table_new_table()
{
	struct mcc_symbol_table *table = malloc(sizeof(*table));
	if (!table) {
		return NULL;
	}

	table->head = NULL;

	return table;
}

void mcc_symbol_table_insert_scope(struct mcc_symbol_table *table, struct mcc_symbol_table_scope *scope)
{
	assert(table);
	assert(scope);

	struct mcc_symbol_table_scope **head_ref = &table->head;
	struct mcc_symbol_table_scope *last_scope = *head_ref;

	if (!*head_ref) {
		*head_ref = scope;
		return;
	}
	while (last_scope->next_scope) {
		last_scope = last_scope->next_scope;
	}
	last_scope->next_scope = scope;
	return;
}

// insert a new scope to the given table. Returns 0 on success
int insert_new_scope(struct mcc_symbol_table *table)
{
	assert(table);

	struct mcc_symbol_table_scope *scope = mcc_symbol_table_new_scope();
	if (!scope) {
		return 1;
	}

	mcc_symbol_table_insert_scope(table, scope);
	return 0;
}

void mcc_symbol_table_delete_table(struct mcc_symbol_table *table)
{
	assert(table);

	if (table->head) {
		mcc_symbol_table_delete_all_scopes(table->head);
	}

	free(table);
}

// --------------------------------------------------------------- traversing AST and create symbol table

// Create a row based on a given declaration at the end of a given scope, returns 0 on success.
static int create_row_declaration(struct mcc_ast_declaration *declaration, struct mcc_symbol_table_scope *scope)
{
	assert(scope);
	assert(declaration);

	struct mcc_symbol_table_row *row = NULL;

	switch (declaration->declaration_type) {
	case MCC_AST_DECLARATION_TYPE_VARIABLE:
		row = mcc_symbol_table_new_row_variable(declaration->variable_identifier->identifier_name,
		                                        convert_enum(declaration->variable_type->type_value),
		                                        &(declaration->node));
		break;
	case MCC_AST_DECLARATION_TYPE_ARRAY:
		row = mcc_symbol_table_new_row_array(
		    declaration->array_identifier->identifier_name, declaration->array_size->i_value,
		    convert_enum(declaration->array_type->type_value), &(declaration->node));
		break;
	}
	if (!row) {
		return 1;
	}
	mcc_symbol_table_scope_append_row(scope, row);
	declaration->row = row;
	return 0;
}

// Create rows of the function parameters of a given function definition, returns 0 on success.
static int create_rows_function_parameters(struct mcc_ast_function_definition *function_definition,
                                           struct mcc_symbol_table_row *row)
{
	assert(function_definition);
	assert(row);

	struct mcc_symbol_table_scope *child_scope = mcc_symbol_table_new_scope();
	if (!child_scope) {
		return 1;
	}
	mcc_symbol_table_row_append_child_scope(row, child_scope);

	if (!function_definition->parameters->is_empty) {
		struct mcc_ast_parameters *parameters = function_definition->parameters;

		do {
			if (create_row_declaration(parameters->declaration, child_scope)) {
				return 1;
			}
			parameters = parameters->next_parameters;
		} while (parameters);
	}

	return 0;
}

static struct mcc_symbol_table_row *create_pseudo_row(struct mcc_symbol_table_scope *scope)
{
	assert(scope);

	if (!scope->head) {
		struct mcc_symbol_table_row *row =
		    mcc_symbol_table_new_row_variable("-", MCC_SYMBOL_TABLE_ROW_TYPE_PSEUDO, NULL);
		if (!row) {
			return NULL;
		}
		mcc_symbol_table_scope_append_row(scope, row);
	}
	return scope->head;
}

static struct mcc_symbol_table_scope *append_child_scope_to_last_row(struct mcc_symbol_table_scope *scope)
{
	assert(scope);

	struct mcc_symbol_table_row *row = mcc_symbol_table_scope_get_last_row(scope);
	// if row == NULL create pseudo row
	if (!row) {
		row = create_pseudo_row(scope);
		// if still row == NULL malloc failed
		if (!row) {
			return NULL;
		}
	}
	struct mcc_symbol_table_scope *new_scope = mcc_symbol_table_new_scope();
	if (!new_scope) {
		return NULL;
	}
	mcc_symbol_table_row_append_child_scope(row, new_scope);

	return new_scope;
}

// Creates rows of a compound statement at its next compound statements, returns 0 on success
static int create_rows_compound_statement(struct mcc_ast_compound_statement *compound_stmt,
                                          struct mcc_symbol_table_scope *scope)
{
	assert(compound_stmt);
	assert(scope);

	if (compound_stmt->statement) {
		if (create_rows_statement(compound_stmt->statement, scope)) {
			return 1;
		}
	}

	while (compound_stmt->next_compound_statement) {
		compound_stmt = compound_stmt->next_compound_statement;
		if (create_rows_statement(compound_stmt->statement, scope)) {
			return 1;
		}
	}
	return 0;
}

// Links the expression of an assignment in the AST with the next declaration or pseudo row in the symbol table.
// Returns 0 on success.
static int link_pointer_assignment(struct mcc_ast_assignment *assignment, struct mcc_symbol_table_scope *scope)
{
	assert(assignment);
	assert(scope);

	struct mcc_symbol_table_row *row = mcc_symbol_table_scope_get_last_row(scope);

	if (!row) {
		row = create_pseudo_row(scope);
		// if still row == NULL malloc failed
		if (!row) {
			return 1;
		}
	}

	int exit_code = 0;
	assignment->row = row;

	switch (assignment->assignment_type) {
	case MCC_AST_ASSIGNMENT_TYPE_VARIABLE:
		exit_code += link_pointer_expression(assignment->variable_assigned_value, scope);
		break;
	case MCC_AST_ASSIGNMENT_TYPE_ARRAY:
		exit_code += link_pointer_expression(assignment->array_assigned_value, scope);
		exit_code += link_pointer_expression(assignment->array_index, scope);
		break;
	}
	return exit_code;
}

// Links expressions of an argument with next declaration or pseudo row, returns 0 on success.
static int link_pointer_arguments(struct mcc_ast_arguments *arguments, struct mcc_symbol_table_scope *scope)
{
	assert(arguments);
	assert(scope);

	if (!arguments->expression) {
		return 0;
	}
	do {
		if (link_pointer_expression(arguments->expression, scope)) {
			return 1;
		}
		arguments = arguments->next_arguments;
	} while (arguments);

	return 0;
}

// Links an expression with the next declaration or pseudo row, returns 0 on success.
static int link_pointer_expression(struct mcc_ast_expression *expression, struct mcc_symbol_table_scope *scope)
{
	assert(expression);
	assert(scope);

	struct mcc_symbol_table_row *row = mcc_symbol_table_scope_get_last_row(scope);
	if (!row) {
		row = create_pseudo_row(scope);
		// if still row == NULL malloc failed
		if (!row) {
			return 1;
		}
	}

	int exit_code = 0;
	switch (expression->type) {
	case MCC_AST_EXPRESSION_TYPE_LITERAL:
		// do nothing
		break;
	case MCC_AST_EXPRESSION_TYPE_BINARY_OP:
		exit_code = link_pointer_expression(expression->lhs, scope);
		exit_code += link_pointer_expression(expression->rhs, scope);
		break;
	case MCC_AST_EXPRESSION_TYPE_PARENTH:
		exit_code = link_pointer_expression(expression->expression, scope);
		break;
	case MCC_AST_EXPRESSION_TYPE_UNARY_OP:
		exit_code = link_pointer_expression(expression->child, scope);
		break;
	case MCC_AST_EXPRESSION_TYPE_VARIABLE:
		expression->variable_row = row;
		break;
	case MCC_AST_EXPRESSION_TYPE_ARRAY_ELEMENT:
		exit_code = link_pointer_expression(expression->index, scope);
		expression->array_row = row;
		break;
	case MCC_AST_EXPRESSION_TYPE_FUNCTION_CALL:
		expression->function_row = row;
		exit_code = link_pointer_arguments(expression->arguments, scope);
		break;
	}
	return exit_code;
}

// Links the expression of a return statement with the next declaration or pseudo row, returns 0 on success.
static int link_pointer_return(struct mcc_ast_statement *statement, struct mcc_symbol_table_scope *scope)
{
	assert(statement);
	assert(scope);

	if (statement->return_value) {
		if (link_pointer_expression(statement->return_value, scope)) {
			return 1;
		}
	}
	return 0;
}

// Creates a row or links the pointers of a given statement, returns 0 on success.
static int create_rows_statement(struct mcc_ast_statement *statement, struct mcc_symbol_table_scope *scope)
{
	assert(statement);
	assert(scope);

	int exit_code = 0;
	struct mcc_symbol_table_scope *new_child = NULL;
	if (statement->type == MCC_AST_STATEMENT_TYPE_COMPOUND_STMT) {
		new_child = append_child_scope_to_last_row(scope);
		if (!new_child) {
			return 1;
		}
	}
	switch (statement->type) {
	case MCC_AST_STATEMENT_TYPE_DECLARATION:
		exit_code = create_row_declaration(statement->declaration, scope);
		break;
	case MCC_AST_STATEMENT_TYPE_IF_STMT:
		exit_code = link_pointer_expression(statement->if_condition, scope);
		exit_code += create_rows_statement(statement->if_on_true, scope);
		break;
	case MCC_AST_STATEMENT_TYPE_IF_ELSE_STMT:
		exit_code = link_pointer_expression(statement->if_else_condition, scope);
		exit_code += create_rows_statement(statement->if_else_on_true, scope);
		exit_code += create_rows_statement(statement->if_else_on_false, scope);
		break;
	case MCC_AST_STATEMENT_TYPE_WHILE:
		exit_code = link_pointer_expression(statement->while_condition, scope);
		exit_code += create_rows_statement(statement->while_on_true, scope);
		break;
	case MCC_AST_STATEMENT_TYPE_COMPOUND_STMT:
		exit_code = create_rows_compound_statement(statement->compound_statement, new_child);
		break;
	case MCC_AST_STATEMENT_TYPE_ASSIGNMENT:
		exit_code = link_pointer_assignment(statement->assignment, scope);
		break;
	case MCC_AST_STATEMENT_TYPE_EXPRESSION:
		exit_code = link_pointer_expression(statement->stmt_expression, scope);
		break;
	case MCC_AST_STATEMENT_TYPE_RETURN:
		exit_code = link_pointer_return(statement, scope);
		break;
	}
	return exit_code;
}

// Creates the row of the function body of a given function definition, returns 0 on success.
static int create_rows_function_body(struct mcc_ast_function_definition *function_definition,
                                     struct mcc_symbol_table_row *row)
{
	assert(function_definition);
	assert(row);

	if (!row->child_scope) {
		struct mcc_symbol_table_scope *child_scope = mcc_symbol_table_new_scope();
		if (!child_scope) {
			return 1;
		}
		mcc_symbol_table_row_append_child_scope(row, child_scope);
	}

	struct mcc_ast_compound_statement *compound_stmt = function_definition->compound_stmt;
	if (compound_stmt) {
		if (create_rows_compound_statement(compound_stmt, row->child_scope)) {
			return 1;
		}
	}
	return 0;
}

// Creates the row for a given function definition, returns 0 on success.
static int create_row_function_definition(struct mcc_ast_function_definition *function_definition,
                                          struct mcc_symbol_table *table)
{
	assert(function_definition);
	assert(table);

	if (!table->head) {
		if (insert_new_scope(table)) {
			return 1;
		}
	}

	struct mcc_symbol_table_row *row = NULL;

	switch (function_definition->type) {
	case INT:
		row = mcc_symbol_table_new_row_function(function_definition->identifier->identifier_name,
		                                        MCC_SYMBOL_TABLE_ROW_TYPE_INT, &(function_definition->node));
		break;
	case FLOAT:
		row = mcc_symbol_table_new_row_function(function_definition->identifier->identifier_name,
		                                        MCC_SYMBOL_TABLE_ROW_TYPE_FLOAT, &(function_definition->node));
		break;
	case STRING:
		row = mcc_symbol_table_new_row_function(function_definition->identifier->identifier_name,
		                                        MCC_SYMBOL_TABLE_ROW_TYPE_STRING, &(function_definition->node));
		break;
	case BOOL:
		row = mcc_symbol_table_new_row_function(function_definition->identifier->identifier_name,
		                                        MCC_SYMBOL_TABLE_ROW_TYPE_BOOL, &(function_definition->node));
		break;
	case VOID:
		row = mcc_symbol_table_new_row_function(function_definition->identifier->identifier_name,
		                                        MCC_SYMBOL_TABLE_ROW_TYPE_VOID, &(function_definition->node));
		break;
	}
	if (!row) {
		return 1;
	}
	mcc_symbol_table_scope_append_row(table->head, row);
	if (create_rows_function_parameters(function_definition, row)) {
		return 1;
	}
	if (create_rows_function_body(function_definition, row)) {
		return 1;
	}
	return 0;
}

// Check if there is a declaration of the given name in the symbol table above (including) the given row and returns
// the row, otherwise NULL. However, checks on function level are not done.
struct mcc_symbol_table_row *mcc_symbol_table_check_upwards_for_declaration(const char *wanted_name,
                                                                            struct mcc_symbol_table_row *start_row)
{
	assert(wanted_name);
	assert(start_row);

	struct mcc_symbol_table_row *row = start_row;
	struct mcc_symbol_table_scope *scope = row->scope;

	if (strcmp(wanted_name, row->name) == 0) {
		return row;
	}

	while (scope->parent_row) {
		if (strcmp(wanted_name, row->name) == 0) {
			return row;
		}

		while (row->prev_row) {
			row = row->prev_row;

			if (strcmp(wanted_name, row->name) == 0) {
				return row;
			}
		}
		row = scope->parent_row;
		scope = row->scope;
	}
	return NULL;
}

// Checks if there is function declaration with the given name in the symbol table and returns the row, otherwise NULL
struct mcc_symbol_table_row *mcc_symbol_table_check_for_function_declaration(const char *wanted_name,
                                                                             struct mcc_symbol_table_row *start_row)
{
	assert(wanted_name);
	assert(start_row);

	struct mcc_symbol_table_row *row = start_row;
	struct mcc_symbol_table_scope *scope = row->scope;

	// go to top level
	while (scope->parent_row) {
		row = scope->parent_row;
		scope = row->scope;
	}

	// iterate through top level
	row = scope->head;
	if (!row) {
		return NULL;
	}
	do {
		if (strcmp(wanted_name, row->name) == 0) {
			return row;
		}
		row = row->next_row;
	} while (row);

	return NULL;
}

// inserts the corresponding rows of a ast program into a given table
static struct mcc_symbol_table *create_program(struct mcc_ast_program *program, struct mcc_symbol_table *table)
{
	assert(program);
	assert(table);

	if (program->function) {

		struct mcc_ast_function_definition *function = program->function;
		if (create_row_function_definition(function, table)) {
			mcc_symbol_table_delete_table(table);
			return NULL;
		}

		while (program->next_function) {
			program = program->next_function;
			function = program->function;
			if (create_row_function_definition(function, table)) {
				mcc_symbol_table_delete_table(table);
				return NULL;
			}
		}
	}

	return table;
}

// Creates the symbol table to the given program. Returns NULL if error occured.
struct mcc_symbol_table *mcc_symbol_table_create(struct mcc_ast_program *program)
{
	assert(program);

	// Add builtin functions before creating symbol table
	if (!mcc_ast_add_built_ins(program)) {
		return NULL;
	}
	struct mcc_symbol_table *table = mcc_symbol_table_new_table();
	if (!table) {
		return NULL;
	}

	table = create_program(program, table);

	return table;
}
