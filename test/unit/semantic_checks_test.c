#include <CuTest.h>

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "mcc/ast.h"
#include "mcc/semantic_checks.h"
#include "mcc/symbol_table.h"

void positive(CuTest *tc)
{
	// Define test input and create symbol table
	const char input[] = "int main(){int a; a = 2; int b; int c; b = 2; c = a + (b/c); return c;} \
                         void func1(int a, bool[10] b){a = a + 2; int i; i = 0; b[i] = true;} \
                         bool func2(){return true;} \
                         int func3(){int a; if(true)if(true){while(a==1){return 2;}} return 3;} \
                         int func4(){if(true){} return 3;} \
                         bool func5(){bool a; a = true; return !a;} \
                         float func6(){float a; a = 2.3; return -a;} \
                         bool func7(){bool a; bool b; a = true; b = false; return a && b;} \
                         int func8(){int[42] a; int b; a[10] = 10; return a[10] + func3();} \
                         void func9(){int[10] a; int[10] c; int[10] b; int i; i = 0; \
                                    while (i < 10) { c[i] = a[i] + b[i]; i = i + 1; }}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
	struct mcc_semantic_check *checks = mcc_semantic_check_run_all((&parser_result)->program, table);

	if (checks->status == MCC_SEMANTIC_CHECK_FAIL) {
		if (checks->error_buffer == NULL) {
			printf("Semantic check failed, error buffer is empty.");
		} else {
			printf("Semantic check failed:\n%s", checks->error_buffer);
		}
	}

	CuAssertIntEquals(tc, checks->status, MCC_SEMANTIC_CHECK_OK);
	CuAssertPtrEquals(tc, NULL, checks->error_buffer);

	CuAssertPtrNotNull(tc, checks);

	// Cleanup
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
	mcc_semantic_check_delete_single_check(checks);
}

// ensure variable shadowing
void ensure_variable_shadowing(CuTest *tc)
{
	// Define test input and create symbol table
	const char input[] = "int main(){int a; a = 10; if(true) { float a; a = func(); } a = func2(); return 0;} \
                          float func(){return 1.23456;} int func2(){return 0;}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
	struct mcc_semantic_check *checks = mcc_semantic_check_run_all((&parser_result)->program, table);

	if (checks->status == MCC_SEMANTIC_CHECK_FAIL) {
		if (checks->error_buffer == NULL) {
			printf("Semantic check failed, error buffer is empty.");
		} else {
			printf("Semantic check failed:\n%s", checks->error_buffer);
		}
	}

	CuAssertIntEquals(tc, checks->status, MCC_SEMANTIC_CHECK_OK);
	CuAssertPtrEquals(tc, NULL, checks->error_buffer);

	// Cleanup
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
	mcc_semantic_check_delete_single_check(checks);
}

// Invalid add with float and int
void type_conversion_expression(CuTest *tc)
{
	// Define test input and create symbol table
	const char input[] = "int main(){int a; int b; float c; a = b + c;}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
	struct mcc_semantic_check *check = mcc_semantic_check_initialize_check();
	CuAssertPtrNotNull(tc,check);
	enum mcc_semantic_check_error_code error = mcc_semantic_check_run_type_check((&parser_result)->program, table, check);
	CuAssertIntEquals(tc,MCC_SEMANTIC_CHECK_ERROR_OK,error);

	CuAssertPtrNotNull(tc, check->error_buffer);
	CuAssertPtrNotNull(tc, check);
	CuAssertIntEquals(tc, check->status, MCC_SEMANTIC_CHECK_FAIL);

	// Cleanup
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
	mcc_semantic_check_delete_single_check(check);
}

// Invalid add with function of type float and int
void type_conversion_expression2(CuTest *tc)
{
	// Define test input and create symbol table
	const char input[] = "int main(){int a; int b; a = b + func();} float func(){float a; return a;}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
	struct mcc_semantic_check *check = mcc_semantic_check_initialize_check();
	CuAssertPtrNotNull(tc,check);
	enum mcc_semantic_check_error_code error = mcc_semantic_check_run_type_check((&parser_result)->program, table, check);
	CuAssertIntEquals(tc,MCC_SEMANTIC_CHECK_ERROR_OK,error);

	CuAssertPtrNotNull(tc, check->error_buffer);
	CuAssertPtrNotNull(tc, check);
	CuAssertIntEquals(tc, check->status, MCC_SEMANTIC_CHECK_FAIL);

	// Cleanup
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
	mcc_semantic_check_delete_single_check(check);
}

// Invalid binary operation on ints
void type_conversion_expression3(CuTest *tc)
{
	// Define test input and create symbol table
	const char input[] = "int main(){int a; int b; a = b && a;}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
	struct mcc_semantic_check *check = mcc_semantic_check_initialize_check();
	CuAssertPtrNotNull(tc,check);
	enum mcc_semantic_check_error_code error = mcc_semantic_check_run_type_check((&parser_result)->program, table, check);
	CuAssertIntEquals(tc,MCC_SEMANTIC_CHECK_ERROR_OK,error);

	CuAssertPtrNotNull(tc, check->error_buffer);
	CuAssertPtrNotNull(tc, check);
	CuAssertIntEquals(tc, check->status, MCC_SEMANTIC_CHECK_FAIL);

	// Cleanup
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
	mcc_semantic_check_delete_single_check(check);
}

// Invalid binary operation on bools
void type_conversion_expression4(CuTest *tc)
{
	// Define test input and create symbol table
	const char input[] = "int main(){bool a; bool b; a = b + a;}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
	struct mcc_semantic_check *check = mcc_semantic_check_initialize_check();
	CuAssertPtrNotNull(tc,check);
	enum mcc_semantic_check_error_code error = mcc_semantic_check_run_type_check((&parser_result)->program, table, check);
	CuAssertIntEquals(tc,MCC_SEMANTIC_CHECK_ERROR_OK,error);

	CuAssertPtrNotNull(tc, check->error_buffer);
	CuAssertPtrNotNull(tc, check);
	CuAssertIntEquals(tc, check->status, MCC_SEMANTIC_CHECK_FAIL);

	// Cleanup
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
	mcc_semantic_check_delete_single_check(check);
}

