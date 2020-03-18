#include <CuTest.h>

#include "mcc/ast.h"
#include "mcc/parser.h"
#include "mcc/symbol_table.h"
#include "mcc/symbol_table_print.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void empty_table(CuTest *tc)
{
    struct mcc_symbol_table *table = mcc_symbol_table_new_table();

    CuAssertTrue(tc, table->head == NULL);

    mcc_symbol_table_delete_table(table);
}

void multiple_rows(CuTest *tc)
{
    // {
    //   int i;
    //   bool j;
    //   float k;
    //   string str;
    // }
    struct mcc_symbol_table_row *row_int = mcc_symbol_table_new_row_variable("i", MCC_SYMBOL_TABLE_ROW_TYPE_INT);
    struct mcc_symbol_table_row *row_bool = mcc_symbol_table_new_row_variable("j", MCC_SYMBOL_TABLE_ROW_TYPE_BOOL);
    struct mcc_symbol_table_row *row_float = mcc_symbol_table_new_row_variable("k", MCC_SYMBOL_TABLE_ROW_TYPE_FLOAT);
    struct mcc_symbol_table_row *row_string = mcc_symbol_table_new_row_variable("str", MCC_SYMBOL_TABLE_ROW_TYPE_STRING);

    struct mcc_symbol_table_scope *scope = mcc_symbol_table_new_scope();

    mcc_symbol_table_scope_append_row(scope, row_int);
    mcc_symbol_table_scope_append_row(scope, row_bool);
    mcc_symbol_table_scope_append_row(scope, row_float);
    mcc_symbol_table_scope_append_row(scope, row_string);

    struct mcc_symbol_table_row *current_row = scope->head;

    CuAssertStrEquals(tc, "i", current_row->name);
    CuAssertIntEquals(tc, MCC_SYMBOL_TABLE_ROW_TYPE_INT, current_row->row_type);
    CuAssertStrEquals(tc, "j", current_row->next_row->name);
    CuAssertIntEquals(tc, MCC_SYMBOL_TABLE_ROW_TYPE_BOOL, current_row->next_row->row_type);
    CuAssertStrEquals(tc, "k", current_row->next_row->next_row->name);
    CuAssertIntEquals(tc, MCC_SYMBOL_TABLE_ROW_TYPE_FLOAT, current_row->next_row->next_row->row_type);
    CuAssertStrEquals(tc, "str", current_row->next_row->next_row->next_row->name);
    CuAssertIntEquals(tc, MCC_SYMBOL_TABLE_ROW_TYPE_STRING, current_row->next_row->next_row->next_row->row_type);

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
    struct mcc_symbol_table_row *row_int = mcc_symbol_table_new_row_variable("i", MCC_SYMBOL_TABLE_ROW_TYPE_INT);
    struct mcc_symbol_table_row *row_bool = mcc_symbol_table_new_row_variable("j", MCC_SYMBOL_TABLE_ROW_TYPE_BOOL);
    struct mcc_symbol_table_row *row_float = mcc_symbol_table_new_row_variable("k", MCC_SYMBOL_TABLE_ROW_TYPE_FLOAT);
    struct mcc_symbol_table_row *row_string = mcc_symbol_table_new_row_variable("str", MCC_SYMBOL_TABLE_ROW_TYPE_STRING);

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
    CuAssertTrue(tc, current_scope->next_scope == second_scope);

    current_scope = current_scope->next_scope;

    CuAssertTrue(tc, current_scope->head == NULL);
    CuAssertTrue(tc, current_scope->next_scope == third_scope);

    current_scope = current_scope->next_scope;
    current_row = current_scope->head;

    CuAssertStrEquals(tc, "k", current_row->name);
    CuAssertStrEquals(tc, "str", current_row->next_row->name);
    CuAssertTrue(tc, current_scope->next_scope == NULL);

    mcc_symbol_table_delete_table(table);
}

