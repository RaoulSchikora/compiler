//
// Created by oliver on 20.03.20.
//

#include <CuTest.h>

#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "mcc/ast.h"
#include "mcc/symbol_table.h"
#include "mcc/semantic_checks.h"

void positive(CuTest *tc)
{
    // Define test input and create symbol table
    const char input[] = "int main(){int a; a = 2; int b; int c; b = 2; c = a + b; return c;}";
    struct mcc_parser_result parser_result;
    parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
    CuAssertIntEquals(tc,parser_result.status,MCC_PARSER_STATUS_OK);
    struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
    struct mcc_semantic_check_all_checks *checks = mcc_semantic_check_run_all((&parser_result)->program,table);

    CuAssertIntEquals(tc,checks->status,MCC_SEMANTIC_CHECK_OK);
    CuAssertPtrEquals(tc,NULL, checks->error_buffer);

    CuAssertPtrNotNull(tc, checks);
    CuAssertPtrNotNull(tc,checks->type_check);
    CuAssertPtrNotNull(tc,checks->nonvoid_check);
    CuAssertPtrNotNull(tc,checks->main_function);
    CuAssertPtrNotNull(tc,checks->unknown_function_call);
    CuAssertPtrNotNull(tc,checks->multiple_function_definitions);
    CuAssertPtrNotNull(tc,checks->multiple_variable_declarations);
    CuAssertPtrNotNull(tc,checks->use_undeclared_variable);

    CuAssertIntEquals(tc,checks->type_check->type,MCC_SEMANTIC_CHECK_TYPE_CHECK);
    CuAssertIntEquals(tc,checks->nonvoid_check->type,MCC_SEMANTIC_CHECK_NONVOID_CHECK);
    CuAssertIntEquals(tc,checks->main_function->type,MCC_SEMANTIC_CHECK_MAIN_FUNCTION);
    CuAssertIntEquals(tc,checks->unknown_function_call->type,MCC_SEMANTIC_CHECK_UNKNOWN_FUNCTION_CALL);
    CuAssertIntEquals(tc,checks->multiple_function_definitions->type,MCC_SEMANTIC_CHECK_MULTIPLE_FUNCTION_DEFINITIONS);
    CuAssertIntEquals(tc,checks->multiple_variable_declarations->type,MCC_SEMANTIC_CHECK_MULTIPLE_VARIABLE_DECLARATIONS);
    CuAssertIntEquals(tc,checks->use_undeclared_variable->type,MCC_SEMANTIC_CHECK_USE_UNDECLARED_VARIABLE);

    CuAssertIntEquals(tc,checks->type_check->status,MCC_SEMANTIC_CHECK_OK);
    CuAssertIntEquals(tc,checks->nonvoid_check->status,MCC_SEMANTIC_CHECK_OK);
    CuAssertIntEquals(tc,checks->main_function->status,MCC_SEMANTIC_CHECK_OK);
    CuAssertIntEquals(tc,checks->unknown_function_call->status,MCC_SEMANTIC_CHECK_OK);
    CuAssertIntEquals(tc,checks->multiple_function_definitions->status,MCC_SEMANTIC_CHECK_OK);
    CuAssertIntEquals(tc,checks->multiple_variable_declarations->status,MCC_SEMANTIC_CHECK_OK);
    CuAssertIntEquals(tc,checks->use_undeclared_variable->status,MCC_SEMANTIC_CHECK_OK);

    // Cleanup
    mcc_ast_delete(parser_result.program);
    mcc_symbol_table_delete_table(table);
    mcc_semantic_check_delete_all_checks(checks);

}

// Invalid add with float and int
void type_check(CuTest *tc)
{
    // Define test input and create symbol table
    const char input[] = "int main(){int a; int b; float c; a = b + c;}";
    struct mcc_parser_result parser_result;
    parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
    CuAssertIntEquals(tc,parser_result.status,MCC_PARSER_STATUS_OK);
    struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
    struct mcc_semantic_check *check = mcc_semantic_check_run_type_check((&parser_result)->program,table);

    CuAssertPtrNotNull(tc, check->error_buffer);
    CuAssertPtrNotNull(tc, check);
    CuAssertIntEquals(tc,check->status,MCC_SEMANTIC_CHECK_FAIL);
    CuAssertIntEquals(tc,check->type,MCC_SEMANTIC_CHECK_TYPE_CHECK);

    // Cleanup
    mcc_ast_delete(parser_result.program);
    mcc_symbol_table_delete_table(table);
    mcc_semantic_check_delete_single_check(check);
}