// Invalid unary operation on int
void type_conversion_expression5(CuTest *tc)
{
	// Define test input and create symbol table
	const char input[] = "int main(){int a; int b; a = !b;}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
	struct mcc_semantic_check *check = mcc_semantic_check_initialize_check();
	CuAssertPtrNotNull(tc,check);
	enum mcc_semantic_check_error_code error = mcc_semantic_check_run_type_check((&parser_result)->program, table, check);
	CuAssertIntEquals(tc,MCC_SEMANTIC_CHECK_ERROR_OK,error);

	CuAssertPtrNotNull(tc, check->error_buffer);
	CuAssertPtrNotNull(tc, check);
	CuAssertIntEquals(tc, check->status, MCC_SEMANTIC_CHECK_FAIL);

	// Cleanup
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
	mcc_semantic_check_delete_single_check(check);
}

// Invalid unary operation on bools
void type_conversion_expression6(CuTest *tc)
{
	// Define test input and create symbol table
	const char input[] = "int main(){bool a; bool b; a = -b;}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
	struct mcc_semantic_check *check = mcc_semantic_check_initialize_check();
	CuAssertPtrNotNull(tc,check);
	enum mcc_semantic_check_error_code error = mcc_semantic_check_run_type_check((&parser_result)->program, table, check);
	CuAssertIntEquals(tc,MCC_SEMANTIC_CHECK_ERROR_OK,error);

	CuAssertPtrNotNull(tc, check->error_buffer);
	CuAssertPtrNotNull(tc, check);
	CuAssertIntEquals(tc, check->status, MCC_SEMANTIC_CHECK_FAIL);

	// Cleanup
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
	mcc_semantic_check_delete_single_check(check);
}

// no operations on strings are supported
void type_conversion_expression7(CuTest *tc)
{
	// Define test input and create symbol table
	const char input[] = "void test(){string a; string b; b = a + b;}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
	struct mcc_semantic_check *check = mcc_semantic_check_initialize_check();
	CuAssertPtrNotNull(tc,check);
	enum mcc_semantic_check_error_code error = mcc_semantic_check_run_type_check((&parser_result)->program, table, check);
	CuAssertIntEquals(tc,MCC_SEMANTIC_CHECK_ERROR_OK,error);

	CuAssertPtrNotNull(tc, check->error_buffer);
	CuAssertPtrNotNull(tc, check);
	CuAssertIntEquals(tc, check->status, MCC_SEMANTIC_CHECK_FAIL);

	// Cleanup
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
	mcc_semantic_check_delete_single_check(check);
}

// no operations on whole array are supported
void type_conversion_expression8(CuTest *tc)
{
	// Define test input and create symbol table
	const char input[] = "void test(){int[42] array1; int[42] array2; array1 = array1 + array2;}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
	struct mcc_semantic_check *check = mcc_semantic_check_initialize_check();
	CuAssertPtrNotNull(tc,check);
	enum mcc_semantic_check_error_code error = mcc_semantic_check_run_type_check((&parser_result)->program, table, check);
	CuAssertIntEquals(tc,MCC_SEMANTIC_CHECK_ERROR_OK,error);

	CuAssertPtrNotNull(tc, check->error_buffer);
	CuAssertPtrNotNull(tc, check);
	CuAssertIntEquals(tc, check->status, MCC_SEMANTIC_CHECK_FAIL);

	// Cleanup
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
	mcc_semantic_check_delete_single_check(check);
}

// expression in if statement is malformed
void type_conversion_expression9(CuTest *tc)
{
	// Define test input and create symbol table
	const char input[] = "void test(){int a; float b; a = 0; b = 0.5; if(a + b > 3){}}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
	struct mcc_semantic_check *check = mcc_semantic_check_initialize_check();
	CuAssertPtrNotNull(tc,check);
	enum mcc_semantic_check_error_code error = mcc_semantic_check_run_type_check((&parser_result)->program, table, check);
	CuAssertIntEquals(tc,MCC_SEMANTIC_CHECK_ERROR_OK,error);

	CuAssertPtrNotNull(tc, check->error_buffer);
	CuAssertPtrNotNull(tc, check);
	CuAssertIntEquals(tc, check->status, MCC_SEMANTIC_CHECK_FAIL);

	// Cleanup
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
	mcc_semantic_check_delete_single_check(check);
}

// condition of if statement is not a boolean expression
void type_conversion_if(CuTest *tc)
{
	// Define test input and create symbol table
	const char input[] = "int test(){int a; int b; a = 0; b = 1; if(a + b) return 3; return 4;}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
	struct mcc_semantic_check *check = mcc_semantic_check_initialize_check();
	CuAssertPtrNotNull(tc,check);
	enum mcc_semantic_check_error_code error = mcc_semantic_check_run_type_check((&parser_result)->program, table, check);
	CuAssertIntEquals(tc,MCC_SEMANTIC_CHECK_ERROR_OK,error);

	CuAssertPtrNotNull(tc, check->error_buffer);
	CuAssertPtrNotNull(tc, check);
	CuAssertIntEquals(tc, check->status, MCC_SEMANTIC_CHECK_FAIL);

	// Cleanup
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
	mcc_semantic_check_delete_single_check(check);
}

