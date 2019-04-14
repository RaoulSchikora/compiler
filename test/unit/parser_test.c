#include <CuTest.h>

#include "mcc/ast.h"
#include "mcc/parser.h"

#include <stdio.h>

// Threshold for floating point comparisions.
static const double EPS = 1e-3;

void BinaryOp_1(CuTest *tc)
{
	const char input[] = "192 + 3.14";
	struct mcc_parser_result result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_EXPRESSION);

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
	struct mcc_parser_result result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_EXPRESSION);

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
    struct mcc_parser_result result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_EXPRESSION);

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
    struct mcc_parser_result result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_EXPRESSION);

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
    struct mcc_parser_result result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_EXPRESSION);

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

	mcc_ast_delete(expr);

}

void Variable(CuTest *tc)
{
	const char input[] = "teststring";
	struct mcc_parser_result result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_EXPRESSION);

	CuAssertIntEquals(tc, MCC_PARSER_STATUS_OK, result.status);

	struct mcc_ast_expression *expr = result.expression;

	// root
	CuAssertIntEquals(tc, MCC_AST_EXPRESSION_TYPE_VARIABLE, expr->type);

	// root -> identifier -> identifier_name
	CuAssertStrEquals(tc, "teststring", expr->identifier->identifier_name);

	mcc_ast_delete(expr);

}

void Array_Element(CuTest *tc){

	const char input[] = "test [2.3 + 3]";
	struct mcc_parser_result result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_EXPRESSION);

	CuAssertIntEquals(tc, MCC_PARSER_STATUS_OK, result.status);

	struct mcc_ast_expression *expr = result.expression;

	// root
	CuAssertIntEquals(tc, MCC_AST_EXPRESSION_TYPE_ARRAY_ELEMENT, expr->type);

	// root -> array_identifier -> identifier_name
	CuAssertStrEquals(tc, "test", expr->array_identifier->identifier_name);

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

void VariableDeclaration(CuTest *tc){

	const char input[] = "float a";
	struct mcc_parser_result result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_DECLARATION);

	CuAssertIntEquals(tc, MCC_PARSER_STATUS_OK, result.status);


	struct mcc_ast_declaration *decl = result.declaration;

	// root -> declaration_type
	CuAssertIntEquals(tc, MCC_AST_DECLARATION_TYPE_VARIABLE,decl->declaration_type);

	// root -> identifier -> identifier_name
	CuAssertStrEquals(tc, "a", decl->variable_identifier->identifier_name);

	// root -> type -> type_value
	CuAssertIntEquals(tc, FLOAT, decl->variable_type->type_value);

	mcc_ast_delete(decl);
}

void ArrayDeclaration(CuTest *tc){
	const char input[] = "bool [13] my_array";
	struct mcc_parser_result result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_DECLARATION);


	CuAssertIntEquals(tc, MCC_PARSER_STATUS_OK, result.status);

	struct mcc_ast_declaration *array_decl = result.declaration;

	// root -> declaration_type
	CuAssertIntEquals(tc, MCC_AST_DECLARATION_TYPE_ARRAY,array_decl->declaration_type);

	// root -> identifier -> identifier_name
	CuAssertStrEquals(tc, "my_array", array_decl->array_identifier->identifier_name);

	// root -> type -> type_value
	CuAssertIntEquals(tc, BOOL, array_decl->array_type->type_value);

	// root -> size -> type
	CuAssertIntEquals(tc, MCC_AST_LITERAL_TYPE_INT,array_decl->array_size->type);

	// root -> size -> i_value
	CuAssertIntEquals(tc, 13,array_decl->array_size->i_value);

	mcc_ast_delete(array_decl);

}

void NestedExpression_1(CuTest *tc)
{
	const char input[] = "42 * (192 + 3.14)";
	struct mcc_parser_result result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_EXPRESSION);

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
	CuAssertDblEquals(tc, 3.14, subexpr->rhs->literal->f_value,EPS);

	mcc_ast_delete(expr);
}

