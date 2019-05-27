#include "mcc/symbol_table.h"

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
    row->next_row = NULL;
    row->prev_row = NULL;

    return row;
}

void mcc_symbol_table_delete_row(struct mcc_symbol_table_row *row)
{
    assert(row);

    if(row->prev_row == NULL && row->next_row != NULL){
        row->next_row->prev_row = NULL;
    }
    if(row->prev_row != NULL && row->next_row == NULL){
        row->prev_row->next_row = NULL;
    }
    if(row->prev_row != NULL && row->next_row != NULL) {
        row->next_row->prev_row = row->prev_row;
        row->prev_row->next_row = row->next_row;
    }

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

// --------------------------------------------------------- Symbol Table scope

struct mcc_symbol_table_scope *mcc_symbol_table_new_scope()
{

    struct mcc_symbol_table_scope *scope = malloc(sizeof(*scope));
    if(!scope){
        return NULL;
    }

    scope->head = NULL;
    scope->has_next = false;
    scope->next_scope = NULL;
    scope->parent_scope = NULL;
    scope->child_scope = NULL;

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

    // delete childs
    if(scope->child_scope){
        mcc_symbol_table_delete_all_scopes(scope->child_scope);
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

void mcc_symbol_table_insert_child_scope(struct mcc_symbol_table_scope *parent, struct mcc_symbol_table_scope *child)
{
    assert(parent);
    assert(child);

    struct mcc_symbol_table_scope *last_child = parent->child_scope;

    if(!parent->child_scope){
        parent->child_scope = child;
        child->parent_scope = parent;
        return;
    }
    while(last_child->next_scope){
        last_child = last_child->next_scope;
    }
    last_child->has_next = true;
    last_child->next_scope = child;
    child->parent_scope = parent;

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
    last_scope->has_next = true;
    last_scope->next_scope = scope;
    return;
}

void mcc_symbol_table_delete_table(struct mcc_symbol_table *table)
{
    assert(table);

    if(table->head){
        mcc_symbol_table_delete_all_scopes(table->head);
    }

    free(table);
}