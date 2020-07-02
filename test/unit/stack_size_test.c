#include <CuTest.h>

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "mcc/asm.h"
#include "mcc/ast.h"
#include "mcc/ir.h"
#include "mcc/semantic_checks.h"
#include "mcc/stack_size.h"
#include "mcc/symbol_table.h"

void test_int(CuTest *tc)
{
	// Define test input and create IR
	const char input[] = "int main(){int a; a = 1;return a;}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
	struct mcc_semantic_check *checks = mcc_semantic_check_run_all((&parser_result)->program, table);
	CuAssertIntEquals(tc, checks->status, MCC_SEMANTIC_CHECK_OK);
	struct mcc_ir_row *ir = mcc_ir_generate((&parser_result)->program );
	CuAssertPtrNotNull(tc, ir);

	struct mcc_annotated_ir *an_ir = mcc_annotate_ir(ir);
	struct mcc_annotated_ir *first = an_ir;

	// an_ir
	CuAssertPtrNotNull(tc, an_ir);
	CuAssertIntEquals(tc, STACK_SIZE_INT, an_ir->stack_size);

	// Cleanup
	mcc_ir_delete_ir(ir);
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
	mcc_semantic_check_delete_single_check(checks);
	mcc_delete_annotated_ir(first);
}

void test_ints(CuTest *tc)
{
	// Define test input and create IR
	const char input[] = "int main(){int a; a = 1;int b; b = 0; return a;}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
	struct mcc_semantic_check *checks = mcc_semantic_check_run_all((&parser_result)->program, table);
	CuAssertIntEquals(tc, checks->status, MCC_SEMANTIC_CHECK_OK);
	struct mcc_ir_row *ir = mcc_ir_generate((&parser_result)->program );
	CuAssertPtrNotNull(tc, ir);

	struct mcc_annotated_ir *an_ir = mcc_annotate_ir(ir);
	struct mcc_annotated_ir *first = an_ir;

	// an_ir
	CuAssertPtrNotNull(tc, an_ir);
	CuAssertIntEquals(tc, 8, an_ir->stack_size);

	// Stack position of a
	CuAssertIntEquals(tc, -STACK_SIZE_INT, an_ir->next->stack_position);

	// Stack position of b
	CuAssertIntEquals(tc, -2 * STACK_SIZE_INT, an_ir->next->next->stack_position);

	// Cleanup
	mcc_ir_delete_ir(ir);
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
	mcc_semantic_check_delete_single_check(checks);
	mcc_delete_annotated_ir(first);
}

void test_int_temporaries(CuTest *tc)
{
	// Define test input and create IR -> produces 2 temporaries
	const char input[] = "int main(){int a; a = 1 + (2*2); a = 1; return a;}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
	struct mcc_semantic_check *checks = mcc_semantic_check_run_all((&parser_result)->program, table);
	CuAssertIntEquals(tc, checks->status, MCC_SEMANTIC_CHECK_OK);
	struct mcc_ir_row *ir = mcc_ir_generate((&parser_result)->program );
	CuAssertPtrNotNull(tc, ir);

	struct mcc_annotated_ir *an_ir = mcc_annotate_ir(ir);
	struct mcc_annotated_ir *first = an_ir;

	// an_ir
	CuAssertPtrNotNull(tc, an_ir);
	CuAssertIntEquals(tc, 3 * STACK_SIZE_INT, an_ir->stack_size);
	CuAssertIntEquals(tc, -1 * STACK_SIZE_INT, an_ir->next->stack_position);
	CuAssertIntEquals(tc, -2 * STACK_SIZE_INT, an_ir->next->next->stack_position);
	CuAssertIntEquals(tc, -3 * STACK_SIZE_INT, an_ir->next->next->next->stack_position);

	// Cleanup
	mcc_ir_delete_ir(ir);
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
	mcc_semantic_check_delete_single_check(checks);
	mcc_delete_annotated_ir(first);
}