// condition of if else statement is not a boolean expression
void type_conversion_if_else(CuTest *tc)
{
	// Define test input and create symbol table
	const char input[] = "int test(){float a; float b; a = 0.5; b = 1.5; if(a + b){}else{} return 4;}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
	struct mcc_semantic_check *check = mcc_semantic_check_initialize_check();
	CuAssertPtrNotNull(tc,check);
	enum mcc_semantic_check_error_code error = mcc_semantic_check_run_type_check((&parser_result)->program, table, check);
	CuAssertIntEquals(tc,MCC_SEMANTIC_CHECK_ERROR_OK,error);

	CuAssertPtrNotNull(tc, check->error_buffer);
	CuAssertPtrNotNull(tc, check);
	CuAssertIntEquals(tc, check->status, MCC_SEMANTIC_CHECK_FAIL);

	// Cleanup
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
	mcc_semantic_check_delete_single_check(check);
}
// condition of while loop is not a boolean expression
void type_conversion_while(CuTest *tc)
{
	// Define test input and create symbol table
	const char input[] = "void test(){string str; str = \"hallo\"; while(str){}}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
	struct mcc_semantic_check *check = mcc_semantic_check_initialize_check();
	CuAssertPtrNotNull(tc,check);
	enum mcc_semantic_check_error_code error = mcc_semantic_check_run_type_check((&parser_result)->program, table, check);
	CuAssertIntEquals(tc,MCC_SEMANTIC_CHECK_ERROR_OK,error);

	CuAssertPtrNotNull(tc, check->error_buffer);
	CuAssertPtrNotNull(tc, check);
	CuAssertIntEquals(tc, check->status, MCC_SEMANTIC_CHECK_FAIL);

	// Cleanup
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
	mcc_semantic_check_delete_single_check(check);
}

// Using return value as the wrong type
void type_conversion_assignment(CuTest *tc)
{
	// Define test input and create symbol table
	const char input[] = "int main(){int a; a = read_float();}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
	struct mcc_semantic_check *check = mcc_semantic_check_initialize_check();
	CuAssertPtrNotNull(tc,check);
	enum mcc_semantic_check_error_code error = mcc_semantic_check_run_type_check((&parser_result)->program, table, check);
	CuAssertIntEquals(tc,MCC_SEMANTIC_CHECK_ERROR_OK,error);

	CuAssertPtrNotNull(tc, check->error_buffer);
	CuAssertPtrNotNull(tc, check);
	CuAssertIntEquals(tc, check->status, MCC_SEMANTIC_CHECK_FAIL);

	// Cleanup
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
	mcc_semantic_check_delete_single_check(check);
}

// Using return value as the wrong type
void type_conversion_assignment2(CuTest *tc)
{
	// Define test input and create symbol table
	const char input[] = "int main(){bool a; a = bool_to_int(a);} int bool_to_int(bool a){int a; a = 1; return a;}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
	struct mcc_semantic_check *check = mcc_semantic_check_initialize_check();
	CuAssertPtrNotNull(tc,check);
	enum mcc_semantic_check_error_code error = mcc_semantic_check_run_type_check((&parser_result)->program, table, check);
	CuAssertIntEquals(tc,MCC_SEMANTIC_CHECK_ERROR_OK,error);

	CuAssertPtrNotNull(tc, check->error_buffer);
	CuAssertPtrNotNull(tc, check);
	CuAssertIntEquals(tc, check->status, MCC_SEMANTIC_CHECK_FAIL);

	// Cleanup
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
	mcc_semantic_check_delete_single_check(check);
}

// Using return value as the wrong type
void type_conversion_assignment3(CuTest *tc)
{
	// Define test input and create symbol table
	const char input[] = "int main(){float a; int b; b = 1; a = b + 2;}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
	struct mcc_semantic_check *check = mcc_semantic_check_initialize_check();
	CuAssertPtrNotNull(tc,check);
	enum mcc_semantic_check_error_code error = mcc_semantic_check_run_type_check((&parser_result)->program, table, check);
	CuAssertIntEquals(tc,MCC_SEMANTIC_CHECK_ERROR_OK,error);

	CuAssertPtrNotNull(tc, check->error_buffer);
	CuAssertPtrNotNull(tc, check);
	CuAssertIntEquals(tc, check->status, MCC_SEMANTIC_CHECK_FAIL);

	// Cleanup
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
	mcc_semantic_check_delete_single_check(check);
}

// Using return value as the wrong type
void type_conversion_assignment4(CuTest *tc)
{
	// Define test input and create symbol table
	const char input[] = "int main(){float a; a = 0.5; int[10] b; b[3] = a + 2.5; return 3;}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
	struct mcc_semantic_check *check = mcc_semantic_check_initialize_check();
	CuAssertPtrNotNull(tc,check);
	enum mcc_semantic_check_error_code error = mcc_semantic_check_run_type_check((&parser_result)->program, table, check);
	CuAssertIntEquals(tc,MCC_SEMANTIC_CHECK_ERROR_OK,error);

	CuAssertPtrNotNull(tc, check->error_buffer);
	CuAssertPtrNotNull(tc, check);
	CuAssertIntEquals(tc, check->status, MCC_SEMANTIC_CHECK_FAIL);

	// Cleanup
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
	mcc_semantic_check_delete_single_check(check);
}

// Assign an int to float
void type_conversion_assignment5(CuTest *tc)
{
	// Define test input and create symbol table
	const char input[] = "int main(){float a; a = 5; return 0;}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
	struct mcc_semantic_check *check = mcc_semantic_check_initialize_check();
	CuAssertPtrNotNull(tc,check);
	enum mcc_semantic_check_error_code error = mcc_semantic_check_run_type_check((&parser_result)->program, table, check);
	CuAssertIntEquals(tc,MCC_SEMANTIC_CHECK_ERROR_OK,error);

	CuAssertPtrNotNull(tc, check->error_buffer);
	CuAssertPtrNotNull(tc, check);
	CuAssertIntEquals(tc, check->status, MCC_SEMANTIC_CHECK_FAIL);

	// Cleanup
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
	mcc_semantic_check_delete_single_check(check);
}

// int-function without return in if_else_on_ture
void nonvoid_check(CuTest *tc)
{

	// Define test input and create symbol table
	const char input[] = "int func(int a){if(a==2){} else {return 3;}}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
	struct mcc_semantic_check *check = mcc_semantic_check_initialize_check();
	CuAssertPtrNotNull(tc,check);
	enum mcc_semantic_check_error_code error = mcc_semantic_check_run_nonvoid_check((&parser_result)->program, table, check);
	CuAssertIntEquals(tc,MCC_SEMANTIC_CHECK_ERROR_OK,error);

	CuAssertPtrNotNull(tc, check);
	CuAssertPtrNotNull(tc, check->error_buffer);
	CuAssertIntEquals(tc, check->status, MCC_SEMANTIC_CHECK_FAIL);

	// Cleanup
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
	mcc_semantic_check_delete_single_check(check);
}