void nesting_scope(CuTest *tc)
{
    // {
    //   int i;
    //      {
    //        bool j;
    //        float k;
    //      }
    //      {
    //      }
    //   string str;
    // }
    struct mcc_symbol_table_row *row_int = mcc_symbol_table_new_row_variable("i", MCC_SYMBOL_TABLE_ROW_TYPE_INT);
    struct mcc_symbol_table_row *row_bool = mcc_symbol_table_new_row_variable("j", MCC_SYMBOL_TABLE_ROW_TYPE_BOOL);
    struct mcc_symbol_table_row *row_float = mcc_symbol_table_new_row_variable("k", MCC_SYMBOL_TABLE_ROW_TYPE_FLOAT);
    struct mcc_symbol_table_row *row_string = mcc_symbol_table_new_row_variable("str", MCC_SYMBOL_TABLE_ROW_TYPE_STRING);

    struct mcc_symbol_table_scope *outer_scope = mcc_symbol_table_new_scope();
    struct mcc_symbol_table_scope *inner_scope1 = mcc_symbol_table_new_scope();
    struct mcc_symbol_table_scope *inner_scope2 = mcc_symbol_table_new_scope();

    struct mcc_symbol_table *table = mcc_symbol_table_new_table();

    mcc_symbol_table_scope_append_row(outer_scope, row_int);
    mcc_symbol_table_scope_append_row(inner_scope1, row_bool);
    mcc_symbol_table_scope_append_row(inner_scope1, row_float);

    mcc_symbol_table_insert_scope(table, outer_scope);
    mcc_symbol_table_row_append_child_scope(row_int, inner_scope1);
    mcc_symbol_table_row_append_child_scope(row_int, inner_scope2);

    mcc_symbol_table_scope_append_row(outer_scope, row_string);

    struct mcc_symbol_table_scope *current_scope = table->head;
    struct mcc_symbol_table_row *current_row = current_scope->head;

    CuAssertTrue(tc, current_scope == outer_scope);
    CuAssertStrEquals(tc, "i", current_row->name);
    CuAssertTrue(tc, current_row->row_structure == MCC_SYMBOL_TABLE_ROW_STRUCTURE_VARIABLE);
    CuAssertStrEquals(tc, "str", current_row->next_row->name);
    CuAssertTrue(tc, current_row->child_scope == inner_scope1);

    current_scope = current_row->child_scope;
    current_row = current_scope->head;

    CuAssertTrue(tc, current_scope->parent_row == row_int);
    CuAssertStrEquals(tc, "j", current_row->name);
    CuAssertStrEquals(tc, "k", current_row->next_row->name);
    CuAssertTrue(tc, current_scope->next_scope == inner_scope2);
    CuAssertTrue(tc, current_scope->next_scope->parent_row == row_int);
    CuAssertTrue(tc, current_scope->next_scope->next_scope == NULL);

    mcc_symbol_table_delete_table(table);
}

void array_row(CuTest *tc)
{
    //{
    //  int[42] i;
    //}
    struct mcc_symbol_table_row *int_array_row = mcc_symbol_table_new_row_array("i", 42, MCC_SYMBOL_TABLE_ROW_TYPE_INT);

    CuAssertTrue(tc, int_array_row->row_structure == MCC_SYMBOL_TABLE_ROW_STRUCTURE_ARRAY);
    CuAssertTrue(tc, int_array_row->array_size == 42);

    mcc_symbol_table_delete_row(int_array_row);
}

void get_last(CuTest *tc)
{
    //{
    //  int i;
    //  bool j;
    //  float k;
    //}
    struct mcc_symbol_table_row *row_int = mcc_symbol_table_new_row_variable("i", MCC_SYMBOL_TABLE_ROW_TYPE_INT);
    struct mcc_symbol_table_row *row_bool = mcc_symbol_table_new_row_variable("j", MCC_SYMBOL_TABLE_ROW_TYPE_BOOL);
    struct mcc_symbol_table_row *row_float = mcc_symbol_table_new_row_variable("k", MCC_SYMBOL_TABLE_ROW_TYPE_FLOAT);

    struct mcc_symbol_table_scope *scope = mcc_symbol_table_new_scope();

    mcc_symbol_table_scope_append_row(scope, row_int);
    mcc_symbol_table_scope_append_row(scope, row_bool);
    mcc_symbol_table_scope_append_row(scope, row_float);

    struct mcc_symbol_table_row *row = mcc_symbol_table_scope_get_last_row(scope);

    CuAssertTrue(tc, row == row_float);

    mcc_symbol_table_delete_scope(scope);
}

