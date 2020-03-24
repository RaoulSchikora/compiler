#include "mcc/symbol_table.h"
#include "mcc/ast_visit.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

// ------------------------------------------------------- Forward declaration

static void create_rows_statement(struct mcc_ast_statement *statement, struct mcc_symbol_table_scope *scope);
static void link_pointer_expression(struct mcc_ast_expression *expression, struct mcc_symbol_table_scope *scope);

// ------------------------------------------------------- converting enum types

static enum mcc_symbol_table_row_type convert_enum(enum mcc_ast_types type)
{
    switch(type){
    case INT:
        return MCC_SYMBOL_TABLE_ROW_TYPE_INT;
    case FLOAT:
        return MCC_SYMBOL_TABLE_ROW_TYPE_FLOAT;
    case BOOL:
        return MCC_SYMBOL_TABLE_ROW_TYPE_BOOL;
    case STRING:
        return MCC_SYMBOL_TABLE_ROW_TYPE_STRING;
    }

    return MCC_SYMBOL_TABLE_ROW_TYPE_PSEUDO;
}

// ------------------------------------------------------- Symbol Table row

struct mcc_symbol_table_row *mcc_symbol_table_new_row_variable(char *name,
                                                               enum mcc_symbol_table_row_type type,
                                                               struct mcc_ast_node *node)
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
    row->node = node;
    row->prev_row = NULL;
    row->next_row = NULL;
    row->scope = NULL;
    row->child_scope = NULL;

    return row;
}

struct mcc_symbol_table_row *mcc_symbol_table_new_row_function(char *name,
                                                               enum mcc_symbol_table_row_type type,
                                                               struct mcc_ast_node *node)
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
    row->node = node;
    row->prev_row = NULL;
    row->next_row = NULL;
    row->scope = NULL;
    row->child_scope = NULL;

    return row;
}

struct mcc_symbol_table_row *mcc_symbol_table_new_row_array(char *name,
                                                            int array_size,
                                                            enum mcc_symbol_table_row_type type,
                                                            struct mcc_ast_node *node)
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
    row->node = node;
    row->prev_row = NULL;
    row->next_row = NULL;
    row->scope = NULL;
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
    row->scope = scope;

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
        row = mcc_symbol_table_new_row_variable(declaration->variable_identifier->identifier_name,
                                                convert_enum(declaration->variable_type->type_value),
                                                &(declaration->node));
        mcc_symbol_table_scope_append_row(scope, row);
        break;
    case MCC_AST_DECLARATION_TYPE_ARRAY:
        row = mcc_symbol_table_new_row_array(declaration->array_identifier->identifier_name,
                                             declaration->array_size->i_value,
                                             convert_enum(declaration->array_type->type_value),
                                             &(declaration->node));
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

    if(!scope->head){
        struct mcc_symbol_table_row *row = mcc_symbol_table_new_row_variable("-", MCC_SYMBOL_TABLE_ROW_TYPE_PSEUDO, NULL);
        mcc_symbol_table_scope_append_row(scope, row);
    }
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

static void link_pointer_assignment(struct mcc_ast_assignment *assignment, struct mcc_symbol_table_scope *scope)
{
    assert(assignment);
    assert(scope);

    struct mcc_symbol_table_row *row = mcc_symbol_table_scope_get_last_row(scope);

    if(!row){
        row = create_pseudo_row(scope);
    }

    assignment->row = row;

    switch (assignment->assignment_type){
    case MCC_AST_ASSIGNMENT_TYPE_VARIABLE:
        link_pointer_expression(assignment->variable_assigned_value, scope);
        break;
    case MCC_AST_ASSIGNMENT_TYPE_ARRAY:
        link_pointer_expression(assignment->array_assigned_value, scope);
        link_pointer_expression(assignment->array_index, scope);
        break;
    }
}

static void link_pointer_expression(struct mcc_ast_expression *expression, struct mcc_symbol_table_scope *scope)
{
    assert(expression);
    assert(scope);

    struct mcc_symbol_table_row *row = mcc_symbol_table_scope_get_last_row(scope);

    if(!row){
        row = create_pseudo_row(scope);
    }

    switch(expression->type){
    case MCC_AST_EXPRESSION_TYPE_LITERAL:
        // do nothing
        break;
    case MCC_AST_EXPRESSION_TYPE_BINARY_OP:
        link_pointer_expression(expression->lhs, scope);
        link_pointer_expression(expression->rhs, scope);
        break;
    case MCC_AST_EXPRESSION_TYPE_PARENTH:
        link_pointer_expression(expression->expression, scope);
        break;
    case MCC_AST_EXPRESSION_TYPE_UNARY_OP:
        link_pointer_expression(expression->child, scope);
        break;
    case MCC_AST_EXPRESSION_TYPE_VARIABLE:
        expression->variable_row = row;
        break;
    case MCC_AST_EXPRESSION_TYPE_ARRAY_ELEMENT:
        link_pointer_expression(expression->index, scope);
        expression->array_row = row;
        break;
    case MCC_AST_EXPRESSION_TYPE_FUNCTION_CALL:
        expression->function_row = row;
        break;
    }
}

