#include "mcc/asm.h"

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "mcc/ir.h"
#include "utils/unused.h"

struct mcc_asm *generate_fake()
{
	struct mcc_asm *mcc_asm = malloc(sizeof(*mcc_asm));
	struct mcc_asm_text_section *text = malloc(sizeof(*text));
	struct mcc_asm_data_section *data = malloc(sizeof(*data));
	mcc_asm->text_section = text;
	mcc_asm->data_section = data;
	struct mcc_asm_function *func = malloc(sizeof(*func));
	text->function = func;
	func->label = (char*)malloc(7 * sizeof(char));
	snprintf(func->label, 6, "main");
	func->next = NULL;
	struct mcc_asm_assembly_line *line1 = malloc(sizeof(*line1));
	struct mcc_asm_assembly_line *line2 = malloc(sizeof(*line2));
	struct mcc_asm_assembly_line *line3 = malloc(sizeof(*line3));
	struct mcc_asm_assembly_line *line4 = malloc(sizeof(*line4));
	struct mcc_asm_assembly_line *line5 = malloc(sizeof(*line5));
	struct mcc_asm_assembly_line *line6 = malloc(sizeof(*line6));
	func->head = line1;
	struct mcc_asm_operand *op11 = malloc(sizeof(*op11));
	op11->type = MCC_ASM_OPERAND_REGISTER;
	op11->reg = MCC_ASM_EBP;
	line1->first = op11;
	line1->second = NULL;
	line1->opcode = MCC_ASM_PUSHL;
	struct mcc_asm_operand *op21 = malloc(sizeof(*op21));
	op21->type = MCC_ASM_OPERAND_REGISTER;
	op21->reg = MCC_ASM_ESP;
	struct mcc_asm_operand *op22 = malloc(sizeof(*op22));
	op22->type = MCC_ASM_OPERAND_REGISTER;
	op22->reg = MCC_ASM_EBP;
	line2->first = op21;
	line2->second = op22;
	line2->opcode = MCC_ASM_MOVL;
	struct mcc_asm_operand *op31 = malloc(sizeof(*op31));
	op31->type = MCC_ASM_OPERAND_LITERAL;
	op31->literal = 4;
	struct mcc_asm_operand *op32 = malloc(sizeof(*op32));
	op32->type = MCC_ASM_OPERAND_REGISTER;
	op32->reg = MCC_ASM_ESP;
	line3->first = op31;
	line3->second = op32;
	line3->opcode = MCC_ASM_SUBL;
	struct mcc_asm_operand *op41 = malloc(sizeof(*op41));
	op41->type = MCC_ASM_OPERAND_LITERAL;
	op41->literal = 0;
	struct mcc_asm_operand *op42 = malloc(sizeof(*op42));
	op42->type = MCC_ASM_OPERAND_REGISTER;
	op42->reg = MCC_ASM_EAX;
	line4->first = op41;
	line4->second = op42;
	line4->opcode = MCC_ASM_MOVL;
	line5->opcode = MCC_ASM_LEAVE;
	line5->first = NULL;
	line5->second = NULL;
	line6->opcode = MCC_ASM_RETURN;
	line6->first = NULL;
	line6->second = NULL;
	line1->next = line2;
	line2->next = line3;
	line3->next = line4;
	line4->next = line5;
	line5->next = line6;
	line6->next = NULL;
	return mcc_asm;
}

struct mcc_asm *mcc_asm_generate(struct mcc_ir_row *ir)
{
	UNUSED(ir);
	struct mcc_asm *code = generate_fake();
	return code;
}

void mcc_asm_delete_asm(struct mcc_asm *head)
{
	UNUSED(head);
}
