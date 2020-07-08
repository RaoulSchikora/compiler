#include <CuTest.h>

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "mcc/ast.h"
#include "mcc/ir.h"
#include "mcc/semantic_checks.h"
#include "mcc/symbol_table.h"

void test1(CuTest *tc)
{
	// Define test input and create symbol table
	const char input[] = "int main(){return 42;}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM, "test");
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
	struct mcc_semantic_check *checks = mcc_semantic_check_run_all((&parser_result)->program, table);
	CuAssertIntEquals(tc, checks->status, MCC_SEMANTIC_CHECK_OK);

	struct mcc_ir_row *ir = mcc_ir_generate((&parser_result)->program);
	struct mcc_ir_row *ir_head = ir;

	CuAssertPtrNotNull(tc, ir);

	CuAssertIntEquals(tc, ir->instr, MCC_IR_INSTR_FUNC_LABEL);
	CuAssertIntEquals(tc, ir->arg1->type, MCC_IR_TYPE_FUNC_LABEL);
	CuAssertStrEquals(tc, ir->arg1->func_label, "main");
	CuAssertPtrEquals(tc, ir->arg2, NULL);

	struct mcc_ir_row *ir_next = ir->next_row;
	CuAssertPtrNotNull(tc, ir);

	CuAssertIntEquals(tc, ir_next->instr, MCC_IR_INSTR_RETURN);
	CuAssertIntEquals(tc, ir_next->arg1->type, MCC_IR_TYPE_LIT_INT);
	CuAssertIntEquals(tc, (int)ir_next->arg1->lit_int, 42);
	CuAssertPtrEquals(tc, ir_next->arg2, NULL);
	// CuAssertPtrEquals(tc, ir_next->next_row, NULL);

	// Cleanup
	mcc_ir_delete_ir(ir_head);
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
	mcc_semantic_check_delete_single_check(checks);
}

void expression(CuTest *tc)
{
	const char input[] = "int main(){ 0 + 0 + 1; return 0;}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM, "test");
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);

	struct mcc_ir_row *ir_head = mcc_ir_generate((&parser_result)->program);
	struct mcc_ir_row *ir = ir_head->next_row;

	CuAssertPtrNotNull(tc, ir);

	CuAssertIntEquals(tc, ir->instr, MCC_IR_INSTR_PLUS);
	CuAssertPtrNotNull(tc, ir->arg1);
	CuAssertIntEquals(tc, ir->arg1->type, MCC_IR_TYPE_LIT_INT);
	CuAssertIntEquals(tc, (int)ir->arg1->lit_int, 0);
	CuAssertIntEquals(tc, ir->arg2->type, MCC_IR_TYPE_LIT_INT);
	CuAssertIntEquals(tc, (int)ir->arg2->lit_int, 0);

	struct mcc_ir_row *next_ir = ir->next_row;
	CuAssertPtrNotNull(tc, next_ir);

	CuAssertIntEquals(tc, next_ir->instr, MCC_IR_INSTR_PLUS);
	CuAssertIntEquals(tc, next_ir->arg1->type, MCC_IR_TYPE_ROW);
	CuAssertPtrEquals(tc, next_ir->arg1->row, ir);
	CuAssertIntEquals(tc, next_ir->arg2->type, MCC_IR_TYPE_LIT_INT);
	CuAssertIntEquals(tc, (int)next_ir->arg2->lit_int, 1);

	// Cleanup
	mcc_ir_delete_ir(ir_head);
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
}

void exp_plus_exp(CuTest *tc)
{
	const char input[] = "int main(){ (1 + 2) - (3 + 4); return 0;}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM, "test");
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);

	struct mcc_ir_row *ir_head = mcc_ir_generate((&parser_result)->program);
	struct mcc_ir_row *ir = ir_head->next_row;

	CuAssertPtrNotNull(tc, ir);

	struct mcc_ir_row *exp_row = ir->next_row->next_row;

	CuAssertIntEquals(tc, exp_row->instr, MCC_IR_INSTR_MINUS);
	CuAssertPtrNotNull(tc, exp_row->arg1);
	CuAssertIntEquals(tc, exp_row->arg1->type, MCC_IR_TYPE_ROW);
	CuAssertPtrEquals(tc, exp_row->arg1->row, ir);
	CuAssertIntEquals(tc, exp_row->arg2->type, MCC_IR_TYPE_ROW);
	CuAssertPtrEquals(tc, exp_row->arg2->row, ir->next_row);

	// Cleanup
	mcc_ir_delete_ir(ir_head);
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
}

