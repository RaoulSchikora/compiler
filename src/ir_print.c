#include "mcc/ir_print.h"
#include "mcc/ir.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

void mcc_ir_print_table_begin(FILE *out)
{

	fprintf(out, "--------------------------------------------------------------------------------\n"
	             " Intermediate representation (TAC)                                             |\n"
	             "--------------------------------------------------------------------------------\n"
	             " label      | line no. | instruction | arg1               | arg2               |\n"
	             "--------------------------------------------------------------------------------\n");
}

void mcc_ir_print_table_end(FILE *out)
{
	fprintf(out, "--------------------------------------------------------------------------------\n");
}

static void print_row(FILE *out, char *label, char *row_no, char *instruction, char *arg1, char *arg2)
{
	fprintf(out, " %-11s| %-7s  | %-11s | %-18s | %-18s |\n", label, row_no, instruction, arg1, arg2);
}

static void
print_row_to_string(size_t length, char *string, char *label, char *row_no, char *instruction, char *arg1, char *arg2)
{
	snprintf(string, length, " %-11s| %-7s  | %-11s | %-18s | %-18s  ", label, row_no, instruction, arg1, arg2);
}

static char *instr_to_string(enum mcc_ir_instruction instr)
{
	switch (instr) {
	case MCC_IR_INSTR_ASSIGN:
		return "assign";
	case MCC_IR_INSTR_JUMP:
		return "jump";
	case MCC_IR_INSTR_FUNC_LABEL:
		return "funclabel";
	case MCC_IR_INSTR_JUMPFALSE:
		return "jumpfalse";
	case MCC_IR_INSTR_CALL:
		return "call";
	case MCC_IR_INSTR_AND:
		return "and";
	case MCC_IR_INSTR_ARRAY:
		return "array";
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
	case MCC_IR_INSTR_RETURN:
		return "return";
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
		return length_of_int(arg->row->row_no) + 2;
	case MCC_IR_TYPE_LIT_INT:
		return length_of_int((int)arg->lit_int) + 1;
	case MCC_IR_TYPE_LIT_FLOAT:
		return length_of_int((int)arg->lit_float) + 8;
	case MCC_IR_TYPE_LIT_BOOL:
		return 6;
	case MCC_IR_TYPE_LIT_STRING:
		return strlen(arg->lit_string) + 2;
	case MCC_IR_TYPE_LABEL:
		return length_of_int(arg->label) + 1;
	case MCC_IR_TYPE_IDENTIFIER:
		return strlen(arg->ident->identifier_name) + 2;
	case MCC_IR_TYPE_ARR_ELEM:
		return strlen(arg->arr_ident->identifier_name) + arg_size(arg->index) + 3;
	case MCC_IR_TYPE_FUNC_LABEL:
		return strlen(arg->func_label);
	default:
		return length_of_int(1000);
	};
}

static void row_no_to_string(char *dest, int no)
{
	sprintf(dest, "(%d)", no);
}

static void bool_to_string(char *dest, bool b)
{
	if (b) {
		sprintf(dest, "true");
	} else {
		sprintf(dest, "false");
	}
}

static void arg_to_string(char *dest, struct mcc_ir_arg *arg)
{
	if (!arg) {
		strcpy(dest, "");
		return;
	}
	int index_size = 0;
	if (arg->type == MCC_IR_TYPE_ARR_ELEM) {
		index_size = arg_size(arg->index);
	}
	char index[index_size];

	switch (arg->type) {
	case MCC_IR_TYPE_ROW:
		row_no_to_string(dest, arg->row->row_no);
		return;
	case MCC_IR_TYPE_LIT_INT:
		sprintf(dest, "%ld", arg->lit_int);
		return;
	case MCC_IR_TYPE_LIT_FLOAT:
		sprintf(dest, "%f", arg->lit_float);
		return;
	case MCC_IR_TYPE_LIT_BOOL:
		bool_to_string(dest, arg->lit_bool);
		return;
	case MCC_IR_TYPE_LIT_STRING:
		sprintf(dest, "\"%s\"", arg->lit_string);
		return;
	case MCC_IR_TYPE_LABEL:
		sprintf(dest, "L%d", arg->label);
		return;
	case MCC_IR_TYPE_IDENTIFIER:
		sprintf(dest, "%s", arg->ident->identifier_name);
		return;
	case MCC_IR_TYPE_ARR_ELEM:
		arg_to_string(index, arg->index);
		sprintf(dest, "%s[%s]", arg->arr_ident->identifier_name, index);
		return;
	case MCC_IR_TYPE_FUNC_LABEL:
		strcpy(dest, arg->func_label);
	};
}
char *mcc_ir_print_ir_row_to_string(struct mcc_ir_row *row)
{
	if (!row)
		return NULL;
	char *ret_string = malloc(sizeof(char) * 81);
	if (!ret_string)
		return NULL;
	char *instr = instr_to_string(row->instr);
	char arg1[arg_size(row->arg1)];
	char arg2[arg_size(row->arg2)];
	char no[length_of_int(row->row_no) + 2];
	row_no_to_string(no, row->row_no);
	arg_to_string(arg1, row->arg1);
	arg_to_string(arg2, row->arg2);
	char label[arg_size(row->arg1)];
	switch (row->instr) {
	case MCC_IR_INSTR_LABEL:
		strcpy(label, arg1);
		strcpy(arg1, "");
		break;
	case MCC_IR_INSTR_FUNC_LABEL:
		strcpy(label, arg1);
		strcpy(arg1, "");
		break;
	default:
		strcpy(label, "");
	}
	print_row_to_string(81, ret_string, label, no, instr, arg1, arg2);
	return ret_string;
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
	char label[arg_size(row->arg1)];
	switch (row->instr) {
	case MCC_IR_INSTR_LABEL:
		strcpy(label, arg1);
		strcpy(arg1, "");
		break;
	case MCC_IR_INSTR_FUNC_LABEL:
		strcpy(label, arg1);
		strcpy(arg1, "");
		break;
	default:
		strcpy(label, "");
	}
	print_row(out, label, no, instr, arg1, arg2);
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