// int-function without return in if_else_on_false
void nonvoid_check2(CuTest *tc)
{

	// Define test input and create symbol table
	const char input[] = "int func(int a){if(a==2){return 3;} else {}}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
	struct mcc_semantic_check *check = mcc_semantic_check_initialize_check();
	CuAssertPtrNotNull(tc,check);
	enum mcc_semantic_check_error_code error = mcc_semantic_check_run_nonvoid_check((&parser_result)->program, table, check);
	CuAssertIntEquals(tc,MCC_SEMANTIC_CHECK_ERROR_OK,error);

	CuAssertPtrNotNull(tc, check);
	CuAssertPtrNotNull(tc, check->error_buffer);
	CuAssertIntEquals(tc, check->status, MCC_SEMANTIC_CHECK_FAIL);

	// Cleanup
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
	mcc_semantic_check_delete_single_check(check);
}

// int-function returns value only in if-condition
void nonvoid_check3(CuTest *tc)
{

	// Define test input and create symbol table
	const char input[] = "int func(int a){if(true){return 3;}}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
	struct mcc_semantic_check *check = mcc_semantic_check_initialize_check();
	CuAssertPtrNotNull(tc,check);
	enum mcc_semantic_check_error_code error = mcc_semantic_check_run_nonvoid_check((&parser_result)->program, table, check);
	CuAssertIntEquals(tc,MCC_SEMANTIC_CHECK_ERROR_OK,error);

	CuAssertPtrNotNull(tc, check);
	CuAssertPtrNotNull(tc, check->error_buffer);
	CuAssertIntEquals(tc, check->status, MCC_SEMANTIC_CHECK_FAIL);

	// Cleanup
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
	mcc_semantic_check_delete_single_check(check);
}

// int-function has no return
void nonvoid_check4(CuTest *tc)
{

	// Define test input and create symbol table
	const char input[] = "int func(int a){int b; if(true){} int c; int d;}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
	struct mcc_semantic_check *check = mcc_semantic_check_initialize_check();
	CuAssertPtrNotNull(tc,check);
	enum mcc_semantic_check_error_code error = mcc_semantic_check_run_nonvoid_check((&parser_result)->program, table, check);
	CuAssertIntEquals(tc,MCC_SEMANTIC_CHECK_ERROR_OK,error);

	CuAssertPtrNotNull(tc, check);
	CuAssertPtrNotNull(tc, check->error_buffer);
	CuAssertIntEquals(tc, check->status, MCC_SEMANTIC_CHECK_FAIL);

	// Cleanup
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
	mcc_semantic_check_delete_single_check(check);
}

// main has wrong signature
void main_function_1(CuTest *tc)
{

	// Define test input and create symbol table
	const char input[] = "int main(int a){if(a==2){return;} else {return 3;}}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
	struct mcc_semantic_check *check = mcc_semantic_check_initialize_check();
	CuAssertPtrNotNull(tc,check);
	enum mcc_semantic_check_error_code error = mcc_semantic_check_run_main_function((&parser_result)->program, table, check);
	CuAssertIntEquals(tc,MCC_SEMANTIC_CHECK_ERROR_OK,error);

	CuAssertPtrNotNull(tc, check);
	CuAssertPtrNotNull(tc, check->error_buffer);
	CuAssertIntEquals(tc, check->status, MCC_SEMANTIC_CHECK_FAIL);

	// Cleanup
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
	mcc_semantic_check_delete_single_check(check);
}

// 2 main-functions exist
void main_function_2(CuTest *tc)
{

	// Define test input and create symbol table
	const char input[] = "int main(){if(a==2){return;} else {return 3;}} int main(){return 1;}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
	struct mcc_semantic_check *check = mcc_semantic_check_initialize_check();
	CuAssertPtrNotNull(tc,check);
	enum mcc_semantic_check_error_code error = mcc_semantic_check_run_main_function((&parser_result)->program, table, check);
	CuAssertIntEquals(tc,MCC_SEMANTIC_CHECK_ERROR_OK,error);

	CuAssertPtrNotNull(tc, check);
	CuAssertPtrNotNull(tc, check->error_buffer);
	CuAssertIntEquals(tc, check->status, MCC_SEMANTIC_CHECK_FAIL);

	// Cleanup
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
	mcc_semantic_check_delete_single_check(check);
}

// No main function is defined
void main_function_3(CuTest *tc)
{

	// Define test input and create symbol table
	const char input[] = "int func(int a){if(a==2){return;} else {return 3;}}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
	struct mcc_semantic_check *check = mcc_semantic_check_initialize_check();
	CuAssertPtrNotNull(tc,check);
	enum mcc_semantic_check_error_code error = mcc_semantic_check_run_main_function((&parser_result)->program, table, check);
	CuAssertIntEquals(tc,MCC_SEMANTIC_CHECK_ERROR_OK,error);

	CuAssertPtrNotNull(tc, check);
	CuAssertPtrNotNull(tc, check->error_buffer);
	CuAssertIntEquals(tc, check->status, MCC_SEMANTIC_CHECK_FAIL);

	// Cleanup
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
	mcc_semantic_check_delete_single_check(check);
}

// An undefined function is called
void unknown_function_call(CuTest *tc)
{

	// Define test input and create symbol table
	const char input[] = "int main(){int a; a = 1; a = unknown_function(a);}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
	struct mcc_semantic_check *check =
	    mcc_semantic_check_initialize_check();
	CuAssertPtrNotNull(tc,check);
	enum mcc_semantic_check_error_code error = mcc_semantic_check_run_type_check((&parser_result)->program, table, check);
	CuAssertIntEquals(tc,MCC_SEMANTIC_CHECK_ERROR_OK,error);

	CuAssertPtrNotNull(tc, check);
	CuAssertPtrNotNull(tc, check->error_buffer);
	CuAssertIntEquals(tc, check->status, MCC_SEMANTIC_CHECK_FAIL);

	// Cleanup
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
	mcc_semantic_check_delete_single_check(check);
}