void expression_var(CuTest *tc)
{
	const char input[] = "int main(){ int a; a = 3; a + 1; return 0;}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM, "test");
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);

	struct mcc_ir_row *ir_head = mcc_ir_generate((&parser_result)->program);
	struct mcc_ir_row *ir = ir_head->next_row;

	CuAssertPtrNotNull(tc, ir);

	CuAssertIntEquals(tc, ir->instr, MCC_IR_INSTR_ASSIGN);
	CuAssertPtrNotNull(tc, ir->arg1);
	CuAssertIntEquals(tc, ir->arg1->type, MCC_IR_TYPE_IDENTIFIER);
	CuAssertStrEquals(tc, ir->arg1->ident, "a");
	CuAssertIntEquals(tc, ir->arg2->type, MCC_IR_TYPE_LIT_INT);
	CuAssertIntEquals(tc, (int)ir->arg2->lit_int, 3);

	struct mcc_ir_row *next_ir = ir->next_row;
	CuAssertPtrNotNull(tc, next_ir);

	CuAssertIntEquals(tc, next_ir->instr, MCC_IR_INSTR_PLUS);
	CuAssertIntEquals(tc, next_ir->arg1->type, MCC_IR_TYPE_IDENTIFIER);
	CuAssertStrEquals(tc, next_ir->arg1->ident, "a");
	CuAssertIntEquals(tc, next_ir->arg2->type, MCC_IR_TYPE_LIT_INT);
	CuAssertIntEquals(tc, (int)next_ir->arg2->lit_int, 1);

	// Cleanup
	mcc_ir_delete_ir(ir_head);
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
}

void expression_arr(CuTest *tc)
{
	const char input[] = "int main(){ int[10] arr; arr[4] = 3; return arr[4];}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM, "test");
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);

	struct mcc_ir_row *ir_head = mcc_ir_generate((&parser_result)->program);
	struct mcc_ir_row *ir = ir_head->next_row;

	CuAssertIntEquals(tc, ir->instr, MCC_IR_INSTR_ARRAY);
	CuAssertPtrNotNull(tc, ir->arg1);
	CuAssertIntEquals(tc, ir->arg1->type, MCC_IR_TYPE_IDENTIFIER);
	CuAssertStrEquals(tc, ir->arg1->arr_ident, "arr");
	CuAssertIntEquals(tc, (int)ir->arg2->type, MCC_IR_TYPE_LIT_INT);
	CuAssertIntEquals(tc, (int)ir->arg2->lit_int, 10);

	ir = ir->next_row;

	CuAssertIntEquals(tc, ir->instr, MCC_IR_INSTR_ASSIGN);
	CuAssertPtrNotNull(tc, ir->arg1);
	CuAssertIntEquals(tc, ir->arg1->type, MCC_IR_TYPE_ARR_ELEM);
	CuAssertStrEquals(tc, ir->arg1->arr_ident, "arr");
	CuAssertIntEquals(tc, (int)ir->arg1->index->type, MCC_IR_TYPE_LIT_INT);
	CuAssertIntEquals(tc, (int)ir->arg1->index->lit_int, 4);
	CuAssertIntEquals(tc, ir->arg2->type, MCC_IR_TYPE_LIT_INT);
	CuAssertIntEquals(tc, (int)ir->arg2->lit_int, 3);

	struct mcc_ir_row *next_ir = ir->next_row;
	CuAssertPtrNotNull(tc, next_ir);

	CuAssertIntEquals(tc, next_ir->instr, MCC_IR_INSTR_RETURN);
	CuAssertIntEquals(tc, next_ir->arg1->type, MCC_IR_TYPE_ARR_ELEM);
	CuAssertStrEquals(tc, ir->arg1->arr_ident, "arr");
	CuAssertIntEquals(tc, (int)ir->arg1->index->type, MCC_IR_TYPE_LIT_INT);
	CuAssertIntEquals(tc, (int)ir->arg1->index->lit_int, 4);

	// Cleanup
	mcc_ir_delete_ir(ir_head);
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
}