void test_int_array(CuTest *tc)
{
	// Define test input and create IR -> produces 2 temporaries
	const char input[] = "int main(){int [42]a; a[0]= 9; a[2] = 9; a[41] = 9; return 0;}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
	struct mcc_semantic_check *checks = mcc_semantic_check_run_all((&parser_result)->program, table);
	CuAssertIntEquals(tc, checks->status, MCC_SEMANTIC_CHECK_OK);
	struct mcc_ir_row *ir = mcc_ir_generate((&parser_result)->program );
	CuAssertPtrNotNull(tc, ir);

	struct mcc_annotated_ir *an_ir = mcc_annotate_ir(ir);
	struct mcc_annotated_ir *first = an_ir;

	// an_ir
	CuAssertPtrNotNull(tc, an_ir);
	// Function
	CuAssertIntEquals(tc, 42 * STACK_SIZE_INT, an_ir->stack_size);
	// Array
	CuAssertIntEquals(tc, -42 * STACK_SIZE_INT + 0 * STACK_SIZE_INT, an_ir->next->stack_position);
	// a[0]
	CuAssertIntEquals(tc, -42 * STACK_SIZE_INT + 0 * STACK_SIZE_INT, an_ir->next->next->stack_position);
	// a[2]
	CuAssertIntEquals(tc, -42 * STACK_SIZE_INT + 2 * STACK_SIZE_INT, an_ir->next->next->next->stack_position);
	// a[41]
	CuAssertIntEquals(tc, -42 * STACK_SIZE_INT + 41 * STACK_SIZE_INT,
	                  an_ir->next->next->next->next->stack_position);

	// Cleanup
	mcc_ir_delete_ir(ir);
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
	mcc_semantic_check_delete_single_check(checks);
	mcc_delete_annotated_ir(first);
}

void test_int_multiple_references(CuTest *tc)
{
	// Define test input and create IR -> produces 2 temporaries
	const char input[] = "int main(){int a; a = 1;int b; b = 1; a = 2;int c; c = 0; b = 1; c = 2; return 0;}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
	struct mcc_semantic_check *checks = mcc_semantic_check_run_all((&parser_result)->program, table);
	CuAssertIntEquals(tc, checks->status, MCC_SEMANTIC_CHECK_OK);
	struct mcc_ir_row *ir = mcc_ir_generate((&parser_result)->program );
	CuAssertPtrNotNull(tc, ir);

	struct mcc_annotated_ir *an_ir = mcc_annotate_ir(ir);
	struct mcc_annotated_ir *first = an_ir;

	// an_ir
	CuAssertPtrNotNull(tc, an_ir);
	// Func label "main"
	CuAssertIntEquals(tc, 3 * STACK_SIZE_INT, an_ir->stack_size);
	// "a = 1"
	an_ir = an_ir->next;
	CuAssertIntEquals(tc, -STACK_SIZE_INT, an_ir->stack_position);
	// "b = 1"
	an_ir = an_ir->next;
	CuAssertIntEquals(tc, -2 * STACK_SIZE_INT, an_ir->stack_position);
	// "a = 2"
	an_ir = an_ir->next;
	CuAssertIntEquals(tc, -STACK_SIZE_INT, an_ir->stack_position);
	// "c = 0"
	an_ir = an_ir->next;
	CuAssertIntEquals(tc, -3 * STACK_SIZE_INT, an_ir->stack_position);
	// "b = 1"
	an_ir = an_ir->next;
	CuAssertIntEquals(tc, -2 * STACK_SIZE_INT, an_ir->stack_position);
	// "c = 2"
	an_ir = an_ir->next;
	CuAssertIntEquals(tc, -3 * STACK_SIZE_INT, an_ir->stack_position);

	// Cleanup
	mcc_ir_delete_ir(ir);
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
	mcc_semantic_check_delete_single_check(checks);
	mcc_delete_annotated_ir(first);
}

void test_bool_int(CuTest *tc)
{
	// Define test input and create IR -> produces 2 temporaries
	const char input[] = "int main(){int a; a = 1;bool b; b = true; a = 2; return 0;}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
	struct mcc_semantic_check *checks = mcc_semantic_check_run_all((&parser_result)->program, table);
	CuAssertIntEquals(tc, checks->status, MCC_SEMANTIC_CHECK_OK);
	struct mcc_ir_row *ir = mcc_ir_generate((&parser_result)->program );
	CuAssertPtrNotNull(tc, ir);

	struct mcc_annotated_ir *an_ir = mcc_annotate_ir(ir);
	struct mcc_annotated_ir *first = an_ir;

	// an_ir
	CuAssertPtrNotNull(tc, an_ir);
	CuAssertIntEquals(tc, STACK_SIZE_INT + STACK_SIZE_BOOL, an_ir->stack_size);
	CuAssertIntEquals(tc, -STACK_SIZE_INT, an_ir->next->stack_position);
	CuAssertIntEquals(tc, -(STACK_SIZE_INT + STACK_SIZE_BOOL), an_ir->next->next->stack_position);

	// Cleanup
	mcc_ir_delete_ir(ir);
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
	mcc_semantic_check_delete_single_check(checks);
	mcc_delete_annotated_ir(first);
}

