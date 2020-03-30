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
	struct mcc_symbol_table_row *row_int =
	    mcc_symbol_table_new_row_variable("i", MCC_SYMBOL_TABLE_ROW_TYPE_INT, NULL);
	struct mcc_symbol_table_row *row_bool =
	    mcc_symbol_table_new_row_variable("j", MCC_SYMBOL_TABLE_ROW_TYPE_BOOL, NULL);
	struct mcc_symbol_table_row *row_float =
	    mcc_symbol_table_new_row_variable("k", MCC_SYMBOL_TABLE_ROW_TYPE_FLOAT, NULL);
	struct mcc_symbol_table_row *row_string =
	    mcc_symbol_table_new_row_variable("str", MCC_SYMBOL_TABLE_ROW_TYPE_STRING, NULL);

	struct mcc_symbol_table_scope *scope = mcc_symbol_table_new_scope();

	mcc_symbol_table_scope_append_row(scope, row_int);
	mcc_symbol_table_scope_append_row(scope, row_bool);
	mcc_symbol_table_scope_append_row(scope, row_float);
	mcc_symbol_table_scope_append_row(scope, row_string);

	struct mcc_symbol_table_row *current_row = scope->head;

	CuAssertTrue(tc, current_row->scope == scope);
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

	CuAssertTrue(tc, current_row->scope == scope);

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
	struct mcc_symbol_table_row *row_int =
	    mcc_symbol_table_new_row_variable("i", MCC_SYMBOL_TABLE_ROW_TYPE_INT, NULL);
	struct mcc_symbol_table_row *row_bool =
	    mcc_symbol_table_new_row_variable("j", MCC_SYMBOL_TABLE_ROW_TYPE_BOOL, NULL);
	struct mcc_symbol_table_row *row_float =
	    mcc_symbol_table_new_row_variable("k", MCC_SYMBOL_TABLE_ROW_TYPE_FLOAT, NULL);
	struct mcc_symbol_table_row *row_string =
	    mcc_symbol_table_new_row_variable("str", MCC_SYMBOL_TABLE_ROW_TYPE_STRING, NULL);

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
	struct mcc_symbol_table_row *row_int =
	    mcc_symbol_table_new_row_variable("i", MCC_SYMBOL_TABLE_ROW_TYPE_INT, NULL);
	struct mcc_symbol_table_row *row_bool =
	    mcc_symbol_table_new_row_variable("j", MCC_SYMBOL_TABLE_ROW_TYPE_BOOL, NULL);
	struct mcc_symbol_table_row *row_float =
	    mcc_symbol_table_new_row_variable("k", MCC_SYMBOL_TABLE_ROW_TYPE_FLOAT, NULL);
	struct mcc_symbol_table_row *row_string =
	    mcc_symbol_table_new_row_variable("str", MCC_SYMBOL_TABLE_ROW_TYPE_STRING, NULL);

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
	struct mcc_symbol_table_row *int_array_row =
	    mcc_symbol_table_new_row_array("i", 42, MCC_SYMBOL_TABLE_ROW_TYPE_INT, NULL);

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
	struct mcc_symbol_table_row *row_int =
	    mcc_symbol_table_new_row_variable("i", MCC_SYMBOL_TABLE_ROW_TYPE_INT, NULL);
	struct mcc_symbol_table_row *row_bool =
	    mcc_symbol_table_new_row_variable("j", MCC_SYMBOL_TABLE_ROW_TYPE_BOOL, NULL);
	struct mcc_symbol_table_row *row_float =
	    mcc_symbol_table_new_row_variable("k", MCC_SYMBOL_TABLE_ROW_TYPE_FLOAT, NULL);

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

	CuAssertTrue(tc, scope->head->scope == scope);
	CuAssertTrue(tc, scope->head->child_scope->head->scope == scope->head->child_scope);
	CuAssertTrue(tc, scope->head->child_scope->head->next_row->scope == scope->head->child_scope);

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
	CuAssertTrue(tc, row->row_structure == MCC_SYMBOL_TABLE_ROW_STRUCTURE_FUNCTION);
	CuAssertTrue(tc, row->row_type == MCC_SYMBOL_TABLE_ROW_TYPE_FLOAT);
	CuAssertTrue(tc, row->array_size == -1);

	CuAssertStrEquals(tc, "func2", row->next_row->name);
	CuAssertTrue(tc, row->next_row->row_structure == MCC_SYMBOL_TABLE_ROW_STRUCTURE_FUNCTION);
	CuAssertTrue(tc, row->next_row->row_type == MCC_SYMBOL_TABLE_ROW_TYPE_BOOL);
	CuAssertTrue(tc, row->next_row->array_size == -1);

	CuAssertStrEquals(tc, "main", row->next_row->next_row->name);
	CuAssertTrue(tc, row->next_row->next_row->row_structure == MCC_SYMBOL_TABLE_ROW_STRUCTURE_FUNCTION);
	CuAssertTrue(tc, row->next_row->next_row->row_type == MCC_SYMBOL_TABLE_ROW_TYPE_INT);
	CuAssertTrue(tc, row->next_row->next_row->array_size == -1);

	CuAssertStrEquals(tc, "func3", row->next_row->next_row->next_row->name);
	CuAssertTrue(tc, row->next_row->next_row->next_row->row_structure == MCC_SYMBOL_TABLE_ROW_STRUCTURE_FUNCTION);
	CuAssertTrue(tc, row->next_row->next_row->next_row->row_type == MCC_SYMBOL_TABLE_ROW_TYPE_STRING);
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

