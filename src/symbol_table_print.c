#include "mcc/symbol_table_print.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ------------------------------------------------------------- Forward declaration

static void print_symbol_table_scope(struct mcc_symbol_table_scope *scope, const char *leading_spaces, FILE *out);

// ------------------------------------------------------------- Implementation

static void print_symbol_table_begin(struct mcc_symbol_table_scope *scope, FILE *out)
{
	assert(scope);
	assert(out);

	if (scope->head && scope->head->node) {
		struct mcc_ast_node node = *(scope->head->node);
		fprintf(out,
		        "----------------------------------------------------------------------\n"
		        "symbol table: %s\n"
		        "----------------------------------------------------------------------\n",
		        node.sloc.filename);
	} else {
		fprintf(out, "----------------------------------------------------------------------\n"
		             "symbol table:\n"
		             "----------------------------------------------------------------------\n");
	}
}

static void print_symbol_table_end(FILE *out)
{
	assert(out);

	fprintf(out, "----------------------------------------------------------------------\n");
}

static const char *print_row_type(enum mcc_symbol_table_row_type type)
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

static void print_row(struct mcc_symbol_table_row *row, const char *leading_spaces, FILE *out)
{
	assert(row);
	assert(leading_spaces);
	assert(out);

	switch (row->row_structure) {
	case MCC_SYMBOL_TABLE_ROW_STRUCTURE_VARIABLE:
		fprintf(out, "%s%s (%s)\n", leading_spaces, row->name, print_row_type(row->row_type));
		break;
	case MCC_SYMBOL_TABLE_ROW_STRUCTURE_FUNCTION:
		fprintf(out, "%s%s (%s function)\n", leading_spaces, row->name, print_row_type(row->row_type));
		break;
	case MCC_SYMBOL_TABLE_ROW_STRUCTURE_ARRAY:
		fprintf(out, "%s%s (%s[%ld])\n", leading_spaces, row->name, print_row_type(row->row_type),
		        row->array_size);
		break;
	}
}

static void print_symbol_table_row(struct mcc_symbol_table_row *row, const char *leading_spaces, FILE *out)
{
	assert(row);
	assert(leading_spaces);
	assert(out);

	print_row(row, leading_spaces, out);

	// don't print scopes of built-ins
	if ((strcmp(row->name, "print") == 0) || (strcmp(row->name, "print_int") == 0) ||
	    (strcmp(row->name, "print_float") == 0)) {
		return;
	}
	struct mcc_symbol_table_scope *child_scope = row->child_scope;

	int size = strlen(leading_spaces) + 8;
	char *new_leading_spaces = (char *)malloc(sizeof(char) * size);
	snprintf(new_leading_spaces, size, "    %s", leading_spaces);

	while (child_scope && child_scope->head && child_scope->head->row_type != MCC_SYMBOL_TABLE_ROW_TYPE_PSEUDO) {
		print_symbol_table_scope(child_scope, new_leading_spaces, out);
		child_scope = child_scope->next_scope;
	}

	free(new_leading_spaces);
}

static void print_symbol_table_scope(struct mcc_symbol_table_scope *scope, const char *leading_spaces, FILE *out)
{
	assert(scope);
	assert(leading_spaces);
	assert(out);

	if (!scope->head) {
		fprintf(out, "\n");
	} else {

		struct mcc_symbol_table_row *row = scope->head;

		while (row) {
			print_symbol_table_row(row, leading_spaces, out);
			row = row->next_row;
		}
	}
}

void mcc_symbol_table_print(struct mcc_symbol_table *table, void *data)
{
	assert(table);
	assert(data);

	FILE *out = data;

	if (!table->head) {
		fprintf(out, "empty symbol table\n");
	} else {

		struct mcc_symbol_table_scope *scope = table->head;

		print_symbol_table_begin(scope, out);

		while (scope) {
			print_symbol_table_scope(scope, "", out);
			scope = scope->next_scope;
		}

		print_symbol_table_end(out);
	}
}
