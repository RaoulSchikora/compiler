#include "mcc/ir_print.h"
#include "mcc/ir.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

void mcc_ir_print_table_begin(FILE *out)
{

	fprintf(out, "--------------------------------------------------------------------------------\n"
	             "| Intermediate representation (TAC)                                            |\n"
	             "--------------------------------------------------------------------------------\n"
	             "| line no.  | instruction      | arg1                  | arg2                  |\n"
	             "--------------------------------------------------------------------------------\n");
}

void mcc_ir_print_table_end(FILE *out)
{
	fprintf(out, "--------------------------------------------------------------------------------\n");
}

static void print_row(FILE *out, char *row_no, char *instruction, char *arg1, char *arg2)
{
	fprintf(out, "| %-7s   | %-16s | %-21s | %-21s |\n", row_no, instruction, arg1, arg2);
}

static char *instr_to_string(enum mcc_ir_instruction instr)
{
	switch (instr) {
	case MCC_IR_INSTR_JUMP:
		return "jump";
	case MCC_IR_INSTR_JUMPFALSE:
		return "jumpfalse";
	case MCC_IR_INSTR_AND:
		return "and";
	case MCC_IR_INSTR_ARRAY_EL:
		return "array el.";
	case MCC_IR_INSTR_DIVIDE:
		return "divide";
	case MCC_IR_INSTR_EQUALS:
		return "equals";
	case MCC_IR_INSTR_NOTEQUALS:
		return "not equal";
	case MCC_IR_INSTR_SMALLER:
		return "smaller";
	case MCC_IR_INSTR_GREATER:
		return "greater";
	case MCC_IR_INSTR_SMALLEREQ:
		return "smaller eq";
	case MCC_IR_INSTR_GREATEREQ:
		return "greater eq";
	case MCC_IR_INSTR_MINUS:
		return "minus";
	case MCC_IR_INSTR_MODULO:
		return "modulo";
	case MCC_IR_INSTR_MULTIPLY:
		return "multiply";
	case MCC_IR_INSTR_OR:
		return "or";
	case MCC_IR_INSTR_PLUS:
		return "plus";
	case MCC_IR_INSTR_POP:
		return "pop";
	case MCC_IR_INSTR_PUSH:
		return "push";
	case MCC_IR_INSTR_LABEL:
		return "label";
	case MCC_IR_INSTR_NEGATIV:
		return "negativ";
	case MCC_IR_INSTR_NOT:
		return "not";
	default:
		return "unknown";
	}
}

static int length_of_int(int num)
{
	if (num == 0)
		return 1;
	return floor(log10(num));
}

static int arg_size(struct mcc_ir_arg *arg)
{
	if (!arg)
		return length_of_int(1000);

	switch (arg->type) {
	case MCC_IR_TYPE_ROW:
		return length_of_int(arg->row->row_no);
	case MCC_IR_TYPE_LIT:
		return strlen(arg->lit);
	case MCC_IR_TYPE_LABEL:
		return length_of_int(arg->label) + 1;
	default:
		return length_of_int(1000);
	};
}

static void row_no_to_string(char *dest, int no)
{
	sprintf(dest, "(%d)", no);
}

static void arg_to_string(char *dest, struct mcc_ir_arg *arg)
{
	if (!arg) {
		strcpy(dest, "");
		return;
	}

	switch (arg->type) {
	case MCC_IR_TYPE_ROW:
		row_no_to_string(dest, arg->row->row_no);
		return;
	case MCC_IR_TYPE_LIT:
		strcpy(dest, arg->lit);
		return;
	case MCC_IR_TYPE_LABEL:
		sprintf(dest, "L%d", arg->label);
	};
}

void mcc_ir_print_ir_row(FILE *out, struct mcc_ir_row *row)
{
	char *instr = instr_to_string(row->instr);
	char arg1[arg_size(row->arg1)];
	char arg2[arg_size(row->arg2)];
	char no[length_of_int(row->row_no) + 2];
	row_no_to_string(no, row->row_no);
	arg_to_string(arg1, row->arg1);
	arg_to_string(arg2, row->arg2);

	print_row(out, no, instr, arg1, arg2);
}

void mcc_ir_print_ir(FILE *out, struct mcc_ir_row *head)
{

	mcc_ir_print_table_begin(out);

	while (head) {
		mcc_ir_print_ir_row(out, head);
		head = head->next_row;
	}

	mcc_ir_print_table_end(out);
}
