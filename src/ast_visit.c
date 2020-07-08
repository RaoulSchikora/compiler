#include "mcc/ast_visit.h"

#include <assert.h>

#define visit(node, callback, visitor) \
	if (callback) { \
		(callback)(node, (visitor)->userdata); \
	}

#define visit_if(cond, node, callback, visitor) \
	if (cond) { \
		visit(node, callback, visitor); \
	}

#define visit_if_pre_order(node, callback, visitor) \
	visit_if((visitor)->order == MCC_AST_VISIT_PRE_ORDER, node, callback, visitor)

#define visit_if_post_order(node, callback, visitor) \
	visit_if((visitor)->order == MCC_AST_VISIT_POST_ORDER, node, callback, visitor)

void mcc_ast_visit_expression(struct mcc_ast_expression *expression, struct mcc_ast_visitor *visitor)
{
	assert(expression);
	assert(visitor);

	visit_if_pre_order(expression, visitor->expression, visitor);

	switch (expression->type) {
	case MCC_AST_EXPRESSION_TYPE_LITERAL:
		visit_if_pre_order(expression, visitor->expression_literal, visitor);
		mcc_ast_visit(expression->literal, visitor);
		visit_if_post_order(expression, visitor->expression_literal, visitor);
		break;

	case MCC_AST_EXPRESSION_TYPE_BINARY_OP:
		visit_if_pre_order(expression, visitor->expression_binary_op, visitor);
		mcc_ast_visit(expression->lhs, visitor);
		mcc_ast_visit(expression->rhs, visitor);
		visit_if_post_order(expression, visitor->expression_binary_op, visitor);
		break;

	case MCC_AST_EXPRESSION_TYPE_PARENTH:
		visit_if_pre_order(expression, visitor->expression_parenth, visitor);
		mcc_ast_visit(expression->expression, visitor);
		visit_if_post_order(expression, visitor->expression_parenth, visitor);
		break;

	case MCC_AST_EXPRESSION_TYPE_UNARY_OP:
		visit_if_pre_order(expression, visitor->expression_unary_op, visitor);
		mcc_ast_visit(expression->child, visitor);
		visit_if_post_order(expression, visitor->expression_unary_op, visitor);
		break;

	case MCC_AST_EXPRESSION_TYPE_VARIABLE:
		visit_if_pre_order(expression, visitor->expression_variable, visitor);
		mcc_ast_visit(expression->identifier, visitor);
		visit_if_post_order(expression, visitor->expression_variable, visitor);
		break;

	case MCC_AST_EXPRESSION_TYPE_ARRAY_ELEMENT:
		visit_if_pre_order(expression, visitor->expression_array_element, visitor);
		mcc_ast_visit(expression->array_identifier, visitor);
		mcc_ast_visit(expression->index, visitor);
		visit_if_post_order(expression, visitor->expression_array_element, visitor);
		break;
	case MCC_AST_EXPRESSION_TYPE_FUNCTION_CALL:
		visit_if_pre_order(expression, visitor->expression_function_call, visitor);
		mcc_ast_visit(expression->function_identifier, visitor);
		if (!expression->arguments->is_empty) {
			mcc_ast_visit(expression->arguments, visitor);
		}
		visit_if_post_order(expression, visitor->expression_function_call, visitor);
	}

	visit_if_post_order(expression, visitor->expression, visitor);
}