void if_stmt_1(CuTest *tc)
{
	const char input[] = "if ( true ) 2 ;" ;
	struct mcc_parser_result result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_STATEMENT);

	CuAssertTrue(tc, MCC_PARSER_STATUS_OK == result.status);

	struct mcc_ast_statement *stmt = result.statement;

	// root
	CuAssertIntEquals(tc, MCC_AST_STATEMENT_TYPE_IF_STMT, stmt->type);

	// root -> cond
	CuAssertIntEquals(tc, MCC_AST_EXPRESSION_TYPE_LITERAL, stmt->if_condition->type);
	CuAssertTrue(tc, stmt->if_condition->literal->bool_value);

	// root -> on_true
	CuAssertIntEquals(tc, MCC_AST_EXPRESSION_TYPE_LITERAL, stmt->if_on_true->stmt_expression->type);
	CuAssertIntEquals(tc, 2, stmt->if_on_true->stmt_expression->literal->i_value);

	mcc_ast_delete(stmt);
}

void while_stmt(CuTest *tc)
{
	const char input[] = "while (true) 5;" ;
	struct mcc_parser_result result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_STATEMENT);

	CuAssertTrue(tc, MCC_PARSER_STATUS_OK == result.status);

	struct mcc_ast_statement *stmt = result.statement;

	// root
	CuAssertIntEquals(tc, MCC_AST_STATEMENT_TYPE_WHILE, stmt->type);

	// root -> cond
	CuAssertIntEquals(tc, MCC_AST_EXPRESSION_TYPE_LITERAL, stmt->if_condition->type);
	CuAssertTrue(tc, stmt->if_condition->literal->bool_value);

	// root -> on_true
	CuAssertIntEquals(tc, MCC_AST_EXPRESSION_TYPE_LITERAL, stmt->if_on_true->stmt_expression->type);
	CuAssertIntEquals(tc, 5, stmt->if_on_true->stmt_expression->literal->i_value);

	mcc_ast_delete(stmt);
}

void assign_stmt(CuTest *tc)
{
	const char input[] = "if (true) a = 1;";

	struct mcc_parser_result result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_STATEMENT);

	CuAssertTrue(tc, MCC_PARSER_STATUS_OK == result.status);

	struct mcc_ast_statement *stmt = result.statement;

	// root
	CuAssertIntEquals(tc, MCC_AST_STATEMENT_TYPE_IF_STMT, stmt->type);

	// root -> cond
	CuAssertIntEquals(tc, MCC_AST_EXPRESSION_TYPE_LITERAL, stmt->if_condition->type);
	CuAssertTrue(tc, stmt->if_condition->literal->bool_value);

	// root -> on_true
	CuAssertIntEquals(tc, MCC_AST_ASSIGNMENT_TYPE_VARIABLE, stmt->if_on_true->assignment->assignment_type);
	CuAssertIntEquals(tc, 1, stmt->if_on_true->assignment->variable_assigned_value->literal->i_value);
	CuAssertIntEquals(tc, MCC_AST_LITERAL_TYPE_INT, stmt->if_on_true->assignment->variable_assigned_value->literal->type);
	CuAssertStrEquals(tc, "a", stmt->if_on_true->assignment->variable_identifier->identifier_name);


	mcc_ast_delete(stmt);
}


void decl_stmt(CuTest *tc)
{
	const char input[] = "if (true) int [2] test;";

	struct mcc_parser_result result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_STATEMENT);

	CuAssertTrue(tc, MCC_PARSER_STATUS_OK == result.status);

	struct mcc_ast_statement *stmt = result.statement;

	// root
	CuAssertIntEquals(tc, MCC_AST_STATEMENT_TYPE_IF_STMT, stmt->type);

	// root -> cond
	CuAssertIntEquals(tc, MCC_AST_EXPRESSION_TYPE_LITERAL, stmt->if_condition->type);
	CuAssertTrue(tc, stmt->if_condition->literal->bool_value);

	// root -> on_true
	CuAssertIntEquals(tc, MCC_AST_DECLARATION_TYPE_ARRAY, stmt->if_on_true->declaration->declaration_type);
	CuAssertIntEquals(tc, MCC_AST_LITERAL_TYPE_INT, stmt->if_on_true->declaration->array_size->type);
	CuAssertIntEquals(tc, 2, stmt->if_on_true->declaration->array_size->i_value);
	CuAssertStrEquals(tc, "test", stmt->if_on_true->declaration->array_identifier->identifier_name);
	CuAssertIntEquals(tc, INT, stmt->if_on_true->declaration->array_type->type_value);


	mcc_ast_delete(stmt);
}

