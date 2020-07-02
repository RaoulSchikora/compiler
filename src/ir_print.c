
#include "mcc/ir_print.h"

#include <stdio.h>
#include <stdlib.h>

#include "mcc/ir.h"
#include "utils/print_string.h"
#include "utils/unused.h"

#define TERMINAL_LINE_LENGTH 120

static void print_instr_first(FILE *out, struct mcc_ir_row *row, bool escape_quotes);
static void print_instr_pop(FILE *out, struct mcc_ir_row *row, bool escape_quotes);
static void print_temporary(FILE *out, struct mcc_ir_row *row, bool escape_quotes);
static void print_assign(FILE *out, struct mcc_ir_row *row, bool escape_quotes);
static void print_label(FILE *out, struct mcc_ir_row *row, bool escape_quotes);
static void print_function_label(FILE *out, struct mcc_ir_row *row, bool escape_quotes);

char *bool_to_string(bool b);
static void print_arg(FILE *out, struct mcc_ir_arg *arg, bool escape_quotes);

char *instr_to_string(enum mcc_ir_instruction instr);

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

void mcc_ir_print_ir(FILE *out, struct mcc_ir_row *head, bool escape_quotes)
{
	mcc_ir_print_table_begin(out);

	while (head) {
		mcc_ir_print_ir_row(out, head, escape_quotes);
		head = head->next_row;
	}

	mcc_ir_print_table_end(out);
}

void mcc_ir_print_ir_row(FILE *out, struct mcc_ir_row *row, bool escape_quotes)
{
	switch (row->instr) {
	// Instruction first
	case MCC_IR_INSTR_JUMP:
	case MCC_IR_INSTR_JUMPFALSE:
	case MCC_IR_INSTR_PUSH:
	case MCC_IR_INSTR_RETURN:
		fprintf(out, "\t    ");
		print_instr_first(out, row, escape_quotes);
		break;

	// Pop
	case MCC_IR_INSTR_POP:
		fprintf(out, "\t    ");
		print_instr_pop(out, row, escape_quotes);
		break;

	// Temporary
	case MCC_IR_INSTR_CALL:
	case MCC_IR_INSTR_AND:
	case MCC_IR_INSTR_ARRAY:
	case MCC_IR_INSTR_DIVIDE:
	case MCC_IR_INSTR_EQUALS:
	case MCC_IR_INSTR_NOTEQUALS:
	case MCC_IR_INSTR_SMALLER:
	case MCC_IR_INSTR_GREATER:
	case MCC_IR_INSTR_SMALLEREQ:
	case MCC_IR_INSTR_GREATEREQ:
	case MCC_IR_INSTR_MINUS:
	case MCC_IR_INSTR_MULTIPLY:
	case MCC_IR_INSTR_OR:
	case MCC_IR_INSTR_PLUS:
	case MCC_IR_INSTR_NEGATIV:
	case MCC_IR_INSTR_NOT:
		fprintf(out, "\t    ");
		print_temporary(out, row, escape_quotes);
		break;

	// Inline
	case MCC_IR_INSTR_ASSIGN:
		fprintf(out, "\t    ");
		print_assign(out, row, escape_quotes);
		break;

	// Function label
	case MCC_IR_INSTR_FUNC_LABEL:
		print_function_label(out, row, escape_quotes);
		break;

	// Label
	case MCC_IR_INSTR_LABEL:
		fprintf(out, "\t");
		print_label(out, row, escape_quotes);
		break;
	default:
		break;
	}
	fprintf(out, "\n");
}

static void print_instr_first(FILE *out, struct mcc_ir_row *row, bool escape_quotes)
{
	fprintf(out, "%s ", instr_to_string(row->instr));
	print_arg(out, row->arg1, escape_quotes);
	fprintf(out, " ");
	print_arg(out, row->arg2, escape_quotes);
}

static void print_instr_pop(FILE *out, struct mcc_ir_row *row, bool escape_quotes)
{
	fprintf(out, "%s $t%d", instr_to_string(row->instr), row->row_no);
	UNUSED(escape_quotes);
}

static void print_temporary(FILE *out, struct mcc_ir_row *row, bool escape_quotes)
{
	fprintf(out, "$t%d = ", row->row_no);
	print_arg(out, row->arg1, escape_quotes);
	fprintf(out, " %s ", instr_to_string(row->instr));
	print_arg(out, row->arg2, escape_quotes);
}
static void print_assign(FILE *out, struct mcc_ir_row *row, bool escape_quotes)
{
	print_arg(out, row->arg1, escape_quotes);
	fprintf(out, " %s ", instr_to_string(row->instr));
	print_arg(out, row->arg2, escape_quotes);
}
static void print_label(FILE *out, struct mcc_ir_row *row, bool escape_quotes)
{
	fprintf(out, "L%d:", row->arg1->label);
	UNUSED(escape_quotes);
}
static void print_function_label(FILE *out, struct mcc_ir_row *row, bool escape_quotes)
{
	fprintf(out, "%s:", row->arg1->func_label);
	UNUSED(escape_quotes);
}

char *instr_to_string(enum mcc_ir_instruction instr)
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
	default:
		return "";
	}
}

char *bool_to_string(bool b)
{
	if (b) {
		return "true";
	} else {
		return "false";
	}
}

static void print_arg(FILE *out, struct mcc_ir_arg *arg, bool escape_quotes)
{
	if (!arg)
		return;
	switch (arg->type) {
	case MCC_IR_TYPE_ROW:
		fprintf(out, "$t%d", arg->row->row_no);
		return;
	case MCC_IR_TYPE_LIT_INT:
		fprintf(out, "%ld", arg->lit_int);
		return;
	case MCC_IR_TYPE_LIT_FLOAT:
		fprintf(out, "%f", arg->lit_float);
		return;
	case MCC_IR_TYPE_LIT_BOOL:
		fprintf(out, "%s", bool_to_string(arg->lit_bool));
		return;
	case MCC_IR_TYPE_LIT_STRING:
		break;
	case MCC_IR_TYPE_LABEL:
		fprintf(out, "L%d", arg->label);
		return;
	case MCC_IR_TYPE_IDENTIFIER:
		fprintf(out, "%s", arg->ident);
		return;
	case MCC_IR_TYPE_ARR_ELEM:
		fprintf(out, "%s[", arg->arr_ident);
		print_arg(out, arg->index, escape_quotes);
		fprintf(out, "]");
		return;
	case MCC_IR_TYPE_FUNC_LABEL:
		fprintf(out, "%s", arg->func_label);
		return;
	};
	// MCC_IR_TYPE_LIT_STRING:
	if (escape_quotes) {
		fprintf(out, "\\\"");
		mcc_print_string_literal(out, arg->lit_string);
		fprintf(out, "\\\"");
	} else {
		fprintf(out, "\"");
		mcc_print_string_literal(out, arg->lit_string);
		fprintf(out, "\"");
	}
}