void function_parameters(CuTest *tc)
{
    const char input[] = "int func(bool a, int b){}";
    struct mcc_parser_result result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);

    struct mcc_symbol_table *table = mcc_symbol_table_create(result.program);

    struct mcc_symbol_table_scope *scope = table->head;

    CuAssertStrEquals(tc, "func", scope->head->name);
    CuAssertStrEquals(tc, "a", scope->head->child_scope->head->name);
    CuAssertStrEquals(tc, "b", scope->head->child_scope->head->next_row->name);

    mcc_symbol_table_delete_table(table);
    mcc_ast_delete(result.program);
}

void function_parameters2(CuTest *tc)
{
    const char input[] = "int func1(int a, float[10] b, string c, bool d){}";
    struct mcc_parser_result result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);

    struct mcc_symbol_table *table = mcc_symbol_table_create(result.program);

    struct mcc_symbol_table_row *row = table->head->head->child_scope->head;

    CuAssertStrEquals(tc, "a", row->name);
    CuAssertTrue(tc, row->row_structure == MCC_SYMBOL_TABLE_ROW_STRUCTURE_VARIABLE);
    CuAssertTrue(tc, row->row_type == MCC_SYMBOL_TABLE_ROW_TYPE_INT);
    CuAssertTrue(tc, row->array_size == -1);

    CuAssertStrEquals(tc, "b", row->next_row->name);
    CuAssertTrue(tc, row->next_row->row_structure == MCC_SYMBOL_TABLE_ROW_STRUCTURE_ARRAY);
    CuAssertTrue(tc, row->next_row->row_type == MCC_SYMBOL_TABLE_ROW_TYPE_FLOAT);
    CuAssertTrue(tc, row->next_row->array_size == 10);

    CuAssertStrEquals(tc, "c", row->next_row->next_row->name);
    CuAssertTrue(tc, row->next_row->next_row->row_structure == MCC_SYMBOL_TABLE_ROW_STRUCTURE_VARIABLE);
    CuAssertTrue(tc, row->next_row->next_row->row_type == MCC_SYMBOL_TABLE_ROW_TYPE_STRING);
    CuAssertTrue(tc, row->next_row->next_row->array_size == -1);

    CuAssertStrEquals(tc, "d", row->next_row->next_row->next_row->name);
    CuAssertTrue(tc, row->next_row->next_row->next_row->row_structure == MCC_SYMBOL_TABLE_ROW_STRUCTURE_VARIABLE);
    CuAssertTrue(tc, row->next_row->next_row->next_row->row_type == MCC_SYMBOL_TABLE_ROW_TYPE_BOOL);
    CuAssertTrue(tc, row->next_row->next_row->next_row->array_size == -1);

    mcc_symbol_table_delete_table(table);
    mcc_ast_delete(result.program);
}

void nested_if(CuTest *tc)
{
    const char input[] = "int main(){if(true)if(true){int a;int b;}return 0;}";
    struct mcc_parser_result result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);

    struct mcc_symbol_table *table = mcc_symbol_table_create(result.program);

    struct mcc_symbol_table_scope *scope = table->head;

    CuAssertStrEquals(tc, "a", scope->head->child_scope->head->child_scope->head->name);
    CuAssertStrEquals(tc, "b", scope->head->child_scope->head->child_scope->head->next_row->name);

    mcc_symbol_table_delete_table(table);
    mcc_ast_delete(result.program);
}