static void link_pointer_return(struct mcc_ast_statement *statement, struct mcc_symbol_table_scope *scope)
{
    assert(statement);
    assert(scope);

    if(statement->return_value){
        link_pointer_expression(statement->return_value, scope);
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
        link_pointer_expression(statement->if_condition, scope);
        create_rows_statement(statement->if_on_true, scope);
        break;
    case MCC_AST_STATEMENT_TYPE_IF_ELSE_STMT:
        link_pointer_expression(statement->if_else_condition, scope);
        create_rows_statement(statement->if_else_on_true, scope);
        create_rows_statement(statement->if_else_on_false, scope);
        break;
    case MCC_AST_STATEMENT_TYPE_WHILE:
        link_pointer_expression(statement->while_condition, scope);
        create_rows_statement(statement->while_on_true, scope);
        break;
    case MCC_AST_STATEMENT_TYPE_COMPOUND_STMT:
        create_rows_compound_statement(statement->compound_statement, append_child_scope_to_last_row(scope));
        break;
    case MCC_AST_STATEMENT_TYPE_ASSIGNMENT:
        link_pointer_assignment(statement->assignment, scope);
        break;
    case MCC_AST_STATEMENT_TYPE_EXPRESSION:
        link_pointer_expression(statement->stmt_expression, scope);
        break;
    case MCC_AST_STATEMENT_TYPE_RETURN:
        link_pointer_return(statement, scope);
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
                                                    MCC_SYMBOL_TABLE_ROW_TYPE_INT,
                                                    &(function_definition->node));
            break;
        case MCC_AST_FUNCTION_TYPE_FLOAT:
            row = mcc_symbol_table_new_row_function(function_definition->identifier->identifier_name,
                                                    MCC_SYMBOL_TABLE_ROW_TYPE_FLOAT,
                                                    &(function_definition->node));
            break;
        case MCC_AST_FUNCTION_TYPE_STRING:
            row = mcc_symbol_table_new_row_function(function_definition->identifier->identifier_name,
                                                    MCC_SYMBOL_TABLE_ROW_TYPE_STRING,
                                                    &(function_definition->node));
            break;
        case MCC_AST_FUNCTION_TYPE_BOOL:
            row = mcc_symbol_table_new_row_function(function_definition->identifier->identifier_name,
                                                    MCC_SYMBOL_TABLE_ROW_TYPE_BOOL,
                                                    &(function_definition->node));
            break;
        case MCC_AST_FUNCTION_TYPE_VOID:
            row = mcc_symbol_table_new_row_function(function_definition->identifier->identifier_name,
                                                    MCC_SYMBOL_TABLE_ROW_TYPE_VOID,
                                                    &(function_definition->node));
            break;
    }

    mcc_symbol_table_scope_append_row(table->head, row);

    create_rows_function_parameters(function_definition, row);
    create_rows_function_body(function_definition, row);
}

// check if there is an declaration of the given name in the symbole table above (including) the given row. However,
// checks on function level are not done.
struct mcc_symbol_table_row *mcc_symbol_table_check_upwards_for_declaration(const char *wanted_name,
                                                                            struct mcc_symbol_table_row *start_row)
{
    assert(wanted_name);
    assert(start_row);

    struct mcc_symbol_table_row *row = start_row;
    struct mcc_symbol_table_scope *scope = row->scope;

    if(strcmp(wanted_name, row->name) == 0){
        return row;
    }

    while(scope->parent_row){
        if(strcmp(wanted_name, row->name) == 0){
            return row;
        }

        while(row->prev_row){
            row = row->prev_row;

            if(strcmp(wanted_name, row->name) == 0){
                return row;
            }
        }
        row = scope->parent_row;
        scope = row->scope;
    }
    return NULL;
}

struct mcc_symbol_table_row *mcc_symbol_table_check_for_function_declaration(const char *wanted_name,
                                                                             struct mcc_symbol_table_row *start_row)
{
    assert(wanted_name);
    assert(start_row);

