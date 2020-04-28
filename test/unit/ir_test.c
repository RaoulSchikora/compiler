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

	CuAssertPtrNotNull(tc, ir);

	CuAssertIntEquals(tc, ir->row_no, 0);
	CuAssertIntEquals(tc, ir->instr, MCC_IR_INSTR_LABEL);
	CuAssertIntEquals(tc, ir->arg1->type, MCC_IR_TYPE_LIT);
	CuAssertStrEquals(tc, ir->arg1->lit, "main");
	CuAssertPtrEquals(tc, ir->arg2, NULL);

	struct mcc_ir_row *ir_next = ir->next_row;
	CuAssertPtrNotNull(tc, ir);

	CuAssertIntEquals(tc, ir_next->row_no, 1);
	CuAssertIntEquals(tc, ir_next->instr, MCC_IR_INSTR_RETURN);
	CuAssertIntEquals(tc, ir_next->arg1->type, MCC_IR_TYPE_LIT);
	CuAssertStrEquals(tc, ir_next->arg1->lit, "42");
	CuAssertPtrEquals(tc, ir_next->arg2, NULL);
	CuAssertPtrEquals(tc, ir_next->next_row, NULL);

	// Cleanup
	mcc_ir_delete_ir(ir);
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

void expression(CuTest *tc)
{
	const char input[] = "int main(){ 0 + 0 + 1; return 0;}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
	
	struct mcc_ir_row *ir_head = mcc_ir_generate((&parser_result)->program, table);
	struct mcc_ir_row *ir = ir_head->next_row;

	CuAssertPtrNotNull(tc, ir);

	CuAssertIntEquals(tc, ir->row_no, 1);
	CuAssertIntEquals(tc, ir->instr, MCC_IR_INSTR_PLUS);
	CuAssertPtrNotNull(tc, ir->arg1);
	CuAssertIntEquals(tc, ir->arg1->type, MCC_IR_TYPE_LIT);
	CuAssertStrEquals(tc, ir->arg1->lit, "0");
	CuAssertIntEquals(tc, ir->arg2->type, MCC_IR_TYPE_LIT);
	CuAssertStrEquals(tc, ir->arg2->lit, "0");

	struct mcc_ir_row *next_ir = ir->next_row;
	CuAssertPtrNotNull(tc, next_ir);

	CuAssertIntEquals(tc, next_ir->row_no, 2);
	CuAssertIntEquals(tc, next_ir->instr, MCC_IR_INSTR_PLUS);
	CuAssertIntEquals(tc, next_ir->arg1->type, MCC_IR_TYPE_ROW);
	CuAssertPtrEquals(tc, next_ir->arg1->row, ir);
	CuAssertIntEquals(tc, next_ir->arg2->type, MCC_IR_TYPE_LIT);
	CuAssertStrEquals(tc, next_ir->arg2->lit, "1");

	// Cleanup
	mcc_ir_delete_ir(ir_head);
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
}