void function_parameters_from_parser(CuTest *tc)
{
	// Define test input and create symbol table
	const char input[] = "int test(int[52] a, bool test){return;}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);

	// check that no other functions and thus no sibling scopes exist
	CuAssertPtrEquals(tc, NULL, table->head->parent_row);
	CuAssertPtrEquals(tc, NULL, table->head->next_scope);
	CuAssertPtrEquals(tc, NULL, table->head->prev_scope);

	// "test": table->head->head
	CuAssertIntEquals(tc, table->head->head->row_type, MCC_SYMBOL_TABLE_ROW_TYPE_INT);
	CuAssertIntEquals(tc, table->head->head->row_structure, MCC_SYMBOL_TABLE_ROW_STRUCTURE_FUNCTION);
	CuAssertIntEquals(tc, table->head->head->array_size, -1);
	CuAssertStrEquals(tc, table->head->head->name, "test");
	CuAssertPtrEquals(tc, NULL, table->head->head->prev_row);

	// "int[52] a": table->head->head->child_scope
	CuAssertPtrEquals(tc, NULL, table->head->head->child_scope->next_scope);
	CuAssertPtrEquals(tc, NULL, table->head->head->child_scope->prev_scope);
	CuAssertPtrEquals(tc, table->head->head, table->head->head->child_scope->parent_row);
	// "int[52] a": table->head->head->child_scope->head
	CuAssertIntEquals(tc, table->head->head->child_scope->head->row_type, MCC_SYMBOL_TABLE_ROW_TYPE_INT);
	CuAssertIntEquals(tc, table->head->head->child_scope->head->row_structure,
	                  MCC_SYMBOL_TABLE_ROW_STRUCTURE_ARRAY);
	CuAssertIntEquals(tc, table->head->head->child_scope->head->array_size, 52);
	CuAssertStrEquals(tc, table->head->head->child_scope->head->name, "a");
	CuAssertPtrEquals(tc, NULL, table->head->head->child_scope->head->prev_row);
	CuAssertPtrEquals(tc, NULL, table->head->head->child_scope->head->child_scope);

	// "bool test": table->head->head->child_scope->head->next_row
	CuAssertIntEquals(tc, table->head->head->child_scope->head->next_row->row_type, MCC_SYMBOL_TABLE_ROW_TYPE_BOOL);
	CuAssertIntEquals(tc, table->head->head->child_scope->head->next_row->row_structure,
	                  MCC_SYMBOL_TABLE_ROW_STRUCTURE_VARIABLE);
	CuAssertIntEquals(tc, table->head->head->child_scope->head->next_row->array_size, -1);
	CuAssertStrEquals(tc, table->head->head->child_scope->head->next_row->name, "test");
	CuAssertPtrEquals(tc, table->head->head->child_scope->head->next_row->prev_row,
	                  table->head->head->child_scope->head);
	CuAssertPtrEquals(tc, NULL, table->head->head->child_scope->head->next_row->next_row);
	CuAssertPtrEquals(tc, NULL, table->head->head->child_scope->head->child_scope);
	CuAssertPtrEquals(tc, table->head->head->child_scope->head,
	                  table->head->head->child_scope->head->next_row->prev_row);

	// Cleanup
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
}

