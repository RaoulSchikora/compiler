#include "mcc/symbol_table.h"
#include "mcc/ast_visit.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

// ------------------------------------------------------- Forward declaration

static void create_rows_statement(struct mcc_ast_statement *statement, struct mcc_symbol_table_scope *scope);

// ------------------------------------------------------- Symbol Table row

struct mcc_symbol_table_row *mcc_symbol_table_new_row_variable(char *name, enum mcc_symbol_table_row_type type)
{
    struct mcc_symbol_table_row *row = malloc(sizeof(*row));
    if(!row){
        return NULL;
    }

    row->row_structure = MCC_SYMBOL_TABLE_ROW_STRUCTURE_VARIABLE;
    row->array_size = -1;
    row->row_type = type;
    row->name = malloc(sizeof(char) * strlen(name) + 1);
    strcpy(row->name, name);
    row->prev_row = NULL;
    row->next_row = NULL;
    row->child_scope = NULL;

    return row;
}

struct mcc_symbol_table_row *mcc_symbol_table_new_row_function(char *name, enum mcc_symbol_table_row_type type)
{
    struct mcc_symbol_table_row *row = malloc(sizeof(*row));
    if(!row){
        return NULL;
    }

    row->row_structure = MCC_SYMBOL_TABLE_ROW_STRUCTURE_FUNCTION;
    row->array_size = -1;
    row->row_type = type;
    row->name = malloc(sizeof(char) * strlen(name) + 1);
    strcpy(row->name, name);
    row->prev_row = NULL;
    row->next_row = NULL;
    row->child_scope = NULL;

    return row;
}

struct mcc_symbol_table_row *mcc_symbol_table_new_row_array(char *name, int array_size,
                                                            enum mcc_symbol_table_row_type type)
{
    struct mcc_symbol_table_row *row = malloc(sizeof(*row));
    if(!row){
        return NULL;
    }

    row->row_structure = MCC_SYMBOL_TABLE_ROW_STRUCTURE_ARRAY;
    row->array_size = array_size;
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

struct mcc_symbol_table_row *mcc_symbol_table_scope_get_last_row(struct mcc_symbol_table_scope *scope)
{
    assert(scope);

    if(!scope->head){
        return NULL;
    }

    struct mcc_symbol_table_row *row = scope->head;
    while(row->next_row){
        row = row->next_row;
    }
    return row;
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

static void create_row_declaration(struct mcc_ast_declaration *declaration, struct mcc_symbol_table_scope *scope)
{
    assert(scope);
    assert(declaration);

    struct mcc_symbol_table_row *row;

    switch (declaration->declaration_type){
    case MCC_AST_DECLARATION_TYPE_VARIABLE:
        // TODO enum-test for equality
        row = mcc_symbol_table_new_row_variable(declaration->variable_identifier->identifier_name,
                declaration->variable_type->type_value);
        mcc_symbol_table_scope_append_row(scope, row);
        break;
    case MCC_AST_DECLARATION_TYPE_ARRAY:
        row = mcc_symbol_table_new_row_array(declaration->array_identifier->identifier_name,
                declaration->array_size->i_value,
                declaration->array_type->type_value);
        mcc_symbol_table_scope_append_row(scope, row);
        break;
    }
}

static void create_rows_function_parameters(struct mcc_ast_function_definition *function_definition,
                                            struct mcc_symbol_table_row *row)
{
    assert(function_definition);
    assert(row);

    struct mcc_symbol_table_scope *child_scope = mcc_symbol_table_new_scope();
    mcc_symbol_table_row_append_child_scope(row, child_scope);

    if(!function_definition->parameters->is_empty){
        struct mcc_ast_parameters *parameters = function_definition->parameters;

        create_row_declaration(parameters->declaration, child_scope);

        while (parameters->next_parameters){
            parameters = parameters->next_parameters;
            create_row_declaration(parameters->declaration, child_scope);
        }
    }
}

static struct mcc_symbol_table_row *create_pseudo_row(struct mcc_symbol_table_scope *scope)
{
    assert(scope);

    scope->head = mcc_symbol_table_new_row_variable("-", MCC_SYMBOL_TABLE_ROW_TYPE_PSEUDO);
    return scope->head;
}

static struct mcc_symbol_table_scope *append_child_scope_to_last_row(struct mcc_symbol_table_scope *scope)
{
    assert(scope);

    struct mcc_symbol_table_row *row = mcc_symbol_table_scope_get_last_row(scope);
    // if row == NULL create pseudo row
    if(!row){
        row = create_pseudo_row(scope);
    }
    struct mcc_symbol_table_scope *new_scope = mcc_symbol_table_new_scope();
    mcc_symbol_table_row_append_child_scope(row, new_scope);

