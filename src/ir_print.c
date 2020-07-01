#include "mcc/ir_print.h"

#include <stdio.h>
#include <stdlib.h>

#include "mcc/ir.h"

#define TERMINAL_LINE_LENGTH 120

void mcc_ir_print_table_begin(FILE *out)
{

	fprintf(out, "----------------------------------------\n"
	             " Intermediate representation (TAC)     |\n"
	             "----------------------------------------\n");
}

void mcc_ir_print_table_end(FILE *out)
{
	fprintf(out, "\n");
}

static void row_to_string(
    char *out, char *label, char *row_no, char *instruction, char *arg1, char *arg2, enum mcc_ir_instruction instr)
{
	switch (instr) {
	case MCC_IR_INSTR_LABEL:
		snprintf(out, TERMINAL_LINE_LENGTH, "%6s %s %s %s", label, instruction, arg1, arg2);
		break;
	case MCC_IR_INSTR_FUNC_LABEL:
	case MCC_IR_INSTR_JUMPFALSE:
	case MCC_IR_INSTR_JUMP:
	case MCC_IR_INSTR_PUSH:
	case MCC_IR_INSTR_RETURN:
		snprintf(out, TERMINAL_LINE_LENGTH, "%-7s%s %s %s", label, instruction, arg1, arg2);
		break;
	// Inline instructions
	case MCC_IR_INSTR_ASSIGN:
		snprintf(out, TERMINAL_LINE_LENGTH, "%-7s%s %s %s", label, arg1, instruction, arg2);
		break;
	case MCC_IR_INSTR_POP:
		snprintf(out, TERMINAL_LINE_LENGTH, "%-7s%s %s", label, instruction, row_no);
		break;
	case MCC_IR_INSTR_ARRAY:
		snprintf(out, TERMINAL_LINE_LENGTH, "%-7s%s: %s ", label, instruction, arg1);
		break;
	case MCC_IR_INSTR_AND:
	case MCC_IR_INSTR_EQUALS:
	case MCC_IR_INSTR_SMALLER:
	case MCC_IR_INSTR_GREATER:
	case MCC_IR_INSTR_GREATEREQ:
	case MCC_IR_INSTR_NOTEQUALS:
	case MCC_IR_INSTR_SMALLEREQ:
	case MCC_IR_INSTR_PLUS:
	case MCC_IR_INSTR_OR:
	case MCC_IR_INSTR_MINUS:
	case MCC_IR_INSTR_DIVIDE:
	case MCC_IR_INSTR_MULTIPLY:
		snprintf(out, TERMINAL_LINE_LENGTH, "%-7s%s = %s %s %s", label, row_no, arg1, instruction, arg2);
		break;
	case MCC_IR_INSTR_NEGATIV:
	case MCC_IR_INSTR_NOT:
		snprintf(out, TERMINAL_LINE_LENGTH, "%-7s%s = %s%s", label, row_no, instruction, arg1);
		break;
	case MCC_IR_INSTR_CALL:
		snprintf(out, TERMINAL_LINE_LENGTH, "%-7s%s = %s %s", label, row_no, instruction, arg1);
		break;
	case MCC_IR_INSTR_UNKNOWN:
		break;
	}
}

static char *instr_to_string(enum mcc_ir_instruction instr)
{
	switch (instr) {
	case MCC_IR_INSTR_ASSIGN:
		return "=";
	case MCC_IR_INSTR_JUMP:
		return "jump";
	case MCC_IR_INSTR_JUMPFALSE:
		return "jumpfalse";
	case MCC_IR_INSTR_CALL:
		return "call";
	case MCC_IR_INSTR_AND:
		return "&&";
	case MCC_IR_INSTR_ARRAY:
		return "array";
	case MCC_IR_INSTR_DIVIDE:
		return "/";
	case MCC_IR_INSTR_EQUALS:
		return "==";
	case MCC_IR_INSTR_NOTEQUALS:
		return "!=";
	case MCC_IR_INSTR_SMALLER:
		return "<";
	case MCC_IR_INSTR_GREATER:
		return ">";
	case MCC_IR_INSTR_SMALLEREQ:
		return "<=";
	case MCC_IR_INSTR_GREATEREQ:
		return ">=";
	case MCC_IR_INSTR_MINUS:
		return "-";
	case MCC_IR_INSTR_MULTIPLY:
		return "*";
	case MCC_IR_INSTR_OR:
		return "||";
	case MCC_IR_INSTR_PLUS:
		return "+";
	case MCC_IR_INSTR_POP:
		return "pop";
	case MCC_IR_INSTR_PUSH:
		return "push";
	case MCC_IR_INSTR_NEGATIV:
		return "-";
	case MCC_IR_INSTR_NOT:
		return "!";
	case MCC_IR_INSTR_RETURN:
		return "return";
	case MCC_IR_INSTR_FUNC_LABEL:
	case MCC_IR_INSTR_LABEL:
		return "";
	default:
		return "unknown";
	}
}

static int arg_size(struct mcc_ir_arg *arg, bool escape_quotes)
{
	if (!arg)
		return length_of_int(1000);

	int quotes_space = 0;
	if (escape_quotes)
		quotes_space = 2;
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
		return strlen(arg->ident) + 2 + quotes_space;
	case MCC_IR_TYPE_ARR_ELEM:
		return strlen(arg->arr_ident) + arg_size(arg->index, escape_quotes) + 3;
	case MCC_IR_TYPE_FUNC_LABEL:
		return strlen(arg->func_label);
	default:
		return length_of_int(1000);
	};
}