void test_strings(CuTest *tc)
{
	// Define test input and create IR -> produces 2 temporaries
	const char input[] = "int main(){string a; a = \"test\"; string b; b = \"test2\"; return 0;}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
	struct mcc_semantic_check *checks = mcc_semantic_check_run_all((&parser_result)->program, table);
	CuAssertIntEquals(tc, checks->status, MCC_SEMANTIC_CHECK_OK);
	struct mcc_ir_row *ir = mcc_ir_generate((&parser_result)->program );
	CuAssertPtrNotNull(tc, ir);

	struct mcc_annotated_ir *an_ir = mcc_annotate_ir(ir);
	struct mcc_annotated_ir *first = an_ir;

	CuAssertPtrNotNull(tc, an_ir);
	CuAssertIntEquals(tc, 2 * STACK_SIZE_STRING, an_ir->stack_size);

	an_ir = an_ir->next;
	CuAssertIntEquals(tc, STACK_SIZE_STRING, an_ir->stack_size);
	CuAssertIntEquals(tc, -STACK_SIZE_STRING, an_ir->stack_position);

	an_ir = an_ir->next;
	CuAssertIntEquals(tc, STACK_SIZE_STRING, an_ir->stack_size);
	CuAssertIntEquals(tc, -2 * STACK_SIZE_STRING, an_ir->stack_position);

	// Cleanup
	mcc_ir_delete_ir(ir);
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
	mcc_semantic_check_delete_single_check(checks);
	mcc_delete_annotated_ir(first);
}

void test_string_array(CuTest *tc)
{
	// Define test input and create IR -> produces 2 temporaries
	const char input[] = "int main(){string [12]a; a[0]= \"test\"; a[2] = \"test2\"; a[11] = \"test3\"; return 0;}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
	struct mcc_semantic_check *checks = mcc_semantic_check_run_all((&parser_result)->program, table);
	CuAssertIntEquals(tc, checks->status, MCC_SEMANTIC_CHECK_OK);
	struct mcc_ir_row *ir = mcc_ir_generate((&parser_result)->program );
	CuAssertPtrNotNull(tc, ir);

	struct mcc_annotated_ir *an_ir = mcc_annotate_ir(ir);
	struct mcc_annotated_ir *first = an_ir;

	// an_ir (Func_label)
	CuAssertPtrNotNull(tc, an_ir);
	CuAssertIntEquals(tc, 12 * STACK_SIZE_STRING, an_ir->stack_size);

	// a = array
	an_ir = an_ir->next;
	CuAssertIntEquals(tc, 12 * STACK_SIZE_STRING, an_ir->stack_size);

	// a[0]
	an_ir = an_ir->next;
	CuAssertIntEquals(tc, -12 * STACK_SIZE_STRING, an_ir->stack_position);

	// a[2]
	an_ir = an_ir->next;
	CuAssertIntEquals(tc, -12 * STACK_SIZE_STRING + 2 * STACK_SIZE_STRING, an_ir->stack_position);

	// a[11]
	an_ir = an_ir->next;
	CuAssertIntEquals(tc, -12 * STACK_SIZE_STRING + 11 * STACK_SIZE_STRING, an_ir->stack_position);

	// Cleanup
	mcc_ir_delete_ir(ir);
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
	mcc_semantic_check_delete_single_check(checks);
	mcc_delete_annotated_ir(first);
}

// clang-format off

#define TESTS \
	TEST(test_int) \
	TEST(test_ints) \
	TEST(test_int_temporaries) \
	TEST(test_int_array) \
	TEST(test_int_multiple_references) \
	TEST(test_strings) \
	TEST(test_string_array)  \

// clang-format on

#include "main_stub.inc"
#undef TESTS