void ret_stmt(CuTest *tc)
{
    //TODO
/*	const char input[] = "return a;";

	struct mcc_parser_result result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_STATEMENT);

	CuAssertTrue(tc, MCC_PARSER_STATUS_OK == result.status);

	struct mcc_ast_statement *stmt = result.statement;

	// root
	CuAssertIntEquals(tc, MCC_AST_STATEMENT_TYPE_RETURN, stmt->type);
	CuAssertTrue(tc, !(stmt->is_empty_return));

	// root -> return_value

	CuAssertIntEquals(tc,  MCC_AST_EXPRESSION_TYPE_VARIABLE, stmt->return_value->type);
	CuAssertStrEquals(tc,  "a", stmt->return_value->identifier->identifier_name);

	mcc_ast_delete(stmt);*/
}

void MissingClosingParenthesis_1(CuTest *tc)
{
	const char input[] = "(42";
	struct mcc_parser_result result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_EXPRESSION);

	CuAssertTrue(tc, MCC_PARSER_STATUS_OK != result.status);
	CuAssertTrue(tc, NULL == result.expression);
}

void SourceLocation_SingleLineColumn(CuTest *tc)
{
	const char input[] = "(42 + 192)";
	struct mcc_parser_result result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_EXPRESSION);

	CuAssertIntEquals(tc, MCC_PARSER_STATUS_OK, result.status);

	struct mcc_ast_expression *expr = result.expression;
/*
	CuAssertIntEquals(tc, MCC_AST_EXPRESSION_TYPE_PARENTH, expr->type);
	CuAssertIntEquals(tc, 1, expr->node.sloc.start_col);

	CuAssertIntEquals(tc, MCC_AST_EXPRESSION_TYPE_BINARY_OP, expr->expression->type);
	CuAssertIntEquals(tc, 2, expr->expression->node.sloc.start_col);

	CuAssertIntEquals(tc, MCC_AST_LITERAL_TYPE_INT, expr->expression->lhs->literal->type);
	CuAssertIntEquals(tc, 2, expr->expression->lhs->literal->node.sloc.start_col);

	CuAssertIntEquals(tc, MCC_AST_LITERAL_TYPE_INT, expr->expression->rhs->literal->type);
	CuAssertIntEquals(tc, 7, expr->expression->rhs->literal->node.sloc.start_col);
*/
	mcc_ast_delete(expr);
}

void UnaryOp_1(CuTest *tc)
{
    const char input[] = "-2";
    struct mcc_parser_result result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_EXPRESSION);

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
    struct mcc_parser_result result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_EXPRESSION);

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

void StringLiteral(CuTest *tc)
{
    const char input[] = "\"hallo ich bin ein test string\"";
	struct mcc_parser_result result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_EXPRESSION);

	CuAssertIntEquals(tc, MCC_PARSER_STATUS_OK, result.status);

	struct mcc_ast_expression *expr = result.expression;

    //root -> type
    CuAssertIntEquals(tc, MCC_AST_EXPRESSION_TYPE_LITERAL, expr->type);

	//root -> literal
	CuAssertIntEquals(tc, MCC_AST_LITERAL_TYPE_STRING, expr->literal->type);
	CuAssertStrEquals(tc, "hallo ich bin ein test string", expr->literal->string_value);

	mcc_ast_delete(expr);
}

