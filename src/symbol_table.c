#include "mcc/symbol_table.h"
#include "mcc/ast_visit.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

// ------------------------------------------------------- Symbol Table row

struct mcc_symbol_table_row *mcc_symbol_table_new_row(char *name, enum mcc_symbol_table_row_type type)
{
    struct mcc_symbol_table_row *row = malloc(sizeof(*row));
    if(!row){
        return NULL;
    }

    row->row_type = type;
    row->name = malloc(sizeof(char) * strlen(name) + 1);
    strcpy(row->name, name);
    row->prev_row = NULL;
    row->next_row = NULL;
    row->child_scope = NULL;

    return row;
}

void mcc_symbol_table_delete_row(struct mcc_symbol_table_row *row)
{
    assert(row);

    // if row has a child scope delete that scope
    if(row->child_scope){
        mcc_symbol_table_delete_all_scopes(row->child_scope);
    }

    // rearrange pointer structure
    if(!row->prev_row && row->next_row){
        row->next_row->prev_row = NULL;
    }
    if(row->prev_row && !row->next_row){
        row->prev_row->next_row = NULL;
    }
    if(row->prev_row && row->next_row) {
        row->next_row->prev_row = row->prev_row;
        row->prev_row->next_row = row->next_row;
    }

    // free row
    free(row->name);
    free(row);
}

void mcc_symbol_table_delete_all_rows(struct mcc_symbol_table_row *head)
{
    assert(head);

    if(head){
        while(head->next_row){
            mcc_symbol_table_delete_row(head->next_row);
        }
        mcc_symbol_table_delete_row(head);
    }

    return;
}

void mcc_symbol_table_row_append_child_scope(struct mcc_symbol_table_row *row, struct mcc_symbol_table_scope *child)
{
    assert(row);
    assert(child);

    if(!row->child_scope){
        row->child_scope = child;
        child->parent_row = row;

        return;
    } else {
        struct mcc_symbol_table_scope *last_child = row->child_scope;

        while(last_child->next_scope){
            last_child = last_child->next_scope;
        }
        last_child->next_scope = child;
        child->parent_row = row;
    }

    return;
}

// --------------------------------------------------------- Symbol Table scope

struct mcc_symbol_table_scope *mcc_symbol_table_new_scope()
{

    struct mcc_symbol_table_scope *scope = malloc(sizeof(*scope));
    if(!scope){
        return NULL;
    }

    scope->head = NULL;
    scope->parent_row = NULL;
    scope->next_scope = NULL;
    scope->prev_scope = NULL;

    return scope;
}

void mcc_symbol_table_scope_append_row(struct mcc_symbol_table_scope *scope, struct mcc_symbol_table_row *row)
{
    assert(scope);
    assert(row);

    struct mcc_symbol_table_row **head_ref = &scope->head;
    struct mcc_symbol_table_row *last_row = *head_ref;

    if(!*head_ref){
        *head_ref = row;
        return;
    }
    while (last_row->next_row){
        last_row = last_row->next_row;
    }
    last_row->next_row = row;
    row->prev_row = last_row;
    return;
}

void mcc_symbol_table_delete_scope(struct mcc_symbol_table_scope *scope)
{
    assert(scope);

    // delete rows
    if(scope->head){
        mcc_symbol_table_delete_all_rows(scope->head);
    }

    free(scope);
}

void mcc_symbol_table_delete_all_scopes(struct mcc_symbol_table_scope *head)
{
    assert(head);

    struct mcc_symbol_table_scope *tmp = head;

    if(head){
        while(head){
            tmp = head->next_scope;
            mcc_symbol_table_delete_scope(head);
            head = tmp;
        }
    }

    return;
}

// ------------------------------------------------------- Symbol Table

struct mcc_symbol_table *mcc_symbol_table_new_table()
{
    struct mcc_symbol_table *table = malloc(sizeof(*table));
    if(!table){
        return NULL;
    }

    table->head = NULL;

    return table;
}

void mcc_symbol_table_insert_scope(struct mcc_symbol_table *table, struct mcc_symbol_table_scope *scope)
{
    assert(table);
    assert(scope);

    struct mcc_symbol_table_scope **head_ref = &table->head;
    struct mcc_symbol_table_scope *last_scope = *head_ref;

    if(!*head_ref){
        *head_ref = scope;
        return;
    }
    while (last_scope->next_scope){
        last_scope = last_scope->next_scope;
    }
    last_scope->next_scope = scope;
    return;
}

void mcc_symbol_table_insert_new_scope(struct mcc_symbol_table *table)
{
    assert(table);

    mcc_symbol_table_insert_scope(table, mcc_symbol_table_new_scope());
}

void mcc_symbol_table_delete_table(struct mcc_symbol_table *table)
{
    assert(table);

    if(table->head){
        mcc_symbol_table_delete_all_scopes(table->head);
    }

    free(table);
}

// --------------------------------------------------------------- traversing AST and create symbol table

static void create_row_function_definition(struct mcc_ast_function_definition *function_definition, void *data)
{
    assert(function_definition);
    assert(data);

    struct mcc_symbol_table *table = data;

    if(!table->head){
        mcc_symbol_table_insert_new_scope(table);
    }

    struct mcc_symbol_table_row *row = mcc_symbol_table_new_row(function_definition->identifier->identifier_name,
            MCC_SYMBOL_TABLE_ROW_TYPE_FUNCTION);
    mcc_symbol_table_scope_append_row(table->head, row);

    // TODO insert function body
}

// Setup an AST visitor for traversing the AST and filling the symbol table.
static struct mcc_ast_visitor create_symbol_table_visitor(struct mcc_symbol_table *table)
{
    assert(table);

    return (struct mcc_ast_visitor){
            .traversal = MCC_AST_VISIT_DEPTH_FIRST,
            .order = MCC_AST_VISIT_PRE_ORDER,

            .userdata = table,

            .function_definition = create_row_function_definition,
    };
}

struct mcc_symbol_table *mcc_symbol_table_create(struct mcc_ast_program *program)
{
    assert(program);

    struct mcc_symbol_table *table = mcc_symbol_table_new_table();

    struct mcc_ast_visitor visitor = create_symbol_table_visitor(table);
    mcc_ast_visit(program, &visitor);

    return table;
}