void if_stmt(CuTest *tc)
{
	const char input[] = "int main(){if(0==1) 1*2+2; return 0;}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM, "test");
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);

	struct mcc_ir_row *ir = mcc_ir_generate((&parser_result)->program);
	struct mcc_ir_row *ir_head = ir;
	struct mcc_ir_row *tmp = ir;
	// Skip first row
	ir = ir->next_row;

	// Condition
	CuAssertPtrNotNull(tc, ir);
	CuAssertIntEquals(tc, ir->instr, MCC_IR_INSTR_EQUALS);
	CuAssertPtrNotNull(tc, ir->arg1);
	CuAssertIntEquals(tc, (int)ir->arg1->type, MCC_IR_TYPE_LIT_INT);
	CuAssertIntEquals(tc, ir->arg1->lit_int, 0);
	CuAssertIntEquals(tc, ir->arg2->type, MCC_IR_TYPE_LIT_INT);
	CuAssertIntEquals(tc, (int)ir->arg2->lit_int, 1);

	// Jumpfalse L0
	tmp = ir;
	ir = ir->next_row;
	CuAssertPtrNotNull(tc, ir);
	CuAssertIntEquals(tc, ir->instr, MCC_IR_INSTR_JUMPFALSE);
	CuAssertPtrNotNull(tc, ir->arg1);
	CuAssertIntEquals(tc, ir->arg1->type, MCC_IR_TYPE_ROW);
	CuAssertPtrEquals(tc, ir->arg1->row, tmp);
	CuAssertIntEquals(tc, ir->arg2->type, MCC_IR_TYPE_LABEL);
	CuAssertIntEquals(tc, ir->arg2->label, 0);

	// On true: 1*2
	ir = ir->next_row;
	CuAssertPtrNotNull(tc, ir);
	CuAssertIntEquals(tc, ir->instr, MCC_IR_INSTR_MULTIPLY);
	CuAssertPtrNotNull(tc, ir->arg1);
	CuAssertPtrNotNull(tc, ir->arg2);
	CuAssertIntEquals(tc, ir->arg1->type, MCC_IR_TYPE_LIT_INT);
	CuAssertIntEquals(tc, ir->arg2->type, MCC_IR_TYPE_LIT_INT);
	CuAssertIntEquals(tc, ir->arg1->lit_int, 1);
	CuAssertIntEquals(tc, ir->arg2->lit_int, 2);

	// On true: x + 2
	tmp = ir;
	ir = ir->next_row;
	CuAssertPtrNotNull(tc, ir);
	CuAssertIntEquals(tc, ir->instr, MCC_IR_INSTR_PLUS);
	CuAssertPtrNotNull(tc, ir->arg1);
	CuAssertPtrNotNull(tc, ir->arg2);
	CuAssertIntEquals(tc, ir->arg1->type, MCC_IR_TYPE_ROW);
	CuAssertIntEquals(tc, ir->arg2->type, MCC_IR_TYPE_LIT_INT);
	CuAssertPtrEquals(tc, ir->arg1->row, tmp);
	CuAssertIntEquals(tc, ir->arg2->lit_int, 2);

	// L0
	tmp = ir;
	ir = ir->next_row;
	CuAssertPtrNotNull(tc, ir);
	CuAssertIntEquals(tc, ir->instr, MCC_IR_INSTR_LABEL);
	CuAssertPtrEquals(tc, ir->arg2, NULL);
	CuAssertIntEquals(tc, ir->arg1->type, MCC_IR_TYPE_LABEL);
	CuAssertIntEquals(tc, ir->arg1->label, 0);

	// Cleanup
	mcc_ir_delete_ir(ir_head);
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
}

