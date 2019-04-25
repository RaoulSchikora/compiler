#include "mcc/ast.h"

#include <assert.h>
#include <stdlib.h>

// included for debugging:

#include <stdio.h>

// ---------------------------------------------------------------- Expressions

struct mcc_ast_expression *mcc_ast_new_expression_literal(struct mcc_ast_literal *literal)
{
	assert(literal);

	struct mcc_ast_expression *expr = malloc(sizeof(*expr));
	if (!expr) {
		return NULL;
	}

	expr->type = MCC_AST_EXPRESSION_TYPE_LITERAL;
	expr->literal = literal;
	return expr;
}

struct mcc_ast_expression *mcc_ast_new_expression_binary_op(enum mcc_ast_binary_op op,
                                                            struct mcc_ast_expression *lhs,
                                                            struct mcc_ast_expression *rhs)
{
	assert(lhs);
	assert(rhs);

	struct mcc_ast_expression *expr = malloc(sizeof(*expr));
	if (!expr) {
		return NULL;
	}

	expr->type = MCC_AST_EXPRESSION_TYPE_BINARY_OP;
	expr->op = op;
	expr->lhs = lhs;
	expr->rhs = rhs;
	return expr;
}

struct mcc_ast_expression *mcc_ast_new_expression_parenth(struct mcc_ast_expression *expression)
{
	assert(expression);

	struct mcc_ast_expression *expr = malloc(sizeof(*expr));
	if (!expr) {
		return NULL;
	}

	expr->type = MCC_AST_EXPRESSION_TYPE_PARENTH;
	expr->expression = expression;
	return expr;
}

struct mcc_ast_expression *mcc_ast_new_expression_unary_op(enum mcc_ast_unary_op u_op, struct mcc_ast_expression *child)
{
	assert(child);

	struct mcc_ast_expression *expr = malloc(sizeof(*expr));
	if (!expr) {
		return NULL;
	}

	expr->type = MCC_AST_EXPRESSION_TYPE_UNARY_OP;
	expr->u_op = u_op;
	expr->child = child;
	return expr;
}

struct mcc_ast_expression *mcc_ast_new_expression_variable(char *identifier)
{

	assert(identifier);

	struct mcc_ast_expression *expr = malloc(sizeof(*expr));
	if (!expr) {
		return NULL;
	}

	struct mcc_ast_identifier *id = mcc_ast_new_identifier(identifier);

	expr->type = MCC_AST_EXPRESSION_TYPE_VARIABLE;
	expr->identifier = id;

	return expr;
}

struct mcc_ast_expression *mcc_ast_new_expression_array_element(char *identifier, struct mcc_ast_expression *index)
{

	assert(identifier);
	assert(index);

	struct mcc_ast_expression *expr = malloc(sizeof(*expr));
	if (!expr) {
		return NULL;
	}

	struct mcc_ast_identifier *id = mcc_ast_new_identifier(identifier);

	expr->type = MCC_AST_EXPRESSION_TYPE_ARRAY_ELEMENT;
	expr->array_identifier = id;
	expr->index = index;

	return expr;
}

struct mcc_ast_expression *mcc_ast_new_expression_function_call(struct mcc_ast_identifier *identifier,
                                                                struct mcc_ast_arguments *arguments)
{
	assert(identifier);

	struct mcc_ast_expression *expr = malloc(sizeof(*expr));
	if (!expr) {
		return NULL;
	}

	expr->type = MCC_AST_EXPRESSION_TYPE_FUNCTION_CALL;
	expr->function_identifier = identifier;
	expr->arguments = arguments;

	return expr;
}