void VariableAssignment(CuTest *tc)
{
	const char input[] = "myVariable2 = 4.23";
	struct mcc_parser_result result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_EXPRESSION);

	CuAssertIntEquals(tc, MCC_PARSER_STATUS_OK, result.status);

	struct mcc_ast_assignment *variable_assignment = result.assignment;

	//root -> assignment_type
	CuAssertIntEquals(tc, MCC_AST_ASSIGNMENT_TYPE_VARIABLE, variable_assignment->assignment_type);

	//root -> identifier -> identifier_name
	CuAssertStrEquals(tc, "myVariable2", variable_assignment->variable_identifier->identifier_name);

	// root -> assigned_value -> type
	CuAssertIntEquals(tc, MCC_AST_EXPRESSION_TYPE_LITERAL, variable_assignment->variable_assigned_value->type);

	// root -> assigned_value -> literal -> type
	CuAssertIntEquals(tc, MCC_AST_LITERAL_TYPE_FLOAT, variable_assignment->variable_assigned_value->literal->type);

	// root -> assigned_value -> literal -> f_value
	CuAssertDblEquals(tc, 4.23, variable_assignment->variable_assigned_value->literal->f_value,EPS);

	mcc_ast_delete(variable_assignment);

}


void ArrayAssignment(CuTest *tc)
{
	const char input[] = "myVariable [12] = true";
	struct mcc_parser_result result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_EXPRESSION);

	CuAssertIntEquals(tc, MCC_PARSER_STATUS_OK, result.status);

	struct mcc_ast_assignment *array_assignment = result.assignment;

	// root -> assignment_type
	CuAssertIntEquals(tc, MCC_AST_ASSIGNMENT_TYPE_ARRAY, array_assignment->assignment_type);

	// root -> identifier -> identifier_name
	CuAssertStrEquals(tc, "myVariable", array_assignment->array_identifier->identifier_name);

	// root -> assigned_value -> type
	CuAssertIntEquals(tc, MCC_AST_EXPRESSION_TYPE_LITERAL, array_assignment->array_assigned_value->type);

	// root -> assigned_value -> literal -> type
	CuAssertIntEquals(tc, MCC_AST_LITERAL_TYPE_BOOL, array_assignment->array_assigned_value->literal->type);

	// root -> assigned_value -> literal -> bool_value
	CuAssertTrue(tc, array_assignment->array_assigned_value->literal->bool_value);

	// root -> index -> literal -> type
	CuAssertIntEquals(tc, MCC_AST_LITERAL_TYPE_INT, array_assignment->array_index->literal->type);

	// root -> index -> literal -> f_value
	CuAssertIntEquals(tc, 12, array_assignment->array_index->literal->i_value);

	mcc_ast_delete(array_assignment);
}

void CompoundStatement(CuTest *tc)
{
	const char input[] = "{int a; a = 1;}";
	struct mcc_parser_result result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_STATEMENT);

	CuAssertIntEquals(tc, MCC_PARSER_STATUS_OK, result.status);
	CuAssertIntEquals(tc, MCC_PARSER_ENTRY_POINT_COMPOUND_STATEMENT, result.entry_point);

	struct mcc_ast_compound_statement *compound_statement = result.compound_statement;

	// root -> statement
	CuAssertIntEquals(tc, compound_statement->statement->type, MCC_AST_STATEMENT_TYPE_DECLARATION);
	CuAssertIntEquals(tc, compound_statement->statement->declaration->declaration_type, MCC_AST_DECLARATION_TYPE_VARIABLE);
	CuAssertIntEquals(tc, compound_statement->statement->declaration->variable_type->type_value, INT);
	CuAssertStrEquals(tc, compound_statement->statement->declaration->variable_identifier->identifier_name, "a" );

	// root -> next_compound_statement

	CuAssertTrue(tc, compound_statement->has_next_statement);
	CuAssertIntEquals(tc, compound_statement->next_compound_statement->statement->type, MCC_AST_STATEMENT_TYPE_ASSIGNMENT);
	CuAssertIntEquals(tc, compound_statement->next_compound_statement->statement->assignment->assignment_type, MCC_AST_ASSIGNMENT_TYPE_VARIABLE);
	CuAssertStrEquals(tc, compound_statement->next_compound_statement->statement->assignment->variable_identifier->identifier_name, "a" );

	// root -> next_compound_statement -> assignment -> assigned_value

	CuAssertIntEquals(tc, compound_statement->next_compound_statement->statement->assignment->variable_assigned_value->type, MCC_AST_EXPRESSION_TYPE_LITERAL);
	CuAssertIntEquals(tc, compound_statement->next_compound_statement->statement->assignment->variable_assigned_value->literal->type, MCC_AST_LITERAL_TYPE_INT);
	CuAssertIntEquals(tc, compound_statement->next_compound_statement->statement->assignment->variable_assigned_value->literal->i_value, 1);

	mcc_ast_delete(compound_statement);

}


