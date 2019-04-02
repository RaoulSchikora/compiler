#include <CuTest.h>

#include "mcc/ast.h"
#include "mcc/parser.h"

#include <stdio.h>

// Threshold for floating point comparisions.
static const double EPS = 1e-3;

void BinaryOp_1(CuTest *tc)
{
	const char input[] = "192 + 3.14";
	struct mcc_parser_result result = mcc_parse_string(input);

	CuAssertIntEquals(tc, MCC_PARSER_STATUS_OK, result.status);

	struct mcc_ast_expression *expr = result.expression;

	// root
	CuAssertIntEquals(tc, MCC_AST_EXPRESSION_TYPE_BINARY_OP, expr->type);
	CuAssertIntEquals(tc, MCC_AST_BINARY_OP_ADD, expr->op);

	// root -> lhs
	CuAssertIntEquals(tc, MCC_AST_EXPRESSION_TYPE_LITERAL, expr->lhs->type);

	// root -> lhs -> literal
	CuAssertIntEquals(tc, MCC_AST_LITERAL_TYPE_INT, expr->lhs->literal->type);
	CuAssertIntEquals(tc, 192, expr->lhs->literal->i_value);

	// root -> rhs
	CuAssertIntEquals(tc, MCC_AST_EXPRESSION_TYPE_LITERAL, expr->rhs->type);

	// root -> rhs -> literal
	CuAssertIntEquals(tc, MCC_AST_LITERAL_TYPE_FLOAT, expr->rhs->literal->type);
	CuAssertDblEquals(tc, 3.14, expr->rhs->literal->f_value, EPS);

	mcc_ast_delete(expr);
}

void BinaryOp_2(CuTest *tc)
{
	const char input[] = "192 - 3.14";
	struct mcc_parser_result result = mcc_parse_string(input);

	CuAssertIntEquals(tc, MCC_PARSER_STATUS_OK, result.status);

	struct mcc_ast_expression *expr = result.expression;

	// root
	CuAssertIntEquals(tc, MCC_AST_EXPRESSION_TYPE_BINARY_OP, expr->type);
	CuAssertIntEquals(tc, MCC_AST_BINARY_OP_SUB, expr->op);

	// root -> lhs
	CuAssertIntEquals(tc, MCC_AST_EXPRESSION_TYPE_LITERAL, expr->lhs->type);

	// root -> lhs -> literal
	CuAssertIntEquals(tc, MCC_AST_LITERAL_TYPE_INT, expr->lhs->literal->type);
	CuAssertIntEquals(tc, 192, expr->lhs->literal->i_value);

	// root -> rhs
	CuAssertIntEquals(tc, MCC_AST_EXPRESSION_TYPE_LITERAL, expr->rhs->type);

	// root -> rhs -> literal
	CuAssertIntEquals(tc, MCC_AST_LITERAL_TYPE_FLOAT, expr->rhs->literal->type);
	CuAssertDblEquals(tc, 3.14, expr->rhs->literal->f_value, EPS);

	mcc_ast_delete(expr);
}

void BinaryOp_3(CuTest *tc)
{
    const char input[] = "2 < 3";
    struct mcc_parser_result result = mcc_parse_string(input);

    CuAssertIntEquals(tc, MCC_PARSER_STATUS_OK, result.status);

    struct mcc_ast_expression *expr = result.expression;

    // root
    CuAssertIntEquals(tc, MCC_AST_EXPRESSION_TYPE_BINARY_OP, expr->type);
    CuAssertIntEquals(tc, MCC_AST_BINARY_OP_SMALLER, expr->op);

    // root -> lhs
    CuAssertIntEquals(tc, MCC_AST_EXPRESSION_TYPE_LITERAL, expr->lhs->type);

    // root -> lhs -> literal
    CuAssertIntEquals(tc, MCC_AST_LITERAL_TYPE_INT, expr->lhs->literal->type);
    CuAssertIntEquals(tc, 2, expr->lhs->literal->i_value);

    // root -> rhs
    CuAssertIntEquals(tc, MCC_AST_EXPRESSION_TYPE_LITERAL, expr->rhs->type);

    // root -> rhs -> literal
    CuAssertIntEquals(tc, MCC_AST_LITERAL_TYPE_INT, expr->lhs->literal->type);
    CuAssertIntEquals(tc, 3, expr->rhs->literal->i_value);

    mcc_ast_delete(expr);
}