void mcc_ast_delete_expression(struct mcc_ast_expression *expression)
{
	assert(expression);

	switch (expression->type) {
	case MCC_AST_EXPRESSION_TYPE_LITERAL:
		mcc_ast_delete_literal(expression->literal);
		break;

	case MCC_AST_EXPRESSION_TYPE_BINARY_OP:
		mcc_ast_delete_expression(expression->lhs);
		mcc_ast_delete_expression(expression->rhs);
		break;

	case MCC_AST_EXPRESSION_TYPE_PARENTH:
		mcc_ast_delete_expression(expression->expression);
		break;

	case MCC_AST_EXPRESSION_TYPE_UNARY_OP:
		mcc_ast_delete_expression(expression->child);
		break;

	case MCC_AST_EXPRESSION_TYPE_VARIABLE:
		mcc_ast_delete_identifier(expression->identifier);
		break;

	case MCC_AST_EXPRESSION_TYPE_ARRAY_ELEMENT:
		mcc_ast_delete_identifier(expression->array_identifier);
		mcc_ast_delete_expression(expression->index);
		break;

	case MCC_AST_EXPRESSION_TYPE_FUNCTION_CALL:
		mcc_ast_delete_identifier(expression->function_identifier);
		mcc_ast_delete_arguments(expression->arguments);
		break;
	}

	free(expression);
}

// ------------------------------------------------------------------ Types

void mcc_ast_delete_type(struct mcc_ast_type *type)
{
	assert(type);
	free(type);
}

struct mcc_ast_type *mcc_ast_new_type(enum mcc_ast_types type)
{
	struct mcc_ast_type *newtype = malloc(sizeof(*newtype));
	if (!newtype) {
		return NULL;
	}
	newtype->type_value = type;
	return newtype;
}

// ------------------------------------------------------------------
// Declarations

struct mcc_ast_declaration *mcc_ast_new_variable_declaration(enum mcc_ast_types type, char *identifier)
{

	assert(identifier);

	struct mcc_ast_declaration *decl = malloc(sizeof(*decl));
	if (!decl) {
		return NULL;
	}

	struct mcc_ast_identifier *id = mcc_ast_new_identifier(identifier);

	struct mcc_ast_type *newtype = mcc_ast_new_type(type);

	decl->variable_type = newtype;
	decl->variable_identifier = id;
	decl->declaration_type = MCC_AST_DECLARATION_TYPE_VARIABLE;

	return decl;
}

void mcc_ast_delete_variable_declaration(struct mcc_ast_declaration *decl)
{
	assert(decl);
	mcc_ast_delete_identifier(decl->variable_identifier);
	mcc_ast_delete_type(decl->variable_type);
	free(decl);
}

struct mcc_ast_declaration *
mcc_ast_new_array_declaration(enum mcc_ast_types type, struct mcc_ast_literal *size, char *identifier)
{

	assert(identifier);
	assert(size);

	struct mcc_ast_declaration *array_decl = malloc(sizeof(*array_decl));
	if (!array_decl) {
		return NULL;
	}

	struct mcc_ast_identifier *id = mcc_ast_new_identifier(identifier);

	struct mcc_ast_type *newtype = mcc_ast_new_type(type);

	array_decl->array_type = newtype;
	array_decl->array_identifier = id;
	array_decl->array_size = size;
	array_decl->declaration_type = MCC_AST_DECLARATION_TYPE_ARRAY;

	return array_decl;
}

void mcc_ast_delete_array_declaration(struct mcc_ast_declaration *array_decl)
{
	assert(array_decl);
	mcc_ast_delete_identifier(array_decl->array_identifier);
	mcc_ast_delete_type(array_decl->array_type);
	mcc_ast_delete_literal(array_decl->array_size);
	free(array_decl);
}

void mcc_ast_delete_declaration(struct mcc_ast_declaration *decl)
{
	assert(decl);
	switch (decl->declaration_type) {
	case MCC_AST_DECLARATION_TYPE_VARIABLE:
		mcc_ast_delete_variable_declaration(decl);
		break;
	case MCC_AST_DECLARATION_TYPE_ARRAY:
		mcc_ast_delete_array_declaration(decl);
		break;
	}
}

// ------------------------------------------------------------------
// Assignments

struct mcc_ast_assignment *mcc_ast_new_variable_assignment(char *identifier, struct mcc_ast_expression *assigned_value)
{
	assert(identifier);
	assert(assigned_value);
	struct mcc_ast_assignment *assignment = malloc(sizeof(*assignment));
	if (assignment == NULL) {
		return NULL;
	}
	assignment->variable_identifier = mcc_ast_new_identifier(identifier);
	assignment->variable_assigned_value = assigned_value;
	assignment->assignment_type = MCC_AST_ASSIGNMENT_TYPE_VARIABLE;
	return assignment;
}