void FunctionCallArguments(CuTest *tc){

	const char input[] = "func1(a, h)";
	struct mcc_parser_result result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_EXPRESSION);

	CuAssertIntEquals(tc, MCC_PARSER_STATUS_OK, result.status);

	struct mcc_ast_expression *function_call = result.expression;

	CuAssertIntEquals(tc, MCC_AST_EXPRESSION_TYPE_FUNCTION_CALL, function_call->type);

	// root -> identifier

	CuAssertStrEquals(tc, "func1", function_call->function_identifier->identifier_name);

	// root -> arguments

	CuAssertTrue(tc, !function_call->arguments->is_empty);
	CuAssertTrue(tc, function_call->arguments->has_next_expression);

	// root -> arguments -> expression

	CuAssertIntEquals(tc, MCC_AST_EXPRESSION_TYPE_VARIABLE, function_call->arguments->expression->type);
	CuAssertStrEquals(tc, "a", function_call->arguments->expression->identifier->identifier_name);

	// root -> arguments -> next_arguments

	CuAssertPtrEquals(tc, NULL, function_call->arguments->next_arguments->next_arguments);
	CuAssertTrue(tc, !(function_call->arguments->next_arguments->has_next_expression));
	CuAssertStrEquals(tc, "h", function_call->arguments->next_arguments->expression->identifier->identifier_name);

	mcc_ast_delete(function_call);
}

void FunctionDefParameters(CuTest *tc)
{

	const char input[] = "int func(bool a){a = 2;}";
	struct mcc_parser_result result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_FUNCTION_DEFINITION);

	CuAssertIntEquals(tc, MCC_PARSER_STATUS_OK, result.status);

	struct mcc_ast_function_definition *function_definition = result.function_definition;

	// root

	CuAssertIntEquals(tc, function_definition->type, MCC_AST_FUNCTION_TYPE_INT);

	// root -> identifier

	CuAssertStrEquals(tc, function_definition->identifier->identifier_name, "func");

	// root -> parameters

	CuAssertTrue(tc, !(function_definition->parameters->is_empty));
	CuAssertTrue(tc, !(function_definition->parameters->has_next_parameter));
	CuAssertPtrEquals(tc, function_definition->parameters->next_parameters, NULL);

	// root -> parameters -> declaration

	CuAssertIntEquals( tc, function_definition->parameters->declaration->declaration_type, MCC_AST_DECLARATION_TYPE_VARIABLE);
	CuAssertIntEquals( tc, function_definition->parameters->declaration->variable_type->type_value, BOOL);
	CuAssertStrEquals( tc, function_definition->parameters->declaration->variable_identifier->identifier_name, "a");

	// root -> compound_stmt

	CuAssertTrue (tc, !(function_definition->compound_stmt->has_next_statement));
	CuAssertPtrEquals (tc, function_definition->compound_stmt->next_compound_statement, NULL);

	// root -> compound_stmt -> statement

	CuAssertIntEquals(tc, function_definition->compound_stmt->statement->type, MCC_AST_STATEMENT_TYPE_ASSIGNMENT);
	CuAssertIntEquals(tc, function_definition->compound_stmt->statement->assignment->assignment_type, MCC_AST_ASSIGNMENT_TYPE_VARIABLE);
	CuAssertStrEquals(tc, function_definition->compound_stmt->statement->assignment->variable_identifier->identifier_name, "a");

	// root -> compound_stmt -> statement -> assignment -> variable_assigned_value

	CuAssertIntEquals(tc, function_definition->compound_stmt->statement->assignment->variable_assigned_value->type, MCC_AST_EXPRESSION_TYPE_LITERAL);
	CuAssertIntEquals(tc, function_definition->compound_stmt->statement->assignment->variable_assigned_value->literal->type, MCC_AST_LITERAL_TYPE_INT);
	CuAssertIntEquals(tc, function_definition->compound_stmt->statement->assignment->variable_assigned_value->literal->i_value, 2);

	mcc_ast_delete(function_definition);

}