void pseudo_row(CuTest *tc)
{
	// Define test input and create symbol table
	const char input[] = "int test(){{int a;}}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);

	// check that no other functions and thus no sibling scopes exist
	CuAssertPtrEquals(tc, NULL, table->head->parent_row);
	CuAssertPtrEquals(tc, NULL, table->head->next_scope);
	CuAssertPtrEquals(tc, NULL, table->head->prev_scope);

	// "test": table->head->head
	CuAssertIntEquals(tc, table->head->head->row_type, MCC_SYMBOL_TABLE_ROW_TYPE_INT);
	CuAssertIntEquals(tc, table->head->head->row_structure, MCC_SYMBOL_TABLE_ROW_STRUCTURE_FUNCTION);
	CuAssertIntEquals(tc, table->head->head->array_size, -1);
	CuAssertStrEquals(tc, table->head->head->name, "test");
	CuAssertPtrEquals(tc, NULL, table->head->head->prev_row);

	// pseudo_row: table->head->head->child_scope
	CuAssertPtrEquals(tc, table->head->head, table->head->head->child_scope->parent_row);
	CuAssertPtrEquals(tc, NULL, table->head->head->child_scope->next_scope);
	CuAssertPtrEquals(tc, NULL, table->head->head->child_scope->prev_scope);

	// pseudo_row: table->head->head->child_scope->head
	CuAssertIntEquals(tc, table->head->head->child_scope->head->row_type, MCC_SYMBOL_TABLE_ROW_TYPE_PSEUDO);
	CuAssertIntEquals(tc, table->head->head->child_scope->head->row_structure,
	                  MCC_SYMBOL_TABLE_ROW_STRUCTURE_VARIABLE);
	CuAssertIntEquals(tc, table->head->head->child_scope->head->array_size, -1);
	CuAssertStrEquals(tc, table->head->head->child_scope->head->name, "-");
	CuAssertPtrEquals(tc, NULL, table->head->head->child_scope->head->prev_row);
	CuAssertPtrEquals(tc, NULL, table->head->head->child_scope->head->next_row);

	// "int a": table->head->head->child_scope->head->child_scope
	CuAssertPtrEquals(tc, table->head->head->child_scope->head,
	                  table->head->head->child_scope->head->child_scope->parent_row);
	CuAssertPtrEquals(tc, NULL, table->head->head->child_scope->head->child_scope->next_scope);
	CuAssertPtrEquals(tc, NULL, table->head->head->child_scope->head->child_scope->prev_scope);

	// "int a": table->head->head->child_scope->head->child_scope->head
	CuAssertIntEquals(tc, table->head->head->child_scope->head->child_scope->head->row_type,
	                  MCC_SYMBOL_TABLE_ROW_TYPE_INT);
	CuAssertIntEquals(tc, table->head->head->child_scope->head->child_scope->head->row_structure,
	                  MCC_SYMBOL_TABLE_ROW_STRUCTURE_VARIABLE);
	CuAssertIntEquals(tc, table->head->head->child_scope->head->child_scope->head->array_size, -1);
	CuAssertStrEquals(tc, table->head->head->child_scope->head->child_scope->head->name, "a");
	CuAssertPtrEquals(tc, NULL, table->head->head->child_scope->head->child_scope->head->prev_row);
	CuAssertPtrEquals(tc, NULL, table->head->head->child_scope->head->child_scope->head->next_row);
	CuAssertPtrEquals(tc, NULL, table->head->head->child_scope->head->child_scope->head->child_scope);

	// Cleanup
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
}

