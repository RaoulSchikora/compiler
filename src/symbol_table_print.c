//
// Created by raoul on 07.03.20.
//

#include "mcc/symbol_table_print.h"

#include <assert.h>

//forward declaration
static void print_dot_symbol_table_scope(struct mcc_symbol_table_scope *scope, FILE *out);


static void print_dot_symbol_table_begin(FILE *out)
{
    assert(out);

    fprintf(out, "digraph {\n\n"
                 "tbl [\n\n"
                 "shape=plaintext\n"
                 "label=<\n\n"
                 "<table border='0' cellborder='1' cellspacing='0'>\n"
                 "<tr><td>Symbol Table</td></tr>\n");
}

static void print_dot_symbol_table_end(FILE *out)
{
    assert(out);

    fprintf(out, "</table>\n\n"
                 ">];\n\n"
                 "}\n\n");
}

static void print_dot_symbol_table_open_new_scope(FILE *out)
{
    assert(out);

    fprintf(out, "<tr><td cellpadding='4'>\n\n"
                 "<table cellspacing='0'>\n");
}

static void print_dot_symbol_table_close_new_scope(FILE *out)
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
    }

    return "unknown type";
}

static void print_dot_symbol_table_row(struct mcc_symbol_table_row *row, FILE *out)
{
    assert(row);
    assert(out);

    switch (row->row_structure) {
    case MCC_SYMBOL_TABLE_ROW_STRUCTURE_VARIABLE:
        fprintf(out, "<tr><td>%s (%s)</td></tr>\n", row->name, print_dot_row_type(row->row_type));
        break;
    case MCC_SYMBOL_TABLE_ROW_STRUCTURE_FUNCTION:
        fprintf(out, "<tr><td>%s (%s function)</td></tr>\n", row->name, print_dot_row_type(row->row_type));
        break;
    case MCC_SYMBOL_TABLE_ROW_STRUCTURE_ARRAY:
        fprintf(out, "<tr><td>%s (%s[%d])</td></tr>\n", row->name, print_dot_row_type(row->row_type), row->array_size);
        break;
    }

    if(row->child_scope){
        struct mcc_symbol_table_scope *child_scope = row->child_scope;

        while(child_scope){
            print_dot_symbol_table_open_new_scope(out);
            print_dot_symbol_table_scope(child_scope, out);
            print_dot_symbol_table_close_new_scope(out);

            child_scope = child_scope->next_scope;
        }
    }
}

static void print_dot_symbol_table_scope(struct mcc_symbol_table_scope *scope, FILE *out)
{
    assert(scope);
    assert(out);

    if(!scope->head) {
        fprintf(out, "<tr><td> ---- </td></tr>\n");
    } else {

        struct mcc_symbol_table_row *row = scope->head;

        while(row) {
            print_dot_symbol_table_row(row, out);
            row = row->next_row;
        }
    }
}

void mcc_symbol_table_print_dot(struct mcc_symbol_table *table, void *data)
{
    assert(table);
    assert(data);

    FILE *out = data;

    if(!table->head) {
        fprintf(out, "<tr><td> ---- </td></tr>\n");
    } else {

        struct mcc_symbol_table_scope *scope = table->head;

        print_dot_symbol_table_begin(out);

        while(scope){

            print_dot_symbol_table_open_new_scope(out);
            print_dot_symbol_table_scope(scope, out);
            print_dot_symbol_table_close_new_scope(out);

            scope = scope->next_scope;
        }

        print_dot_symbol_table_end(out);
    }
}
