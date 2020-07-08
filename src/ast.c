#include "mcc/ast.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

// ---------------------------------------------------------------- Expressions

struct mcc_ast_expression *mcc_ast_new_expression_literal(struct mcc_ast_literal *literal)
{
	if (!literal)
		return NULL;

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
	if (!rhs || !lhs)
		return NULL;

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
	if (!expression)
		return NULL;

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
	if (!child)
		return NULL;

	struct mcc_ast_expression *expr = malloc(sizeof(*expr));
	if (!expr) {
		return NULL;
	}

	expr->type = MCC_AST_EXPRESSION_TYPE_UNARY_OP;
	expr->u_op = u_op;
	expr->child = child;
	return expr;
}

struct mcc_ast_expression *mcc_ast_new_expression_variable(struct mcc_ast_identifier *identifier)
{

	if (!identifier)
		return NULL;

	struct mcc_ast_expression *expr = malloc(sizeof(*expr));
	if (!expr) {
		return NULL;
	}

	expr->type = MCC_AST_EXPRESSION_TYPE_VARIABLE;
	expr->identifier = identifier;

	return expr;
}

struct mcc_ast_expression *mcc_ast_new_expression_array_element(struct mcc_ast_identifier *identifier,
                                                                struct mcc_ast_expression *index)
{

	if (!identifier || !index)
		return NULL;

	struct mcc_ast_expression *expr = malloc(sizeof(*expr));
	if (!expr) {
		return NULL;
	}

	expr->type = MCC_AST_EXPRESSION_TYPE_ARRAY_ELEMENT;
	expr->array_identifier = identifier;
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
	if (!expression)
		return;

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

struct mcc_ast_type *mcc_ast_new_type(enum mcc_ast_types type)
{
	struct mcc_ast_type *newtype = malloc(sizeof(*newtype));
	if (!newtype) {
		return NULL;
	}
	newtype->type_value = type;
	return newtype;
}

void mcc_ast_delete_type(struct mcc_ast_type *type)
{
	free(type);
}

// ------------------------------------------------------------------
// Declarations

struct mcc_ast_declaration *mcc_ast_new_variable_declaration(enum mcc_ast_types type,
                                                             struct mcc_ast_identifier *identifier)
{

	if (!identifier)
		return NULL;

	struct mcc_ast_declaration *decl = malloc(sizeof(*decl));
	if (!decl) {
		return NULL;
	}

	struct mcc_ast_type *newtype = mcc_ast_new_type(type);

	if (!newtype) {
		free(decl);
		return NULL;
	}

	decl->variable_type = newtype;
	decl->variable_identifier = identifier;
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

struct mcc_ast_declaration *mcc_ast_new_array_declaration(enum mcc_ast_types type,
                                                          struct mcc_ast_literal *size,
                                                          struct mcc_ast_identifier *identifier)
{

	if (!identifier || !size)
		return NULL;

	struct mcc_ast_declaration *array_decl = malloc(sizeof(*array_decl));
	if (!array_decl) {
		return NULL;
	}

	struct mcc_ast_type *newtype = mcc_ast_new_type(type);
	if (!newtype) {
		free(array_decl);
		return NULL;
	}

	array_decl->array_type = newtype;
	array_decl->array_identifier = identifier;
	array_decl->array_size = size;
	array_decl->declaration_type = MCC_AST_DECLARATION_TYPE_ARRAY;

