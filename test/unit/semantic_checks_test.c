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

void test_1(CuTest *tc)
{
    // Define test input and create symbol table
    const char input[] = "int test(){{int a;}}";
    struct mcc_parser_result parser_result;
    parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
    CuAssertIntEquals(tc,parser_result.status,MCC_PARSER_STATUS_OK);
    struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
    struct mcc_semantic_check_all_checks *checks = mcc_semantic_check_run_all((&parser_result)->program,table);

    // TODO: Invoke a semantic check


    // Cleanup
    mcc_ast_delete(parser_result.program);
    mcc_symbol_table_delete_table(table);
    mcc_semantic_check_delete_all_checks(checks);

}

void test_2(CuTest *tc)
{
    // Define test input and create symbol table
    const char input[] = "int test(){{int a;}}";
    struct mcc_parser_result parser_result;
    parser_result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
    CuAssertIntEquals(tc,parser_result.status,MCC_PARSER_STATUS_OK);
    struct mcc_symbol_table *table = mcc_symbol_table_create((&parser_result)->program);
    struct mcc_semantic_check_all_checks *checks = mcc_semantic_check_run_all((&parser_result)->program,table);

    // TODO: Invoke a semantic check


    // Cleanup
    mcc_ast_delete(parser_result.program);
    mcc_symbol_table_delete_table(table);
    mcc_semantic_check_delete_all_checks(checks);

}

#define TESTS \
    TEST(test_1)         \
    TEST(test_2)
#include "main_stub.inc"
#undef TESTS