void Program(CuTest *tc)
{

	const char input[] = "int func(bool a){a = 2;} bool func2(int a){a[1] = 1;}";
	struct mcc_parser_result result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_EXPRESSION);

	CuAssertIntEquals(tc, MCC_PARSER_STATUS_OK, result.status);
	CuAssertIntEquals(tc, MCC_PARSER_ENTRY_POINT_PROGRAM, result.entry_point);

	struct mcc_ast_program *program = result.program;

	CuAssertTrue(tc, program->has_next_function);
	CuAssertTrue(tc, !(program->next_function->has_next_function));

	// root -> function -> identifier -> identifier_name
	CuAssertStrEquals(tc, program->function->identifier->identifier_name, "func");

	// root -> next_function -> function -> identifier -> identifier_name
	CuAssertStrEquals(tc, program->next_function->function->identifier->identifier_name, "func2");

	mcc_ast_delete(program);
}

void EmptyCompound(CuTest *tc){

	const char input[] = "{}";
	struct mcc_parser_result result = mcc_parse_string(input,MCC_PARSER_ENTRY_POINT_STATEMENT);

	CuAssertIntEquals(tc, MCC_PARSER_STATUS_OK, result.status);
	CuAssertIntEquals(tc, MCC_PARSER_ENTRY_POINT_COMPOUND_STATEMENT, result.entry_point);
	mcc_ast_delete(result.compound_statement);
}

void EmptyFunctionCall(CuTest *tc){
	const char input[] = "func()";
	struct mcc_parser_result result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_EXPRESSION);

	CuAssertIntEquals(tc, MCC_PARSER_STATUS_OK, result.status);
	CuAssertIntEquals(tc, MCC_PARSER_ENTRY_POINT_EXPRESSION, result.entry_point);

	struct mcc_ast_expression* expression = result.expression;

	// root

	CuAssertIntEquals(tc, MCC_AST_EXPRESSION_TYPE_FUNCTION_CALL,expression->type);
	CuAssertStrEquals(tc, expression->function_identifier->identifier_name,"func");

	// root -> arguments

	CuAssertTrue(tc, expression->arguments->is_empty);

	mcc_ast_delete(expression);
}

void EmptyParameters(CuTest *tc){

	const char input[] = "int func(){}";
	struct mcc_parser_result result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_EXPRESSION);

	CuAssertIntEquals(tc, MCC_PARSER_STATUS_OK, result.status);
	CuAssertIntEquals(tc, MCC_PARSER_ENTRY_POINT_FUNCTION_DEFINITION, result.entry_point);

	struct mcc_ast_function_definition* function_definition = result.function_definition;

	CuAssertTrue(tc, function_definition->parameters->is_empty);

	mcc_ast_delete(function_definition);
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
	TEST(if_stmt_1) \
	TEST(while_stmt) \
	TEST(ret_stmt) \
	TEST(Array_Element) \
	TEST(VariableDeclaration) \
	TEST(ArrayDeclaration) \
	TEST(StringLiteral) \
	TEST(VariableAssignment) \
	TEST(assign_stmt) \
	TEST(decl_stmt) \
	TEST(CompoundStatement) \
	TEST(FunctionCallArguments) \
	TEST(FunctionDefParameters) \
	TEST(Program) \
	TEST(EmptyCompound) \
	TEST(EmptyFunctionCall) \
	TEST(EmptyParameters) \

#include "main_stub.inc"
#undef TESTS
