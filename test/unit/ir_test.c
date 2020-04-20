#include <CuTest.h>

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "mcc/ast.h"
#include "mcc/semantic_checks.h"
#include "mcc/symbol_table.h"
#include "mcc/ir.h"

void test1(CuTest *tc)
{
	// Define test input and create symbol table
	const char input[] = "int main(){return 42;}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
	struct mcc_semantic_check *checks = mcc_semantic_check_run_all((&parser_result)->program, table);
	CuAssertIntEquals(tc, checks->status, MCC_SEMANTIC_CHECK_OK);

	struct mcc_ir_row *ir = mcc_ir_generate((&parser_result)->program, table);

	CuAssertIntEquals(tc, ir->row_no, 0);
	CuAssertIntEquals(tc, ir->instr, MCC_IR_INSTR_LABEL);
	CuAssertIntEquals(tc, ir->arg1->type, MCC_IR_TYPE_VAR);
	CuAssertStrEquals(tc, ir->arg1->var, "main");
	CuAssertPtrEquals(tc, ir->arg2, NULL);

	ir = ir->next_row;
	CuAssertPtrNotNull(tc, ir);

	CuAssertIntEquals(tc, ir->row_no, 1);
	CuAssertIntEquals(tc, ir->instr, MCC_IR_INSTR_RETURN);
	CuAssertIntEquals(tc, ir->arg1->type, MCC_IR_TYPE_VAR);
	CuAssertStrEquals(tc, ir->arg1->var, "42");
	CuAssertPtrEquals(tc, ir->arg2, NULL);
	CuAssertPtrEquals(tc, ir->next_row, NULL);

	// Cleanup
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
	mcc_semantic_check_delete_single_check(checks);
}

void test2(CuTest *tc)
{
	// Define test input and create symbol table
	const char input[] = "int main(){int a; a = 2; int b; int c; b = 2; c = a + (b/c); return c;}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
	struct mcc_semantic_check *checks = mcc_semantic_check_run_all((&parser_result)->program, table);
	CuAssertIntEquals(tc, checks->status, MCC_SEMANTIC_CHECK_OK);

	//TODO: Generate IR and check it


	// Cleanup
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
	mcc_semantic_check_delete_single_check(checks);
}

#define TESTS \
	TEST(test1) \
	TEST(test2)
#include "main_stub.inc"
#undef TESTS