void nested_statement(CuTest *tc)
{
	// Define test input and create symbol table
	const char input[] = "int test(){int a;{int b;int c;}}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);

	// check that no other functions and thus no sibling scopes exist
	CuAssertPtrEquals(tc, NULL, table->head->parent_row);
	CuAssertPtrEquals(tc, NULL, table->head->next_scope);
	CuAssertPtrEquals(tc, NULL, table->head->prev_scope);

	// "test": table->head->head
	CuAssertIntEquals(tc, table->head->head->row_type, MCC_SYMBOL_TABLE_ROW_TYPE_INT);
	CuAssertIntEquals(tc, table->head->head->row_structure, MCC_SYMBOL_TABLE_ROW_STRUCTURE_FUNCTION);
	CuAssertIntEquals(tc, table->head->head->array_size, -1);
	CuAssertStrEquals(tc, table->head->head->name, "test");
	CuAssertPtrEquals(tc, NULL, table->head->head->prev_row);

	// "int a": table->head->head->child_scope
	CuAssertPtrEquals(tc, NULL, table->head->head->child_scope->next_scope);
	CuAssertPtrEquals(tc, NULL, table->head->head->child_scope->prev_scope);
	CuAssertPtrEquals(tc, table->head->head, table->head->head->child_scope->parent_row);
	// "int a": table->head->head->child_scope->head
	CuAssertIntEquals(tc, table->head->head->child_scope->head->row_type, MCC_SYMBOL_TABLE_ROW_TYPE_INT);
	CuAssertIntEquals(tc, table->head->head->child_scope->head->row_structure,
	                  MCC_SYMBOL_TABLE_ROW_STRUCTURE_VARIABLE);
	CuAssertIntEquals(tc, table->head->head->child_scope->head->array_size, -1);
	CuAssertStrEquals(tc, table->head->head->child_scope->head->name, "a");
	CuAssertPtrEquals(tc, NULL, table->head->head->child_scope->head->prev_row);
	CuAssertPtrEquals(tc, NULL, table->head->head->child_scope->head->next_row);

	// "int b": table->head->head->child_scope->head->child_scope
	CuAssertPtrEquals(tc, NULL, table->head->head->child_scope->head->child_scope->next_scope);
	CuAssertPtrEquals(tc, NULL, table->head->head->child_scope->head->child_scope->prev_scope);
	CuAssertPtrEquals(tc, table->head->head->child_scope->head,
	                  table->head->head->child_scope->head->child_scope->parent_row);
	// "int b": table->head->head->child_scope->head->child_scope->head
	CuAssertIntEquals(tc, table->head->head->child_scope->head->child_scope->head->row_type,
	                  MCC_SYMBOL_TABLE_ROW_TYPE_INT);
	CuAssertIntEquals(tc, table->head->head->child_scope->head->child_scope->head->row_structure,
	                  MCC_SYMBOL_TABLE_ROW_STRUCTURE_VARIABLE);
	CuAssertIntEquals(tc, table->head->head->child_scope->head->child_scope->head->array_size, -1);
	CuAssertStrEquals(tc, table->head->head->child_scope->head->child_scope->head->name, "b");
	CuAssertPtrEquals(tc, NULL, table->head->head->child_scope->head->child_scope->head->prev_row);
	CuAssertPtrEquals(tc, NULL, table->head->head->child_scope->head->child_scope->head->child_scope);

	// "int c": table->head->head->child_scope->head->child_scope->next_row
	CuAssertIntEquals(tc, table->head->head->child_scope->head->child_scope->head->next_row->row_type,
	                  MCC_SYMBOL_TABLE_ROW_TYPE_INT);
	CuAssertIntEquals(tc, table->head->head->child_scope->head->child_scope->head->next_row->row_structure,
	                  MCC_SYMBOL_TABLE_ROW_STRUCTURE_VARIABLE);
	CuAssertIntEquals(tc, table->head->head->child_scope->head->child_scope->head->next_row->array_size, -1);
	CuAssertStrEquals(tc, table->head->head->child_scope->head->child_scope->head->next_row->name, "c");
	CuAssertPtrEquals(tc, table->head->head->child_scope->head->child_scope->head,
	                  table->head->head->child_scope->head->child_scope->head->next_row->prev_row);
	CuAssertPtrEquals(tc, NULL, table->head->head->child_scope->head->child_scope->head->next_row->next_row);
	CuAssertPtrEquals(tc, NULL, table->head->head->child_scope->head->child_scope->head->next_row->child_scope);

	// Cleanup
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
}

void multiple_functions(CuTest *tc)
{

	// Define test input and create symbol table
	const char input[] = "int test(){return;} void main(int a){a=1;}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);

	// symbol table: no previous or next scopes, no parent row
	CuAssertPtrEquals(tc, NULL, table->head->parent_row);
	CuAssertPtrEquals(tc, NULL, table->head->next_scope);
	CuAssertPtrEquals(tc, NULL, table->head->prev_scope);

	// "test": table->head->head
	CuAssertIntEquals(tc, table->head->head->row_type, MCC_SYMBOL_TABLE_ROW_TYPE_INT);
	CuAssertIntEquals(tc, table->head->head->row_structure, MCC_SYMBOL_TABLE_ROW_STRUCTURE_FUNCTION);
	CuAssertIntEquals(tc, table->head->head->array_size, -1);
	CuAssertStrEquals(tc, table->head->head->name, "test");
	CuAssertPtrEquals(tc, NULL, table->head->head->prev_row);

	// "return"
	CuAssertPtrEquals(tc, NULL, table->head->head->child_scope->head);
	CuAssertPtrEquals(tc, NULL, table->head->head->child_scope->next_scope);
	CuAssertPtrEquals(tc, NULL, table->head->head->child_scope->prev_scope);
	CuAssertPtrEquals(tc, table->head->head, table->head->head->child_scope->parent_row);

	// "main": table->head->head->next_row;
	CuAssertIntEquals(tc, table->head->head->next_row->row_type, MCC_SYMBOL_TABLE_ROW_TYPE_VOID);
	CuAssertIntEquals(tc, table->head->head->next_row->row_structure, MCC_SYMBOL_TABLE_ROW_STRUCTURE_FUNCTION);
	CuAssertIntEquals(tc, table->head->head->next_row->array_size, -1);
	CuAssertStrEquals(tc, table->head->head->next_row->name, "main");
	CuAssertPtrEquals(tc, table->head->head, table->head->head->next_row->prev_row);

	// "int a" : table->head->head->next_row->child_scope
	CuAssertPtrEquals(tc, NULL, table->head->head->next_row->child_scope->next_scope);
	CuAssertPtrEquals(tc, NULL, table->head->head->next_row->child_scope->prev_scope);
	CuAssertPtrEquals(tc, table->head->head->next_row, table->head->head->next_row->child_scope->parent_row);

	// "int a" : table->head->head->next_row->child_scope->head
	CuAssertIntEquals(tc, table->head->head->next_row->child_scope->head->row_type, MCC_SYMBOL_TABLE_ROW_TYPE_INT);
	CuAssertIntEquals(tc, table->head->head->next_row->child_scope->head->row_structure,
	                  MCC_SYMBOL_TABLE_ROW_STRUCTURE_VARIABLE);
	CuAssertIntEquals(tc, table->head->head->next_row->child_scope->head->array_size, -1);
	CuAssertStrEquals(tc, table->head->head->next_row->child_scope->head->name, "a");
	CuAssertPtrEquals(tc, NULL, table->head->head->next_row->child_scope->head->prev_row);
	CuAssertPtrEquals(tc, NULL, table->head->head->next_row->child_scope->head->next_row);
	CuAssertPtrEquals(tc, NULL, table->head->head->next_row->child_scope->head->child_scope);

	// Cleanup
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
}

