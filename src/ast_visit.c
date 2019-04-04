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