void if_else_stmt(CuTest *tc)
{
	const char input[] = "int main(){if(0==1) {1*2;} else {3+4;} return 0;}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM, "test");
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);

	struct mcc_ir_row *ir = mcc_ir_generate((&parser_result)->program);
	struct mcc_ir_row *ir_head = ir;
	struct mcc_ir_row *tmp = ir;
	// Skip first row
	ir = ir->next_row;

	// Condition
	CuAssertPtrNotNull(tc, ir);
	CuAssertIntEquals(tc, ir->instr, MCC_IR_INSTR_EQUALS);
	CuAssertPtrNotNull(tc, ir->arg1);
	CuAssertIntEquals(tc, (int)ir->arg1->type, MCC_IR_TYPE_LIT_INT);
	CuAssertIntEquals(tc, ir->arg1->lit_int, 0);
	CuAssertIntEquals(tc, ir->arg2->type, MCC_IR_TYPE_LIT_INT);
	CuAssertIntEquals(tc, (int)ir->arg2->lit_int, 1);

	// Jumpfalse L0
	tmp = ir;
	ir = ir->next_row;
	CuAssertPtrNotNull(tc, ir);
	CuAssertIntEquals(tc, ir->instr, MCC_IR_INSTR_JUMPFALSE);
	CuAssertPtrNotNull(tc, ir->arg1);
	CuAssertIntEquals(tc, ir->arg1->type, MCC_IR_TYPE_ROW);
	CuAssertPtrEquals(tc, ir->arg1->row, tmp);
	CuAssertIntEquals(tc, ir->arg2->type, MCC_IR_TYPE_LABEL);
	CuAssertIntEquals(tc, ir->arg2->label, 0);

	// On true: 1*2
	ir = ir->next_row;
	CuAssertPtrNotNull(tc, ir);
	CuAssertIntEquals(tc, ir->instr, MCC_IR_INSTR_MULTIPLY);
	CuAssertPtrNotNull(tc, ir->arg1);
	CuAssertPtrNotNull(tc, ir->arg2);
	CuAssertIntEquals(tc, ir->arg1->type, MCC_IR_TYPE_LIT_INT);
	CuAssertIntEquals(tc, ir->arg2->type, MCC_IR_TYPE_LIT_INT);
	CuAssertIntEquals(tc, ir->arg1->lit_int, 1);
	CuAssertIntEquals(tc, ir->arg2->lit_int, 2);

	// jump L1
	tmp = ir;
	ir = ir->next_row;
	CuAssertPtrNotNull(tc, ir);
	CuAssertIntEquals(tc, ir->instr, MCC_IR_INSTR_JUMP);
	CuAssertPtrNotNull(tc, ir->arg1);
	CuAssertPtrEquals(tc, NULL, ir->arg2);
	CuAssertIntEquals(tc, ir->arg1->type, MCC_IR_TYPE_LABEL);
	CuAssertIntEquals(tc, ir->arg1->label, 1);

	// L0
	tmp = ir;
	ir = ir->next_row;
	CuAssertPtrNotNull(tc, ir);
	CuAssertIntEquals(tc, ir->instr, MCC_IR_INSTR_LABEL);
	CuAssertPtrEquals(tc, ir->arg2, NULL);
	CuAssertIntEquals(tc, ir->arg1->type, MCC_IR_TYPE_LABEL);
	CuAssertIntEquals(tc, ir->arg1->label, 0);

	// On false: 3+4
	ir = ir->next_row;
	CuAssertPtrNotNull(tc, ir);
	CuAssertIntEquals(tc, ir->instr, MCC_IR_INSTR_PLUS);
	CuAssertPtrNotNull(tc, ir->arg1);
	CuAssertPtrNotNull(tc, ir->arg2);
	CuAssertIntEquals(tc, ir->arg1->type, MCC_IR_TYPE_LIT_INT);
	CuAssertIntEquals(tc, ir->arg2->type, MCC_IR_TYPE_LIT_INT);
	CuAssertIntEquals(tc, ir->arg1->lit_int, 3);
	CuAssertIntEquals(tc, ir->arg2->lit_int, 4);

	// L1
	tmp = ir;
	ir = ir->next_row;
	CuAssertPtrNotNull(tc, ir);
	CuAssertIntEquals(tc, ir->instr, MCC_IR_INSTR_LABEL);
	CuAssertPtrEquals(tc, ir->arg2, NULL);
	CuAssertIntEquals(tc, ir->arg1->type, MCC_IR_TYPE_LABEL);
	CuAssertIntEquals(tc, ir->arg1->label, 1);

	// Cleanup
	mcc_ir_delete_ir(ir_head);
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
}

