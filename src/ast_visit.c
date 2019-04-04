#include "mcc/ast_visit.h"

#include <assert.h>

#define visit(node, callback, visitor) \
	do { \
		if (callback) { \
			(callback)(node, (visitor)->userdata); \
		} \
	} while (0)

#define visit_if(cond, node, callback, visitor) \
	do { \
		if (cond) { \
			visit(node, callback, visitor); \
		} \
	} while (0)

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

	}

	visit_if_post_order(expression, visitor->expression, visitor);
}

void mcc_ast_visit_statement(struct mcc_ast_statement *statement, struct mcc_ast_visitor *visitor)
{
	assert(statement);
	assert(visitor);

	visit_if_pre_order(statement, visitor->statement, visitor);

	switch(statement->type) {
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
	}
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
	}

	visit_if_post_order(literal, visitor->literal, visitor);
}

void mcc_ast_visit_declaration(struct mcc_ast_declaration *declaration, struct mcc_ast_visitor *visitor){
	assert(declaration);
	assert(visitor);


	switch(declaration->declaration_type){
		case MCC_AST_DECLARATION_TYPE_VARIABLE:
			visit_if_pre_order(declaration, visitor->variable_declaration, visitor);
			visit(declaration,visitor->variable_declaration,visitor);
			visit_if_post_order(declaration,visitor->variable_declaration,visitor);
			break;
		case MCC_AST_DECLARATION_TYPE_ARRAY:
			visit_if_pre_order(declaration, visitor->array_declaration, visitor);
			visit(declaration,visitor->array_declaration,visitor);
			visit_if_post_order(declaration,visitor->array_declaration,visitor);
			break;
	}

}

void mcc_ast_visit_assignment(struct mcc_ast_assignment *assignment, struct mcc_ast_visitor *visitor){
	assert(assignment);
	assert(visitor);


	switch(assignment->assignment_type){
		case MCC_AST_ASSIGNMENT_TYPE_VARIABLE:
			visit_if_pre_order(assignment, visitor->variable_assignment, visitor);
			visit(assignment,visitor->variable_assignment,visitor);
			visit_if_post_order(assignment,visitor->variable_assignment,visitor);
			break;
		case MCC_AST_ASSIGNMENT_TYPE_ARRAY:
			visit_if_pre_order(assignment, visitor->array_assignment, visitor);
			visit(assignment,visitor->array_assignment,visitor);
			visit_if_post_order(assignment,visitor->array_assignment,visitor);
			break;
	}


}

void mcc_ast_visit_type(struct mcc_ast_type *type, struct mcc_ast_visitor *visitor){
	assert(type);
	assert(visitor);

    visit(type,visitor->type,visitor);

}

void mcc_ast_visit_identifier(struct mcc_ast_identifier *identifier, struct mcc_ast_visitor *visitor){
	assert(identifier);
	assert(visitor);

	visit(identifier,visitor->identifier,visitor);

}
