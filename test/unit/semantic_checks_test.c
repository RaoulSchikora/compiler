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
    const char input[] = "int main(){int a; a = 2; int b; int c; b = 2; c = a + b; return c;} \
                         void func1(int a, bool[10] b){a = a + 2; int i; i = 0; b[i] = 0;} \
                         bool func2(){return true;} \
                         int func3(){if(true)if(true){while(false){return 2;}} return 3;}";
    struct mcc_parser_result parser_result;
    parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
    CuAssertIntEquals(tc,parser_result.status,MCC_PARSER_STATUS_OK);
    struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
    struct mcc_semantic_check_all_checks *checks = mcc_semantic_check_run_all((&parser_result)->program,table);

    if(checks->status == MCC_SEMANTIC_CHECK_FAIL){
        if (checks->error_buffer == NULL){
            printf("Semantic check failed, error buffer is empty.");
        } else {
            printf("Semantic check failed:\n%s", checks->error_buffer);
        }
    }

    CuAssertIntEquals(tc,checks->status,MCC_SEMANTIC_CHECK_OK);
    CuAssertPtrEquals(tc,NULL, checks->error_buffer);

    CuAssertPtrNotNull(tc, checks);
    //CuAssertPtrNotNull(tc,checks->type_conversion);
    //CuAssertPtrNotNull(tc,checks->array_types);
    //CuAssertPtrNotNull(tc,checks->function_arguments);
    //CuAssertPtrNotNull(tc,checks->nonvoid_check);
    CuAssertPtrNotNull(tc,checks->main_function);
    CuAssertPtrNotNull(tc,checks->unknown_function_call);
    CuAssertPtrNotNull(tc,checks->multiple_function_definitions);
    CuAssertPtrNotNull(tc,checks->multiple_variable_declarations);
    CuAssertPtrNotNull(tc,checks->use_undeclared_variable);
    CuAssertPtrNotNull(tc,checks->define_built_in);

    //CuAssertIntEquals(tc,checks->type_check->type,MCC_SEMANTIC_CHECK_TYPE_CONVERSION);
    //CuAssertIntEquals(tc,checks->type_check->type,MCC_SEMANTIC_CHECK_TYPE_ARRAY_TYPES);
    //CuAssertIntEquals(tc,checks->type_check->type,MCC_SEMANTIC_CHECK_TYPE_FUNCTION_ARGUMENTS);
    //CuAssertIntEquals(tc,checks->nonvoid_check->type,MCC_SEMANTIC_CHECK_NONVOID_CHECK);
    CuAssertIntEquals(tc,checks->main_function->type,MCC_SEMANTIC_CHECK_MAIN_FUNCTION);
    CuAssertIntEquals(tc,checks->unknown_function_call->type,MCC_SEMANTIC_CHECK_UNKNOWN_FUNCTION_CALL);
    CuAssertIntEquals(tc,checks->multiple_function_definitions->type,MCC_SEMANTIC_CHECK_MULTIPLE_FUNCTION_DEFINITIONS);
    CuAssertIntEquals(tc,checks->multiple_variable_declarations->type,MCC_SEMANTIC_CHECK_MULTIPLE_VARIABLE_DECLARATIONS);
    CuAssertIntEquals(tc,checks->use_undeclared_variable->type,MCC_SEMANTIC_CHECK_USE_UNDECLARED_VARIABLE);
    CuAssertIntEquals(tc,checks->define_built_in->type,MCC_SEMANTIC_CHECK_DEFINE_BUILT_IN);

    //CuAssertIntEquals(tc,checks->type_conversion->status,MCC_SEMANTIC_CHECK_OK);
    //CuAssertIntEquals(tc,checks->array_types->status,MCC_SEMANTIC_CHECK_OK);
    //CuAssertIntEquals(tc,checks->function_arguments->status,MCC_SEMANTIC_CHECK_OK);
    //CuAssertIntEquals(tc,checks->nonvoid_check->status,MCC_SEMANTIC_CHECK_OK);
    CuAssertIntEquals(tc,checks->main_function->status,MCC_SEMANTIC_CHECK_OK);
    CuAssertIntEquals(tc,checks->unknown_function_call->status,MCC_SEMANTIC_CHECK_OK);
    CuAssertIntEquals(tc,checks->multiple_function_definitions->status,MCC_SEMANTIC_CHECK_OK);
    CuAssertIntEquals(tc,checks->multiple_variable_declarations->status,MCC_SEMANTIC_CHECK_OK);
    CuAssertIntEquals(tc,checks->use_undeclared_variable->status,MCC_SEMANTIC_CHECK_OK);
    CuAssertIntEquals(tc,checks->define_built_in->status,MCC_SEMANTIC_CHECK_OK);

    // Cleanup
    mcc_ast_delete(parser_result.program);
    mcc_symbol_table_delete_table(table);
    mcc_semantic_check_delete_all_checks(checks);

}