void BinaryOp_4(CuTest *tc)
{
    const char input[] = "((true || false) && (true != false)) == true";
    struct mcc_parser_result result = mcc_parse_string(input);

    CuAssertIntEquals(tc, MCC_PARSER_STATUS_OK, result.status);

    struct mcc_ast_expression *expr = result.expression;

    // root
    CuAssertIntEquals(tc, MCC_AST_EXPRESSION_TYPE_BINARY_OP, expr->type);
    CuAssertIntEquals(tc, MCC_AST_BINARY_OP_EQUAL, expr->op);

    // root -> lhs
    CuAssertIntEquals(tc, MCC_AST_EXPRESSION_TYPE_PARENTH, expr->lhs->type);

    // root -> rhs
    CuAssertIntEquals(tc, MCC_AST_LITERAL_TYPE_BOOL, expr->rhs->literal->type);
    CuAssertTrue(tc, expr->rhs->literal->bool_value);

    struct mcc_ast_expression *subexpr1 = expr->lhs->expression;

    // root -> lhs -> subexpr1
    CuAssertIntEquals(tc, MCC_AST_EXPRESSION_TYPE_BINARY_OP, subexpr1->type);
    CuAssertIntEquals(tc, MCC_AST_BINARY_OP_CONJ, subexpr1->op);

    // root -> lhs -> subexpr1 -> lhs
    CuAssertIntEquals(tc, MCC_AST_EXPRESSION_TYPE_PARENTH, subexpr1->lhs->type);

    struct mcc_ast_expression *subexpr2 = subexpr1->lhs->expression;

    // subexpr2
    CuAssertIntEquals(tc, MCC_AST_EXPRESSION_TYPE_BINARY_OP, subexpr2->type);
    CuAssertIntEquals(tc, MCC_AST_BINARY_OP_DISJ, subexpr2->op);

    struct mcc_ast_expression *subexpr3 = subexpr1->rhs->expression;

    // subexpr3
    CuAssertIntEquals(tc, MCC_AST_EXPRESSION_TYPE_BINARY_OP, subexpr3->type);
    CuAssertIntEquals(tc, MCC_AST_BINARY_OP_NOTEQUAL, subexpr3->op);

    // root -> lhs -> subexpr1 -> lhs -> subexpr2 -> lhs -> literal
    CuAssertIntEquals(tc, MCC_AST_LITERAL_TYPE_BOOL, subexpr2->lhs->literal->type);
    CuAssertTrue(tc, subexpr2->lhs->literal->bool_value);
    // root -> lhs -> subexpr1 -> lhs -> subexpr2 -> rhs -> literal
    CuAssertIntEquals(tc, MCC_AST_LITERAL_TYPE_BOOL, subexpr2->rhs->literal->type);
    CuAssertTrue(tc, !subexpr2->rhs->literal->bool_value);

    // root -> lhs -> subexpr1 -> rhs -> subexpr3 -> lhs -> literal
    CuAssertIntEquals(tc, MCC_AST_LITERAL_TYPE_BOOL, subexpr3->lhs->literal->type);
    CuAssertTrue(tc, subexpr3->lhs->literal->bool_value);
    // root -> lhs -> subexpr1 -> rhs -> subexpr3 -> rhs -> literal
    CuAssertIntEquals(tc, MCC_AST_LITERAL_TYPE_BOOL, subexpr3->rhs->literal->type);
    CuAssertTrue(tc, !subexpr3->rhs->literal->bool_value);

    mcc_ast_delete(expr);
}

void BinaryPrecedenceAssociativity(CuTest *tc)
{
    const char input[] = "2 < 3 + 4 || true";
    struct mcc_parser_result result = mcc_parse_string(input);

    CuAssertIntEquals(tc, MCC_PARSER_STATUS_OK, result.status);

    struct mcc_ast_expression *expr = result.expression;

    // root
    CuAssertIntEquals(tc, MCC_AST_EXPRESSION_TYPE_BINARY_OP, expr->type);
    CuAssertIntEquals(tc, MCC_AST_BINARY_OP_DISJ, expr->op);

    struct mcc_ast_expression *subexpr1 = expr->lhs;

    // root -> lhs
    CuAssertIntEquals(tc, MCC_AST_EXPRESSION_TYPE_BINARY_OP, subexpr1->type);
    CuAssertIntEquals(tc, MCC_AST_BINARY_OP_SMALLER, subexpr1->op);

    // root -> rhs
    CuAssertIntEquals(tc, MCC_AST_LITERAL_TYPE_BOOL, expr->rhs->literal->type);
    CuAssertTrue(tc, expr->rhs->literal->bool_value);

    // root -> lhs -> lhs
    CuAssertIntEquals(tc, MCC_AST_LITERAL_TYPE_INT, subexpr1->lhs->literal->type);
    CuAssertIntEquals(tc, 2, subexpr1->lhs->literal->i_value);

    struct mcc_ast_expression *subexpr2 = subexpr1->rhs;

    // root -> lhs -> subexpr1 -> rhs ->subexpr2
    CuAssertIntEquals(tc, MCC_AST_EXPRESSION_TYPE_BINARY_OP, subexpr2->type);
    CuAssertIntEquals(tc, MCC_AST_BINARY_OP_ADD, subexpr2->op);

    // root -> lhs -> subexpr1 -> rhs -> subexpr2 -> lhs
    CuAssertIntEquals(tc, MCC_AST_LITERAL_TYPE_INT, subexpr2->lhs->literal->type);
    CuAssertIntEquals(tc, 3, subexpr2->lhs->literal->i_value);

    // root -> lhs -> subexpr1 -> rhs -> subexpr2 -> rhs
    CuAssertIntEquals(tc, MCC_AST_LITERAL_TYPE_INT, subexpr2->rhs->literal->type);
    CuAssertIntEquals(tc, 4, subexpr2->rhs->literal->i_value);

}