void while_stmt(CuTest *tc)
{
	const char input[] = "int main(){int a; a = 1; while(a<11) {a = a + 1;} return 0;}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM, "test");
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);

	struct mcc_ir_row *ir = mcc_ir_generate((&parser_result)->program);
	struct mcc_ir_row *ir_head = ir;
	struct mcc_ir_row *tmp = ir;
	// Skip first row
	ir = ir->next_row;

	// a = 1
	CuAssertPtrNotNull(tc, ir);
	CuAssertIntEquals(tc, ir->instr, MCC_IR_INSTR_ASSIGN);
	CuAssertPtrNotNull(tc, ir->arg1);
	CuAssertIntEquals(tc, ir->arg1->type, MCC_IR_TYPE_IDENTIFIER);
	CuAssertStrEquals(tc, ir->arg1->ident, "a");
	CuAssertIntEquals(tc, ir->arg2->type, MCC_IR_TYPE_LIT_INT);
	CuAssertIntEquals(tc, ir->arg2->lit_int, 1);

	// L0
	tmp = ir;
	ir = ir->next_row;
	CuAssertPtrNotNull(tc, ir);
	CuAssertIntEquals(tc, ir->instr, MCC_IR_INSTR_LABEL);
	CuAssertPtrEquals(tc, ir->arg2, NULL);
	CuAssertIntEquals(tc, ir->arg1->type, MCC_IR_TYPE_LABEL);
	CuAssertIntEquals(tc, ir->arg1->label, 0);

	// condition
	tmp = ir;
	ir = ir->next_row;
	CuAssertPtrNotNull(tc, ir);
	CuAssertIntEquals(tc, ir->instr, MCC_IR_INSTR_SMALLER);
	CuAssertPtrNotNull(tc, ir->arg1);
	CuAssertIntEquals(tc, ir->arg1->type, MCC_IR_TYPE_IDENTIFIER);
	CuAssertStrEquals(tc, ir->arg1->ident, "a");
	CuAssertIntEquals(tc, ir->arg2->type, MCC_IR_TYPE_LIT_INT);
	CuAssertIntEquals(tc, ir->arg2->label, 11);

	// Jumpfalse L1
	tmp = ir;
	ir = ir->next_row;
	CuAssertPtrNotNull(tc, ir);
	CuAssertIntEquals(tc, ir->instr, MCC_IR_INSTR_JUMPFALSE);
	CuAssertPtrNotNull(tc, ir->arg1);
	CuAssertIntEquals(tc, ir->arg1->type, MCC_IR_TYPE_ROW);
	CuAssertPtrEquals(tc, ir->arg1->row, tmp);
	CuAssertIntEquals(tc, ir->arg2->type, MCC_IR_TYPE_LABEL);
	CuAssertIntEquals(tc, ir->arg2->label, 1);

	// On true: a + 1
	ir = ir->next_row;
	CuAssertPtrNotNull(tc, ir);
	CuAssertIntEquals(tc, ir->instr, MCC_IR_INSTR_PLUS);
	CuAssertPtrNotNull(tc, ir->arg1);
	CuAssertPtrNotNull(tc, ir->arg2);
	CuAssertIntEquals(tc, ir->arg1->type, MCC_IR_TYPE_IDENTIFIER);
	CuAssertStrEquals(tc, ir->arg1->ident, "a");
	CuAssertIntEquals(tc, ir->arg2->type, MCC_IR_TYPE_LIT_INT);
	CuAssertIntEquals(tc, ir->arg2->lit_int, 1);

	// On true: a = x
	tmp = ir;
	ir = ir->next_row;
	CuAssertPtrNotNull(tc, ir);
	CuAssertIntEquals(tc, ir->instr, MCC_IR_INSTR_ASSIGN);
	CuAssertPtrNotNull(tc, ir->arg1);
	CuAssertPtrNotNull(tc, ir->arg2);
	CuAssertIntEquals(tc, ir->arg1->type, MCC_IR_TYPE_IDENTIFIER);
	CuAssertStrEquals(tc, ir->arg1->ident, "a");
	CuAssertIntEquals(tc, ir->arg2->type, MCC_IR_TYPE_ROW);
	CuAssertPtrEquals(tc, ir->arg2->row, tmp);

	// jump L0
	tmp = ir;
	ir = ir->next_row;
	CuAssertPtrNotNull(tc, ir);
	CuAssertIntEquals(tc, ir->instr, MCC_IR_INSTR_JUMP);
	CuAssertPtrNotNull(tc, ir->arg1);
	CuAssertPtrEquals(tc, NULL, ir->arg2);
	CuAssertIntEquals(tc, ir->arg1->type, MCC_IR_TYPE_LABEL);
	CuAssertIntEquals(tc, ir->arg1->label, 0);

	// L1
	tmp = ir;
	ir = ir->next_row;
	CuAssertPtrNotNull(tc, ir);
	CuAssertIntEquals(tc, ir->instr, MCC_IR_INSTR_LABEL);
	CuAssertPtrEquals(tc, ir->arg2, NULL);
	CuAssertIntEquals(tc, ir->arg1->type, MCC_IR_TYPE_LABEL);
	CuAssertIntEquals(tc, ir->arg1->label, 1);

	// Cleanup
	mcc_ir_delete_ir(ir_head);
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
}