	return array_decl;
}

void mcc_ast_delete_array_declaration(struct mcc_ast_declaration *array_decl)
{
	if (!array_decl)
		return;
	mcc_ast_delete_identifier(array_decl->array_identifier);
	mcc_ast_delete_type(array_decl->array_type);
	mcc_ast_delete_literal(array_decl->array_size);
	free(array_decl);
}

void mcc_ast_delete_declaration(struct mcc_ast_declaration *decl)
{
	if (!decl)
		return;
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

struct mcc_ast_assignment *mcc_ast_new_variable_assignment(struct mcc_ast_identifier *identifier,
                                                           struct mcc_ast_expression *assigned_value)
{
	if (!identifier || !assigned_value)
		return NULL;
	struct mcc_ast_assignment *assignment = malloc(sizeof(*assignment));
	if (assignment == NULL) {
		return NULL;
	}
	assignment->variable_identifier = identifier;
	assignment->variable_assigned_value = assigned_value;
	assignment->assignment_type = MCC_AST_ASSIGNMENT_TYPE_VARIABLE;
	return assignment;
}

struct mcc_ast_assignment *mcc_ast_new_array_assignment(struct mcc_ast_identifier *identifier,
                                                        struct mcc_ast_expression *index,
                                                        struct mcc_ast_expression *assigned_value)
{

	if (!index || !identifier || !assigned_value)
		return NULL;

	struct mcc_ast_assignment *assignment = malloc(sizeof(*assignment));
	if (!assignment) {
		return NULL;
	}
	assignment->array_identifier = identifier;
	assignment->array_assigned_value = assigned_value;
	assignment->array_index = index;
	assignment->assignment_type = MCC_AST_ASSIGNMENT_TYPE_ARRAY;
	return assignment;
}

void mcc_ast_delete_assignment(struct mcc_ast_assignment *assignment)
{
	if (!assignment)
		return;
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
	if (!assignment)
		return;
	mcc_ast_delete_identifier(assignment->variable_identifier);
	mcc_ast_delete_expression(assignment->variable_assigned_value);
	free(assignment);
}

void mcc_ast_delete_array_assignment(struct mcc_ast_assignment *assignment)
{
	if (!assignment)
		return;
	mcc_ast_delete_identifier(assignment->array_identifier);
	mcc_ast_delete_expression(assignment->array_assigned_value);
	mcc_ast_delete_expression(assignment->array_index);
	free(assignment);
}

// ------------------------------------------------------------------ Identifier

struct mcc_ast_identifier *mcc_ast_new_identifier(char *identifier)
{
	if (!identifier)
		return NULL;

	struct mcc_ast_identifier *expr = malloc(sizeof(*expr));
	if (!expr) {
		return NULL;
	}

	expr->identifier_name = identifier;

	return expr;
}

void mcc_ast_delete_identifier(struct mcc_ast_identifier *identifier)
{
	if (!identifier)
		return;
	if (identifier->identifier_name)
		free(identifier->identifier_name);
	free(identifier);
}

// -------------------------------------------------------------------  Statements

struct mcc_ast_statement *mcc_ast_new_statement_if_stmt(struct mcc_ast_expression *condition,
                                                        struct mcc_ast_statement *on_true)
{
	if (!condition || !on_true)
		return NULL;

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
	if (!condition || !on_true || !on_false)
		return NULL;

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
	if (!expression)
		return NULL;

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
	if (!condition || !on_true)
		return NULL;

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
	if (!declaration)
		return NULL;

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
	if (!assignment)
		return NULL;

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
		if (!expression)
			return NULL;
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
	if (!compound_statement)
		return NULL;

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
	if (!statement)
		return;

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
		if (!statement)
			return NULL;
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
	if (!compound_statement)
		return;
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
	if (!value)
		return NULL;
	struct mcc_ast_literal *lit = malloc(sizeof(*lit));
	if (!lit) {
		return NULL;
	}

	char *string_no_quotes = mcc_remove_quotes_from_string(value);
	if (!string_no_quotes) {
		free(lit);
		return NULL;
	}

	lit->type = MCC_AST_LITERAL_TYPE_STRING;
	lit->string_value = string_no_quotes;

	return lit;
}

char *mcc_remove_quotes_from_string(char *string)
{

	assert(string);
	char *intermediate = (char *)malloc((strlen(string) - 1) * sizeof(char));
	if (!intermediate)
		return NULL;
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
	if (!literal)
		return;
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
	if (!identifier)
		return NULL;

	struct mcc_ast_function_definition *function_definition = malloc(sizeof(*function_definition));
	if (!function_definition) {
		return NULL;
	}

	function_definition->type = VOID;
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
	if (!identifier)
		return NULL;

	struct mcc_ast_function_definition *function_definition = malloc(sizeof(*function_definition));
	if (!function_definition) {
		return NULL;
	}

	function_definition->type = type;
	function_definition->identifier = identifier;
	function_definition->parameters = parameters;
	function_definition->compound_stmt = compound_statement;

	return function_definition;
}

void mcc_ast_delete_function_definition(struct mcc_ast_function_definition *function_definition)
{
	if (!function_definition)
		return;
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
	if (!function_definition)
		return NULL;

	struct mcc_ast_program *program = malloc(sizeof(*program));
	if (!program)
		return NULL;

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

struct mcc_ast_program *mcc_ast_new_empty_program(char *name)
{
	struct mcc_ast_program *program = malloc(sizeof(*program));
	if (!program)
		return NULL;

	program->function = NULL;
	program->has_next_function = false;
	program->next_function = NULL;
	program->node.sloc.start_line = 0;
	program->node.sloc.start_col = 0;
	program->node.sloc.end_line = 0;
	program->node.sloc.end_col = 0;
	program->node.sloc.filename = name;

	return program;
}

void mcc_ast_delete_program(struct mcc_ast_program *program)
{
	if (!program)
		return;

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
		if (!declaration)
			return NULL;
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
	if (!parameters)
		return;
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
		if (!expression)
			return NULL;
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
	if (!arguments)
		return;

	if (arguments->has_next_expression == true) {
		mcc_ast_delete_arguments(arguments->next_arguments);
	}
	if (!(arguments->is_empty)) {
		mcc_ast_delete_expression(arguments->expression);
	}
	free(arguments);
}

// ------------------------------------------------------------------- Add and remove built_ins

int mcc_ast_add_built_ins(struct mcc_ast_program *program)
{
	assert(program);

	const char input[] = "void print(string str){} 	  \
						  void print_nl(){} 		  \
						  void print_int(int a){} 	  \
						  void print_float(float b){} \
						  int read_int(){return 0;}   \
						  float read_float(){return 0.0;}";
	struct mcc_parser_result result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
	if (result.status != MCC_PARSER_STATUS_OK) {
		return 1;
	}

	if (!program->function) {
		program->function = (&result)->program->function;
		program->has_next_function = true;
		program->next_function = (&result)->program->next_function;
		(&result)->program->function = NULL;
		free((&result)->program);
	} else if (!program->next_function) {
		program->next_function = (&result)->program;
		program->has_next_function = true;
		(&result)->program = NULL;
	} else {
		struct mcc_ast_program *program_iter = program->next_function;
		while (program_iter->next_function) {
			program_iter = program_iter->next_function;
		}
		program_iter->next_function = (&result)->program;
		program_iter->has_next_function = true;
		(&result)->program = NULL;
	}

	return 0;
}

static void remove_function(struct mcc_ast_program *program, struct mcc_ast_program *previous)
{
	assert(program);
	assert(previous);
	assert(previous->next_function == program);
	previous->has_next_function = program->has_next_function;
	previous->next_function = program->next_function;
	program->has_next_function = false;
	program->next_function = NULL;
	mcc_ast_delete_program(program);
}

static bool name_is_built_in(char *function_name)
{
	if (strcmp(function_name, "print") == 0) {
		return true;
	} else if (strcmp(function_name, "print_nl") == 0) {
		return true;
	} else if (strcmp(function_name, "print_int") == 0) {
		return true;
	} else if (strcmp(function_name, "print_float") == 0) {
		return true;
	} else if (strcmp(function_name, "read_int") == 0) {
		return true;
	} else if (strcmp(function_name, "read_float") == 0) {
		return true;
	} else {
		return false;
	}
}

struct mcc_ast_program *mcc_ast_remove_built_ins(struct mcc_ast_program *program)
{
	assert(program);
	struct mcc_ast_program *head;
	head = program;
	struct mcc_ast_program *previous;

	// Remove all built_ins from the beginning of the AST.
	while (name_is_built_in(program->function->identifier->identifier_name)) {
		previous = program;
		program = program->next_function;
		previous->has_next_function = false;
		previous->next_function = NULL;
		mcc_ast_delete_program(previous);
	}

	head = program;
	previous = program;
	program = previous->next_function;

	// Remove all intermediate built_ins from the AST
	while (program) {
		if (name_is_built_in(program->function->identifier->identifier_name)) {
			remove_function(program, previous);
			program = previous->next_function;
			continue;
		}
		previous = program;
		program = program->next_function;
	}
	return head;
}

// ------------------------------------------------------------------- Transforming the complete AST

// Remove everything but one function from the AST
struct mcc_parser_result *mcc_ast_limit_result_to_function(struct mcc_parser_result *result, char *wanted_function_name)
{

	// INPUT: 	- pointer to struct mcc_parser_result that is allocated on the heap
	//			- string that contains the name of the function that the result should be limited to
	// RETURN: 	- pointer to struct mcc_parser_result that is allocated on the heap and contains only
	//			  the function that was specified by the input string
	//			- returns NULL in case of runtime errors (in that case the input result won't have been
	// deleted)
	// RECOMMENDED USE: call it like this: 		ptr = limit_result_to_function_scope(ptr,function_name)

	// New struct mcc_parser_result that will be returned
	struct mcc_parser_result *new_result = malloc(sizeof(struct mcc_parser_result));
	if (!new_result) {
		return NULL;
	}

	// Set default values of the mcc_parser_result struct
	new_result->status = MCC_PARSER_STATUS_OK;
	new_result->entry_point = MCC_PARSER_ENTRY_POINT_PROGRAM;

	// Decleare pointer that will be iterated over and initialize it with the first function of the input
	struct mcc_ast_program *cur_program = result->program;

	// Set up two pointers to keep track of the found function and its predecessor
	struct mcc_ast_program *right_function = NULL;
	struct mcc_ast_program *predecessor = NULL;

	// Set up boolean to keep track of wether the function was encountered yet
	bool found_function = false;

	// Check if wanted function is the toplevel function
	if (strcmp(wanted_function_name, cur_program->function->identifier->identifier_name) == 0) {
		right_function = cur_program;
		found_function = true;
	}

	// Iterate over the rest of the tree
	struct mcc_ast_program *intermediate = cur_program;
	while (cur_program->has_next_function) {
		intermediate = cur_program;
		cur_program = cur_program->next_function;

		// check if wanted function is found
		if (strcmp(wanted_function_name, cur_program->function->identifier->identifier_name) == 0) {
			if (!found_function) {
				free(new_result);
				return NULL;
			} else {
				right_function = cur_program;
				predecessor = intermediate;
				found_function = true;
			}
		}
	}

	// Check if the function was found
	if (found_function == true) {
		// Delete all nodes followed by the found function
		if (right_function->has_next_function) {
			mcc_ast_delete_program(right_function->next_function);
		}
		// Tell the predecessor of the found function to forget about its successor
		if (predecessor != NULL) {
			predecessor->has_next_function = false;
			predecessor->next_function = NULL;
		}
		// Set up the new result with the found function
		new_result->program = right_function;
		right_function->has_next_function = false;
		right_function->next_function = NULL;
		// Delete the previous AST up to including the predecessor
		if (predecessor != NULL) {
			mcc_ast_delete_result(result);
		}

		return new_result;
	} else {
		free(new_result);
		return NULL;
	}
}

// Merge an array of parser results into one AST
struct mcc_parser_result *mcc_ast_merge_results(struct mcc_parser_result *array, int size)
{

	struct mcc_ast_program *cur_prog = NULL;

	// Iterate over array
	for (int i = 0; i < size - 1; i++) {
		// Iterate over programs within result
		cur_prog = array[i].program;
		while (cur_prog->has_next_function) {
			cur_prog = cur_prog->next_function;
		}
		cur_prog->has_next_function = true;
		cur_prog->next_function = array[i + 1].program;
	}

	return array;
}
