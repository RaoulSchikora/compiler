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