void exp_plus_exp(CuTest *tc)
{
	const char input[] = "int main(){ (1 + 2) - (3 + 4); return 0;}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
	
	struct mcc_ir_row *ir_head = mcc_ir_generate((&parser_result)->program, table);
	struct mcc_ir_row *ir = ir_head->next_row;

	CuAssertPtrNotNull(tc, ir); 

	struct mcc_ir_row * exp_row = ir->next_row->next_row;

	CuAssertIntEquals(tc, exp_row->row_no, 3);
	CuAssertIntEquals(tc, exp_row->instr, MCC_IR_INSTR_MINUS);
	CuAssertPtrNotNull(tc, exp_row->arg1);
	CuAssertIntEquals(tc, exp_row->arg1->type, MCC_IR_TYPE_ROW);
	CuAssertIntEquals(tc, exp_row->arg1->row->row_no, 1);
	CuAssertPtrEquals(tc, exp_row->arg1->row, ir);
	CuAssertIntEquals(tc, exp_row->arg2->type, MCC_IR_TYPE_ROW);
	CuAssertIntEquals(tc, exp_row->arg2->row->row_no, 2);
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
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
	
	struct mcc_ir_row *ir_head = mcc_ir_generate((&parser_result)->program, table);
	struct mcc_ir_row *ir = ir_head->next_row;

	CuAssertPtrNotNull(tc, ir); 

	CuAssertIntEquals(tc, ir->row_no, 1);
	CuAssertIntEquals(tc, ir->instr, MCC_IR_INSTR_ASSIGN);
	CuAssertPtrNotNull(tc, ir->arg1);
	CuAssertIntEquals(tc, ir->arg1->type, MCC_IR_TYPE_LIT);
	CuAssertStrEquals(tc, ir->arg1->lit, "a");
	CuAssertIntEquals(tc, ir->arg2->type, MCC_IR_TYPE_LIT);
	CuAssertStrEquals(tc, ir->arg2->lit, "3");

	struct mcc_ir_row *next_ir = ir->next_row;
	CuAssertPtrNotNull(tc, next_ir);

	CuAssertIntEquals(tc, next_ir->row_no, 2);
	CuAssertIntEquals(tc, next_ir->instr, MCC_IR_INSTR_PLUS);
	CuAssertIntEquals(tc, next_ir->arg1->type, MCC_IR_TYPE_ROW);
	CuAssertPtrEquals(tc, next_ir->arg1->row, ir);
	CuAssertIntEquals(tc, next_ir->arg2->type, MCC_IR_TYPE_LIT);
	CuAssertStrEquals(tc, next_ir->arg2->lit, "1");

	// Cleanup
	mcc_ir_delete_ir(ir_head);
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
}

void if_stmt(CuTest *tc){
	const char input[] = "int main(){if(0==1) 1*2+2; return 0;}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
	
	struct mcc_ir_row *ir = mcc_ir_generate((&parser_result)->program, table);
        // Skip first row
	ir = ir->next_row;

        // Condition
	CuAssertPtrNotNull(tc, ir); 
	CuAssertIntEquals(tc, ir->row_no, 1);
	CuAssertIntEquals(tc, ir->instr, MCC_IR_INSTR_EQUALS);
	CuAssertPtrNotNull(tc, ir->arg1);
	CuAssertIntEquals(tc, ir->arg1->type, MCC_IR_TYPE_LIT);
	CuAssertStrEquals(tc, ir->arg1->lit, "0");
	CuAssertIntEquals(tc, ir->arg2->type, MCC_IR_TYPE_LIT);
	CuAssertStrEquals(tc, ir->arg2->lit, "1");

        // Jumpfalse L0
        struct mcc_ir_row * tmp = ir;
	ir = ir->next_row;
	CuAssertPtrNotNull(tc, ir); 
	CuAssertIntEquals(tc, ir->row_no, 2);
	CuAssertIntEquals(tc, ir->instr, MCC_IR_INSTR_JUMPFALSE);
	CuAssertPtrNotNull(tc, ir->arg1);
	CuAssertIntEquals(tc, ir->arg1->type, MCC_IR_TYPE_ROW);
	CuAssertPtrEquals(tc, ir->arg1->row,tmp);
	CuAssertIntEquals(tc, ir->arg2->type, MCC_IR_TYPE_LABEL);
	CuAssertIntEquals(tc, ir->arg2->label,0);

        // On true: 1*2
        ir = ir->next_row;
	CuAssertPtrNotNull(tc, ir); 
	CuAssertIntEquals(tc, ir->row_no, 3);
        CuAssertIntEquals(tc,ir->instr, MCC_IR_INSTR_MULTIPLY);
        CuAssertPtrNotNull(tc, ir->arg1);
        CuAssertPtrNotNull(tc, ir->arg2);
	CuAssertIntEquals(tc, ir->arg1->type, MCC_IR_TYPE_LIT);
	CuAssertIntEquals(tc, ir->arg2->type, MCC_IR_TYPE_LIT);

        // TODO: Finish
       
        // On true: x + 2

        // L0

	// Cleanup
	mcc_ir_delete_ir(ir);
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
}

#define TESTS \
	TEST(test1) \
	TEST(test2)	\
	TEST(expression) \
	TEST(exp_plus_exp) \
	TEST(expression_var)
#include "main_stub.inc"
#undef TESTS
