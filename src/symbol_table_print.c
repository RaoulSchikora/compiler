//
// Created by raoul on 07.03.20.
//

#include "mcc/symbol_table_print.h"

#include <assert.h>


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

static void print_dot_symbol_table_scope(struct mcc_symbol_table_scope *scope, FILE *out)
{
    assert(scope);
    assert(out);

    if(!scope->head) {
        fprintf(out, "<tr><td> ---- </td></tr>\n");
    } else {

        struct mcc_symbol_table_row *row = scope->head;

        while(row) {
            fprintf(out, "<tr><td>%s (%d)</td></tr>\n", row->name, row->row_type);
            if(row->child_scope){
                print_dot_symbol_table_open_new_scope(out);
                print_dot_symbol_table_scope(row->child_scope, out);
                print_dot_symbol_table_close_new_scope(out);
            }
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
