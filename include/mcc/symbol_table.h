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

// ------------------------------------------------------------ Symbol Table row

enum mcc_symbol_table_row_type{
    MCC_SYMBOL_TABLE_ROW_TYPE_BOOL,
    MCC_SYMBOL_TABLE_ROW_TYPE_INT,
    MCC_SYMBOL_TABLE_ROW_TYPE_FLOAT,
    MCC_SYMBOL_TABLE_ROW_TYPE_STRING,
};

struct mcc_symbol_table_row {
    enum mcc_symbol_table_row_type row_type;
    char *name;
    struct mcc_symbol_table_row *prev_row;
    struct mcc_symbol_table_row *next_row;
};

struct mcc_symbol_table_row *mcc_symbol_table_new_row(char *name, enum mcc_symbol_table_row_type type);
void mcc_symbol_table_delete_row(struct mcc_symbol_table_row *row);
void mcc_symbol_table_delete_all_rows(struct mcc_symbol_table_row *head);

// ------------------------------------------------------------- Symbol Table scope

enum mcc_symbol_table_scope_type{
    MCC_SYMBOL_TABLE_SCOPE_TYPE_STATEMENT,
    MCC_SYMBOL_TABLE_SCOPE_TYPE_COMPOUND_STATEMENT,
};

struct mcc_symbol_table_scope{
    // list of rows
    struct mcc_symbol_table_row *head;

    // pointer to the statement or compound_statement in the ast
//    enum mcc_symbol_table_scope_type scope_type;
//    union {
//        struct mcc_ast_statement *statement;
//        struct mcc_ast_compound_statement *compound_statement;
//    };

    // next sibling scope
    struct mcc_symbol_table_scope *next_scope;

    // parent and child scope
    struct mcc_symbol_table_scope *parent_scope;
    struct mcc_symbol_table_scope *child_scope;
};

struct mcc_symbol_table_scope *mcc_symbol_table_new_scope();
void mcc_symbol_table_scope_append_row(struct mcc_symbol_table_scope *scope, struct mcc_symbol_table_row *row);
void mcc_symbol_table_delete_scope(struct mcc_symbol_table_scope *scope);
void mcc_symbol_table_delete_all_scopes(struct mcc_symbol_table_scope *head);
void mcc_symbol_table_insert_child_scope(struct mcc_symbol_table_scope *parent, struct mcc_symbol_table_scope *child);

// -------------------------------------------------------------- Symbol Table

struct mcc_symbol_table{
    // list of scopes
    struct mcc_symbol_table_scope *head;
};

struct mcc_symbol_table *mcc_symbol_table_new_table();
void mcc_symbol_table_insert_scope(struct mcc_symbol_table *table, struct mcc_symbol_table_scope *scope);
void mcc_symbol_table_delete_table(struct mcc_symbol_table *table);

#endif //MCC_SYMBOL_TABLE_H