void mcc_ast_visit_statement(struct mcc_ast_statement *statement, struct mcc_ast_visitor *visitor)
{
	assert(statement);
	assert(visitor);

	visit_if_pre_order(statement, visitor->statement, visitor);

	switch (statement->type) {
	case MCC_AST_STATEMENT_TYPE_IF_STMT:
		visit_if_pre_order(statement, visitor->statement_if_stmt, visitor);
		mcc_ast_visit(statement->if_condition, visitor);
		mcc_ast_visit(statement->if_on_true, visitor);
		visit_if_post_order(statement, visitor->statement_if_stmt, visitor);
		break;
	case MCC_AST_STATEMENT_TYPE_IF_ELSE_STMT:
		visit_if_pre_order(statement, visitor->statement_if_else_stmt, visitor);
		mcc_ast_visit(statement->if_else_condition, visitor);
		mcc_ast_visit(statement->if_else_on_true, visitor);
		mcc_ast_visit(statement->if_else_on_false, visitor);
		visit_if_post_order(statement, visitor->statement_if_else_stmt, visitor);
		break;
	case MCC_AST_STATEMENT_TYPE_EXPRESSION:
		visit_if_pre_order(statement, visitor->statement_expression_stmt, visitor);
		mcc_ast_visit(statement->stmt_expression, visitor);
		visit_if_post_order(statement, visitor->statement_expression_stmt, visitor);
		break;
	case MCC_AST_STATEMENT_TYPE_WHILE:
		visit_if_pre_order(statement, visitor->statement_while, visitor);
		mcc_ast_visit(statement->while_condition, visitor);
		mcc_ast_visit(statement->while_on_true, visitor);
		visit_if_post_order(statement, visitor->statement_while, visitor);
		break;
	case MCC_AST_STATEMENT_TYPE_DECLARATION:
		visit_if_pre_order(statement, visitor->statement_declaration, visitor);
		mcc_ast_visit(statement->declaration, visitor);
		visit_if_post_order(statement, visitor->statement_declaration, visitor);
		break;
	case MCC_AST_STATEMENT_TYPE_ASSIGNMENT:
		visit_if_pre_order(statement, visitor->statement_assignment, visitor);
		mcc_ast_visit(statement->assignment, visitor);
		visit_if_post_order(statement, visitor->statement_assignment, visitor);
		break;
	case MCC_AST_STATEMENT_TYPE_RETURN:
		visit_if_pre_order(statement, visitor->statement_return, visitor);
		if (!(statement->is_empty_return)) {
			mcc_ast_visit(statement->return_value, visitor);
		}
		visit_if_post_order(statement, visitor->statement_return, visitor);
		break;
	case MCC_AST_STATEMENT_TYPE_COMPOUND_STMT:
		visit_if_pre_order(statement, visitor->statement_compound_stmt, visitor);
		mcc_ast_visit(statement->compound_statement, visitor);
		visit_if_post_order(statement, visitor->statement_compound_stmt, visitor);
		break;
	}

	visit_if_post_order(statement, visitor->statement, visitor);
}

void mcc_ast_visit_compound_statement(struct mcc_ast_compound_statement *compound_statement,
                                      struct mcc_ast_visitor *visitor)
{
	assert(compound_statement);
	assert(visitor);

	visit_if_pre_order(compound_statement, visitor->compound_statement, visitor);
	if (!compound_statement->is_empty) {
		mcc_ast_visit(compound_statement->statement, visitor);
	}
	if (compound_statement->has_next_statement) {
		mcc_ast_visit(compound_statement->next_compound_statement, visitor);
	}
	visit_if_post_order(compound_statement, visitor->compound_statement, visitor);
}

void mcc_ast_visit_literal(struct mcc_ast_literal *literal, struct mcc_ast_visitor *visitor)
{
	assert(literal);
	assert(visitor);

	visit_if_pre_order(literal, visitor->literal, visitor);

	switch (literal->type) {
	case MCC_AST_LITERAL_TYPE_INT:
		visit(literal, visitor->literal_int, visitor);
		break;
	case MCC_AST_LITERAL_TYPE_FLOAT:
		visit(literal, visitor->literal_float, visitor);
		break;
	case MCC_AST_LITERAL_TYPE_BOOL:
		visit(literal, visitor->literal_bool, visitor);
		break;
	case MCC_AST_LITERAL_TYPE_STRING:
		visit(literal, visitor->literal_string, visitor);
		break;
	}

	visit_if_post_order(literal, visitor->literal, visitor);
}