void function_definition(CuTest *tc)
{
    const char input[] = "float func1(){} bool func2(){} int main(){} string func3(){}";
    struct mcc_parser_result result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);

    struct mcc_symbol_table *table = mcc_symbol_table_create(result.program);

    struct mcc_symbol_table_row *row = table->head->head;

    CuAssertStrEquals(tc, "func1", row->name);
    CuAssertTrue(tc, row->row_structure == MCC_SYMBOL_TABLE_ROW_STRUCTURE_VARIABLE);
    CuAssertTrue(tc, row->row_type == MCC_SYMBOL_TABLE_ROW_TYPE_FUNCTION);
    CuAssertTrue(tc, row->array_size == -1);

    CuAssertStrEquals(tc, "func2", row->next_row->name);
    CuAssertTrue(tc, row->next_row->row_structure == MCC_SYMBOL_TABLE_ROW_STRUCTURE_VARIABLE);
    CuAssertTrue(tc, row->next_row->row_type == MCC_SYMBOL_TABLE_ROW_TYPE_FUNCTION);
    CuAssertTrue(tc, row->next_row->array_size == -1);

    CuAssertStrEquals(tc, "main", row->next_row->next_row->name);
    CuAssertTrue(tc, row->next_row->next_row->row_structure == MCC_SYMBOL_TABLE_ROW_STRUCTURE_VARIABLE);
    CuAssertTrue(tc, row->next_row->next_row->row_type == MCC_SYMBOL_TABLE_ROW_TYPE_FUNCTION);
    CuAssertTrue(tc, row->next_row->next_row->array_size == -1);

    CuAssertStrEquals(tc, "func3", row->next_row->next_row->next_row->name);
    CuAssertTrue(tc, row->next_row->next_row->next_row->row_structure == MCC_SYMBOL_TABLE_ROW_STRUCTURE_VARIABLE);
    CuAssertTrue(tc, row->next_row->next_row->next_row->row_type == MCC_SYMBOL_TABLE_ROW_TYPE_FUNCTION);
    CuAssertTrue(tc, row->next_row->next_row->next_row->array_size == -1);

    mcc_symbol_table_delete_table(table);
    mcc_ast_delete(result.program);
}

void function_body(CuTest *tc)
{
    const char input[] = "float func(){int n;n=0; while(n<42)n=n+1; int b;}";
    struct mcc_parser_result result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);

    struct mcc_symbol_table *table = mcc_symbol_table_create(result.program);

    struct mcc_symbol_table_row *row = table->head->head->child_scope->head;

    CuAssertStrEquals(tc, "n", row->name);
    CuAssertStrEquals(tc, "b", row->next_row->name);

    mcc_symbol_table_delete_table(table);
    mcc_ast_delete(result.program);
}

void empty_nested_function_body(CuTest *tc)
{
    const char input[] = "float func(){ { { {} } } }";
    struct mcc_parser_result result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);

    struct mcc_symbol_table *table = mcc_symbol_table_create(result.program);

    struct mcc_symbol_table_row *row = table->head->head;

    CuAssertTrue(tc, row->child_scope->head->row_type == MCC_SYMBOL_TABLE_ROW_TYPE_PSEUDO);

    row = row->child_scope->head->child_scope->head->child_scope->head;

    CuAssertTrue(tc, row->row_type == MCC_SYMBOL_TABLE_ROW_TYPE_PSEUDO);
    CuAssertTrue(tc, row->child_scope->head == NULL);

    mcc_symbol_table_delete_table(table);
    mcc_ast_delete(result.program);
}

#define TESTS \
    TEST(empty_table)         \
	TEST(multiple_rows)       \
	TEST(scope_siblings)      \
	TEST(nesting_scope)       \
	TEST(array_row)           \
	TEST(get_last)            \
	TEST(function_parameters) \
	TEST(function_parameters2)\
	TEST(nested_if)           \
	TEST(function_definition) \
	TEST(function_body)       \
	TEST(empty_nested_function_body)
#include "main_stub.inc"
#undef TESTS