// An undefined function is called
void unknown_function_call2(CuTest *tc)
{

	// Define test input and create symbol table
	const char input[] = "int main(){test();}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
	struct mcc_semantic_check *check =
	    mcc_semantic_check_initialize_check();
	CuAssertPtrNotNull(tc,check);
	enum mcc_semantic_check_error_code error = mcc_semantic_check_run_type_check((&parser_result)->program, table, check);
	CuAssertIntEquals(tc,MCC_SEMANTIC_CHECK_ERROR_OK,error);

	CuAssertPtrNotNull(tc, check);
	CuAssertPtrNotNull(tc, check->error_buffer);
	CuAssertIntEquals(tc, check->status, MCC_SEMANTIC_CHECK_FAIL);

	// Cleanup
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
	mcc_semantic_check_delete_single_check(check);
}

// A function is defined multiple times
void multiple_function_definitions(CuTest *tc)
{

	// Define test input and create symbol table
	const char input[] = "int test(int a){return a;} int test(int a){return a + 1 ;}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
	struct mcc_semantic_check *check =
	    mcc_semantic_check_initialize_check();
	CuAssertPtrNotNull(tc,check);
	enum mcc_semantic_check_error_code error = mcc_semantic_check_run_multiple_function_definitions((&parser_result)->program, table, check);
	CuAssertIntEquals(tc,MCC_SEMANTIC_CHECK_ERROR_OK,error);

	CuAssertPtrNotNull(tc, check);
	CuAssertPtrNotNull(tc, check->error_buffer);
	CuAssertIntEquals(tc, check->status, MCC_SEMANTIC_CHECK_FAIL);

	// Cleanup
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
	mcc_semantic_check_delete_single_check(check);
}

// a function is defined multiple times within a couple of functions
void multiple_function_definitions2(CuTest *tc)
{

	// Define test input and create symbol table
	const char input[] = "int func1(){} int test(){} int func2(){} int func3(){} int test(){} int func4(){}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
	struct mcc_semantic_check *check =
	    mcc_semantic_check_initialize_check();
	CuAssertPtrNotNull(tc,check);
	enum mcc_semantic_check_error_code error = mcc_semantic_check_run_multiple_function_definitions((&parser_result)->program, table, check);
	CuAssertIntEquals(tc,MCC_SEMANTIC_CHECK_ERROR_OK,error);

	CuAssertPtrNotNull(tc, check->error_buffer);
	CuAssertPtrNotNull(tc, check);
	CuAssertIntEquals(tc, check->status, MCC_SEMANTIC_CHECK_FAIL);

	// Cleanup
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
	mcc_semantic_check_delete_single_check(check);
}

// a function is defined multiple times within a couple of functions at the end
void multiple_function_definitions3(CuTest *tc)
{

	// Define test input and create symbol table
	const char input[] = "int func1(){} int func2(){} int func3(){} int func4(){} int test(){} int test(){} ";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
	struct mcc_semantic_check *check =
	    mcc_semantic_check_initialize_check();
	CuAssertPtrNotNull(tc,check);
	enum mcc_semantic_check_error_code error = mcc_semantic_check_run_multiple_function_definitions((&parser_result)->program, table, check);
	CuAssertIntEquals(tc,MCC_SEMANTIC_CHECK_ERROR_OK,error);

	CuAssertPtrNotNull(tc, check->error_buffer);
	CuAssertPtrNotNull(tc, check);
	CuAssertIntEquals(tc, check->status, MCC_SEMANTIC_CHECK_FAIL);

	// Cleanup
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
	mcc_semantic_check_delete_single_check(check);
}

// A variable is declared more than once in the same scope
void multiple_variable_declarations(CuTest *tc)
{

	// Define test input and create symbol table
	const char input[] = "int main(){ int a; {int b; int b;}}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
	struct mcc_semantic_check *check =
	    mcc_semantic_check_initialize_check();
	CuAssertPtrNotNull(tc,check);
	enum mcc_semantic_check_error_code error = mcc_semantic_check_run_multiple_variable_declarations((&parser_result)->program, table, check);
	CuAssertIntEquals(tc,MCC_SEMANTIC_CHECK_ERROR_OK,error);

	CuAssertPtrNotNull(tc, check);
	CuAssertPtrNotNull(tc, check->error_buffer);
	CuAssertIntEquals(tc, check->status, MCC_SEMANTIC_CHECK_FAIL);

	// Cleanup
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
	mcc_semantic_check_delete_single_check(check);
}

// A variable is declared more than once in the same scope
void multiple_variable_declarations2(CuTest *tc)
{

	// Define test input and create symbol table
	const char input[] = "int main(){ int a; {int b;} int c; int a;}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
	struct mcc_semantic_check *check =
	    mcc_semantic_check_initialize_check();
	CuAssertPtrNotNull(tc,check);
	enum mcc_semantic_check_error_code error = mcc_semantic_check_run_multiple_variable_declarations((&parser_result)->program, table, check);
	CuAssertIntEquals(tc,MCC_SEMANTIC_CHECK_ERROR_OK,error);

	CuAssertPtrNotNull(tc, check);
	CuAssertPtrNotNull(tc, check->error_buffer);
	CuAssertIntEquals(tc, check->status, MCC_SEMANTIC_CHECK_FAIL);

	// Cleanup
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
	mcc_semantic_check_delete_single_check(check);
}

// An undeclared variable is used
void use_undeclared_variable(CuTest *tc)
{

	// Define test input and create symbol table
	const char input[] = "int main(){ int a; {int b; b = c;}}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
	struct mcc_semantic_check *check =
	    mcc_semantic_check_initialize_check();
	CuAssertPtrNotNull(tc,check);
	enum mcc_semantic_check_error_code error = mcc_semantic_check_run_type_check((&parser_result)->program, table, check);
	CuAssertIntEquals(tc,MCC_SEMANTIC_CHECK_ERROR_OK,error);

	CuAssertPtrNotNull(tc, check);
	CuAssertPtrNotNull(tc, check->error_buffer);
	CuAssertIntEquals(tc, check->status, MCC_SEMANTIC_CHECK_FAIL);

	// Cleanup
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
	mcc_semantic_check_delete_single_check(check);
}