// int-function returns without value
void nonvoid_check(CuTest *tc){

    // Define test input and create symbol table
    const char input[] = "int main(int a){if(a==2){return;} else {return 3;}}";
    struct mcc_parser_result parser_result;
    parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
    CuAssertIntEquals(tc,parser_result.status,MCC_PARSER_STATUS_OK);
    struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
    struct mcc_semantic_check *check = mcc_semantic_check_run_nonvoid_check((&parser_result)->program,table);

    CuAssertPtrNotNull(tc, check);
    CuAssertPtrNotNull(tc, check->error_buffer);
    CuAssertIntEquals(tc,check->status,MCC_SEMANTIC_CHECK_FAIL);
    CuAssertIntEquals(tc,check->type,MCC_SEMANTIC_CHECK_NONVOID_CHECK);

    // Cleanup
    mcc_ast_delete(parser_result.program);
    mcc_symbol_table_delete_table(table);
    mcc_semantic_check_delete_single_check(check);
}

// main has wrong signature
void main_function_1(CuTest *tc){

    // Define test input and create symbol table
    const char input[] = "int main(int a){if(a==2){return;} else {return 3;}}";
    struct mcc_parser_result parser_result;
    parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
    CuAssertIntEquals(tc,parser_result.status,MCC_PARSER_STATUS_OK);
    struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
    struct mcc_semantic_check *check = mcc_semantic_check_run_main_function((&parser_result)->program,table);

    CuAssertPtrNotNull(tc, check);
    CuAssertPtrNotNull(tc, check->error_buffer);
    CuAssertIntEquals(tc,check->status,MCC_SEMANTIC_CHECK_FAIL);
    CuAssertIntEquals(tc,check->type,MCC_SEMANTIC_CHECK_MAIN_FUNCTION);

    // Cleanup
    mcc_ast_delete(parser_result.program);
    mcc_symbol_table_delete_table(table);
    mcc_semantic_check_delete_single_check(check);
}

// 2 main-functions exist
void main_function_2(CuTest *tc){

    // Define test input and create symbol table
    const char input[] = "int main(){if(a==2){return;} else {return 3;}} int main(){return 1;}";
    struct mcc_parser_result parser_result;
    parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
    CuAssertIntEquals(tc,parser_result.status,MCC_PARSER_STATUS_OK);
    struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
    struct mcc_semantic_check *check = mcc_semantic_check_run_main_function((&parser_result)->program,table);

    CuAssertPtrNotNull(tc, check);
    CuAssertPtrNotNull(tc, check->error_buffer);
    CuAssertIntEquals(tc,check->status,MCC_SEMANTIC_CHECK_FAIL);
    CuAssertIntEquals(tc,check->type,MCC_SEMANTIC_CHECK_MAIN_FUNCTION);

    // Cleanup
    mcc_ast_delete(parser_result.program);
    mcc_symbol_table_delete_table(table);
    mcc_semantic_check_delete_single_check(check);
}

// No main function is defined
void main_function_3(CuTest *tc){

    // Define test input and create symbol table
    const char input[] = "int test(int a){if(a==2){return;} else {return 3;}}";
    struct mcc_parser_result parser_result;
    parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
    CuAssertIntEquals(tc,parser_result.status,MCC_PARSER_STATUS_OK);
    struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
    struct mcc_semantic_check *check = mcc_semantic_check_run_main_function((&parser_result)->program,table);

    CuAssertPtrNotNull(tc, check);
    CuAssertPtrNotNull(tc, check->error_buffer);
    CuAssertIntEquals(tc,check->status,MCC_SEMANTIC_CHECK_FAIL);
    CuAssertIntEquals(tc,check->type,MCC_SEMANTIC_CHECK_MAIN_FUNCTION);

    // Cleanup
    mcc_ast_delete(parser_result.program);
    mcc_symbol_table_delete_table(table);
    mcc_semantic_check_delete_single_check(check);
}

// An undefined function is called
void unknown_function_call(CuTest *tc){

    // Define test input and create symbol table
    const char input[] = "int main(){int a; a = 1; a = unknown_function(a);}";
    struct mcc_parser_result parser_result;
    parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
    CuAssertIntEquals(tc,parser_result.status,MCC_PARSER_STATUS_OK);
    struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
    struct mcc_semantic_check *check = mcc_semantic_check_run_unknown_function_call((&parser_result)->program,table);

    CuAssertPtrNotNull(tc, check);
    CuAssertPtrNotNull(tc, check->error_buffer);
    CuAssertIntEquals(tc,check->status,MCC_SEMANTIC_CHECK_FAIL);
    CuAssertIntEquals(tc,check->type,MCC_SEMANTIC_CHECK_UNKNOWN_FUNCTION_CALL);

    // Cleanup
    mcc_ast_delete(parser_result.program);
    mcc_symbol_table_delete_table(table);
    mcc_semantic_check_delete_single_check(check);

}