    return new_scope;
}

static void create_rows_compound_statement(struct mcc_ast_compound_statement *compound_stmt,
        struct mcc_symbol_table_scope *scope)
{
    assert(compound_stmt);
    assert(scope);

    if(compound_stmt->statement){
        create_rows_statement(compound_stmt->statement, scope);
    }

    while(compound_stmt->next_compound_statement){
        compound_stmt = compound_stmt->next_compound_statement;
        create_rows_statement(compound_stmt->statement, scope);
    }
}

static void create_rows_statement(struct mcc_ast_statement *statement, struct mcc_symbol_table_scope *scope)
{
    assert(statement);
    assert(scope);

    switch (statement->type){
    case MCC_AST_STATEMENT_TYPE_DECLARATION:
        create_row_declaration(statement->declaration, scope);
        break;
    case MCC_AST_STATEMENT_TYPE_IF_STMT:
        create_rows_statement(statement->if_on_true, scope);
        break;
    case MCC_AST_STATEMENT_TYPE_IF_ELSE_STMT:
        create_rows_statement(statement->if_else_on_true, scope);
        create_rows_statement(statement->if_else_on_false, scope);
        break;
    case MCC_AST_STATEMENT_TYPE_WHILE:
        create_rows_statement(statement->while_on_true, scope);
        break;
    case MCC_AST_STATEMENT_TYPE_COMPOUND_STMT:
        create_rows_compound_statement(statement->compound_statement, append_child_scope_to_last_row(scope));
        break;
    case MCC_AST_STATEMENT_TYPE_EXPRESSION:
        // do nothing
        break;
    case MCC_AST_STATEMENT_TYPE_ASSIGNMENT:
        // do nothing
        break;
    case MCC_AST_STATEMENT_TYPE_RETURN:
        // do nothing
        break;
    }
}

static void create_rows_function_body(struct mcc_ast_function_definition *function_definition,
                                      struct mcc_symbol_table_row *row)
{
    assert(function_definition);
    assert(row);

    if(!row->child_scope){
        struct mcc_symbol_table_scope *child_scope = mcc_symbol_table_new_scope();
        mcc_symbol_table_row_append_child_scope(row, child_scope);
    }

    struct mcc_ast_compound_statement *compound_stmt = function_definition->compound_stmt;

    if(compound_stmt->statement) {
        create_rows_statement(compound_stmt->statement, row->child_scope);
    }

    while(compound_stmt->next_compound_statement){
        compound_stmt = compound_stmt->next_compound_statement;
        create_rows_statement(compound_stmt->statement, row->child_scope);
    }
}

static void create_row_function_definition(struct mcc_ast_function_definition *function_definition,
        struct mcc_symbol_table *table)
{
    assert(function_definition);
    assert(table);

    if(!table->head){
        mcc_symbol_table_insert_new_scope(table);
    }

    struct mcc_symbol_table_row *row = NULL;

    switch(function_definition->type){
        case MCC_AST_FUNCTION_TYPE_INT:
            row = mcc_symbol_table_new_row_function(function_definition->identifier->identifier_name,
                    MCC_SYMBOL_TABLE_ROW_TYPE_INT);
            break;
        case MCC_AST_FUNCTION_TYPE_FLOAT:
            row = mcc_symbol_table_new_row_function(function_definition->identifier->identifier_name,
                                                    MCC_SYMBOL_TABLE_ROW_TYPE_FLOAT);
            break;
        case MCC_AST_FUNCTION_TYPE_STRING:
            row = mcc_symbol_table_new_row_function(function_definition->identifier->identifier_name,
                                                    MCC_SYMBOL_TABLE_ROW_TYPE_STRING);
            break;
        case MCC_AST_FUNCTION_TYPE_BOOL:
            row = mcc_symbol_table_new_row_function(function_definition->identifier->identifier_name,
                                                    MCC_SYMBOL_TABLE_ROW_TYPE_BOOL);
            break;
        case MCC_AST_FUNCTION_TYPE_VOID:
            row = mcc_symbol_table_new_row_function(function_definition->identifier->identifier_name,
                                                    MCC_SYMBOL_TABLE_ROW_TYPE_VOID);
            break;
    }

    mcc_symbol_table_scope_append_row(table->head, row);

    create_rows_function_parameters(function_definition, row);
    create_rows_function_body(function_definition, row);
}

struct mcc_symbol_table *mcc_symbol_table_create(struct mcc_ast_program *program)
{
    assert(program);

    struct mcc_symbol_table *table = mcc_symbol_table_new_table();

    if(program->function){

        struct mcc_ast_function_definition *function = program->function;
        create_row_function_definition(function, table);

        while(program->next_function){
            program = program->next_function;
            function = program->function;
            create_row_function_definition(function, table);
        }
    }

    return table;
}