void mcc_ast_visit_declaration(struct mcc_ast_declaration *declaration, struct mcc_ast_visitor *visitor)
{
	assert(declaration);
	assert(visitor);

	switch (declaration->declaration_type) {
	case MCC_AST_DECLARATION_TYPE_VARIABLE:
		visit_if_pre_order(declaration, visitor->variable_declaration, visitor);
		mcc_ast_visit(declaration->variable_type, visitor);
		mcc_ast_visit(declaration->variable_identifier, visitor);
		visit_if_post_order(declaration, visitor->variable_declaration, visitor);
		break;
	case MCC_AST_DECLARATION_TYPE_ARRAY:
		visit_if_pre_order(declaration, visitor->array_declaration, visitor);
		mcc_ast_visit(declaration->array_type, visitor);
		mcc_ast_visit(declaration->array_size, visitor);
		mcc_ast_visit(declaration->array_identifier, visitor);
		visit_if_post_order(declaration, visitor->array_declaration, visitor);
		break;
	}
}

void mcc_ast_visit_assignment(struct mcc_ast_assignment *assignment, struct mcc_ast_visitor *visitor)
{
	assert(assignment);
	assert(visitor);

	switch (assignment->assignment_type) {
	case MCC_AST_ASSIGNMENT_TYPE_VARIABLE:
		visit_if_pre_order(assignment, visitor->variable_assignment, visitor);
		mcc_ast_visit(assignment->variable_identifier, visitor);
		mcc_ast_visit(assignment->variable_assigned_value, visitor);
		visit_if_post_order(assignment, visitor->variable_assignment, visitor);
		break;
	case MCC_AST_ASSIGNMENT_TYPE_ARRAY:
		visit_if_pre_order(assignment, visitor->array_assignment, visitor);
		mcc_ast_visit(assignment->array_identifier, visitor);
		mcc_ast_visit(assignment->array_index, visitor);
		mcc_ast_visit(assignment->array_assigned_value, visitor);
		visit_if_post_order(assignment, visitor->array_assignment, visitor);
		break;
	}
}

void mcc_ast_visit_type(struct mcc_ast_type *type, struct mcc_ast_visitor *visitor)
{
	assert(type);
	assert(visitor);

	visit(type, visitor->type, visitor);
}

void mcc_ast_visit_identifier(struct mcc_ast_identifier *identifier, struct mcc_ast_visitor *visitor)
{
	assert(identifier);
	assert(visitor);

	visit(identifier, visitor->identifier, visitor);
}

void mcc_ast_visit_function_definition(struct mcc_ast_function_definition *function_definition,
                                       struct mcc_ast_visitor *visitor)
{
	assert(function_definition);
	assert(visitor);

	visit_if_pre_order(function_definition, visitor->function_definition, visitor);
	mcc_ast_visit(function_definition->identifier, visitor);
	mcc_ast_visit(function_definition->parameters, visitor);
	mcc_ast_visit(function_definition->compound_stmt, visitor);
	visit_if_post_order(function_definition, visitor->function_definition, visitor);
}

void mcc_ast_visit_parameters(struct mcc_ast_parameters *parameters, struct mcc_ast_visitor *visitor)
{
	assert(parameters);
	assert(visitor);

	visit_if_pre_order(parameters, visitor->parameters, visitor);
	if (parameters->has_next_parameter) {
		mcc_ast_visit(parameters->next_parameters, visitor);
	}
	if (!(parameters->is_empty)) {
		mcc_ast_visit(parameters->declaration, visitor);
	}
	visit_if_post_order(parameters, visitor->parameters, visitor);
}

void mcc_ast_visit_arguments(struct mcc_ast_arguments *arguments, struct mcc_ast_visitor *visitor)
{
	assert(arguments);
	assert(visitor);

	visit_if_pre_order(arguments, visitor->arguments, visitor);
	mcc_ast_visit(arguments->expression, visitor);
	if (arguments->has_next_expression == true) {
		mcc_ast_visit(arguments->next_arguments, visitor);
	}
	visit_if_post_order(arguments, visitor->arguments, visitor);
}

void mcc_ast_visit_program(struct mcc_ast_program *program, struct mcc_ast_visitor *visitor)
{
	assert(program);
	assert(visitor);

	visit_if_pre_order(program, visitor->program, visitor);
	if (!program->function) {
		return;
	}
	mcc_ast_visit(program->function, visitor);
	if (program->has_next_function) {
		mcc_ast_visit(program->next_function, visitor);
	}
	visit_if_post_order(program, visitor->program, visitor);
}