// Invalid add with float and int
void type_conversion(CuTest *tc)
{
    // Define test input and create symbol table
    const char input[] = "int main(){int a; int b; float c; a = b + c;}";
    struct mcc_parser_result parser_result;
    parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
    CuAssertIntEquals(tc,parser_result.status,MCC_PARSER_STATUS_OK);
    struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
    struct mcc_semantic_check *check = mcc_semantic_check_run_type_conversion((&parser_result)->program,table);

    CuAssertPtrNotNull(tc, check->error_buffer);
    CuAssertPtrNotNull(tc, check);
    CuAssertIntEquals(tc,check->status,MCC_SEMANTIC_CHECK_FAIL);
    CuAssertIntEquals(tc,check->type,MCC_SEMANTIC_CHECK_TYPE_CONVERSION);

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

// a function is defined multiple times within a couple of functions at the end
void multiple_function_definitions3(CuTest *tc){

    // Define test input and create symbol table
    const char input[] = "int func1(){} int func2(){} int func3(){} int func4(){} int test(){} int test(){} ";
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

// A variable is declared more than once in the same scope
void multiple_variable_declarations2(CuTest *tc){

    // Define test input and create symbol table
    const char input[] = "int main(){ int a; {int b;} int c; int a;}";
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

// An undeclared variable is used in an if-condition
void use_undeclared_variable2(CuTest *tc){

    // Define test input and create symbol table
    const char input[] = "int main(){ int a; if(i == 0) return a;return a;}";
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

// An undeclared variable is used in an while-condition
void use_undeclared_variable3(CuTest *tc){

    // Define test input and create symbol table
    const char input[] = "int main(){ int a; while(i == 0) return a;return a;}";
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

// An undeclared variable is used in a return-statement
void use_undeclared_variable4(CuTest *tc){

    // Define test input and create symbol table
    const char input[] = "int main(){ int a; return i[10];}";
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

// An undeclared variable is used in an while-condition
void use_undeclared_variable5(CuTest *tc){

    // Define test input and create symbol table
    const char input[] = "int main(){ int a; b = a;}";
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

// An undeclared variable is used in an while-condition
void use_undeclared_variable6(CuTest *tc){

    // Define test input and create symbol table
    const char input[] = "int main(){ int a; b[1] = a;}";
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

// A function is declared that has the same name as one of the built ins
void define_built_in(CuTest *tc){

    // Define test input and create symbol table
    const char input[] = "int print_nl(){}";
    struct mcc_parser_result parser_result;
    parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
    CuAssertIntEquals(tc,parser_result.status,MCC_PARSER_STATUS_OK);
    struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
    struct mcc_semantic_check *check = mcc_semantic_check_run_define_built_in((&parser_result)->program,table);

    CuAssertPtrNotNull(tc, check);
    CuAssertIntEquals(tc,check->status,MCC_SEMANTIC_CHECK_FAIL);
    CuAssertPtrNotNull(tc, check->error_buffer);
    CuAssertIntEquals(tc,check->type,MCC_SEMANTIC_CHECK_DEFINE_BUILT_IN);

    // Cleanup
    mcc_ast_delete(parser_result.program);
    mcc_symbol_table_delete_table(table);
    mcc_semantic_check_delete_single_check(check);
}

// Using return value as the wrong type
void type_conversion1(CuTest *tc)
{
    // Define test input and create symbol table
    const char input[] = "int main(){int a; a = read_float();}";
    struct mcc_parser_result parser_result;
    parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
    CuAssertIntEquals(tc,parser_result.status,MCC_PARSER_STATUS_OK);
    struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
    struct mcc_semantic_check *check = mcc_semantic_check_run_type_conversion((&parser_result)->program,table);

    CuAssertPtrNotNull(tc, check->error_buffer);
    CuAssertPtrNotNull(tc, check);
    CuAssertIntEquals(tc,check->status,MCC_SEMANTIC_CHECK_FAIL);
    CuAssertIntEquals(tc,check->type,MCC_SEMANTIC_CHECK_TYPE_CONVERSION);

    // Cleanup
    mcc_ast_delete(parser_result.program);
    mcc_symbol_table_delete_table(table);
    mcc_semantic_check_delete_single_check(check);
}

// Using return value as the wrong type
void type_conversion2(CuTest *tc)
{
    // Define test input and create symbol table
    const char input[] = "int main(){bool a; a = bool_to_int(a);} int bool_to_int(bool a){int a; a = 1; return a;}";
    struct mcc_parser_result parser_result;
    parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
    CuAssertIntEquals(tc,parser_result.status,MCC_PARSER_STATUS_OK);
    struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
    struct mcc_semantic_check *check = mcc_semantic_check_run_type_conversion((&parser_result)->program,table);

    CuAssertPtrNotNull(tc, check->error_buffer);
    CuAssertPtrNotNull(tc, check);
    CuAssertIntEquals(tc,check->status,MCC_SEMANTIC_CHECK_FAIL);
    CuAssertIntEquals(tc,check->type,MCC_SEMANTIC_CHECK_TYPE_CONVERSION);

    // Cleanup
    mcc_ast_delete(parser_result.program);
    mcc_symbol_table_delete_table(table);
    mcc_semantic_check_delete_single_check(check);
}

// Calling a function with the wrong type of parameters
void function_arguments1(CuTest *tc)
{
    // Define test input and create symbol table
    const char input[] = "int main(){float a; a = 1.0; print_int(a);}";
    struct mcc_parser_result parser_result;
    parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
    CuAssertIntEquals(tc,parser_result.status,MCC_PARSER_STATUS_OK);
    struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
    struct mcc_semantic_check *check = mcc_semantic_check_run_function_arguments((&parser_result)->program,table);

    CuAssertPtrNotNull(tc, check->error_buffer);
    CuAssertPtrNotNull(tc, check);
    CuAssertIntEquals(tc,check->status,MCC_SEMANTIC_CHECK_FAIL);
    CuAssertIntEquals(tc,check->type,MCC_SEMANTIC_CHECK_FUNCTION_ARGUMENTS);

    // Cleanup
    mcc_ast_delete(parser_result.program);
    mcc_symbol_table_delete_table(table);
    mcc_semantic_check_delete_single_check(check);
}

// Calling a function with the wrong type of parameters
void function_arguments2(CuTest *tc)
{
    // Define test input and create symbol table
    const char input[] = "int main(){string a; a = \" teststring \"; call_me_with_int(a);} "
                         "void call_me_with_int(int a){return;}";
    struct mcc_parser_result parser_result;
    parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
    CuAssertIntEquals(tc,parser_result.status,MCC_PARSER_STATUS_OK);
    struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
    struct mcc_semantic_check *check = mcc_semantic_check_run_function_arguments((&parser_result)->program,table);

    CuAssertPtrNotNull(tc, check->error_buffer);
    CuAssertPtrNotNull(tc, check);
    CuAssertIntEquals(tc,check->status,MCC_SEMANTIC_CHECK_FAIL);
    CuAssertIntEquals(tc,check->type,MCC_SEMANTIC_CHECK_FUNCTION_ARGUMENTS);

    // Cleanup
    mcc_ast_delete(parser_result.program);
    mcc_symbol_table_delete_table(table);
    mcc_semantic_check_delete_single_check(check);
}

#define TESTS \
    TEST(positive)                        \
    TEST(main_function_1)                 \
    TEST(main_function_2)                 \
    TEST(main_function_3)                 \
    TEST(unknown_function_call)           \
    TEST(multiple_function_definitions)   \
    TEST(multiple_function_definitions2)  \
    TEST(multiple_function_definitions3)  \
    TEST(multiple_variable_declarations)  \
    TEST(multiple_variable_declarations2) \
    TEST(use_undeclared_variable)         \
    TEST(use_undeclared_variable2)        \
    TEST(use_undeclared_variable3)        \
    TEST(use_undeclared_variable4)        \
    TEST(use_undeclared_variable5)        \
    TEST(use_undeclared_variable6)        \
    TEST(define_built_in)
    //TEST(type_conversion)               \
    //TEST(type_conversion1)              \
    //TEST(type_conversion2)              \
    //TEST(funciton_arguments1)           \
    //TEST(function_arguments2)           \
    //TEST(nonvoid_check)                 \
    //TEST(use_undeclared_variable)
#include "main_stub.inc"
#undef TESTS