struct mcc_ast_assignment *mcc_ast_new_array_assignment(char *identifier,
                                                        struct mcc_ast_expression *index,
                                                        struct mcc_ast_expression *assigned_value)
{

	assert(index);
	assert(identifier);
	assert(assigned_value);
	struct mcc_ast_assignment *assignment = malloc(sizeof(*assignment));
	if (assignment == NULL) {
		return NULL;
	}
	assignment->array_identifier = mcc_ast_new_identifier(identifier);
	assignment->array_assigned_value = assigned_value;
	assignment->array_index = index;
	assignment->assignment_type = MCC_AST_ASSIGNMENT_TYPE_ARRAY;
	return assignment;
}

void mcc_ast_delete_assignment(struct mcc_ast_assignment *assignment)
{
	assert(assignment);
	switch (assignment->assignment_type) {
	case MCC_AST_ASSIGNMENT_TYPE_VARIABLE:
		mcc_ast_delete_variable_assignment(assignment);
		break;
	case MCC_AST_ASSIGNMENT_TYPE_ARRAY:
		mcc_ast_delete_array_assignment(assignment);
		break;
	}
}

void mcc_ast_delete_variable_assignment(struct mcc_ast_assignment *assignment)
{
	assert(assignment);
	mcc_ast_delete_identifier(assignment->variable_identifier);
	mcc_ast_delete_expression(assignment->variable_assigned_value);
	free(assignment);
}

void mcc_ast_delete_array_assignment(struct mcc_ast_assignment *assignment)
{
	assert(assignment);
	mcc_ast_delete_identifier(assignment->array_identifier);
	mcc_ast_delete_expression(assignment->array_assigned_value);
	mcc_ast_delete_expression(assignment->array_index);
	free(assignment);
}

// ------------------------------------------------------------------ Identifier

struct mcc_ast_identifier *mcc_ast_new_identifier(char *identifier)
{
	assert(identifier);

	struct mcc_ast_identifier *expr = malloc(sizeof(*expr));
	if (!expr) {
		return NULL;
	}

	expr->identifier_name = identifier;

	return expr;
}

void mcc_ast_delete_identifier(struct mcc_ast_identifier *identifier)
{
	assert(identifier);
	assert(identifier->identifier_name);
	free(identifier->identifier_name);
	free(identifier);
}

// -------------------------------------------------------------------
// Statements

struct mcc_ast_statement *mcc_ast_new_statement_if_stmt(struct mcc_ast_expression *condition,
                                                        struct mcc_ast_statement *on_true)
{
	assert(condition);
	assert(on_true);

	struct mcc_ast_statement *statement = malloc(sizeof(*statement));
	if (!statement) {
		return NULL;
	}

	statement->type = MCC_AST_STATEMENT_TYPE_IF_STMT;
	statement->if_condition = condition;
	statement->if_on_true = on_true;

	return statement;
}

struct mcc_ast_statement *mcc_ast_new_statement_if_else_stmt(struct mcc_ast_expression *condition,
                                                             struct mcc_ast_statement *on_true,
                                                             struct mcc_ast_statement *on_false)
{
	assert(condition);
	assert(on_true);
	assert(on_false);

	struct mcc_ast_statement *statement = malloc(sizeof(*statement));
	if (!statement) {
		return NULL;
	}

	statement->type = MCC_AST_STATEMENT_TYPE_IF_ELSE_STMT;
	statement->if_else_condition = condition;
	statement->if_else_on_true = on_true;
	statement->if_else_on_false = on_false;

	return statement;
}

struct mcc_ast_statement *mcc_ast_new_statement_expression(struct mcc_ast_expression *expression)
{
	assert(expression);

	struct mcc_ast_statement *statement = malloc(sizeof(*statement));
	if (!statement) {
		return NULL;
	}

	statement->type = MCC_AST_STATEMENT_TYPE_EXPRESSION;
	statement->stmt_expression = expression;

	return statement;
}

struct mcc_ast_statement *mcc_ast_new_statement_while(struct mcc_ast_expression *condition,
                                                      struct mcc_ast_statement *on_true)
{
	assert(condition);
	assert(on_true);

