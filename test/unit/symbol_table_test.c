#include <CuTest.h>

#include "mcc/ast.h"
#include "mcc/parser.h"
#include "mcc/symbol_table.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Threshold for floating point comparisions.
static const double EPS = 1e-3;

void multiple_rows(CuTest *tc)
{
    // {
    //   int i;
    //   bool j;
    //   float k;
    //   string str;
    // }
    struct mcc_symbol_table_row *row_int = mcc_symbol_table_new_row("i", MCC_SYMBOL_TABLE_ROW_TYPE_INT);
    struct mcc_symbol_table_row *row_bool = mcc_symbol_table_new_row("j", MCC_SYMBOL_TABLE_ROW_TYPE_BOOL);
    struct mcc_symbol_table_row *row_float = mcc_symbol_table_new_row("k", MCC_SYMBOL_TABLE_ROW_TYPE_FLOAT);
    struct mcc_symbol_table_row *row_string = mcc_symbol_table_new_row("str", MCC_SYMBOL_TABLE_ROW_TYPE_STRING);

    struct mcc_symbol_table_scope *scope = mcc_symbol_table_new_scope();

    mcc_symbol_table_scope_append_row(scope, row_int);
    mcc_symbol_table_scope_append_row(scope, row_bool);
    mcc_symbol_table_scope_append_row(scope, row_float);
    mcc_symbol_table_scope_append_row(scope, row_string);

    struct mcc_symbol_table_row *current_row = scope->head;

    CuAssertStrEquals(tc, "i", current_row->name);
    CuAssertStrEquals(tc, "j", current_row->next_row->name);
    CuAssertStrEquals(tc, "k", current_row->next_row->next_row->name);
    CuAssertStrEquals(tc, "str", current_row->next_row->next_row->next_row->name);

    current_row = current_row->next_row->next_row->next_row;

    CuAssertStrEquals(tc, "k", current_row->prev_row->name);
    CuAssertStrEquals(tc, "j", current_row->prev_row->prev_row->name);
    CuAssertStrEquals(tc, "i", current_row->prev_row->prev_row->prev_row->name);

    mcc_symbol_table_delete_scope(scope);
}

void scope_siblings(CuTest *tc)
{
    // {
    //    int i;
    //    bool j;
    // }
    // {
    // }
    // {
    //    float k;
    //    string str;
    // }
    struct mcc_symbol_table_row *row_int = mcc_symbol_table_new_row("i", MCC_SYMBOL_TABLE_ROW_TYPE_INT);
    struct mcc_symbol_table_row *row_bool = mcc_symbol_table_new_row("j", MCC_SYMBOL_TABLE_ROW_TYPE_BOOL);
    struct mcc_symbol_table_row *row_float = mcc_symbol_table_new_row("k", MCC_SYMBOL_TABLE_ROW_TYPE_FLOAT);
    struct mcc_symbol_table_row *row_string = mcc_symbol_table_new_row("str", MCC_SYMBOL_TABLE_ROW_TYPE_STRING);

    struct mcc_symbol_table_scope *first_scope = mcc_symbol_table_new_scope();
    struct mcc_symbol_table_scope *second_scope = mcc_symbol_table_new_scope();
    struct mcc_symbol_table_scope *third_scope = mcc_symbol_table_new_scope();

    mcc_symbol_table_scope_append_row(first_scope, row_int);
    mcc_symbol_table_scope_append_row(first_scope, row_bool);
    mcc_symbol_table_scope_append_row(third_scope, row_float);
    mcc_symbol_table_scope_append_row(third_scope, row_string);

    struct mcc_symbol_table *table = mcc_symbol_table_new_table();

    mcc_symbol_table_insert_scope(table, first_scope);
    mcc_symbol_table_insert_scope(table, second_scope);
    mcc_symbol_table_insert_scope(table, third_scope);

    struct mcc_symbol_table_scope *current_scope = table->head;
    struct mcc_symbol_table_row *current_row = current_scope->head;

    CuAssertTrue(tc, current_scope == first_scope);
    CuAssertStrEquals(tc, "i", current_row->name);
    CuAssertStrEquals(tc, "j", current_row->next_row->name);
    CuAssertTrue(tc, current_scope->has_next);
    CuAssertTrue(tc, current_scope->next_scope == second_scope);

    current_scope = current_scope->next_scope;

    CuAssertTrue(tc, current_scope->head == NULL);
    CuAssertTrue(tc, current_scope->next_scope == third_scope);

    current_scope = current_scope->next_scope;
    current_row = current_scope->head;

    CuAssertStrEquals(tc, "k", current_row->name);
    CuAssertStrEquals(tc, "str", current_row->next_row->name);
    CuAssertTrue(tc, !current_scope->has_next);
    CuAssertTrue(tc, current_scope->next_scope == NULL);
}

void nesting_scope(CuTest *tc)
{
    // {
    //   int i;
    //      {
    //        bool j;
    //        string str;
    //      }
    //   float k;
    // }

}

#define TESTS \
	TEST(multiple_rows)   \
//	TEST(scope_siblings)  \
	TEST(nesting_scope)
#include "main_stub.inc"
#undef TESTS