void assignment_linking(CuTest *tc)
{
	const char input[] = "int func(){int n;if(true){n=n+1;}}";
	struct mcc_parser_result parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);

	struct mcc_symbol_table *table = mcc_symbol_table_create(parser_result.program);

	struct mcc_symbol_table_row *row = table->head->head->child_scope->head->child_scope->head;

	struct mcc_ast_program *program = parser_result.program;
	struct mcc_ast_statement *statement = program->function->compound_stmt->next_compound_statement->statement;
	struct mcc_ast_assignment *assignment = statement->if_on_true->compound_statement->statement->assignment;

	CuAssertTrue(tc, assignment->row == row);

	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
}

void check_upward(CuTest *tc)
{
	const char input[] = "int func(){int a;if(true){a=a+1;int b;}int c;float d;}";
	struct mcc_parser_result parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);

	struct mcc_symbol_table *table = mcc_symbol_table_create(parser_result.program);

	struct mcc_symbol_table_row *row_a = table->head->head->child_scope->head;
	struct mcc_symbol_table_row *row = table->head->head->child_scope->head->child_scope->head->next_row;

	CuAssertTrue(tc, row_a == mcc_symbol_table_check_upwards_for_declaration("a", row));

	row = table->head->head->child_scope->head->next_row->next_row;

	CuAssertTrue(tc, NULL == mcc_symbol_table_check_upwards_for_declaration("b", row));
	CuAssertTrue(tc, row_a == mcc_symbol_table_check_upwards_for_declaration("a", row));

	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
}

void check_upward_same_scope(CuTest *tc)
{

	// Define test input and create symbol table
	const char input[] = "int func(){int a; int b; int c; a=a+1;}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);

	// symbol table: no previous or next scopes, no parent row
	CuAssertPtrEquals(tc, NULL, table->head->parent_row);
	CuAssertPtrEquals(tc, NULL, table->head->next_scope);
	CuAssertPtrEquals(tc, NULL, table->head->prev_scope);

	struct mcc_symbol_table_row *row_a = table->head->head->child_scope->head;
	struct mcc_symbol_table_row *row = table->head->head->child_scope->head->next_row->next_row;

	CuAssertTrue(tc, row_a == mcc_symbol_table_check_upwards_for_declaration("a", row));

	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
}

void variable_expression_linking(CuTest *tc)
{

	// Define test input and create symbol table
	const char input[] = "int func(){int a; a=a+1;}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);

	struct mcc_symbol_table_row *row = table->head->head->child_scope->head;
	struct mcc_ast_compound_statement *stmt =
	    (&parser_result)->program->function->compound_stmt->next_compound_statement;
	struct mcc_ast_expression *expr = stmt->statement->assignment->variable_assigned_value->lhs;

	CuAssertPtrNotNull(tc, row);
	CuAssertPtrNotNull(tc, expr);
	CuAssertIntEquals(tc, expr->type, MCC_AST_EXPRESSION_TYPE_VARIABLE);
	CuAssertPtrEquals(tc, expr->variable_row, row);

	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
}

void if_condition_expression(CuTest *tc)
{

	// Define test input and create symbol table
	const char input[] = "int func(){int a; if(a==6){}}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);

	struct mcc_symbol_table_row *row = table->head->head->child_scope->head;
	struct mcc_ast_compound_statement *stmt =
	    (&parser_result)->program->function->compound_stmt->next_compound_statement;
	struct mcc_ast_expression *expr = stmt->statement->if_condition->lhs;

	CuAssertPtrNotNull(tc, row);
	CuAssertPtrNotNull(tc, expr);
	CuAssertIntEquals(tc, expr->type, MCC_AST_EXPRESSION_TYPE_VARIABLE);
	CuAssertPtrEquals(tc, expr->variable_row, row);

	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
}