	struct mcc_ast_statement *statement = malloc(sizeof(*statement));
	if (!statement) {
		return NULL;
	}

	statement->type = MCC_AST_STATEMENT_TYPE_WHILE;
	statement->while_condition = condition;
	statement->while_on_true = on_true;

	return statement;
}

struct mcc_ast_statement *mcc_ast_new_statement_declaration(struct mcc_ast_declaration *declaration)
{
	assert(declaration);

	struct mcc_ast_statement *statement = malloc(sizeof(*statement));
	if (!statement) {
		return NULL;
	}

	statement->type = MCC_AST_STATEMENT_TYPE_DECLARATION;
	statement->declaration = declaration;

	return statement;
}

struct mcc_ast_statement *mcc_ast_new_statement_assignment(struct mcc_ast_assignment *assignment)
{
	assert(assignment);

	struct mcc_ast_statement *statement = malloc(sizeof(*statement));
	if (!statement) {
		return NULL;
	}

	statement->type = MCC_AST_STATEMENT_TYPE_ASSIGNMENT;
	statement->assignment = assignment;

	return statement;
}

struct mcc_ast_statement *mcc_ast_new_statement_return(bool is_empty_return, struct mcc_ast_expression *expression)
{
	if (!is_empty_return) {
		assert(expression);
	}
	struct mcc_ast_statement *statement = malloc(sizeof(*statement));
	if (!statement) {
		return NULL;
	}

	statement->type = MCC_AST_STATEMENT_TYPE_RETURN;
	statement->is_empty_return = is_empty_return;

	if (is_empty_return) {
		statement->return_value = NULL;
	} else {
		statement->return_value = expression;
	}

	return statement;
}

struct mcc_ast_statement *mcc_ast_new_statement_compound_stmt(struct mcc_ast_compound_statement *compound_statement)
{
	assert(compound_statement);

	struct mcc_ast_statement *statement = malloc(sizeof(*statement));
	if (!statement) {
		return NULL;
	}

	statement->type = MCC_AST_STATEMENT_TYPE_COMPOUND_STMT;
	statement->compound_statement = compound_statement;

	return statement;
}

void mcc_ast_delete_statement(struct mcc_ast_statement *statement)
{
	assert(statement);

	switch (statement->type) {
	case MCC_AST_STATEMENT_TYPE_IF_STMT:
		mcc_ast_delete_expression(statement->if_condition);
		mcc_ast_delete_statement(statement->if_on_true);
		break;
	case MCC_AST_STATEMENT_TYPE_IF_ELSE_STMT:
		mcc_ast_delete_expression(statement->if_else_condition);
		mcc_ast_delete_statement(statement->if_else_on_true);
		mcc_ast_delete_statement(statement->if_else_on_false);
		break;
	case MCC_AST_STATEMENT_TYPE_EXPRESSION:
		mcc_ast_delete_expression(statement->stmt_expression);
		break;
	case MCC_AST_STATEMENT_TYPE_WHILE:
		mcc_ast_delete_expression(statement->while_condition);
		mcc_ast_delete_statement(statement->while_on_true);
		break;
	case MCC_AST_STATEMENT_TYPE_DECLARATION:
		mcc_ast_delete_declaration(statement->declaration);
		break;
	case MCC_AST_STATEMENT_TYPE_ASSIGNMENT:
		mcc_ast_delete_assignment(statement->assignment);
		break;
	case MCC_AST_STATEMENT_TYPE_RETURN:
		if (statement->is_empty_return) {
			break;
		} else {
			mcc_ast_delete_expression(statement->return_value);
			break;
		}
	case MCC_AST_STATEMENT_TYPE_COMPOUND_STMT:
		mcc_ast_delete_compound_statement(statement->compound_statement);
	}
	free(statement);
}

// ------------------------------------------------------------------- Compound
// Statement

struct mcc_ast_compound_statement *mcc_ast_new_compound_stmt(bool is_empty,
                                                             struct mcc_ast_statement *statement,
                                                             struct mcc_ast_compound_statement *next_compound_stmt)
{
	if (is_empty != true) {
		assert(statement);
	}

