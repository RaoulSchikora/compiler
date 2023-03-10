#include "mcc/symbol_table_print_dot.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mcc/ast.h"

// ------------------------------------------------------------- Implementation

void mcc_symbol_table_print_dot_begin(struct mcc_symbol_table_scope *scope, FILE *out)
{
	assert(out);
	assert(scope);

	if (scope->head && scope->head->node) {
		struct mcc_ast_node node = *(scope->head->node);
		fprintf(out,
		        "digraph {\n\n"
		        "tbl [\n\n"
		        "shape=plaintext\n"
		        "label=<\n\n"
		        "<table border='0' cellborder='0' cellspacing='0'>\n"
		        "<tr><td>Symbol Table: %s</td></tr>\n",
		        node.sloc.filename);
	} else {
		fprintf(out, "digraph {\n\n"
		             "tbl [\n\n"
		             "shape=plaintext\n"
		             "label=<\n\n"
		             "<table border='0' cellborder='0' cellspacing='0'>\n"
		             "<tr><td>Symbol Table</td></tr>\n");
	}
}

void mcc_symbol_table_print_dot_end(FILE *out)
{
	assert(out);

	fprintf(out, "</table>\n\n"
	             ">];\n\n"
	             "}\n\n");
}

void mcc_symbol_table_print_dot_open_new_scope(FILE *out)
{
	assert(out);

	fprintf(out, "<tr><td cellpadding='2'>\n\n"
	             "<table cellspacing='0'>\n");
}

void mcc_symbol_table_print_dot_close_new_scope(FILE *out)
{
	assert(out);

	fprintf(out, "</table>\n\n"
	             "</td></tr>\n");
}

static const char *print_dot_row_type(enum mcc_symbol_table_row_type type)
{
	switch (type) {
	case MCC_SYMBOL_TABLE_ROW_TYPE_BOOL:
		return "bool";
	case MCC_SYMBOL_TABLE_ROW_TYPE_INT:
		return "int";
	case MCC_SYMBOL_TABLE_ROW_TYPE_FLOAT:
		return "float";
	case MCC_SYMBOL_TABLE_ROW_TYPE_STRING:
		return "string";
	case MCC_SYMBOL_TABLE_ROW_TYPE_VOID:
		return "void";
	case MCC_SYMBOL_TABLE_ROW_TYPE_PSEUDO:
		return "-";
	default:
		return "unknown type";
	}
}

static void print_dot_row(struct mcc_symbol_table_row *row, const char *leading_spaces, FILE *out)
{
	assert(row);
	assert(leading_spaces);
	assert(out);

	switch (row->row_structure) {
	case MCC_SYMBOL_TABLE_ROW_STRUCTURE_VARIABLE:
		fprintf(out, "<tr><td align='left'>%s%s (%s)</td></tr>\n", leading_spaces, row->name,
		        print_dot_row_type(row->row_type));
		break;
	case MCC_SYMBOL_TABLE_ROW_STRUCTURE_FUNCTION:
		fprintf(out, "<tr><td align='left'>%s%s (%s function)</td></tr>\n", leading_spaces, row->name,
		        print_dot_row_type(row->row_type));
		break;
	case MCC_SYMBOL_TABLE_ROW_STRUCTURE_ARRAY:
		fprintf(out, "<tr><td align='left'>%s%s (%s[%ld])</td></tr>\n", leading_spaces, row->name,
		        print_dot_row_type(row->row_type), row->array_size);
		break;
	}
}

void mcc_symbol_table_print_dot_row(struct mcc_symbol_table_row *row, const char *leading_spaces, FILE *out)
{
	assert(row);
	assert(leading_spaces);
	assert(out);

	print_dot_row(row, leading_spaces, out);

	// don't print scopes of built-ins
	if ((strcmp(row->name, "print") == 0) || (strcmp(row->name, "print_int") == 0) ||
	    (strcmp(row->name, "print_float") == 0)) {
		return;
	}
	struct mcc_symbol_table_scope *child_scope = row->child_scope;

	int size = strlen(leading_spaces) + 8;
	char *new_leading_spaces = (char *)malloc(sizeof(char) * size);
	if (!new_leading_spaces)
		return;
	snprintf(new_leading_spaces, size, "        %s", leading_spaces);

	while (child_scope && child_scope->head && child_scope->head->row_type != MCC_SYMBOL_TABLE_ROW_TYPE_PSEUDO) {
		if (row->row_structure == MCC_SYMBOL_TABLE_ROW_STRUCTURE_FUNCTION) {
			mcc_symbol_table_print_dot_open_new_scope(out);
		}
		mcc_symbol_table_print_dot_scope(child_scope, new_leading_spaces, out);
		if (row->row_structure == MCC_SYMBOL_TABLE_ROW_STRUCTURE_FUNCTION) {
			mcc_symbol_table_print_dot_close_new_scope(out);
		}
		child_scope = child_scope->next_scope;
	}

	free(new_leading_spaces);
}

void mcc_symbol_table_print_dot_scope(struct mcc_symbol_table_scope *scope, const char *leading_spaces, FILE *out)
{
	assert(scope);
	assert(leading_spaces);
	assert(out);

	if (!scope->head) {
		fprintf(out, "\n");
	} else {

		struct mcc_symbol_table_row *row = scope->head;

		while (row) {
			mcc_symbol_table_print_dot_row(row, leading_spaces, out);
			row = row->next_row;
		}
	}
}

void mcc_symbol_table_print_dot(struct mcc_symbol_table *table, FILE *out)
{
	assert(table);
	assert(out);

	if (!table->head) {
		fprintf(out, "<tr><td> ---- </td></tr>\n");
	} else {

		struct mcc_symbol_table_scope *scope = table->head;

		mcc_symbol_table_print_dot_begin(scope, out);

		while (scope) {

			mcc_symbol_table_print_dot_open_new_scope(out);
			mcc_symbol_table_print_dot_scope(scope, "", out);
			mcc_symbol_table_print_dot_close_new_scope(out);

			scope = scope->next_scope;
		}

		mcc_symbol_table_print_dot_end(out);
	}
}