void func_def(CuTest *tc)
{
	const char input[] = "int test(int a, float b){return 0;} int main(){return 0;}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM, "test");
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);

	struct mcc_ir_row *ir = mcc_ir_generate((&parser_result)->program);
	struct mcc_ir_row *tmp = ir;
	struct mcc_ir_row *ir_head = ir;

	// Label test
	CuAssertPtrNotNull(tc, ir);
	CuAssertIntEquals(tc, ir->instr, MCC_IR_INSTR_FUNC_LABEL);
	CuAssertPtrNotNull(tc, ir->arg1);
	CuAssertPtrEquals(tc, NULL, ir->arg2);
	CuAssertIntEquals(tc, ir->arg1->type, MCC_IR_TYPE_FUNC_LABEL);
	CuAssertStrEquals(tc, ir->arg1->func_label, "test");

	// Pop and assign a
	ir = ir->next_row;
	CuAssertPtrNotNull(tc, ir);
	CuAssertIntEquals(tc, ir->instr, MCC_IR_INSTR_POP);
	CuAssertPtrEquals(tc, NULL, ir->arg1);
	CuAssertPtrEquals(tc, NULL, ir->arg2);

	tmp = ir;
	ir = ir->next_row;
	CuAssertPtrNotNull(tc, ir);
	CuAssertIntEquals(tc, ir->instr, MCC_IR_INSTR_ASSIGN);
	CuAssertIntEquals(tc, ir->arg1->type, MCC_IR_TYPE_IDENTIFIER);
	CuAssertStrEquals(tc, ir->arg1->ident, "a");
	CuAssertIntEquals(tc, ir->arg2->type, MCC_IR_TYPE_ROW);
	CuAssertPtrEquals(tc, ir->arg2->row, tmp);

	// Pop and assign b
	ir = ir->next_row;
	CuAssertPtrNotNull(tc, ir);
	CuAssertIntEquals(tc, ir->instr, MCC_IR_INSTR_POP);
	CuAssertPtrEquals(tc, NULL, ir->arg1);
	CuAssertPtrEquals(tc, NULL, ir->arg2);

	tmp = ir;
	ir = ir->next_row;
	CuAssertPtrNotNull(tc, ir);
	CuAssertIntEquals(tc, ir->instr, MCC_IR_INSTR_ASSIGN);
	CuAssertIntEquals(tc, ir->arg1->type, MCC_IR_TYPE_IDENTIFIER);
	CuAssertStrEquals(tc, ir->arg1->ident, "b");
	CuAssertIntEquals(tc, ir->arg2->type, MCC_IR_TYPE_ROW);
	CuAssertPtrEquals(tc, ir->arg2->row, tmp);

	// Return 0
	ir = ir->next_row;
	CuAssertPtrNotNull(tc, ir);
	CuAssertIntEquals(tc, ir->instr, MCC_IR_INSTR_RETURN);
	CuAssertIntEquals(tc, ir->arg1->type, MCC_IR_TYPE_LIT_INT);
	CuAssertIntEquals(tc, ir->arg1->lit_int, 0);
	CuAssertPtrEquals(tc, NULL, ir->arg2);

	// Cleanup
	mcc_ir_delete_ir(ir_head);
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
}