    struct mcc_symbol_table_row *row = start_row;
    struct mcc_symbol_table_scope *scope = row->scope;

    // go to top level
    do{
        row = scope->parent_row;
        scope = row->scope;
    } while(scope->parent_row);

    // iterate through top level
    row = scope->head;
    if(!row){
        return NULL;
    }
    do{
        if(strcmp(wanted_name, row->name) == 0){
            return row;
        }
        row = row->next_row;
    } while(row->next_row);

    return NULL;
}

static void add_built_in_function_definitions(struct mcc_symbol_table *table)
{
    assert(table);

    if(!table->head){
        mcc_symbol_table_insert_new_scope(table);
    }

    struct mcc_symbol_table_row *row_print = mcc_symbol_table_new_row_function("print",
                                                                               MCC_SYMBOL_TABLE_ROW_TYPE_VOID, NULL);
    struct mcc_symbol_table_row *row_print_nl = mcc_symbol_table_new_row_function("print_nl",
                                                                                  MCC_SYMBOL_TABLE_ROW_TYPE_VOID, NULL);
    struct mcc_symbol_table_row *row_print_int = mcc_symbol_table_new_row_function("print_int",
                                                                                   MCC_SYMBOL_TABLE_ROW_TYPE_VOID, NULL);
    struct mcc_symbol_table_row *row_print_float = mcc_symbol_table_new_row_function("print_float",
                                                                                     MCC_SYMBOL_TABLE_ROW_TYPE_VOID, NULL);
    struct mcc_symbol_table_row *row_read_int = mcc_symbol_table_new_row_function("read_int",
                                                                                  MCC_SYMBOL_TABLE_ROW_TYPE_INT, NULL);
    struct mcc_symbol_table_row *row_read_float = mcc_symbol_table_new_row_function("read_float",
                                                                                     MCC_SYMBOL_TABLE_ROW_TYPE_FLOAT, NULL);

    mcc_symbol_table_scope_append_row(table->head, row_print);
    mcc_symbol_table_scope_append_row(table->head, row_print_nl);
    mcc_symbol_table_scope_append_row(table->head, row_print_int);
    mcc_symbol_table_scope_append_row(table->head, row_print_float);
    mcc_symbol_table_scope_append_row(table->head, row_read_int);
    mcc_symbol_table_scope_append_row(table->head, row_read_float);

    // No function body scope needed (will not be checked by semantic checks anyways)

    // Function parameters:

    struct mcc_symbol_table_scope *child_scope_print = mcc_symbol_table_new_scope();
    struct mcc_symbol_table_scope *child_scope_print_nl = mcc_symbol_table_new_scope();
    struct mcc_symbol_table_scope *child_scope_print_int = mcc_symbol_table_new_scope();
    struct mcc_symbol_table_scope *child_scope_print_float = mcc_symbol_table_new_scope();
    struct mcc_symbol_table_scope *child_scope_read_int = mcc_symbol_table_new_scope();
    struct mcc_symbol_table_scope *child_scope_read_float = mcc_symbol_table_new_scope();

    mcc_symbol_table_row_append_child_scope(row_print,child_scope_print);
    mcc_symbol_table_row_append_child_scope(row_print_nl,child_scope_print_nl);
    mcc_symbol_table_row_append_child_scope(row_print_int,child_scope_print_int);
    mcc_symbol_table_row_append_child_scope(row_print_float,child_scope_print_float);
    mcc_symbol_table_row_append_child_scope(row_read_int,child_scope_read_int);
    mcc_symbol_table_row_append_child_scope(row_read_float,child_scope_read_float);

    // print (string)
    struct mcc_symbol_table_row *row_print_params;
    row_print_params = mcc_symbol_table_new_row_variable("a",MCC_SYMBOL_TABLE_ROW_TYPE_STRING, NULL);
    mcc_symbol_table_scope_append_row(child_scope_print, row_print_params);

    // print_int(int)
    struct mcc_symbol_table_row *row_print_int_params;
    row_print_int_params = mcc_symbol_table_new_row_variable("a",MCC_SYMBOL_TABLE_ROW_TYPE_INT, NULL);
    mcc_symbol_table_scope_append_row(child_scope_print_int, row_print_int_params);

    // print_float(float)
    struct mcc_symbol_table_row *row_print_float_params;
    row_print_float_params = mcc_symbol_table_new_row_variable("a",MCC_SYMBOL_TABLE_ROW_TYPE_FLOAT, NULL);
    mcc_symbol_table_scope_append_row(child_scope_print_float, row_print_float_params);



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

    add_built_in_function_definitions(table);

    return table;
}