void Variable(CuTest *tc)
{
	const char input[] = "teststring";
	struct mcc_parser_result result = mcc_parse_string(input);

	CuAssertIntEquals(tc, MCC_PARSER_STATUS_OK, result.status);

	struct mcc_ast_expression *expr = result.expression;

	// root
	CuAssertIntEquals(tc, MCC_AST_EXPRESSION_TYPE_VARIABLE, expr->type);

	// root -> identifier
	CuAssertStrEquals(tc, "teststring", expr->identifier);

	mcc_ast_delete(expr);

}

void Array_Element(CuTest *tc){

	const char input[] = "test [2.3 + 3]";
	struct mcc_parser_result result = mcc_parse_string(input);

	CuAssertIntEquals(tc, MCC_PARSER_STATUS_OK, result.status);

	struct mcc_ast_expression *expr = result.expression;

	// root
	CuAssertIntEquals(tc, MCC_AST_EXPRESSION_TYPE_ARRAY_ELEMENT, expr->type);

	// root -> array_identifier
	CuAssertStrEquals(tc, "test", expr->array_identifier);

	// root -> index = Binary_op
	CuAssertIntEquals(tc, MCC_AST_EXPRESSION_TYPE_BINARY_OP,expr->index->type);
	CuAssertIntEquals(tc, MCC_AST_BINARY_OP_ADD,expr->index->op);


	// root -> index -> lhs
	CuAssertIntEquals(tc, MCC_AST_EXPRESSION_TYPE_LITERAL, expr->index->lhs->type);

	// root -> index -> lhs -> literal
	CuAssertIntEquals(tc, MCC_AST_LITERAL_TYPE_FLOAT, expr->index->lhs->literal->type);
	CuAssertDblEquals(tc, 2.3, expr->index->lhs->literal->f_value,EPS);

	// root -> index -> rhs
	CuAssertIntEquals(tc, MCC_AST_EXPRESSION_TYPE_LITERAL, expr->index->rhs->type);

	// root -> index -> rhs -> literal
	CuAssertIntEquals(tc, MCC_AST_LITERAL_TYPE_INT, expr->index->rhs->literal->type);
	CuAssertIntEquals(tc, 3, expr->index->rhs->literal->i_value);

    mcc_ast_delete(expr);
}

void NestedExpression_1(CuTest *tc)
{
	const char input[] = "42 * (192 + 3.14)";
	struct mcc_parser_result result = mcc_parse_string(input);

	CuAssertIntEquals(tc, MCC_PARSER_STATUS_OK, result.status);

	struct mcc_ast_expression *expr = result.expression;

	// root
	CuAssertIntEquals(tc, MCC_AST_EXPRESSION_TYPE_BINARY_OP, expr->type);
	CuAssertIntEquals(tc, MCC_AST_BINARY_OP_MUL, expr->op);

	// root -> lhs
	CuAssertIntEquals(tc, MCC_AST_EXPRESSION_TYPE_LITERAL, expr->lhs->type);

	// root -> lhs -> literal
	CuAssertIntEquals(tc, MCC_AST_LITERAL_TYPE_INT, expr->lhs->literal->type);
	CuAssertIntEquals(tc, 42, expr->lhs->literal->i_value);

	// root -> rhs
	CuAssertIntEquals(tc, MCC_AST_EXPRESSION_TYPE_PARENTH, expr->rhs->type);

	struct mcc_ast_expression *subexpr = expr->rhs->expression;

	// subexpr
	CuAssertIntEquals(tc, MCC_AST_EXPRESSION_TYPE_BINARY_OP, subexpr->type);
	CuAssertIntEquals(tc, MCC_AST_BINARY_OP_ADD, subexpr->op);

	// subexpr -> lhs
	CuAssertIntEquals(tc, MCC_AST_EXPRESSION_TYPE_LITERAL, subexpr->lhs->type);

	// subexpr -> lhs -> literal
	CuAssertIntEquals(tc, MCC_AST_LITERAL_TYPE_INT, subexpr->lhs->literal->type);
	CuAssertIntEquals(tc, 192, subexpr->lhs->literal->i_value);

	// subexpr -> rhs
	CuAssertIntEquals(tc, MCC_AST_EXPRESSION_TYPE_LITERAL, subexpr->rhs->type);

	// subexpr -> rhs -> literal
	CuAssertIntEquals(tc, MCC_AST_LITERAL_TYPE_FLOAT, subexpr->rhs->literal->type);
	CuAssertIntEquals(tc, 3.14, subexpr->rhs->literal->f_value);

	mcc_ast_delete(expr);
}