void built_ins(CuTest *tc)
{

	// Define test input and create symbol table
	const char input[] = "int test(){return;}";
	struct mcc_parser_result parser_result;
	parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
	CuAssertIntEquals(tc, parser_result.status, MCC_PARSER_STATUS_OK);
	struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);

	// symbol table: no previous or next scopes, no parent row
	CuAssertPtrEquals(tc, NULL, table->head->parent_row);
	CuAssertPtrEquals(tc, NULL, table->head->next_scope);
	CuAssertPtrEquals(tc, NULL, table->head->prev_scope);

	// "test": table->head->head
	CuAssertIntEquals(tc, table->head->head->row_type, MCC_SYMBOL_TABLE_ROW_TYPE_INT);
	CuAssertIntEquals(tc, table->head->head->row_structure, MCC_SYMBOL_TABLE_ROW_STRUCTURE_FUNCTION);
	CuAssertIntEquals(tc, table->head->head->array_size, -1);
	CuAssertStrEquals(tc, table->head->head->name, "test");
	CuAssertPtrEquals(tc, NULL, table->head->head->prev_row);

	// "return"
	CuAssertPtrEquals(tc, NULL, table->head->head->child_scope->head);
	CuAssertPtrEquals(tc, NULL, table->head->head->child_scope->next_scope);
	CuAssertPtrEquals(tc, NULL, table->head->head->child_scope->prev_scope);
	CuAssertPtrEquals(tc, table->head->head, table->head->head->child_scope->parent_row);

	// -------------------------------------------------------------------------------- First built_in: print(string
	// a) "print": table->head->head->next_row;
	CuAssertIntEquals(tc, table->head->head->next_row->row_type, MCC_SYMBOL_TABLE_ROW_TYPE_VOID);
	CuAssertIntEquals(tc, table->head->head->next_row->row_structure, MCC_SYMBOL_TABLE_ROW_STRUCTURE_FUNCTION);
	CuAssertIntEquals(tc, table->head->head->next_row->array_size, -1);
	CuAssertStrEquals(tc, table->head->head->next_row->name, "print");
	CuAssertPtrEquals(tc, table->head->head, table->head->head->next_row->prev_row);

	// "string a" : table->head->head->next_row->child_scope
	CuAssertPtrEquals(tc, NULL, table->head->head->next_row->child_scope->next_scope);
	CuAssertPtrEquals(tc, NULL, table->head->head->next_row->child_scope->prev_scope);
	CuAssertPtrEquals(tc, table->head->head->next_row, table->head->head->next_row->child_scope->parent_row);

	// "string a" : table->head->head->next_row->child_scope->head
	CuAssertIntEquals(tc, table->head->head->next_row->child_scope->head->row_type,
	                  MCC_SYMBOL_TABLE_ROW_TYPE_STRING);
	CuAssertIntEquals(tc, table->head->head->next_row->child_scope->head->row_structure,
	                  MCC_SYMBOL_TABLE_ROW_STRUCTURE_VARIABLE);
	CuAssertIntEquals(tc, table->head->head->next_row->child_scope->head->array_size, -1);
	CuAssertStrEquals(tc, table->head->head->next_row->child_scope->head->name, "a");
	CuAssertPtrEquals(tc, NULL, table->head->head->next_row->child_scope->head->prev_row);
	CuAssertPtrEquals(tc, NULL, table->head->head->next_row->child_scope->head->next_row);
	CuAssertPtrEquals(tc, NULL, table->head->head->next_row->child_scope->head->child_scope);

	// -------------------------------------------------------------------------------- Second built_in: print_nl()
	// "print": table->head->head->next_row->next_row;
	CuAssertIntEquals(tc, table->head->head->next_row->next_row->row_type, MCC_SYMBOL_TABLE_ROW_TYPE_VOID);
	CuAssertIntEquals(tc, table->head->head->next_row->next_row->row_structure,
	                  MCC_SYMBOL_TABLE_ROW_STRUCTURE_FUNCTION);
	CuAssertIntEquals(tc, table->head->head->next_row->next_row->array_size, -1);
	CuAssertStrEquals(tc, table->head->head->next_row->next_row->name, "print_nl");
	CuAssertPtrEquals(tc, table->head->head->next_row, table->head->head->next_row->next_row->prev_row);

	// "()" : table->head->head->next_row->next_row->child_scope
	CuAssertPtrEquals(tc, NULL, table->head->head->next_row->next_row->child_scope->next_scope);
	CuAssertPtrEquals(tc, NULL, table->head->head->next_row->next_row->child_scope->prev_scope);
	CuAssertPtrEquals(tc, table->head->head->next_row->next_row,
	                  table->head->head->next_row->next_row->child_scope->parent_row);

	// "()" : table->head->head->next_row->next_row->child_scope->head
	CuAssertPtrEquals(tc, NULL, table->head->head->next_row->next_row->child_scope->head);

	// -------------------------------------------------------------------------------- Third built_in:
	// print_int(int a) "print_int": table->head->head->next_row->next_row->next_row;
	CuAssertIntEquals(tc, table->head->head->next_row->next_row->next_row->row_type,
	                  MCC_SYMBOL_TABLE_ROW_TYPE_VOID);
	CuAssertIntEquals(tc, table->head->head->next_row->next_row->next_row->row_structure,
	                  MCC_SYMBOL_TABLE_ROW_STRUCTURE_FUNCTION);
	CuAssertIntEquals(tc, table->head->head->next_row->next_row->next_row->array_size, -1);
	CuAssertStrEquals(tc, table->head->head->next_row->next_row->next_row->name, "print_int");
	CuAssertPtrEquals(tc, table->head->head->next_row->next_row,
	                  table->head->head->next_row->next_row->next_row->prev_row);

	// "int a" : table->head->head->next_row->next_row->next_row->child_scope
	CuAssertPtrEquals(tc, NULL, table->head->head->next_row->next_row->next_row->child_scope->next_scope);
	CuAssertPtrEquals(tc, NULL, table->head->head->next_row->next_row->next_row->child_scope->prev_scope);
	CuAssertPtrEquals(tc, table->head->head->next_row->next_row->next_row,
	                  table->head->head->next_row->next_row->next_row->child_scope->parent_row);

	// "int a" : table->head->head->next_row->next_row->next_row->child_scope->head
	CuAssertIntEquals(tc, table->head->head->next_row->next_row->next_row->child_scope->head->row_type,
	                  MCC_SYMBOL_TABLE_ROW_TYPE_INT);
	CuAssertIntEquals(tc, table->head->head->next_row->next_row->next_row->child_scope->head->row_structure,
	                  MCC_SYMBOL_TABLE_ROW_STRUCTURE_VARIABLE);
	CuAssertIntEquals(tc, table->head->head->next_row->next_row->next_row->child_scope->head->array_size, -1);
	CuAssertStrEquals(tc, table->head->head->next_row->next_row->next_row->child_scope->head->name, "a");
	CuAssertPtrEquals(tc, NULL, table->head->head->next_row->next_row->next_row->child_scope->head->prev_row);
	CuAssertPtrEquals(tc, NULL, table->head->head->next_row->next_row->next_row->child_scope->head->next_row);
	CuAssertPtrEquals(tc, NULL, table->head->head->next_row->next_row->next_row->child_scope->head->child_scope);

	// -------------------------------------------------------------------------- Fourth built_in: print_float(float
	// a) "print_float": table->head->head->next_row->next_row->next_row->next_row;
	CuAssertIntEquals(tc, table->head->head->next_row->next_row->next_row->next_row->row_type,
	                  MCC_SYMBOL_TABLE_ROW_TYPE_VOID);
	CuAssertIntEquals(tc, table->head->head->next_row->next_row->next_row->next_row->row_structure,
	                  MCC_SYMBOL_TABLE_ROW_STRUCTURE_FUNCTION);
	CuAssertIntEquals(tc, table->head->head->next_row->next_row->next_row->next_row->array_size, -1);
	CuAssertStrEquals(tc, table->head->head->next_row->next_row->next_row->next_row->name, "print_float");
	CuAssertPtrEquals(tc, table->head->head->next_row->next_row->next_row,
	                  table->head->head->next_row->next_row->next_row->next_row->prev_row);

	// "float a" : table->head->head->next_row->next_row->next_row->next_row->child_scope
	CuAssertPtrEquals(tc, NULL, table->head->head->next_row->next_row->next_row->next_row->child_scope->next_scope);
	CuAssertPtrEquals(tc, NULL, table->head->head->next_row->next_row->next_row->next_row->child_scope->prev_scope);
	CuAssertPtrEquals(tc, table->head->head->next_row->next_row->next_row->next_row,
	                  table->head->head->next_row->next_row->next_row->next_row->child_scope->parent_row);

	// "float a" : table->head->head->next_row->next_row->next_row->next_row->child_scope->head
	CuAssertIntEquals(tc, table->head->head->next_row->next_row->next_row->next_row->child_scope->head->row_type,
	                  MCC_SYMBOL_TABLE_ROW_TYPE_FLOAT);
	CuAssertIntEquals(tc,
	                  table->head->head->next_row->next_row->next_row->next_row->child_scope->head->row_structure,
	                  MCC_SYMBOL_TABLE_ROW_STRUCTURE_VARIABLE);
	CuAssertIntEquals(tc, table->head->head->next_row->next_row->next_row->next_row->child_scope->head->array_size,
	                  -1);
	CuAssertStrEquals(tc, table->head->head->next_row->next_row->next_row->next_row->child_scope->head->name, "a");
	CuAssertPtrEquals(tc, NULL,
	                  table->head->head->next_row->next_row->next_row->next_row->child_scope->head->prev_row);
	CuAssertPtrEquals(tc, NULL,
	                  table->head->head->next_row->next_row->next_row->next_row->child_scope->head->next_row);
	CuAssertPtrEquals(tc, NULL,
	                  table->head->head->next_row->next_row->next_row->next_row->child_scope->head->child_scope);

	// -------------------------------------------------------------------------- Fifth built_in: read_int()
	// "read_int": table->head->head->next_row->next_row->next_row->next_row->next_row;
	CuAssertIntEquals(tc, table->head->head->next_row->next_row->next_row->next_row->next_row->row_type,
	                  MCC_SYMBOL_TABLE_ROW_TYPE_INT);
	CuAssertIntEquals(tc, table->head->head->next_row->next_row->next_row->next_row->next_row->row_structure,
	                  MCC_SYMBOL_TABLE_ROW_STRUCTURE_FUNCTION);
	CuAssertIntEquals(tc, table->head->head->next_row->next_row->next_row->next_row->next_row->array_size, -1);
	CuAssertStrEquals(tc, table->head->head->next_row->next_row->next_row->next_row->next_row->name, "read_int");
	CuAssertPtrEquals(tc, table->head->head->next_row->next_row->next_row->next_row,
	                  table->head->head->next_row->next_row->next_row->next_row->next_row->prev_row);

	// "()" : table->head->head->next_row->next_row->next_row->next_row->next_row->child_scope
	CuAssertPtrEquals(tc, NULL,
	                  table->head->head->next_row->next_row->next_row->next_row->next_row->child_scope->next_scope);
	CuAssertPtrEquals(tc, NULL,
	                  table->head->head->next_row->next_row->next_row->next_row->next_row->child_scope->prev_scope);
	CuAssertPtrEquals(tc, table->head->head->next_row->next_row->next_row->next_row->next_row,
	                  table->head->head->next_row->next_row->next_row->next_row->next_row->child_scope->parent_row);

	// -------------------------------------------------------------------------- Sixth built_in: read_float()
	// "read_float": table->head->head->next_row->next_row->next_row->next_row->next_row->next_row;
	CuAssertIntEquals(tc, table->head->head->next_row->next_row->next_row->next_row->next_row->next_row->row_type,
	                  MCC_SYMBOL_TABLE_ROW_TYPE_FLOAT);
	CuAssertIntEquals(tc,
	                  table->head->head->next_row->next_row->next_row->next_row->next_row->next_row->row_structure,
	                  MCC_SYMBOL_TABLE_ROW_STRUCTURE_FUNCTION);
	CuAssertIntEquals(tc, table->head->head->next_row->next_row->next_row->next_row->next_row->next_row->array_size,
	                  -1);
	CuAssertStrEquals(tc, table->head->head->next_row->next_row->next_row->next_row->next_row->next_row->name,
	                  "read_float");
	CuAssertPtrEquals(tc, table->head->head->next_row->next_row->next_row->next_row->next_row,
	                  table->head->head->next_row->next_row->next_row->next_row->next_row->next_row->prev_row);

	// "()" : table->head->head->next_row->next_row->next_row->next_row->next_row->next_row->child_scope
	CuAssertPtrEquals(
	    tc, NULL,
	    table->head->head->next_row->next_row->next_row->next_row->next_row->next_row->child_scope->next_scope);
	CuAssertPtrEquals(
	    tc, NULL,
	    table->head->head->next_row->next_row->next_row->next_row->next_row->next_row->child_scope->prev_scope);
	CuAssertPtrEquals(
	    tc, table->head->head->next_row->next_row->next_row->next_row->next_row->next_row,
	    table->head->head->next_row->next_row->next_row->next_row->next_row->next_row->child_scope->parent_row);

	// Cleanup
	mcc_ast_delete(parser_result.program);
	mcc_symbol_table_delete_table(table);
}

#define TESTS \
	TEST(empty_table) \
	TEST(multiple_rows) \
	TEST(scope_siblings) \
	TEST(nesting_scope) \
	TEST(array_row) \
	TEST(get_last) \
	TEST(function_parameters) \
	TEST(function_parameters2) \
	TEST(nested_if) \
	TEST(function_definition) \
	TEST(function_body) \
	TEST(empty_nested_function_body) \
	TEST(function_parameters_from_parser) \
	TEST(pseudo_row) \
	TEST(nested_statement) \
	TEST(multiple_functions) \
	TEST(assignment_linking) \
	TEST(check_upward) \
	TEST(check_upward_same_scope) \
	TEST(variable_expression_linking) \
	TEST(if_condition_expression) \
	TEST(built_ins)
#include "main_stub.inc"
#undef TESTS