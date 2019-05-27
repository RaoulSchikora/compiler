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
    struct mcc_symbol_table_row *last = *head_ref;

    if(*head_ref == NULL){
        *head_ref = row;
        return;
    }
    while (last->next_row != NULL){
        last = last->next_row;
    }
    last->next_row = row;
    row->prev_row = last;
    return;
}

void mcc_symbol_table_delete_scope(struct mcc_symbol_table_scope *scope)
{
    assert(scope);

    if(scope->head){
        mcc_symbol_table_delete_all_rows(scope->head);
    }
    //TODO
    // free childs
    // link next and prev scope

    free(scope);
}

void mcc_symbol_table_insert_child_scope(struct mcc_symbol_table_scope *parent, struct mcc_symbol_table_scope *child)
{
    assert(parent);
    assert(child);

    if (parent->child_scope == NULL){
        parent->child_scope = child;
        child->parent_scope = parent;
    } else {
        struct mcc_symbol_table_scope *tmp_scope = parent->child_scope;
        while (tmp_scope->has_next){
            tmp_scope = tmp_scope->next_scope;
        }
        tmp_scope->has_next = true;
        tmp_scope->next_scope = child;
        child->parent_scope = parent;
    }
}

void mcc_symbol_table_scope_delete(struct mcc_symbol_table_scope *scope)
{

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

}