void MissingClosingParenthesis_1(CuTest *tc)
{
	// TODO: fix memory leak

	const char input[] = "(42";
	struct mcc_parser_result result = mcc_parse_string(input);

	CuAssertTrue(tc, MCC_PARSER_STATUS_OK != result.status);
	CuAssertTrue(tc, NULL == result.expression);
}

void SourceLocation_SingleLineColumn(CuTest *tc)
{
	const char input[] = "(42 + 192)";
	struct mcc_parser_result result = mcc_parse_string(input);

	CuAssertIntEquals(tc, MCC_PARSER_STATUS_OK, result.status);

	struct mcc_ast_expression *expr = result.expression;

	CuAssertIntEquals(tc, MCC_AST_EXPRESSION_TYPE_PARENTH, expr->type);
	CuAssertIntEquals(tc, 1, expr->node.sloc.start_col);

	CuAssertIntEquals(tc, MCC_AST_EXPRESSION_TYPE_BINARY_OP, expr->expression->type);
	CuAssertIntEquals(tc, 2, expr->expression->node.sloc.start_col);

	CuAssertIntEquals(tc, MCC_AST_LITERAL_TYPE_INT, expr->expression->lhs->literal->type);
	CuAssertIntEquals(tc, 2, expr->expression->lhs->literal->node.sloc.start_col);

	CuAssertIntEquals(tc, MCC_AST_LITERAL_TYPE_INT, expr->expression->rhs->literal->type);
	CuAssertIntEquals(tc, 7, expr->expression->rhs->literal->node.sloc.start_col);

	mcc_ast_delete(expr);
}


void UnaryOp_1(CuTest *tc)
{
    const char input[] = "-2";
    struct mcc_parser_result result = mcc_parse_string(input);

    CuAssertIntEquals(tc, MCC_PARSER_STATUS_OK, result.status);

    struct mcc_ast_expression *expr = result.expression;

    CuAssertIntEquals(tc, MCC_AST_EXPRESSION_TYPE_UNARY_OP, expr->type);
    CuAssertIntEquals(tc, MCC_AST_UNARY_OP_NEGATIV, expr->op);

    //root -> child
    CuAssertIntEquals(tc, MCC_AST_EXPRESSION_TYPE_LITERAL, expr->child->type);

    //root -> child -> literal
    CuAssertIntEquals(tc, MCC_AST_LITERAL_TYPE_INT, expr->child->literal->type);
    CuAssertIntEquals(tc, 2, expr->child->literal->i_value);

    mcc_ast_delete(expr);
}

void UnaryOp_2(CuTest *tc)
{
    const char input[] = "!false";
    struct mcc_parser_result result = mcc_parse_string(input);

    CuAssertIntEquals(tc, MCC_PARSER_STATUS_OK, result.status);

    struct mcc_ast_expression *expr = result.expression;

    CuAssertIntEquals(tc, MCC_AST_EXPRESSION_TYPE_UNARY_OP, expr->type);
    CuAssertIntEquals(tc, MCC_AST_UNARY_OP_NOT, expr->op);

    //root -> child
    CuAssertIntEquals(tc, MCC_AST_EXPRESSION_TYPE_LITERAL, expr->child->type);

    //root -> child -> literal
    CuAssertIntEquals(tc, MCC_AST_LITERAL_TYPE_BOOL, expr->child->literal->type);
    CuAssertTrue(tc, !expr->child->literal->bool_value);

    mcc_ast_delete(expr);
}

#define TESTS \
	TEST(BinaryOp_1) \
	TEST(BinaryOp_2) \
	TEST(BinaryOp_3) \
	TEST(BinaryOp_4) \
	TEST(BinaryPrecedenceAssociativity) \
	TEST(NestedExpression_1) \
	TEST(MissingClosingParenthesis_1) \
	TEST(SourceLocation_SingleLineColumn) \
	TEST(UnaryOp_1) \
	TEST(UnaryOp_2) \
	TEST(Variable) \
	TEST(Array_Element)

#include "main_stub.inc"
#undef TESTS
