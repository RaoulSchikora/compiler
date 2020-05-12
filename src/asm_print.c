#include "mcc/asm_print.h"
#include "mcc/asm.h"

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils/unused.h"

//---------------------------------------------------------------------------------------- Functions: Print ASM

static void asm_print_begin(FILE *out)
{
	UNUSED(out);
}

static void asm_print_end(FILE *out)
{
	UNUSED(out);
}

static char *opcode_to_string(enum mcc_asm_opcode op)
{
	switch (op) {
	case MCC_ASM_MOVL:
		return "movl";
	case MCC_ASM_PUSHL:
		return "pushl";
	case MCC_ASM_SUBL:
		return "subl";
	case MCC_ASM_LEAVE:
		return "leave";
	case MCC_ASM_RETURN:
		return "ret";

	default:
		return "unknown opcode";
	}
	return "error";
}

static char *register_to_string(enum mcc_asm_register reg)
{
	switch (reg) {
	case MCC_ASM_EAX:
		return "%eax";
	case MCC_ASM_EBX:
		return "%ebx";
	case MCC_ASM_ECX:
		return "%ecx";
	case MCC_ASM_EDX:
		return "%edx";
	case MCC_ASM_ESP:
		return "%esp";
	case MCC_ASM_EBP:
		return "%ebp";
	default:
		return "unknown register";
	}
}

static char *op_to_string(char *dest, struct mcc_asm_operand *op)
{
	if (!op) {
		return NULL;
	}
	switch (op->type) {
	case MCC_ASM_OPERAND_REGISTER:
		sprintf(dest, "%s", register_to_string(op->reg));
		break;
	case MCC_ASM_OPERAND_DATA:
		break;
	case MCC_ASM_OPERAND_LITERAL:
		sprintf(dest, "$%d", op->literal);
		break;

	default:
		return "unknown operand";
	}
	return dest;
}

static int length_of_op(struct mcc_asm_operand *op)
{
	if (!op) {
		return 0;
	}
	switch (op->type) {
	case MCC_ASM_OPERAND_REGISTER:
		return 4;
	case MCC_ASM_OPERAND_DATA:
		break;
	case MCC_ASM_OPERAND_LITERAL:
		return length_of_int(op->literal);

	default:
		return 0;
	}
	return 0;
}

static void asm_print_line(FILE *out, struct mcc_asm_assembly_line *line)
{
	char op1[length_of_op(line->first)];
	char op2[length_of_op(line->second)];
	if (line->first && line->second) {
		fprintf(out, "        %-7s %s, %s\n", opcode_to_string(line->opcode), op_to_string(op1, line->first),
		        op_to_string(op2, line->second));
	} else if (line->first) {
		fprintf(out, "        %-7s %s\n", opcode_to_string(line->opcode), op_to_string(op1, line->first));
	} else {
		fprintf(out, "        %-7s\n", opcode_to_string(line->opcode));
	}
}

static void asm_print_func(FILE *out, struct mcc_asm_function *func)
{
	fprintf(out, "        .globl %s\n", func->label);
	fprintf(out, "%s:\n", func->label);
	struct mcc_asm_assembly_line *line = func->head;
	while (line) {
		asm_print_line(out, line);
		line = line->next;
	}
}

static void asm_print_decl(FILE *out, struct mcc_asm_declaration *decl)
{
	fprintf(out, "%s:\n", decl->identifier);
	switch (decl->type)
	{
	case MCC_ASM_DECLARATION_TYPE_FLOAT:
		fprintf(out, "       .float %f\n", decl->float_value);
		break;
	case MCC_ASM_DECLARATION_TYPE_DB:
		fprintf(out, "db %s", decl->db_value);
	
	default:
		break;
	}
}

static void asm_print_text_sec(FILE *out, struct mcc_asm_text_section *text)
{
	fprintf(out, ".text\n\n");
	struct mcc_asm_function *func = text->function;
	while (func) {
		asm_print_func(out, func);
		func = func->next;
	}
}

static void asm_print_data_sec(FILE *out, struct mcc_asm_data_section *data)
{	
	fprintf(out, "\n.data\n");
	struct mcc_asm_declaration *decl = data->head;
	while(decl){
		asm_print_decl(out, decl);
		decl = decl->next;
	}
}

void mcc_asm_print_asm(FILE *out, struct mcc_asm *head)
{
	asm_print_begin(out);
	asm_print_text_sec(out, head->text_section);
	if(head->data_section){
		asm_print_data_sec(out, head->data_section);
	}
	asm_print_end(out);
}
