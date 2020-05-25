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
	case MCC_ASM_ADDL:
		return "addl";
	case MCC_ASM_SUBL:
		return "subl";
	case MCC_ASM_LEAVE:
		return "leave";
	case MCC_ASM_CALL:
		return "calll";
	case MCC_ASM_RETURN:
		return "ret";

	default:
		return "unknown opcode";
	}
	return "error";
}

static char *register_name_to_string(enum mcc_asm_register reg)
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

static void register_to_string(char *dest, int len, enum mcc_asm_register reg, int offset)
{
	if (offset == 0){
		snprintf(dest, len, "%s", register_name_to_string(reg));
	}
	if (offset != 0){
		snprintf(dest, len + length_of_int(offset) + 2, "%d(%s)", offset, register_name_to_string(reg));
	}
}

static char *op_to_string(char *dest, int len, struct mcc_asm_operand *op)
{
	if (!op) {
		return NULL;
	}
	switch (op->type) {
	case MCC_ASM_OPERAND_REGISTER:
		register_to_string(dest, len, op->reg, op->offset);
		break;
	case MCC_ASM_OPERAND_DATA:
		break;
	case MCC_ASM_OPERAND_LITERAL:
		snprintf(dest, len, "$%d", op->literal);
		break;
	case MCC_ASM_OPERAND_FUNCTION:
		snprintf(dest, len, "%s", op->func_name);
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
		return length_of_int(op->literal) + 1;
	case MCC_ASM_OPERAND_FUNCTION:
		return strlen(op->func_name) + 1;
	default:
		return 0;
	}
	return 0;
}

static void asm_print_line(FILE *out, struct mcc_asm_assembly_line *line)
{
	int len1 = length_of_op(line->first) + 1;
	int len2 = length_of_op(line->second) + 1;
	char op1[len1];
	char op2[len2];
	if (line->first && line->second) {
		fprintf(out, "        %-7s %s, %s\n", opcode_to_string(line->opcode),
		        op_to_string(op1, len1, line->first), op_to_string(op2, len2, line->second));
	} else if (line->first) {
		fprintf(out, "        %-7s %s\n", opcode_to_string(line->opcode), op_to_string(op1, len1, line->first));
	} else {
		fprintf(out, "        %-7s\n", opcode_to_string(line->opcode));
	}
}

static void asm_print_func(FILE *out, struct mcc_asm_function *func)
{
	fprintf(out, "\n        .globl %s\n", func->label);
	fprintf(out, "%s:\n", func->label);
	struct mcc_asm_assembly_line *line = func->head;
	while (line) {
		asm_print_line(out, line);
		line = line->next;
	}
}

// TODO: Allocate space for string, bool and float arrays
static void asm_print_decl(FILE *out, struct mcc_asm_declaration *decl)
{
	fprintf(out, "%s:\n", decl->identifier);
	switch (decl->type) {
	case MCC_ASM_DECLARATION_TYPE_FLOAT:
		fprintf(out, "       .float %f\n", decl->float_value);
		break;
	case MCC_ASM_DECLARATION_TYPE_DB:
		fprintf(out, "db %s", decl->db_value);
		break;
	case MCC_ASM_DECLARATION_TYPE_ARRAY_INT:
		fprintf(out, "       .fill %d\n", decl->array_size);
		break;
	default:
		break;
	}
}

static void asm_print_text_sec(FILE *out, struct mcc_asm_text_section *text)
{
	fprintf(out, ".text\n");
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
	while (decl) {
		asm_print_decl(out, decl);
		decl = decl->next;
	}
}

void mcc_asm_print_asm(FILE *out, struct mcc_asm *head)
{
	asm_print_begin(out);
	asm_print_text_sec(out, head->text_section);
	if (head->data_section) {
		asm_print_data_sec(out, head->data_section);
	}
	asm_print_end(out);
}