void func_call(CuTest *tc)
{
	const char input[] = "int main(){int a; a = 1; float b; b = 2.0; int c; c = test(a,b); return 0;} int test(int "
	                     "a, float b){return 0;} ";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM, "test");
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);

	struct mcc_ir_row *ir = mcc_ir_generate((&parser_result)->program);
	struct mcc_ir_row *tmp = ir;
	struct mcc_ir_row *ir_head = ir;

	tmp = tmp->next_row->next_row->next_row->next_row;

	CuAssertPtrNotNull(tc, tmp);
	CuAssertIntEquals(tc, tmp->instr, MCC_IR_INSTR_PUSH);
	CuAssertIntEquals(tc, tmp->arg1->type, MCC_IR_TYPE_IDENTIFIER);
	CuAssertStrEquals(tc, tmp->arg1->ident, "b");
	CuAssertPtrEquals(tc, NULL, tmp->arg2);

	tmp = tmp->next_row;

	CuAssertPtrNotNull(tc, tmp);
	CuAssertIntEquals(tc, tmp->instr, MCC_IR_INSTR_PUSH);
	CuAssertIntEquals(tc, tmp->arg1->type, MCC_IR_TYPE_IDENTIFIER);
	CuAssertStrEquals(tc, tmp->arg1->ident, "a");
	CuAssertPtrEquals(tc, NULL, tmp->arg2);

	tmp = tmp->next_row;

	CuAssertPtrNotNull(tc, tmp);
	CuAssertIntEquals(tc, tmp->instr, MCC_IR_INSTR_CALL);
	CuAssertIntEquals(tc, tmp->arg1->type, MCC_IR_TYPE_IDENTIFIER);
	CuAssertStrEquals(tc, tmp->arg1->ident, "test");
	CuAssertPtrEquals(tc, NULL, tmp->arg2);

	struct mcc_ir_row *ass = tmp->next_row;

	CuAssertPtrNotNull(tc, ass);
	CuAssertIntEquals(tc, ass->instr, MCC_IR_INSTR_ASSIGN);
	CuAssertIntEquals(tc, ass->arg1->type, MCC_IR_TYPE_IDENTIFIER);
	CuAssertStrEquals(tc, ass->arg1->ident, "c");
	CuAssertPtrNotNull(tc, ass->arg2);
	CuAssertIntEquals(tc, ass->arg2->type, MCC_IR_TYPE_ROW);
	CuAssertPtrEquals(tc, ass->arg2->row, tmp);

	// Cleanup
	mcc_ir_delete_ir(ir_head);
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
}

void variable_shadowing(CuTest *tc)
{
	const char input[] = "int main(){int a; a = 42; while (true) { int a;a = 43;if (true) {int a;a = 45;a = a + "
	                     "2;}a = a + 3;}a = a + 4;return 0;}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM, "test");
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);

	struct mcc_ir_row *ir = mcc_ir_generate((&parser_result)->program);
	struct mcc_ir_row *ir_head = ir;
	struct mcc_ir_row *tmp = ir->next_row;

	CuAssertIntEquals(tc, tmp->instr, MCC_IR_INSTR_ASSIGN);
	CuAssertIntEquals(tc, tmp->arg1->type, MCC_IR_TYPE_IDENTIFIER);
	CuAssertStrEquals(tc, tmp->arg1->ident, "a");

	tmp = tmp->next_row->next_row->next_row;

	CuAssertIntEquals(tc, tmp->instr, MCC_IR_INSTR_ASSIGN);
	CuAssertIntEquals(tc, tmp->arg1->type, MCC_IR_TYPE_IDENTIFIER);
	CuAssertStrEquals(tc, tmp->arg1->ident, "$r1");

	tmp = tmp->next_row->next_row;

	CuAssertIntEquals(tc, tmp->instr, MCC_IR_INSTR_ASSIGN);
	CuAssertIntEquals(tc, tmp->arg1->type, MCC_IR_TYPE_IDENTIFIER);
	CuAssertStrEquals(tc, tmp->arg1->ident, "$r0");

	tmp = tmp->next_row;

	CuAssertIntEquals(tc, tmp->instr, MCC_IR_INSTR_PLUS);
	CuAssertIntEquals(tc, tmp->arg1->type, MCC_IR_TYPE_IDENTIFIER);
	CuAssertStrEquals(tc, tmp->arg1->ident, "$r0");

	tmp = tmp->next_row;

	CuAssertIntEquals(tc, tmp->instr, MCC_IR_INSTR_ASSIGN);
	CuAssertIntEquals(tc, tmp->arg1->type, MCC_IR_TYPE_IDENTIFIER);
	CuAssertStrEquals(tc, tmp->arg1->ident, "$r0");

	tmp = tmp->next_row->next_row->next_row;

	CuAssertIntEquals(tc, tmp->instr, MCC_IR_INSTR_ASSIGN);
	CuAssertIntEquals(tc, tmp->arg1->type, MCC_IR_TYPE_IDENTIFIER);
	CuAssertStrEquals(tc, tmp->arg1->ident, "$r1");

	tmp = tmp->next_row->next_row->next_row->next_row;

	CuAssertIntEquals(tc, tmp->instr, MCC_IR_INSTR_ASSIGN);
	CuAssertIntEquals(tc, tmp->arg1->type, MCC_IR_TYPE_IDENTIFIER);
	CuAssertStrEquals(tc, tmp->arg1->ident, "a");

	// Cleanup
	mcc_ir_delete_ir(ir_head);
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
}