// An undeclared variable is used in an if-condition
void use_undeclared_variable2(CuTest *tc)
{

	// Define test input and create symbol table
	const char input[] = "int main(){ int a; if(i == 0) return a;return a;}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
	struct mcc_semantic_check *check =
	    mcc_semantic_check_initialize_check();
	CuAssertPtrNotNull(tc,check);
	enum mcc_semantic_check_error_code error = mcc_semantic_check_run_type_check((&parser_result)->program, table, check);
	CuAssertIntEquals(tc,MCC_SEMANTIC_CHECK_ERROR_OK,error);

	CuAssertPtrNotNull(tc, check);
	CuAssertPtrNotNull(tc, check->error_buffer);
	CuAssertIntEquals(tc, check->status, MCC_SEMANTIC_CHECK_FAIL);

	// Cleanup
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
	mcc_semantic_check_delete_single_check(check);
}

// An undeclared variable is used in an while-condition
void use_undeclared_variable3(CuTest *tc)
{

	// Define test input and create symbol table
	const char input[] = "int main(){ int a; while(i == 0) return a;return a;}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
	struct mcc_semantic_check *check =
	    mcc_semantic_check_initialize_check();
	CuAssertPtrNotNull(tc,check);
	enum mcc_semantic_check_error_code error = mcc_semantic_check_run_type_check((&parser_result)->program, table, check);
	CuAssertIntEquals(tc,MCC_SEMANTIC_CHECK_ERROR_OK,error);

	CuAssertPtrNotNull(tc, check);
	CuAssertPtrNotNull(tc, check->error_buffer);
	CuAssertIntEquals(tc, check->status, MCC_SEMANTIC_CHECK_FAIL);

	// Cleanup
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
	mcc_semantic_check_delete_single_check(check);
}

// An undeclared variable is used in a return-statement
void use_undeclared_variable4(CuTest *tc)
{

	// Define test input and create symbol table
	const char input[] = "int main(){ int a; return i[10];}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
	struct mcc_semantic_check *check =
	    mcc_semantic_check_initialize_check();
	CuAssertPtrNotNull(tc,check);
	enum mcc_semantic_check_error_code error = mcc_semantic_check_run_type_check((&parser_result)->program, table, check);
	CuAssertIntEquals(tc,MCC_SEMANTIC_CHECK_ERROR_OK,error);

	CuAssertPtrNotNull(tc, check);
	CuAssertPtrNotNull(tc, check->error_buffer);
	CuAssertIntEquals(tc, check->status, MCC_SEMANTIC_CHECK_FAIL);

	// Cleanup
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
	mcc_semantic_check_delete_single_check(check);
}

// An undeclared variable is used in an while-condition
void use_undeclared_variable5(CuTest *tc)
{

	// Define test input and create symbol table
	const char input[] = "int main(){ int a; b = a;}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
	struct mcc_semantic_check *check =
	    mcc_semantic_check_initialize_check();
	CuAssertPtrNotNull(tc,check);
	enum mcc_semantic_check_error_code error = mcc_semantic_check_run_type_check((&parser_result)->program, table, check);
	CuAssertIntEquals(tc,MCC_SEMANTIC_CHECK_ERROR_OK,error);

	CuAssertPtrNotNull(tc, check);
	CuAssertPtrNotNull(tc, check->error_buffer);
	CuAssertIntEquals(tc, check->status, MCC_SEMANTIC_CHECK_FAIL);

	// Cleanup
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
	mcc_semantic_check_delete_single_check(check);
}

// An undeclared variable is used in an while-condition
void use_undeclared_variable6(CuTest *tc)
{

	// Define test input and create symbol table
	const char input[] = "int main(){ int a; b[1] = a;}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
	struct mcc_semantic_check *check =
	    mcc_semantic_check_initialize_check();
	CuAssertPtrNotNull(tc,check);
	enum mcc_semantic_check_error_code error = mcc_semantic_check_run_type_check((&parser_result)->program, table, check);
	CuAssertIntEquals(tc,MCC_SEMANTIC_CHECK_ERROR_OK,error);

	CuAssertPtrNotNull(tc, check);
	CuAssertPtrNotNull(tc, check->error_buffer);
	CuAssertIntEquals(tc, check->status, MCC_SEMANTIC_CHECK_FAIL);

	// Cleanup
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
	mcc_semantic_check_delete_single_check(check);
}

// An undeclared variable is used in a return-statement
void use_undeclared_variable7(CuTest *tc)
{

	// Define test input and create symbol table
	const char input[] = "int func(){ return i;}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
	struct mcc_semantic_check *check =
	    mcc_semantic_check_initialize_check();
	CuAssertPtrNotNull(tc,check);
	enum mcc_semantic_check_error_code error = mcc_semantic_check_run_type_check((&parser_result)->program, table, check);
	CuAssertIntEquals(tc,MCC_SEMANTIC_CHECK_ERROR_OK,error);

	CuAssertPtrNotNull(tc, check);
	CuAssertPtrNotNull(tc, check->error_buffer);
	CuAssertIntEquals(tc, check->status, MCC_SEMANTIC_CHECK_FAIL);

	// Cleanup
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
	mcc_semantic_check_delete_single_check(check);
}

// An undeclared variable is assigned
void use_undeclared_variable8(CuTest *tc)
{

	// Define test input and create symbol table
	const char input[] = "int func(){ das = 0; return 0;}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
	struct mcc_semantic_check *check =
	    mcc_semantic_check_initialize_check();
	CuAssertPtrNotNull(tc,check);
	enum mcc_semantic_check_error_code error = mcc_semantic_check_run_type_check((&parser_result)->program, table, check);
	CuAssertIntEquals(tc,MCC_SEMANTIC_CHECK_ERROR_OK,error);

	CuAssertPtrNotNull(tc, check);
	CuAssertPtrNotNull(tc, check->error_buffer);
	CuAssertIntEquals(tc, check->status, MCC_SEMANTIC_CHECK_FAIL);

	// Cleanup
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
	mcc_semantic_check_delete_single_check(check);
}