static void row_no_to_string(char *dest, int no)
{
	sprintf(dest, "$t%d", no);
}

static void bool_to_string(char *dest, bool b)
{
	if (b) {
		sprintf(dest, "true");
	} else {
		sprintf(dest, "false");
	}
}

static void arg_to_string(char *dest, struct mcc_ir_arg *arg, bool escape_quotes)
{
	if (!arg) {
		strcpy(dest, "");
		return;
	}
	int index_size = 0;
	if (arg->type == MCC_IR_TYPE_ARR_ELEM) {
		index_size = arg_size(arg->index, escape_quotes);
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
		break;
	case MCC_IR_TYPE_LABEL:
		sprintf(dest, "L%d", arg->label);
		return;
	case MCC_IR_TYPE_IDENTIFIER:
		sprintf(dest, "%s", arg->ident);
		return;
	case MCC_IR_TYPE_ARR_ELEM:
		arg_to_string(index, arg->index, escape_quotes);
		sprintf(dest, "%s[%s]", arg->arr_ident, index);
		return;
	case MCC_IR_TYPE_FUNC_LABEL:
		sprintf(dest, "%s", arg->func_label);
		return;
	};
	// MCC_IR_TYPE_LIT_STRING:
	if (escape_quotes) {
		sprintf(dest, "\\\"%s\\\"", arg->lit_string);
	} else {
		sprintf(dest, "\"%s\"", arg->lit_string);
	}
}

static int row_type_size(struct mcc_ir_row_type *type)
{
	int size = 0;
	switch (type->type) {
	case MCC_IR_ROW_INT:
		size = 4;
		break;
	case MCC_IR_ROW_FLOAT:
		size = 6;
		break;
	case MCC_IR_ROW_BOOL:
		size = 5;
		break;
	case MCC_IR_ROW_STRING:
		size = 7;
		break;
	case MCC_IR_ROW_TYPELESS:
		size = 1;
		break;
	}
	if (type->array_size > -1) {
		size += length_of_int((int)type->array_size);
	}
	return size;
}

static void row_type_to_string(char *row_type_string, struct mcc_ir_row_type *type)
{
	int size = 0;
	if (type->array_size != -1) {
		size = length_of_int((int)type->array_size);
	}
	char str_end[size + 2];
	if (size > 0) {
		snprintf(str_end, size + 3, "[%d]", (int)type->array_size);
	} else {
		snprintf(str_end, 2, "%s", " ");
	}
	switch (type->type) {
	case MCC_IR_ROW_INT:
		snprintf(row_type_string, size + 6, "%s%s", "INT", str_end);
		break;
	case MCC_IR_ROW_FLOAT:
		snprintf(row_type_string, size + 8, "%s%s", "FLOAT", str_end);
		break;
	case MCC_IR_ROW_BOOL:
		snprintf(row_type_string, size + 7, "%s%s", "BOOL", str_end);
		break;
	case MCC_IR_ROW_STRING:
		snprintf(row_type_string, size + 9, "%s%s", "STRING", str_end);
		break;
	case MCC_IR_ROW_TYPELESS:
		snprintf(row_type_string, size + 4, "%s%s", " ", str_end);
		break;
	}
}

static void get_row_string(struct mcc_ir_row *row, char *row_string, bool escape_quotes)
{
	char *instr = instr_to_string(row->instr);
	char arg1[arg_size(row->arg1, escape_quotes)];
	char arg2[arg_size(row->arg2, escape_quotes)];
	char no[length_of_int(row->row_no) + 2];
	char row_type[row_type_size(row->type)];
	row_no_to_string(no, row->row_no);
	arg_to_string(arg1, row->arg1, escape_quotes);
	arg_to_string(arg2, row->arg2, escape_quotes);
	row_type_to_string(row_type, row->type);
	char label[arg_size(row->arg1, escape_quotes)];
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
	char row_string2[TERMINAL_LINE_LENGTH];
	row_to_string(row_string2, label, no, instr, arg1, arg2, row->instr);
	snprintf(row_string, TERMINAL_LINE_LENGTH, "%-30.29s%s\n", row_string2, row_type);
}

void mcc_ir_print_ir_row(FILE *out, struct mcc_ir_row *row, bool escape_quotes)
{
	char row_string[TERMINAL_LINE_LENGTH];
	get_row_string(row, row_string, escape_quotes);
	fprintf(out, "%s", row_string);
}

char *mcc_ir_print_ir_row_to_string(struct mcc_ir_row *row, bool escape_quotes)
{
	char *ret_string = malloc(sizeof(char) * TERMINAL_LINE_LENGTH);
	if (!ret_string)
		return NULL;
	get_row_string(row, ret_string, escape_quotes);
	return ret_string;
}

void mcc_ir_print_ir(FILE *out, struct mcc_ir_row *head, bool escape_quotes)
{
	mcc_ir_print_table_begin(out);

	while (head) {
		mcc_ir_print_ir_row(out, head, escape_quotes);
		head = head->next_row;
	}

	mcc_ir_print_table_end(out);
}