void type_test(CuTest *tc)
{
	const char input[] =
	    "int main(){int i; i = 42; bool b; b = true; float f; f = 3.3; string str; str = \"hallo\"; return i;}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM, "test");
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);

	struct mcc_ir_row *ir = mcc_ir_generate((&parser_result)->program);
	struct mcc_ir_row *ir_head = ir;
	struct mcc_ir_row *tmp = ir->next_row;

	CuAssertIntEquals(tc, tmp->type->type, MCC_IR_ROW_INT);
	CuAssertIntEquals(tc, tmp->type->array_size, -1);

	tmp = tmp->next_row;

	CuAssertIntEquals(tc, tmp->type->type, MCC_IR_ROW_BOOL);
	CuAssertIntEquals(tc, tmp->type->array_size, -1);

	tmp = tmp->next_row;

	CuAssertIntEquals(tc, tmp->type->type, MCC_IR_ROW_FLOAT);
	CuAssertIntEquals(tc, tmp->type->array_size, -1);

	tmp = tmp->next_row;

	CuAssertIntEquals(tc, tmp->type->type, MCC_IR_ROW_FLOAT);
	CuAssertIntEquals(tc, tmp->type->array_size, -1);

	tmp = tmp->next_row;

	CuAssertIntEquals(tc, tmp->type->type, MCC_IR_ROW_STRING);
	CuAssertIntEquals(tc, tmp->type->array_size, -1);

	// Cleanup
	mcc_ir_delete_ir(ir_head);
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
}

void type_array_test(CuTest *tc)
{
	const char input[] = "int main(){int[42] i; i[14] = 4; bool[42] b; b[1] = true; float[32] f; f[12] = 3.3; "
	                     "string[12] str; str[2] = \"hallo\"; return i[14];}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM, "test");
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);

	struct mcc_ir_row *ir = mcc_ir_generate((&parser_result)->program);
	struct mcc_ir_row *ir_head = ir;
	struct mcc_ir_row *tmp = ir->next_row;

	CuAssertIntEquals(tc, tmp->type->type, MCC_IR_ROW_INT);
	CuAssertIntEquals(tc, tmp->type->array_size, 42);

	tmp = tmp->next_row->next_row;

	CuAssertIntEquals(tc, tmp->type->type, MCC_IR_ROW_BOOL);
	CuAssertIntEquals(tc, tmp->type->array_size, 42);

	tmp = tmp->next_row->next_row;

	CuAssertIntEquals(tc, tmp->type->type, MCC_IR_ROW_FLOAT);
	CuAssertIntEquals(tc, tmp->type->array_size, 32);

	tmp = tmp->next_row->next_row;

	CuAssertIntEquals(tc, tmp->type->type, MCC_IR_ROW_STRING);
	CuAssertIntEquals(tc, tmp->type->array_size, 12);

	// Cleanup
	mcc_ir_delete_ir(ir_head);
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
}

// clang-format off

#define TESTS \
	TEST(test1) \
	TEST(expression) \
	TEST(exp_plus_exp) \
	TEST(expression_var) \
	TEST(expression_arr) \
	TEST(if_stmt) \
	TEST(if_else_stmt) \
	TEST(while_stmt) \
	TEST(func_def) \
	TEST(func_call)\
	TEST(variable_shadowing) \
	TEST(type_test) \
	TEST(type_array_test)

// clang-format on

#include "main_stub.inc"
#undef TESTS