// A function is declared that has the same name as one of the built ins
void define_built_in(CuTest *tc)
{

	// Define test input and create symbol table
	const char input[] = "int print_nl(){}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
	struct mcc_semantic_check *check = mcc_semantic_check_initialize_check();
	CuAssertPtrNotNull(tc,check);
	enum mcc_semantic_check_error_code error =
	    mcc_semantic_check_run_multiple_function_definitions((&parser_result)->program, table, check);
	CuAssertIntEquals(tc, MCC_SEMANTIC_CHECK_ERROR_OK, error);

	CuAssertPtrNotNull(tc, check);
	CuAssertIntEquals(tc, check->status, MCC_SEMANTIC_CHECK_FAIL);
	CuAssertPtrNotNull(tc, check->error_buffer);

	// Cleanup
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
	mcc_semantic_check_delete_single_check(check);
}

// Void function that returns something
void function_return_value1(CuTest *tc)
{
	// Define test input and create symbol table
	const char input[] = "void test(int a){return a;}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
	struct mcc_semantic_check *check =
	    mcc_semantic_check_initialize_check();
	CuAssertPtrNotNull(tc,check);
	enum mcc_semantic_check_error_code error = mcc_semantic_check_run_type_check((&parser_result)->program, table, check);
	CuAssertIntEquals(tc,MCC_SEMANTIC_CHECK_ERROR_OK,error);

	CuAssertPtrNotNull(tc, check->error_buffer);
	CuAssertPtrNotNull(tc, check);
	CuAssertIntEquals(tc, check->status, MCC_SEMANTIC_CHECK_FAIL);

	// Cleanup
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
	mcc_semantic_check_delete_single_check(check);
}

// Function that returns wrong type
void function_return_value2(CuTest *tc)
{
	// Define test input and create symbol table
	const char input[] = "int main(){float a; a = 1.0; return a;}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
	struct mcc_semantic_check *check =
	    mcc_semantic_check_initialize_check();
	CuAssertPtrNotNull(tc,check);
	enum mcc_semantic_check_error_code error = mcc_semantic_check_run_type_check((&parser_result)->program, table, check);
	CuAssertIntEquals(tc,MCC_SEMANTIC_CHECK_ERROR_OK,error);

	CuAssertPtrNotNull(tc, check->error_buffer);
	CuAssertPtrNotNull(tc, check);
	CuAssertIntEquals(tc, check->status, MCC_SEMANTIC_CHECK_FAIL);

	// Cleanup
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
	mcc_semantic_check_delete_single_check(check);
}

// Function that returns array but with correct type
void function_return_value3(CuTest *tc)
{
	// Define test input and create symbol table
	const char input[] = "int main(){int[42] a; return a;}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
	struct mcc_semantic_check *check =
	    mcc_semantic_check_initialize_check();
	CuAssertPtrNotNull(tc,check);
	enum mcc_semantic_check_error_code error = mcc_semantic_check_run_type_check((&parser_result)->program, table, check);
	CuAssertIntEquals(tc,MCC_SEMANTIC_CHECK_ERROR_OK,error);

	CuAssertPtrNotNull(tc, check->error_buffer);
	CuAssertPtrNotNull(tc, check);
	CuAssertIntEquals(tc, check->status, MCC_SEMANTIC_CHECK_FAIL);

	// Cleanup
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
	mcc_semantic_check_delete_single_check(check);
}

// Calling a function with the wrong type of parameters
void function_arguments1(CuTest *tc)
{
	// Define test input and create symbol table
	const char input[] = "int main(){float a; a = 1.0; print_int(a);}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
	struct mcc_semantic_check *check = mcc_semantic_check_initialize_check();
	CuAssertPtrNotNull(tc,check);
	enum mcc_semantic_check_error_code error = mcc_semantic_check_run_type_check((&parser_result)->program, table, check);
	CuAssertIntEquals(tc,MCC_SEMANTIC_CHECK_ERROR_OK,error);

	CuAssertPtrNotNull(tc, check->error_buffer);
	CuAssertPtrNotNull(tc, check);
	CuAssertIntEquals(tc, check->status, MCC_SEMANTIC_CHECK_FAIL);

	// Cleanup
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
	mcc_semantic_check_delete_single_check(check);
}

// Calling a function with the wrong type of parameters
void function_arguments2(CuTest *tc)
{
	// Define test input and create symbol table
	const char input[] = "int main(){string a; a = \" teststring \"; call_me_with_int(a);} "
	                     "void call_me_with_int(int a){return;}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
	struct mcc_semantic_check *check = mcc_semantic_check_initialize_check();
	CuAssertPtrNotNull(tc,check);
	enum mcc_semantic_check_error_code error = mcc_semantic_check_run_type_check((&parser_result)->program, table, check);
	CuAssertIntEquals(tc,MCC_SEMANTIC_CHECK_ERROR_OK,error);

	CuAssertPtrNotNull(tc, check->error_buffer);
	CuAssertPtrNotNull(tc, check);
	CuAssertIntEquals(tc, check->status, MCC_SEMANTIC_CHECK_FAIL);

	// Cleanup
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
	mcc_semantic_check_delete_single_check(check);
}

// invalid array operation, assignment of whole array
void invalid_array_operation(CuTest *tc)
{
	// Define test input and create symbol table
	const char input[] = "int main(){int[10] array; array = 1; return 0;}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
	struct mcc_semantic_check *check = mcc_semantic_check_initialize_check();
	CuAssertPtrNotNull(tc,check);
	enum mcc_semantic_check_error_code error = mcc_semantic_check_run_type_check((&parser_result)->program, table, check);
	CuAssertIntEquals(tc,MCC_SEMANTIC_CHECK_ERROR_OK,error);

	CuAssertPtrNotNull(tc, check->error_buffer);
	CuAssertPtrNotNull(tc, check);
	CuAssertIntEquals(tc, check->status, MCC_SEMANTIC_CHECK_FAIL);

	// Cleanup
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
	mcc_semantic_check_delete_single_check(check);
}

