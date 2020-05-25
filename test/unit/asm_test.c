#include <CuTest.h>

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "mcc/asm.h"
#include "mcc/ast.h"
#include "mcc/ir.h"
#include "mcc/semantic_checks.h"
#include "mcc/symbol_table.h"

void test1(CuTest *tc)
{
	// Define test input and create IR
	const char input[] = "int main(){return 42;}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
	struct mcc_semantic_check *checks = mcc_semantic_check_run_all((&parser_result)->program, table);
	CuAssertIntEquals(tc, checks->status, MCC_SEMANTIC_CHECK_OK);
	struct mcc_ir_row *ir = mcc_ir_generate((&parser_result)->program, table);
	CuAssertPtrNotNull(tc, ir);

	struct mcc_asm *code = mcc_asm_generate(ir);
	// asm
	CuAssertPtrNotNull(tc, code);

	// sections
	// CuAssertPtrEquals(tc, NULL, code->data_section);
	CuAssertPtrNotNull(tc, code->text_section);

	// "main"
	CuAssertPtrNotNull(tc, code->text_section->function);
	CuAssertPtrNotNull(tc, code->text_section->function->label);
	CuAssertStrEquals(tc, code->text_section->function->label, "main");
	CuAssertPtrEquals(tc, NULL, code->text_section->function->next);

	// First line
	CuAssertPtrNotNull(tc, code->text_section->function->head);
	CuAssertPtrNotNull(tc, code->text_section->function->head->first);
	CuAssertPtrEquals(tc, NULL, code->text_section->function->head->second);
	CuAssertIntEquals(tc, code->text_section->function->head->opcode, MCC_ASM_PUSHL);
	CuAssertIntEquals(tc, MCC_ASM_OPERAND_REGISTER, code->text_section->function->head->first->type);
	CuAssertIntEquals(tc, MCC_ASM_EBP, code->text_section->function->head->first->reg);

	// Second line
	CuAssertIntEquals(tc, code->text_section->function->head->next->opcode, MCC_ASM_MOVL);
	CuAssertPtrNotNull(tc, code->text_section->function->head->next->first);
	CuAssertPtrNotNull(tc, code->text_section->function->head->next->second);
	CuAssertIntEquals(tc, code->text_section->function->head->next->opcode, MCC_ASM_MOVL);
	CuAssertIntEquals(tc, MCC_ASM_OPERAND_REGISTER, code->text_section->function->head->next->first->type);
	CuAssertIntEquals(tc, MCC_ASM_ESP, code->text_section->function->head->next->first->reg);
	CuAssertIntEquals(tc, MCC_ASM_OPERAND_REGISTER, code->text_section->function->head->next->second->type);
	CuAssertIntEquals(tc, MCC_ASM_EBP, code->text_section->function->head->next->second->reg);

	mcc_ir_delete_ir(ir);
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
	mcc_asm_delete_asm(code);
}

void stack_frame_size_int(CuTest *tc)
{
	// Define test input and create IR
	const char input[] = "int main(){int a; a = 1; int b; b = 1; while (a < 10) { a = a +1;  b "
	                     "= b -1;}  return b;}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
	struct mcc_semantic_check *checks = mcc_semantic_check_run_all((&parser_result)->program, table);
	CuAssertIntEquals(tc, checks->status, MCC_SEMANTIC_CHECK_OK);
	struct mcc_ir_row *ir = mcc_ir_generate((&parser_result)->program, table);
	CuAssertPtrNotNull(tc, ir);

	struct mcc_asm *code = mcc_asm_generate(ir);
	CuAssertPtrNotNull(tc, code);
	CuAssertIntEquals(tc, MCC_ASM_SUBL, code->text_section->function->head->next->next->opcode);
	CuAssertIntEquals(tc, MCC_ASM_OPERAND_LITERAL, code->text_section->function->head->next->next->first->type);
	CuAssertIntEquals(tc, MCC_ASM_OPERAND_REGISTER, code->text_section->function->head->next->next->second->type);
	CuAssertIntEquals(tc, MCC_ASM_ESP, code->text_section->function->head->next->next->second->reg);
	CuAssertIntEquals(tc, 20, code->text_section->function->head->next->next->first->literal);

	mcc_ir_delete_ir(ir);
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
	mcc_asm_delete_asm(code);
}

void stack_frame_size_array(CuTest *tc)
{
	// Define test input and create IR
	const char input[] = "int main(){ int a; a = 1;int[42]c; c[0] = 1; return a;}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
	struct mcc_semantic_check *checks = mcc_semantic_check_run_all((&parser_result)->program, table);
	CuAssertIntEquals(tc, checks->status, MCC_SEMANTIC_CHECK_OK);
	struct mcc_ir_row *ir = mcc_ir_generate((&parser_result)->program, table);
	CuAssertPtrNotNull(tc, ir);

	struct mcc_asm *code = mcc_asm_generate(ir);
	CuAssertPtrNotNull(tc, code);
	CuAssertIntEquals(tc, MCC_ASM_SUBL, code->text_section->function->head->next->next->opcode);
	CuAssertIntEquals(tc, MCC_ASM_OPERAND_LITERAL, code->text_section->function->head->next->next->first->type);
	CuAssertIntEquals(tc, MCC_ASM_OPERAND_REGISTER, code->text_section->function->head->next->next->second->type);
	CuAssertIntEquals(tc, MCC_ASM_ESP, code->text_section->function->head->next->next->second->reg);
	CuAssertIntEquals(tc, 4, code->text_section->function->head->next->next->first->literal);

	mcc_ir_delete_ir(ir);
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
	mcc_asm_delete_asm(code);
}

// TODO: Add float,bool arrays
void array_declaration(CuTest *tc)
{
	// Define test input and create IR
	const char input[] = "int main(){ int[23]d;string[42]c; d[0] = 1; return 0;}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
	struct mcc_semantic_check *checks = mcc_semantic_check_run_all((&parser_result)->program, table);
	CuAssertIntEquals(tc, checks->status, MCC_SEMANTIC_CHECK_OK);
	struct mcc_ir_row *ir = mcc_ir_generate((&parser_result)->program, table);
	CuAssertPtrNotNull(tc, ir);

	struct mcc_asm *code = mcc_asm_generate(ir);
	CuAssertPtrNotNull(tc, code);

	CuAssertPtrNotNull(tc, code->data_section);
	CuAssertPtrNotNull(tc, code->text_section);

	// First declaration: int[23]d
	CuAssertPtrNotNull(tc, code->data_section->head);
	CuAssertIntEquals(tc, 23, code->data_section->head->array_size);
	CuAssertStrEquals(tc, "d", code->data_section->head->identifier);
	CuAssertIntEquals(tc, MCC_ASM_DECLARATION_TYPE_ARRAY_INT, code->data_section->head->type);

	// Second declaration: int[42]c
	CuAssertPtrNotNull(tc, code->data_section->head->next);
	CuAssertIntEquals(tc, 42, code->data_section->head->next->array_size);
	CuAssertStrEquals(tc, "c", code->data_section->head->next->identifier);
	CuAssertIntEquals(tc, MCC_ASM_DECLARATION_TYPE_ARRAY_STRING, code->data_section->head->next->type);

	mcc_ir_delete_ir(ir);
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
	mcc_asm_delete_asm(code);
}

// clang-format off

#define TESTS \
	TEST(test1) \
	TEST(stack_frame_size_int) \
	TEST(stack_frame_size_array) \
	TEST(array_declaration)

// clang-format on

#include "main_stub.inc"
#undef TESTS