	struct mcc_ast_compound_statement *compound_statement = malloc(sizeof(*compound_statement));
	if (!compound_statement) {
		return NULL;
	}
	if (is_empty != true) {
		compound_statement->statement = statement;
	} else {
		compound_statement->statement = NULL;
	}
	if (next_compound_stmt == NULL) {
		compound_statement->has_next_statement = false;
		compound_statement->next_compound_statement = NULL;
	} else {
		compound_statement->has_next_statement = true;
		compound_statement->next_compound_statement = next_compound_stmt;
	}
	compound_statement->is_empty = is_empty;
	return compound_statement;
}

void mcc_ast_delete_compound_statement(struct mcc_ast_compound_statement *compound_statement)
{
	assert(compound_statement);
	if (compound_statement->has_next_statement == true) {
		mcc_ast_delete_compound_statement(compound_statement->next_compound_statement);
	}
	if (!compound_statement->is_empty) {
		mcc_ast_delete_statement(compound_statement->statement);
	}
	free(compound_statement);
}

// ------------------------------------------------------------------- Literals

struct mcc_ast_literal *mcc_ast_new_literal_int(long value)
{

	struct mcc_ast_literal *lit = malloc(sizeof(*lit));
	if (!lit) {
		return NULL;
	}

	lit->type = MCC_AST_LITERAL_TYPE_INT;
	lit->i_value = value;
	return lit;
}

struct mcc_ast_literal *mcc_ast_new_literal_float(double value)
{
	struct mcc_ast_literal *lit = malloc(sizeof(*lit));
	if (!lit) {
		return NULL;
	}

	lit->type = MCC_AST_LITERAL_TYPE_FLOAT;
	lit->f_value = value;
	return lit;
}

struct mcc_ast_literal *mcc_ast_new_literal_string(char *value)
{
	struct mcc_ast_literal *lit = malloc(sizeof(*lit));
	if (!lit) {
		return NULL;
	}

	lit->type = MCC_AST_LITERAL_TYPE_STRING;
	lit->string_value = mcc_remove_quotes_from_string(value);

	return lit;
}

char *mcc_remove_quotes_from_string(char *string)
{

	assert(string);
	char *intermediate = (char *)malloc((strlen(string) - 1) * sizeof(char));
	strncpy(intermediate, string + 1, strlen(string) - 2);
	*(intermediate + strlen(string) - 2) = '\0';
	return intermediate;
}

struct mcc_ast_literal *mcc_ast_new_literal_bool(bool value)
{
	struct mcc_ast_literal *lit = malloc(sizeof(*lit));
	if (!lit) {
		return NULL;
	}

	lit->type = MCC_AST_LITERAL_TYPE_BOOL;
	lit->bool_value = value;
	return lit;
}

void mcc_ast_delete_literal(struct mcc_ast_literal *literal)
{
	assert(literal);
	if (literal->type == MCC_AST_LITERAL_TYPE_STRING) {
		free(literal->string_value);
	}
	free(literal);
}

// ---------------------------------------------------------------------
// Function Definition

struct mcc_ast_function_definition *mcc_ast_new_void_function_def(struct mcc_ast_identifier *identifier,
                                                                  struct mcc_ast_parameters *parameters,
                                                                  struct mcc_ast_compound_statement *compound_statement)
{
	assert(identifier);

	struct mcc_ast_function_definition *function_definition = malloc(sizeof(*function_definition));
	if (!function_definition) {
		return NULL;
	}

	function_definition->type = MCC_AST_FUNCTION_TYPE_VOID;
	function_definition->identifier = identifier;
	function_definition->parameters = parameters;
	function_definition->compound_stmt = compound_statement;

	return function_definition;
}

struct mcc_ast_function_definition *mcc_ast_new_type_function_def(enum mcc_ast_types type,
                                                                  struct mcc_ast_identifier *identifier,
                                                                  struct mcc_ast_parameters *parameters,
                                                                  struct mcc_ast_compound_statement *compound_statement)
{
	assert(identifier);

	struct mcc_ast_function_definition *function_definition = malloc(sizeof(*function_definition));
	if (!function_definition) {
		return NULL;
	}