// invalid array operation, array element access with variable of wrong type
void invalid_array_operation2(CuTest *tc)
{
	// Define test input and create symbol table
	const char input[] = "int main(){float i; i=0.5; int[10] a; a[i] = 10; return 0;}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
	struct mcc_semantic_check *check = mcc_semantic_check_initialize_check();
	CuAssertPtrNotNull(tc,check);
	enum mcc_semantic_check_error_code error = mcc_semantic_check_run_type_check((&parser_result)->program, table, check);
	CuAssertIntEquals(tc,MCC_SEMANTIC_CHECK_ERROR_OK,error);

	CuAssertPtrNotNull(tc, check->error_buffer);
	CuAssertPtrNotNull(tc, check);
	CuAssertIntEquals(tc, check->status, MCC_SEMANTIC_CHECK_FAIL);

	// Cleanup
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
	mcc_semantic_check_delete_single_check(check);
}

// invalid array operation, array assignment with wrong type
void invalid_array_operation3(CuTest *tc)
{
	// Define test input and create symbol table
	const char input[] = "int main(){float f; int[10] a; a[1] = f; return 0;}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
	struct mcc_semantic_check *check = mcc_semantic_check_initialize_check();
	CuAssertPtrNotNull(tc,check);
	enum mcc_semantic_check_error_code error = mcc_semantic_check_run_type_check((&parser_result)->program, table, check);
	CuAssertIntEquals(tc,MCC_SEMANTIC_CHECK_ERROR_OK,error);

	CuAssertPtrNotNull(tc, check->error_buffer);
	CuAssertPtrNotNull(tc, check);
	CuAssertIntEquals(tc, check->status, MCC_SEMANTIC_CHECK_FAIL);

	// Cleanup
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
	mcc_semantic_check_delete_single_check(check);
}

// invalid array operation, array element access with expression of wrong type
void invalid_array_operation4(CuTest *tc)
{
	// Define test input and create symbol table
	const char input[] = "int main(){int[10] a; int b; b = a[1 == 0]; return 0;}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
	struct mcc_semantic_check *check = mcc_semantic_check_initialize_check();
	CuAssertPtrNotNull(tc,check);
	enum mcc_semantic_check_error_code error = mcc_semantic_check_run_type_check((&parser_result)->program, table, check);
	CuAssertIntEquals(tc,MCC_SEMANTIC_CHECK_ERROR_OK,error);

	CuAssertPtrNotNull(tc, check->error_buffer);
	CuAssertPtrNotNull(tc, check);
	CuAssertIntEquals(tc, check->status, MCC_SEMANTIC_CHECK_FAIL);

	// Cleanup
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
	mcc_semantic_check_delete_single_check(check);
}

// invalid array operation, assignment to a variable of array type
void invalid_array_operation5(CuTest *tc)
{
	// Define test input and create symbol table
	const char input[] = "int main(){int[10] a; int[10] c; a = c; return 0;}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
	struct mcc_semantic_check *check = mcc_semantic_check_initialize_check();
	CuAssertPtrNotNull(tc,check);
	enum mcc_semantic_check_error_code error = mcc_semantic_check_run_type_check((&parser_result)->program, table, check);
	CuAssertIntEquals(tc,MCC_SEMANTIC_CHECK_ERROR_OK,error);

	CuAssertPtrNotNull(tc, check->error_buffer);
	CuAssertPtrNotNull(tc, check);
	CuAssertIntEquals(tc, check->status, MCC_SEMANTIC_CHECK_FAIL);

	// Cleanup
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
	mcc_semantic_check_delete_single_check(check);
}

#define TESTS \
	TEST(positive) \
	TEST(ensure_variable_shadowing) \
	TEST(type_conversion_expression) \
	TEST(type_conversion_expression2) \
	TEST(type_conversion_expression3) \
	TEST(type_conversion_expression4) \
	TEST(type_conversion_expression5) \
	TEST(type_conversion_expression6) \
	TEST(type_conversion_expression7) \
	TEST(type_conversion_expression8) \
	TEST(type_conversion_expression9) \
	TEST(type_conversion_if) \
	TEST(type_conversion_if_else) \
	TEST(type_conversion_while) \
	TEST(type_conversion_assignment) \
	TEST(type_conversion_assignment2) \
	TEST(type_conversion_assignment3) \
	TEST(type_conversion_assignment4) \
	TEST(type_conversion_assignment5) \
	TEST(nonvoid_check) \
	TEST(nonvoid_check2) \
	TEST(nonvoid_check3) \
	TEST(nonvoid_check4) \
	TEST(main_function_1) \
	TEST(main_function_2) \
	TEST(main_function_3) \
	TEST(unknown_function_call) \
	TEST(unknown_function_call2) \
	TEST(multiple_function_definitions) \
	TEST(multiple_function_definitions2) \
	TEST(multiple_function_definitions3) \
	TEST(multiple_variable_declarations) \
	TEST(multiple_variable_declarations2) \
	TEST(use_undeclared_variable) \
	TEST(use_undeclared_variable2) \
	TEST(use_undeclared_variable3) \
	TEST(use_undeclared_variable4) \
	TEST(use_undeclared_variable5) \
	TEST(use_undeclared_variable6) \
	TEST(use_undeclared_variable7) \
	TEST(use_undeclared_variable8) \
	TEST(define_built_in) \
	TEST(function_return_value1) \
	TEST(function_return_value2) \
	TEST(function_return_value3) \
	TEST(function_arguments1) \
	TEST(function_arguments2) \
	TEST(invalid_array_operation) \
	TEST(invalid_array_operation2) \
	TEST(invalid_array_operation3) \
	TEST(invalid_array_operation4) \
	TEST(invalid_array_operation5)
#include "main_stub.inc"
#undef TESTS