// A function is defined multiple times
void multiple_function_definitions(CuTest *tc){

    // Define test input and create symbol table
    const char input[] = "int test(int a){return a;} int test(int a){return a + 1 ;}";
    struct mcc_parser_result parser_result;
    parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
    CuAssertIntEquals(tc,parser_result.status,MCC_PARSER_STATUS_OK);
    struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
    struct mcc_semantic_check *check = mcc_semantic_check_run_multiple_function_definitions((&parser_result)->program,table);

    CuAssertPtrNotNull(tc, check);
    CuAssertPtrNotNull(tc, check->error_buffer);
    CuAssertIntEquals(tc,check->status,MCC_SEMANTIC_CHECK_FAIL);
    CuAssertIntEquals(tc,check->type,MCC_SEMANTIC_CHECK_MULTIPLE_FUNCTION_DEFINITIONS);

    // Cleanup
    mcc_ast_delete(parser_result.program);
    mcc_symbol_table_delete_table(table);
    mcc_semantic_check_delete_single_check(check);
}

// a function is defined multiple times within a couple of functions
void multiple_function_definitions2(CuTest *tc){

    // Define test input and create symbol table
    const char input[] = "int func1(){} int test(){} int func2(){} int func3(){} int test(){} int func4(){}";
    struct mcc_parser_result parser_result;
    parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
    CuAssertIntEquals(tc,parser_result.status,MCC_PARSER_STATUS_OK);
    struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
    struct mcc_semantic_check *check = mcc_semantic_check_run_multiple_function_definitions(
            (&parser_result)->program,table);

    CuAssertPtrNotNull(tc, check->error_buffer);
    CuAssertPtrNotNull(tc, check);
    CuAssertIntEquals(tc,check->status,MCC_SEMANTIC_CHECK_FAIL);
    CuAssertIntEquals(tc,check->type,MCC_SEMANTIC_CHECK_MULTIPLE_FUNCTION_DEFINITIONS);

    // Cleanup
    mcc_ast_delete(parser_result.program);
    mcc_symbol_table_delete_table(table);
    mcc_semantic_check_delete_single_check(check);
}

// A variable is declared more than once in the same scope
void multiple_variable_declarations(CuTest *tc){

    // Define test input and create symbol table
    const char input[] = "int main(){ int a; {int b; int b;}}";
    struct mcc_parser_result parser_result;
    parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
    CuAssertIntEquals(tc,parser_result.status,MCC_PARSER_STATUS_OK);
    struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
    struct mcc_semantic_check *check = mcc_semantic_check_run_multiple_variable_declarations((&parser_result)->program,table);

    CuAssertPtrNotNull(tc, check);
    CuAssertPtrNotNull(tc, check->error_buffer);
    CuAssertIntEquals(tc,check->status,MCC_SEMANTIC_CHECK_FAIL);
    CuAssertIntEquals(tc,check->type,MCC_SEMANTIC_CHECK_MULTIPLE_VARIABLE_DECLARATIONS);

    // Cleanup
    mcc_ast_delete(parser_result.program);
    mcc_symbol_table_delete_table(table);
    mcc_semantic_check_delete_single_check(check);

}

// An undeclared variable is used
void use_undeclared_variable(CuTest *tc){

    // Define test input and create symbol table
    const char input[] = "int main(){ int a; {int b; b = c;}}";
    struct mcc_parser_result parser_result;
    parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
    CuAssertIntEquals(tc,parser_result.status,MCC_PARSER_STATUS_OK);
    struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
    struct mcc_semantic_check *check = mcc_semantic_check_run_use_undeclared_variable((&parser_result)->program,table);

    CuAssertPtrNotNull(tc, check);
    CuAssertPtrNotNull(tc, check->error_buffer);
    CuAssertIntEquals(tc,check->status,MCC_SEMANTIC_CHECK_FAIL);
    CuAssertIntEquals(tc,check->type,MCC_SEMANTIC_CHECK_USE_UNDECLARED_VARIABLE);

    // Cleanup
    mcc_ast_delete(parser_result.program);
    mcc_symbol_table_delete_table(table);
    mcc_semantic_check_delete_single_check(check);
}

#define TESTS \
    TEST(main_function_1)                 \
    TEST(main_function_2)                 \
    TEST(main_function_3)
    //TEST(positive)                        \
    //TEST(type_check)                      \
    //TEST(nonvoid_check)                   \
    //TEST(unknown_function_call)           \
    //TEST(multiple_function_definitions)   \
    //TEST(multiple_function_definitions2)  \
    //TEST(multiple_variable_declarations)  \
    //TEST(use_undeclared_variable)
#include "main_stub.inc"
#undef TESTS