	switch (type) {
	case INT:
		function_definition->type = MCC_AST_FUNCTION_TYPE_INT;
		break;
	case FLOAT:
		function_definition->type = MCC_AST_FUNCTION_TYPE_FLOAT;
		break;
	case STRING:
		function_definition->type = MCC_AST_FUNCTION_TYPE_STRING;
		break;
	case BOOL:
		function_definition->type = MCC_AST_FUNCTION_TYPE_BOOL;
		break;
	}
	function_definition->identifier = identifier;
	function_definition->parameters = parameters;
	function_definition->compound_stmt = compound_statement;

	return function_definition;
}

void mcc_ast_delete_function_definition(struct mcc_ast_function_definition *function_definition)
{
	assert(function_definition);
	mcc_ast_delete_identifier(function_definition->identifier);
	mcc_ast_delete_compound_statement(function_definition->compound_stmt);
	if (function_definition->parameters != NULL) {
		mcc_ast_delete_parameters(function_definition->parameters);
	}
	free(function_definition);
}

// --------------------------------------------------------------------- Program

struct mcc_ast_program *mcc_ast_new_program(struct mcc_ast_function_definition *function_definition,
                                            struct mcc_ast_program *next_program)
{
	assert(function_definition);

	struct mcc_ast_program *program = malloc(sizeof(*program));

	if (next_program == NULL) {
		program->has_next_function = false;
		program->next_function = NULL;
	} else {
		program->has_next_function = true;
		program->next_function = next_program;
	}
	program->function = function_definition;

	return program;
}

void mcc_ast_delete_program(struct mcc_ast_program *program)
{
	assert(program);

	if (program->has_next_function == false) {
		mcc_ast_delete_function_definition(program->function);
	} else {
		mcc_ast_delete_program(program->next_function);
		mcc_ast_delete_function_definition(program->function);
	}
	free(program);
}

// ---------------------------------------------------------------------
// Parameters

struct mcc_ast_parameters *mcc_ast_new_parameters(bool is_empty,
                                                  struct mcc_ast_declaration *declaration,
                                                  struct mcc_ast_parameters *next_parameters)
{
	if (!is_empty) {
		assert(declaration);
	}
	struct mcc_ast_parameters *parameters = malloc(sizeof(*parameters));

	if (!parameters) {
		return NULL;
	}

	parameters->is_empty = is_empty;

	if (next_parameters == NULL) {
		parameters->has_next_parameter = false;
		parameters->next_parameters = NULL;
	} else {
		parameters->has_next_parameter = true;
		parameters->next_parameters = next_parameters;
	}
	if (!is_empty) {
		parameters->declaration = declaration;
	} else {
		parameters->declaration = NULL;
	};
	return parameters;
}

void mcc_ast_delete_parameters(struct mcc_ast_parameters *parameters)
{
	assert(parameters);
	if (parameters->has_next_parameter == true) {
		mcc_ast_delete_parameters(parameters->next_parameters);
	}

	if (!(parameters->is_empty)) {
		mcc_ast_delete_declaration(parameters->declaration);
	}
	free(parameters);
}

// ---------------------------------------------------------------------
// Arguments

struct mcc_ast_arguments *
mcc_ast_new_arguments(bool is_empty, struct mcc_ast_expression *expression, struct mcc_ast_arguments *next_arguments)
{
	if (!is_empty) {
		assert(expression);
	}
	struct mcc_ast_arguments *arguments = malloc(sizeof(*arguments));
	if (!arguments) {
		return NULL;
	}

	if (next_arguments == NULL) {
		arguments->has_next_expression = false;
		arguments->next_arguments = NULL;
	} else {
		arguments->has_next_expression = true;
		arguments->next_arguments = next_arguments;
	}
	if (!is_empty) {
		arguments->expression = expression;
	} else {
		arguments->expression = NULL;
	}
	arguments->is_empty = is_empty;
	return arguments;
}

void mcc_ast_delete_arguments(struct mcc_ast_arguments *arguments)
{
	assert(arguments);

	if (arguments->has_next_expression == true) {
		mcc_ast_delete_arguments(arguments->next_arguments);
	}
	if (!(arguments->is_empty)) {
		mcc_ast_delete_expression(arguments->expression);
	}
	free(arguments);
}
