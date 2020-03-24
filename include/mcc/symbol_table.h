//
// Created by raoul on 24.05.19.
//
// Symbol Table
//
// Here the data structure for the symbol table is defined. The symbol table is
// basically a tree of tables. Each table-node represents a scope.
//

#ifndef MCC_SYMBOL_TABLE_H
#define MCC_SYMBOL_TABLE_H

#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "mcc/ast.h"

// ------------------------------------------------------------ Data structure: Symbol Table row

enum mcc_symbol_table_row_structure{
    MCC_SYMBOL_TABLE_ROW_STRUCTURE_VARIABLE,
    MCC_SYMBOL_TABLE_ROW_STRUCTURE_ARRAY,
    MCC_SYMBOL_TABLE_ROW_STRUCTURE_FUNCTION,
};

enum mcc_symbol_table_row_type{
    MCC_SYMBOL_TABLE_ROW_TYPE_INT,
    MCC_SYMBOL_TABLE_ROW_TYPE_FLOAT,
    MCC_SYMBOL_TABLE_ROW_TYPE_BOOL,
    MCC_SYMBOL_TABLE_ROW_TYPE_STRING,
    MCC_SYMBOL_TABLE_ROW_TYPE_VOID,
    MCC_SYMBOL_TABLE_ROW_TYPE_PSEUDO,
};

struct mcc_symbol_table_row {
    enum mcc_symbol_table_row_structure row_structure;
    enum mcc_symbol_table_row_type row_type;
    int array_size; //-1 if no array
    char *name;

    struct mcc_symbol_table_row *prev_row;
    struct mcc_symbol_table_row *next_row;
    struct mcc_symbol_table_scope *scope;
    struct mcc_symbol_table_scope *child_scope;

    struct mcc_ast_node *node;
};

// ------------------------------------------------------------ Functions: Symbol Table row

struct mcc_symbol_table_row *mcc_symbol_table_new_row_variable(char *name,
                                                               enum mcc_symbol_table_row_type type,
                                                               struct mcc_ast_node *node);
struct mcc_symbol_table_row *mcc_symbol_table_new_row_function(char *name,
                                                               enum mcc_symbol_table_row_type type,
                                                               struct mcc_ast_node *node);
struct mcc_symbol_table_row *mcc_symbol_table_new_row_array(char *name,
                                                            int array_size,
                                                            enum mcc_symbol_table_row_type type,
                                                            struct mcc_ast_node *node);
void mcc_symbol_table_delete_row(struct mcc_symbol_table_row *row);
void mcc_symbol_table_delete_all_rows(struct mcc_symbol_table_row *head);
void mcc_symbol_table_row_append_child_scope(struct mcc_symbol_table_row *row, struct mcc_symbol_table_scope *child);

// ------------------------------------------------------------- Data structure: Symbol Table scope

enum mcc_symbol_table_scope_type{
    MCC_SYMBOL_TABLE_SCOPE_TYPE_STATEMENT,
    MCC_SYMBOL_TABLE_SCOPE_TYPE_COMPOUND_STATEMENT,
};

struct mcc_symbol_table_scope{
    // 'list' of rows
    struct mcc_symbol_table_row *head;

    struct mcc_symbol_table_row *parent_row;

    struct mcc_symbol_table_scope *next_scope;
    struct mcc_symbol_table_scope *prev_scope;
};

// ------------------------------------------------------------ Functions: Symbol Table scope

struct mcc_symbol_table_scope *mcc_symbol_table_new_scope();
struct mcc_symbol_table_row *mcc_symbol_table_scope_get_last_row(struct mcc_symbol_table_scope *scope);
void mcc_symbol_table_scope_append_row(struct mcc_symbol_table_scope *scope, struct mcc_symbol_table_row *row);
void mcc_symbol_table_delete_scope(struct mcc_symbol_table_scope *scope);
void mcc_symbol_table_delete_all_scopes(struct mcc_symbol_table_scope *head);

// -------------------------------------------------------------- Data structure: Symbol Table

struct mcc_symbol_table{
    // list of scopes
    struct mcc_symbol_table_scope *head;
};

struct mcc_symbol_table *mcc_symbol_table_new_table();
void mcc_symbol_table_insert_scope(struct mcc_symbol_table *table, struct mcc_symbol_table_scope *scope);
void mcc_symbol_table_delete_table(struct mcc_symbol_table *table);

// --------------------------------------------------------------- Functions: traversing AST and create symbol table

struct mcc_symbol_table_row *mcc_symbol_table_check_upwards_for_declaration(const char *name,
                                                                            struct mcc_symbol_table_row *row);
struct mcc_symbol_table_row *mcc_symbol_table_check_for_function_declaration(const char *name,
                                                                             struct mcc_symbol_table_row *row);
struct mcc_symbol_table *mcc_symbol_table_create(struct mcc_ast_program *program);

#endif //MCC_SYMBOL_TABLE_H
