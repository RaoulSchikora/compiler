#include "mcc/semantic_checks.h"
#include "mcc/ast_visit.h"
#include "utils/unused.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>


// unused.h contains macro to suppress warnings of unused variables

// TODO: Implementation

// ------------------------------------------------------------- Forward declaration

static bool recursively_check_nonvoid_property(struct mcc_ast_compound_statement *compound_statement);

// ------------------------------------------------------------- Convert enum types

static enum mcc_semantic_check_expression_type convert_enum_symbol_table(enum mcc_symbol_table_row_type type)
{
    switch(type){
    case MCC_SYMBOL_TABLE_ROW_TYPE_INT:
        return MCC_SEMANTIC_CHECK_EXPRESSION_TYPE_INT;
    case MCC_SYMBOL_TABLE_ROW_TYPE_FLOAT:
        return MCC_SEMANTIC_CHECK_EXPRESSION_TYPE_FLOAT;
    case MCC_SYMBOL_TABLE_ROW_TYPE_BOOL:
        return MCC_SEMANTIC_CHECK_EXPRESSION_TYPE_BOOL;
    case MCC_SYMBOL_TABLE_ROW_TYPE_STRING:
        return MCC_SEMANTIC_CHECK_EXPRESSION_TYPE_STRING;
    case MCC_SYMBOL_TABLE_ROW_TYPE_VOID:
        return MCC_SEMANTIC_CHECK_EXPRESSION_TYPE_VOID;
    case MCC_SYMBOL_TABLE_ROW_TYPE_PSEUDO:
        return MCC_SEMANTIC_CHECK_EXPRESSION_TYPE_UNKNOWN;
    }
    return MCC_SEMANTIC_CHECK_EXPRESSION_TYPE_UNKNOWN;
}

static enum mcc_semantic_check_expression_type convert_enum_ast_literal(enum mcc_ast_literal_type type)
{
    switch(type){
    case MCC_AST_LITERAL_TYPE_INT:
        return MCC_SEMANTIC_CHECK_EXPRESSION_TYPE_INT;
    case MCC_AST_LITERAL_TYPE_FLOAT:
        return MCC_SEMANTIC_CHECK_EXPRESSION_TYPE_FLOAT;
    case MCC_AST_LITERAL_TYPE_BOOL:
        return MCC_SEMANTIC_CHECK_EXPRESSION_TYPE_BOOL;
    case MCC_AST_LITERAL_TYPE_STRING:
        return MCC_SEMANTIC_CHECK_EXPRESSION_TYPE_STRING;
    }

    return MCC_SEMANTIC_CHECK_EXPRESSION_TYPE_UNKNOWN;
}

// ------------------------------------------------------------- Functions: Running all semantic checks

// Write error message into existing mcc_semantic_check struct
static void write_error_message_to_all_checks(struct mcc_semantic_check_all_checks* checks, const char* string){

    int size = sizeof(char)*(strlen(string)+2);
    char* buffer = malloc(size);
    if(buffer == NULL){
        perror("write_error_message_to_check: malloc");
    }
    snprintf(buffer,size,"%s\n",string);
    checks->error_buffer = buffer;
}

// Run all semantic checks. First encountered error is written into error buffer
struct mcc_semantic_check_all_checks* mcc_semantic_check_run_all(struct mcc_ast_program* ast,
                                                                 struct mcc_symbol_table* symbol_table){
    struct mcc_semantic_check_all_checks* checks = malloc(sizeof(*checks));
    if (checks == NULL){
        return NULL;
    }
    checks->status = MCC_SEMANTIC_CHECK_OK;
    checks->error_buffer = NULL;

    // No invalid array operation
    /*checks->array_types = mcc_semantic_check_run_array_types(ast, symbol_table);
    if(checks->array_types == NULL){
        checks->status = MCC_SEMANTIC_CHECK_FAIL;
        if(checks->error_buffer == NULL){
            write_error_message_to_all_checks(checks,"mcc_semantic_check_run_array_types returned NULL pointer.");
        }
    } else {
        if(checks->array_types->status != MCC_SEMANTIC_CHECK_OK){
            checks->status = MCC_SEMANTIC_CHECK_FAIL;
            if(checks->error_buffer == NULL){
                write_error_message_to_all_checks(checks,checks->array_types->error_buffer);
            }
        }
    }*/

    // No invalid function calls
    /*checks->function_arguments = mcc_semantic_check_run_function_arguments(ast, symbol_table);
    if(checks->function_arguments == NULL){
        checks->status = MCC_SEMANTIC_CHECK_FAIL;
        if(checks->error_buffer == NULL){
            write_error_message_to_all_checks(checks,"mcc_semantic_check_run_function_arguments returned NULL pointer.");
        }
    } else {
        if(checks->function_arguments->status != MCC_SEMANTIC_CHECK_OK){
            checks->status = MCC_SEMANTIC_CHECK_FAIL;
            if(checks->error_buffer == NULL){
                write_error_message_to_all_checks(checks,checks->function_arguments->error_buffer);
            }
        }
    }*/

    // Each execution path of non-void function returns a value
    checks->nonvoid_check = mcc_semantic_check_run_nonvoid_check(ast, symbol_table);
    if(checks->nonvoid_check == NULL){
        checks->status = MCC_SEMANTIC_CHECK_FAIL;
        if(checks->error_buffer == NULL){
            write_error_message_to_all_checks(checks,"mcc_semantic_check_run_nonvoid_check returned NULL pointer.");
        }
    } else {
        if(checks->nonvoid_check->status != MCC_SEMANTIC_CHECK_OK){
            checks->status = MCC_SEMANTIC_CHECK_FAIL;
            if(checks->error_buffer == NULL){
                write_error_message_to_all_checks(checks,checks->nonvoid_check->error_buffer);
            }
        }
    }

    // Main function exists and has correct signature
    checks->main_function = mcc_semantic_check_run_main_function(ast, symbol_table);
    if(checks->main_function == NULL){
        checks->status = MCC_SEMANTIC_CHECK_FAIL;
        if(checks->error_buffer == NULL){
            write_error_message_to_all_checks(checks,"mcc_semantic_check_run_main_function returned NULL pointer.");
        }
    } else {
        if(checks->main_function->status != MCC_SEMANTIC_CHECK_OK){
            checks->status = MCC_SEMANTIC_CHECK_FAIL;
            if(checks->error_buffer == NULL){
                write_error_message_to_all_checks(checks,checks->main_function->error_buffer);
            }
        }
    }

    // No Calls to unknown functions
    checks->unknown_function_call = mcc_semantic_check_run_unknown_function_call(ast, symbol_table);
    if(checks->unknown_function_call == NULL){
        checks->status = MCC_SEMANTIC_CHECK_FAIL;
        if(checks->error_buffer == NULL){
            write_error_message_to_all_checks(checks,
                    "mcc_semantic_check_run_unknown_function_call returned NULL pointer.");
        }
    } else {
        if(checks->unknown_function_call->status != MCC_SEMANTIC_CHECK_OK){
            checks->status = MCC_SEMANTIC_CHECK_FAIL;
            if(checks->error_buffer == NULL){
                write_error_message_to_all_checks(checks,checks->unknown_function_call->error_buffer);
            }
        }
    }

    // No multiple definitions of the same function
    checks->multiple_function_definitions = mcc_semantic_check_run_multiple_function_definitions(ast, symbol_table);
    if(checks->multiple_function_definitions == NULL){
        checks->status = MCC_SEMANTIC_CHECK_FAIL;
        if(checks->error_buffer == NULL){
            write_error_message_to_all_checks(checks,
                    "mcc_semantic_check_run_multiple_function_definitions returned NULL pointer.");
        }
    } else {
        if(checks->multiple_function_definitions->status != MCC_SEMANTIC_CHECK_OK){
            checks->status = MCC_SEMANTIC_CHECK_FAIL;
            if(checks->error_buffer == NULL){
                write_error_message_to_all_checks(checks,checks->multiple_function_definitions->error_buffer);
            }
        }
    }

    // No multiple declarations of a variable in the same scope
    checks->multiple_variable_declarations = mcc_semantic_check_run_multiple_variable_declarations(ast, symbol_table);
    if(checks->multiple_variable_declarations == NULL){
        checks->status = MCC_SEMANTIC_CHECK_FAIL;
        if(checks->error_buffer == NULL){
            write_error_message_to_all_checks(checks,
                    "mcc_semantic_check_run_multiple_variable_declarations returned NULL pointer.");
        }
    } else {
        if(checks->multiple_variable_declarations->status != MCC_SEMANTIC_CHECK_OK){
            checks->status = MCC_SEMANTIC_CHECK_FAIL;
            if(checks->error_buffer == NULL){
                write_error_message_to_all_checks(checks,checks->multiple_variable_declarations->error_buffer);
            }
        }
    }

    // No use of undeclared variables
    checks->use_undeclared_variable = mcc_semantic_check_run_use_undeclared_variable(ast, symbol_table);
    if(checks->use_undeclared_variable == NULL){
        checks->status = MCC_SEMANTIC_CHECK_FAIL;
        if(checks->error_buffer == NULL){
            write_error_message_to_all_checks(checks,
                    "mcc_semantic_check_run_use_undeclared_variable returned NULL pointer.");
        }
    } else {
        if(checks->use_undeclared_variable->status != MCC_SEMANTIC_CHECK_OK){
            checks->status = MCC_SEMANTIC_CHECK_FAIL;
            if(checks->error_buffer == NULL){
                write_error_message_to_all_checks(checks,checks->use_undeclared_variable->error_buffer);
            }
        }
    }

    // No use of the names of the built_in functions in function definitions
    checks->define_built_in = mcc_semantic_check_run_define_built_in(ast, symbol_table);
    if(checks->define_built_in == NULL){
        checks->status = MCC_SEMANTIC_CHECK_FAIL;
        if(checks->error_buffer == NULL){
            write_error_message_to_all_checks(checks,"mcc_semantic_check_run_define_built_in returned NULL pointer.");
        }
    } else {
        if(checks->define_built_in->status != MCC_SEMANTIC_CHECK_OK){
            checks->status = MCC_SEMANTIC_CHECK_FAIL;
            if(checks->error_buffer == NULL){
                write_error_message_to_all_checks(checks,checks->define_built_in->error_buffer);
            }
        }
    }

    // No type conversion
    checks->type_conversion = mcc_semantic_check_run_type_conversion(ast, symbol_table);
    if(checks->type_conversion == NULL){
        checks->status = MCC_SEMANTIC_CHECK_FAIL;
        if(checks->error_buffer == NULL){
            write_error_message_to_all_checks(checks,"mcc_semantic_check_run_type_conversion returned NULL pointer.");
        }
    } else {
        if(checks->type_conversion->status != MCC_SEMANTIC_CHECK_OK){
            checks->status = MCC_SEMANTIC_CHECK_FAIL;
            if(checks->error_buffer == NULL){
                write_error_message_to_all_checks(checks,checks->type_conversion->error_buffer);
            }
        }
    }

    return checks;
}

// ------------------------------------------------------------- Functions: Running single semantic checks

// Write error message into existing mcc_semantic_check struct
static void write_error_message_to_check(struct mcc_semantic_check* check, struct mcc_ast_node node, const char* string)
{
    int size = sizeof(char)*(strlen(string)+strlen(node.sloc.filename)+50);
    char* buffer = malloc(size);
    if(buffer == NULL){
        perror("write_error_message_to_check: malloc");
    }
    snprintf(buffer,size,"%s:%d:%d: %s\n",node.sloc.filename,node.sloc.start_line,node.sloc.start_col,string);
    check->error_buffer = buffer;
}

// ------------------------------------------------------------- No Type conversions in expressions

// generate error msg for type conversion in expression
static void generate_error_msg_type_conversion_expression(struct mcc_ast_node node, struct mcc_semantic_check *check)
{
    if(check->error_buffer){
        return;
    }
    int size = 35;
    char* error_msg = (char *)malloc( sizeof(char) * size);
    snprintf(error_msg, size, "type conversion not possible.");
    write_error_message_to_check(check, node, error_msg);
    check->status = MCC_SEMANTIC_CHECK_FAIL;
    free(error_msg);
}

// generate error msg for using logical connectives with non-boolean variables
static void generate_error_msg_type_conversion_non_boolean(struct mcc_ast_node node, struct mcc_semantic_check *check)
{
    if(check->error_buffer){
        return;
    }
    int size = 71;
    char* error_msg = (char *)malloc( sizeof(char) * size);
    snprintf(error_msg, size, "using non-boolean variable or expression with logical connective.");
    write_error_message_to_check(check, node, error_msg);
    check->status = MCC_SEMANTIC_CHECK_FAIL;
    free(error_msg);
}

// generate error msg for using binary operations on strings or whole arrays
static void generate_error_msg_type_conversion_invalid_operands(struct mcc_ast_node node,
                                                                struct mcc_semantic_check *check)
{
    if(check->error_buffer){
        return;
    }
    int size = 45;
    char* error_msg = (char *)malloc( sizeof(char) * size);
    snprintf(error_msg, size, "invalid operands to binary operation.");
    write_error_message_to_check(check, node, error_msg);
    check->status = MCC_SEMANTIC_CHECK_FAIL;
    free(error_msg);
}

// generate error msg for using unary operations on strings or whole arrays
static void generate_error_msg_type_conversion_invalid_operand(struct mcc_ast_node node,
                                                               struct mcc_semantic_check *check)
{
    if(check->error_buffer){
        return;
    }
    int size = 45;
    char* error_msg = (char *)malloc( sizeof(char) * size);
    snprintf(error_msg, size, "invalid operand to unary operation.");
    write_error_message_to_check(check, node, error_msg);
    check->status = MCC_SEMANTIC_CHECK_FAIL;
    free(error_msg);
}

// look up the type of the expression in the symbol table, only possible if expression is of type variable, array
// element or function call
static enum mcc_semantic_check_expression_type look_up_type_in_symbol_table(struct mcc_ast_expression *expression)
{
    assert((expression->type == MCC_AST_EXPRESSION_TYPE_VARIABLE) ||
    (expression->type == MCC_AST_EXPRESSION_TYPE_ARRAY_ELEMENT) ||
    (expression->type == MCC_AST_EXPRESSION_TYPE_FUNCTION_CALL));

    struct mcc_symbol_table_row *row;

    if(expression->type == MCC_AST_EXPRESSION_TYPE_VARIABLE) {
        row = mcc_symbol_table_check_upwards_for_declaration(expression->identifier->identifier_name,
                expression->variable_row);
    } else if(expression->type == MCC_AST_EXPRESSION_TYPE_ARRAY_ELEMENT){
        row = mcc_symbol_table_check_upwards_for_declaration(expression->array_identifier->identifier_name,
                                                             expression->array_row);
    } else if(expression->type == MCC_AST_EXPRESSION_TYPE_FUNCTION_CALL){
        row = mcc_symbol_table_check_for_function_declaration(expression->function_identifier->identifier_name,
                                                              expression->function_row);
    } else {
        return MCC_SEMANTIC_CHECK_EXPRESSION_TYPE_UNKNOWN;
    }

    if(row){
        return convert_enum_symbol_table(row->row_type);
    } else {
        return MCC_SEMANTIC_CHECK_EXPRESSION_TYPE_UNKNOWN;
    }

}

// recursively generate type of expression, returns _TYPE_UNKNOWN if not of valid type, i.e. subexpressions are not
// compatible
static enum mcc_semantic_check_expression_type get_type(struct mcc_ast_expression *expression)
{
    assert(expression);

    enum mcc_semantic_check_expression_type lhs = MCC_SEMANTIC_CHECK_EXPRESSION_TYPE_UNKNOWN; // only used for binary op
    enum mcc_semantic_check_expression_type rhs = MCC_SEMANTIC_CHECK_EXPRESSION_TYPE_UNKNOWN; // only used for binary op

    switch(expression->type){
    case MCC_AST_EXPRESSION_TYPE_LITERAL:
        return convert_enum_ast_literal(expression->literal->type);
    case MCC_AST_EXPRESSION_TYPE_BINARY_OP:
        lhs = get_type(expression->lhs);
        rhs = get_type(expression->rhs);
        break;
    case MCC_AST_EXPRESSION_TYPE_PARENTH:
        return get_type(expression->expression);
    case MCC_AST_EXPRESSION_TYPE_UNARY_OP:
        return get_type(expression->child);
    case MCC_AST_EXPRESSION_TYPE_VARIABLE:
        return look_up_type_in_symbol_table(expression);
    case MCC_AST_EXPRESSION_TYPE_ARRAY_ELEMENT:
        return look_up_type_in_symbol_table(expression);
    case MCC_AST_EXPRESSION_TYPE_FUNCTION_CALL:
        return look_up_type_in_symbol_table(expression);
    }

    if((lhs == rhs) && (lhs != MCC_SEMANTIC_CHECK_EXPRESSION_TYPE_UNKNOWN)
                    && (rhs != MCC_SEMANTIC_CHECK_EXPRESSION_TYPE_UNKNOWN)){ // only necessary for binary op
        return lhs;
    } else {
        return MCC_SEMANTIC_CHECK_EXPRESSION_TYPE_UNKNOWN;
    }
}

// check if given expression is of type bool
static bool is_bool(struct mcc_ast_expression *expression)
{
    assert(expression);

    enum mcc_semantic_check_expression_type type = get_type(expression);

    return (type == MCC_SEMANTIC_CHECK_EXPRESSION_TYPE_BOOL);
}

// check if given expression is variable of type string
static bool is_string(struct mcc_ast_expression *expression)
{
    assert(expression);

    if((expression->type == MCC_AST_EXPRESSION_TYPE_VARIABLE)
            && (get_type(expression) == MCC_SEMANTIC_CHECK_EXPRESSION_TYPE_STRING)){
        return true;
    } else {
        return false;
    }
}

// check if given expression is whole array (not array element)
static bool is_whole_array(struct mcc_ast_expression *expression)
{
    assert(expression);

    if((expression->type == MCC_AST_EXPRESSION_TYPE_VARIABLE)){
        char *name = expression->identifier->identifier_name;
        struct mcc_symbol_table_row *row = expression->variable_row;
        row = mcc_symbol_table_check_upwards_for_declaration(name, row);
        if(row->row_structure == MCC_SYMBOL_TABLE_ROW_STRUCTURE_ARRAY){
            return true;
        }
    }

    return false;
}

// check if given expressions are of same type
static bool is_of_same_type(struct mcc_ast_expression *expression1, struct mcc_ast_expression *expression2)
{
    assert(expression1);
    assert(expression2);

    enum mcc_semantic_check_expression_type type_expr1 = get_type(expression1);
    enum mcc_semantic_check_expression_type type_expr2 = get_type(expression2);

    if((type_expr1 == MCC_SEMANTIC_CHECK_EXPRESSION_TYPE_UNKNOWN)
            || (type_expr2 == MCC_SEMANTIC_CHECK_EXPRESSION_TYPE_UNKNOWN)){
        return false;
    }
    return (type_expr1 == type_expr2);
}

// callback for checking implicit type conversions in binary operation expressions, since we visit post order the
// innermost expression is visited first
static void cb_type_conversion_expression_binary_op(struct mcc_ast_expression *expression, void *data)
{
    assert(expression);
    assert(data);

    struct mcc_semantic_check *check = data;
    struct mcc_ast_expression *lhs = expression->lhs;
    struct mcc_ast_expression *rhs = expression->rhs;

    if(is_string(lhs) || is_string(rhs) || is_whole_array(lhs) || is_whole_array(rhs)){
        generate_error_msg_type_conversion_invalid_operands(expression->node, check);
        return;
    }

    bool is_permitted_op = true;

    switch(expression->op){
    case MCC_AST_BINARY_OP_ADD:
        is_permitted_op = is_of_same_type(lhs, rhs) && !is_bool(lhs) && !is_bool(rhs);
        break;
    case MCC_AST_BINARY_OP_SUB:
        is_permitted_op = is_of_same_type(lhs, rhs) && !is_bool(lhs) && !is_bool(rhs);
        break;
    case MCC_AST_BINARY_OP_MUL:
        is_permitted_op = is_of_same_type(lhs, rhs) && !is_bool(lhs) && !is_bool(rhs);
        break;
    case MCC_AST_BINARY_OP_DIV:
        is_permitted_op = is_of_same_type(lhs, rhs) && !is_bool(lhs) && !is_bool(rhs);
        break;
    case MCC_AST_BINARY_OP_SMALLER:
        is_permitted_op = is_of_same_type(lhs, rhs) && !is_bool(lhs) && !is_bool(rhs);
        break;
    case MCC_AST_BINARY_OP_GREATER:
        is_permitted_op = is_of_same_type(lhs, rhs) && !is_bool(lhs) && !is_bool(rhs);
        break;
    case MCC_AST_BINARY_OP_SMALLEREQ:
        is_permitted_op = is_of_same_type(lhs, rhs) && !is_bool(lhs) && !is_bool(rhs);
        break;
    case MCC_AST_BINARY_OP_GREATEREQ:
        is_permitted_op = is_of_same_type(lhs, rhs) && !is_bool(lhs) && !is_bool(rhs);
        break;
    case MCC_AST_BINARY_OP_CONJ:
        is_permitted_op = is_bool(lhs) && is_bool(rhs); // Conjunction can only be used with bool
        break;
    case MCC_AST_BINARY_OP_DISJ:
        is_permitted_op = is_bool(lhs) && is_bool(rhs); // Disjunction can only be used with bool
        break;
    case MCC_AST_BINARY_OP_EQUAL:
        is_permitted_op = is_of_same_type(lhs, rhs);
        break;
    case MCC_AST_BINARY_OP_NOTEQUAL:
        is_permitted_op = is_of_same_type(lhs, rhs);
        break;
    }

    // since we visit post order the innermost expression is visited first
    if((!is_permitted_op) && ((expression->op == MCC_AST_BINARY_OP_CONJ)||(expression->op == MCC_AST_BINARY_OP_DISJ))){
        generate_error_msg_type_conversion_non_boolean(expression->node, check);
        return;
    }
    if(!is_permitted_op){
        generate_error_msg_type_conversion_expression(expression->node, check);
    }
}

// callback for checking implicit type conversions in unary operation expressions, since we visit post order the
// innermost expression is visited first
static void cb_type_conversion_expression_unary_op(struct mcc_ast_expression *expression, void *data)
{
    assert(expression);
    assert(data);

    struct mcc_semantic_check *check = data;
    struct mcc_ast_expression *child = expression->child;
    bool child_is_bool = is_bool(child);

    if(is_string(child) || is_whole_array(child)){
        generate_error_msg_type_conversion_invalid_operand(expression->node, check);
        return;
    }

    bool is_permitted_op = true;

    switch(expression->u_op){
    case MCC_AST_UNARY_OP_NEGATIV:
        is_permitted_op = !child_is_bool;
        break;
    case MCC_AST_UNARY_OP_NOT:
        is_permitted_op = child_is_bool;
        break;
    }

    // since we visit post order the innermost expression is visited first
    if((!is_permitted_op) && (expression->u_op == MCC_AST_UNARY_OP_NOT)){
        generate_error_msg_type_conversion_non_boolean(expression->node, check);
        return;
    }
    if(!is_permitted_op){
        generate_error_msg_type_conversion_expression(expression->node, check);
    }
}

// generate error message if condition of if statement is not of type bool
static void generate_error_msg_type_conversion_statement_if(struct mcc_ast_node node, struct mcc_semantic_check *check)
{
    if(check->error_buffer){
        return;
    }
    int size = 60;
    char* error_msg = (char *)malloc( sizeof(char) * size);
    snprintf(error_msg, size, "condition of if statement expected to be of type 'bool'.");
    write_error_message_to_check(check, node, error_msg);
    check->status = MCC_SEMANTIC_CHECK_FAIL;
    free(error_msg);
}

// generate error message if condition of while loop is not of type bool
static void generate_error_msg_type_conversion_statement_while(struct mcc_ast_node node,
                                                               struct mcc_semantic_check *check)
{
    if(check->error_buffer){
        return;
    }
    int size = 65;
    char* error_msg = (char *)malloc( sizeof(char) * size);
    snprintf(error_msg, size, "condition of while loop expected to be of type 'bool'.");
    write_error_message_to_check(check, node, error_msg);
    check->status = MCC_SEMANTIC_CHECK_FAIL;
    free(error_msg);
}

// callback for checking if the condition in an if statement is of type bool
static void cb_type_conversion_statement_if_stmt(struct mcc_ast_statement *statement, void *data)
{
    assert(statement);
    assert(data);

    struct mcc_semantic_check *check = data;

    if(get_type(statement->if_condition) != MCC_SEMANTIC_CHECK_EXPRESSION_TYPE_BOOL){
        generate_error_msg_type_conversion_statement_if(statement->if_condition->node, check);
    }
}

// callback for checking if the condition in an if_else statement is of type bool
static void cb_type_conversion_statement_if_else_stmt(struct mcc_ast_statement *statement, void *data)
{
    assert(statement);
    assert(data);

    struct mcc_semantic_check *check = data;

    if(get_type(statement->if_else_condition) != MCC_SEMANTIC_CHECK_EXPRESSION_TYPE_BOOL){
        generate_error_msg_type_conversion_statement_if(statement->if_else_condition->node, check);
    }
}

// callback for checking if the condition in a while loop is of type bool
static void cb_type_conversion_statement_while(struct mcc_ast_statement *statement, void *data)
{
    assert(statement);
    assert(data);

    struct mcc_semantic_check *check = data;

    if(get_type(statement->while_condition) != MCC_SEMANTIC_CHECK_EXPRESSION_TYPE_BOOL){
        generate_error_msg_type_conversion_statement_while(statement->while_condition->node, check);
    }
}

// generate error message for type conversion in assignment
static void generate_error_msg_type_conversion_assignment(struct mcc_ast_assignment *assignment,
                                                          struct mcc_semantic_check *check)
{
    assert(assignment);
    assert(check);

    if(check->error_buffer){
        return;
    }

    char* name;
    char* var_or_array;

    switch(assignment->assignment_type){
    case MCC_AST_ASSIGNMENT_TYPE_VARIABLE:
        name = assignment->variable_identifier->identifier_name;
        var_or_array = "variable";
        break;
    case MCC_AST_ASSIGNMENT_TYPE_ARRAY:
        name = assignment->array_identifier->identifier_name;
        var_or_array = "array";
        break;
    }

    int size = 50 + strlen(name);
    char* error_msg = (char *)malloc( sizeof(char) * size);
    snprintf(error_msg, size, "implicit type conversion of %s '%s'.", var_or_array, name);
    write_error_message_to_check(check, assignment->node, error_msg);
    check->status = MCC_SEMANTIC_CHECK_FAIL;
    free(error_msg);
}

// callback for checking type conversion in an assignment
static void cb_type_conversion_assignment(struct mcc_ast_statement *statement, void *data)
{
    assert(statement);
    assert(data);

    struct mcc_semantic_check *check = data;
    struct mcc_ast_assignment *assignment = statement->assignment;
    struct mcc_symbol_table_row *row = assignment->row;

    bool is_permitted = false;

    switch(assignment->assignment_type){
    case MCC_AST_ASSIGNMENT_TYPE_VARIABLE:
        row = mcc_symbol_table_check_upwards_for_declaration(assignment->variable_identifier->identifier_name, row);
        is_permitted = (convert_enum_symbol_table(row->row_type) == get_type(assignment->variable_assigned_value));
        break;
    case MCC_AST_ASSIGNMENT_TYPE_ARRAY:
        row = mcc_symbol_table_check_upwards_for_declaration(assignment->array_identifier->identifier_name, row);
        is_permitted = (convert_enum_symbol_table(row->row_type) == get_type(assignment->array_assigned_value));
        break;
    }

    if(!is_permitted){
        generate_error_msg_type_conversion_assignment(assignment, check);
    }
}

// Setup an AST Visitor for checking types within expressions and if conditions are of type bool
static struct mcc_ast_visitor type_conversion_visitor(struct mcc_semantic_check *check)
{

    return (struct mcc_ast_visitor){
            .traversal = MCC_AST_VISIT_DEPTH_FIRST,
            .order = MCC_AST_VISIT_POST_ORDER,

            .userdata = check,

            .expression_binary_op = cb_type_conversion_expression_binary_op,
            .expression_unary_op = cb_type_conversion_expression_unary_op,
            .statement_if_stmt = cb_type_conversion_statement_if_stmt,
            .statement_if_else_stmt = cb_type_conversion_statement_if_else_stmt,
            .statement_while = cb_type_conversion_statement_while,
            .statement_assignment = cb_type_conversion_assignment,
    };
}

// check for type conversion in expressions and if expressions used as conditions are of type bool
struct mcc_semantic_check* mcc_semantic_check_run_type_conversion(struct mcc_ast_program* ast,
                                                                  struct mcc_symbol_table* symbol_table){
    UNUSED(symbol_table);

    struct mcc_semantic_check *check = malloc(sizeof(*check));
    if (!check){
        return NULL;
    }

    check->status = MCC_SEMANTIC_CHECK_OK;
    check->type = MCC_SEMANTIC_CHECK_TYPE_CONVERSION;
    check->error_buffer = NULL;

    struct mcc_ast_visitor visitor = type_conversion_visitor(check);
    mcc_ast_visit(ast, &visitor);
    return check;
}

// ------------------------------------------------------------- No invalid array operations

struct mcc_semantic_check* mcc_semantic_check_run_array_types(struct mcc_ast_program* ast,
                                                              struct mcc_symbol_table* symbol_table){
    UNUSED(ast);
    UNUSED(symbol_table);
    return NULL;
}

// ------------------------------------------------------------- No invalid function calls

struct mcc_semantic_check* mcc_semantic_check_run_function_arguments(struct mcc_ast_program* ast,
                                                                     struct mcc_symbol_table* symbol_table){
    UNUSED(ast);
    UNUSED(symbol_table);
    return NULL;
}

// -------------------------------------------------------------- Each execution path of non-void function returns a
//                                                                value

// generate error msg for nonvoid check
static void generate_error_msg_failed_nonvoid_check(const char *name,
                                                    struct mcc_ast_node node,
                                                    struct mcc_semantic_check *check)
{
    if(check->error_buffer){
        return;
    }
    int size = 50 + strlen(name);
    char* error_msg = (char *)malloc( sizeof(char) * size);
    snprintf(error_msg, size, "control reaches end of non-void function '%s'.", name);
    write_error_message_to_check(check, node, error_msg);
    check->status = MCC_SEMANTIC_CHECK_FAIL;
    free(error_msg);
}

// check a single statement on non-void property, i.e. either success if return or invoke recursive check for if_else
static bool check_nonvoid_property(struct mcc_ast_statement *statement)
{
    assert(statement);

    bool is_successful = false;

    switch(statement->type){
    case MCC_AST_STATEMENT_TYPE_IF_STMT:
        // do nothing
        break;
    case MCC_AST_STATEMENT_TYPE_IF_ELSE_STMT:
        is_successful = check_nonvoid_property(statement->if_else_on_true)
                && check_nonvoid_property(statement->if_else_on_false);
        break;
    case MCC_AST_STATEMENT_TYPE_EXPRESSION:
        // do nothing
        break;
    case MCC_AST_STATEMENT_TYPE_WHILE:
        // do nothing
        break;
    case MCC_AST_STATEMENT_TYPE_DECLARATION:
        // do nothing
        break;
    case MCC_AST_STATEMENT_TYPE_ASSIGNMENT:
        // do nothing
        break;
    case MCC_AST_STATEMENT_TYPE_RETURN:
        is_successful = true;
        break;
    case MCC_AST_STATEMENT_TYPE_COMPOUND_STMT:
        is_successful = recursively_check_nonvoid_property(statement->compound_statement);
        break;
    }

    return is_successful;
}

// recursively check non-void property, i.e. all execution paths end in a return
static bool recursively_check_nonvoid_property(struct mcc_ast_compound_statement *compound_statement)
{
    assert(compound_statement);

    bool is_successful = false;

    // check recursively statements, start with last compound_statement
    if(compound_statement->next_compound_statement){
        is_successful = recursively_check_nonvoid_property(compound_statement->next_compound_statement);
    }
    // if not successfully found any return on all execution paths
    if(is_successful == false && compound_statement->statement){
        struct mcc_ast_statement *statement = compound_statement->statement;
        is_successful = check_nonvoid_property(statement);
    }

    return is_successful;
}

static void run_nonvoid_check(struct mcc_ast_function_definition *function, struct mcc_semantic_check *check)
{
    assert(function);
    assert(check);

    bool is_successful = false;

    if(function->type != MCC_AST_FUNCTION_TYPE_VOID){
        is_successful = recursively_check_nonvoid_property(function->compound_stmt);
    } else {
        is_successful = true;
    }

    if(is_successful == false){
        generate_error_msg_failed_nonvoid_check(function->identifier->identifier_name, function->node, check);
    }
}

// run non-void check
struct mcc_semantic_check* mcc_semantic_check_run_nonvoid_check(struct mcc_ast_program* ast,
                                                                struct mcc_symbol_table* symbol_table){
    UNUSED(symbol_table);
    assert(ast);

    struct mcc_semantic_check* check = malloc(sizeof(*check));
    if (!check){
        return NULL;
    }

    check->status = MCC_SEMANTIC_CHECK_OK;
    check->type = MCC_SEMANTIC_CHECK_NONVOID_CHECK;
    check->error_buffer = NULL;

    do{
        run_nonvoid_check(ast->function, check);

        ast = ast->next_function;
    } while (ast);

    return check;
}

// -------------------------------------------------------------- Main function exists and has correct signature
struct mcc_semantic_check* mcc_semantic_check_run_main_function(struct mcc_ast_program* ast,
                                                                struct mcc_symbol_table* symbol_table){
    UNUSED(symbol_table);

    struct mcc_semantic_check* check = malloc(sizeof(*check));
    if (!check){
        return NULL;
    }

    check->status = MCC_SEMANTIC_CHECK_OK;
    check->type = MCC_SEMANTIC_CHECK_MAIN_FUNCTION;
    check->error_buffer = NULL;

    int number_of_mains = 0;

    if (strcmp(ast->function->identifier->identifier_name,"main")==0){
        number_of_mains += 1;
        if(!(ast->function->parameters->is_empty)){
            write_error_message_to_check(check,ast->function->node,"Main has wrong signature. Must be `int main()`");
            check->status = MCC_SEMANTIC_CHECK_FAIL;
            return check;
        }
    }
    while (ast->has_next_function){
        ast = ast->next_function;
        if (strcmp(ast->function->identifier->identifier_name,"main")==0){
            number_of_mains += 1;
            if (number_of_mains > 1){
                write_error_message_to_check(check,ast->function->node,"Too many main functions defined.");
                check->status = MCC_SEMANTIC_CHECK_FAIL;
            }
            if(!(ast->function->parameters->is_empty)){
                write_error_message_to_check(check,ast->function->node,"Main has wrong signature. Must be `int main()`");
                check->status = MCC_SEMANTIC_CHECK_FAIL;
                return check;
            }
        }
    }
    if (number_of_mains == 0){
        write_error_message_to_check(check,ast->node,"No main function defined.");
        check->status = MCC_SEMANTIC_CHECK_FAIL;
        return check;
    }


    return check;
}


// -------------------------------------------------------------- No Calls to unknown functions

// generate error message
static void generate_error_msg_unknown_function_call(const char* name,
                                                           struct mcc_ast_node node,
                                                           struct mcc_semantic_check *check)
{
    if(check->error_buffer){
        return;
    }
    int size = 60 + strlen(name);
    char* error_msg = (char *)malloc( sizeof(char) * size);
    snprintf(error_msg, size, "function  '%s' undeclared (first use in this function).", name);
    write_error_message_to_check(check, node, error_msg);
    check->status = MCC_SEMANTIC_CHECK_FAIL;
    free(error_msg);
}

// callback for check of call to unknown function
static void cb_unknown_function_call(struct mcc_ast_expression *expression, void *data)
{
    assert(expression);
    assert(data);

    struct mcc_semantic_check *check = data;
    struct mcc_symbol_table_row *row = expression->function_row;
    char* name = expression->function_identifier->identifier_name;

    struct mcc_symbol_table_row *function_declaration = mcc_symbol_table_check_for_function_declaration(name, row);

    if(!function_declaration){
        generate_error_msg_unknown_function_call(name, expression->node, check);
    }
}

// Setup an AST Visitor for checking calls to unknown functions.
static struct mcc_ast_visitor unknown_function_call_visitor(struct mcc_semantic_check *check)
{

    return (struct mcc_ast_visitor){
            .traversal = MCC_AST_VISIT_DEPTH_FIRST,
            .order = MCC_AST_VISIT_PRE_ORDER,

            .userdata = check,

            .expression_function_call = cb_unknown_function_call,
    };
}

// check for calls to unknown functions
struct mcc_semantic_check* mcc_semantic_check_run_unknown_function_call(struct mcc_ast_program* ast,
                                                                        struct mcc_symbol_table* symbol_table){

    UNUSED(symbol_table);

    assert(ast);
    struct mcc_semantic_check *check = malloc(sizeof(*check));
    if (!check){
        return NULL;
    }

    check->status = MCC_SEMANTIC_CHECK_OK;
    check->type = MCC_SEMANTIC_CHECK_UNKNOWN_FUNCTION_CALL;
    check->error_buffer = NULL;

    struct mcc_ast_visitor visitor = unknown_function_call_visitor(check);
    mcc_ast_visit(ast, &visitor);
    return check;
}

// -------------------------------------------------------------- No multiple definitions of the same function
// generate error message
static void generate_error_msg_multiple_function_defintion(const char* name,
                                                           struct mcc_ast_program *program,
                                                           struct mcc_semantic_check *check)
{
    assert(program);
    assert(check);

    if(check->error_buffer){
        return;
    }
    int size = 20 + strlen(name);
    char* error_msg = (char *)malloc( sizeof(char) * size);
    snprintf(error_msg, size, "redefinition of '%s'", name);
    write_error_message_to_check(check,program->node, error_msg);
    check->status = MCC_SEMANTIC_CHECK_FAIL;
    free(error_msg);
}

// check for no multiple definitions of the same function
struct mcc_semantic_check* mcc_semantic_check_run_multiple_function_definitions(struct mcc_ast_program* ast,
                                                                                struct mcc_symbol_table* symbol_table){
    UNUSED(symbol_table);

    assert(ast);
    struct mcc_semantic_check *check = malloc(sizeof(*check));
    if (!check){
        return NULL;
    }

    check->status = MCC_SEMANTIC_CHECK_OK;
    check->type = MCC_SEMANTIC_CHECK_MULTIPLE_FUNCTION_DEFINITIONS;
    check->error_buffer = NULL;

    struct mcc_ast_program *program_to_check = ast;

    // Program has only one function
    if(!program_to_check->next_function){
        return check;
    }

    while(program_to_check->next_function){
        struct mcc_ast_program *program_to_compare = program_to_check->next_function;
        char *name_of_check = program_to_check->function->identifier->identifier_name;
        char *name_of_compare = program_to_compare->function->identifier->identifier_name;

        // if name of program_to_check and name of program_to_compare equals
        if(strcmp(name_of_check, name_of_compare) == 0){
            generate_error_msg_multiple_function_defintion(name_of_compare, program_to_compare, check);
            return check;
        }
        // compare all next_functions
        while(program_to_compare->next_function){
            program_to_compare = program_to_compare->next_function;
            char *name_of_compare = program_to_compare->function->identifier->identifier_name;
            // if name of program_to_check and name of program_to_compare equals
            if(strcmp(name_of_check, name_of_compare) == 0){
                generate_error_msg_multiple_function_defintion(name_of_compare, program_to_compare, check);
                return check;
            }
        }

        program_to_check = program_to_check->next_function;
    }

    return check;
}

// -------------------------------------------------------------- No multiple declarations of a variable in the same
//                                                                scope
// generate error meassage
static void generate_error_msg_multiple_variable_declaration(struct mcc_symbol_table_row *row,
                                                             struct mcc_semantic_check *check)
{
    assert(row);
    assert(check);

    if(check->error_buffer){
        return;
    }

    int size = 20 + strlen(row->name);
    char* error_msg = (char *)malloc( sizeof(char) * size);
    snprintf(error_msg, size, "redefinition of '%s'", row->name);
    write_error_message_to_check(check,*(row->node), error_msg);
    check->status = MCC_SEMANTIC_CHECK_FAIL;
    free(error_msg);
}

// check scopes individually
static void check_scope_for_multiple_variable_declaration(struct mcc_symbol_table_scope *scope,
                                                          struct mcc_semantic_check *check)
{
    assert(scope);
    assert(check);

    if(!scope->head){
        return;
    }

    struct mcc_symbol_table_row *row_to_check = scope->head;

    if(row_to_check->child_scope){
        check_scope_for_multiple_variable_declaration(row_to_check->child_scope, check);
    }

    while(row_to_check->next_row){
        struct mcc_symbol_table_row *row_to_compare = row_to_check->next_row;

        if(strcmp(row_to_check->name, row_to_compare->name) == 0){
            generate_error_msg_multiple_variable_declaration(row_to_compare, check);
            return;
        }

        while(row_to_compare->next_row){
            row_to_compare = row_to_compare->next_row;
            if(strcmp(row_to_check->name, row_to_compare->name) == 0){
                generate_error_msg_multiple_variable_declaration(row_to_compare, check);
                return;
            }
        }

        row_to_check = row_to_check->next_row;
    }

    if(row_to_check->child_scope){
        check_scope_for_multiple_variable_declaration(row_to_check->child_scope, check);
    }

}

// check for no multiple declarations of a variable in the same scope
struct mcc_semantic_check* mcc_semantic_check_run_multiple_variable_declarations(struct mcc_ast_program* ast,
                                                                                 struct mcc_symbol_table* symbol_table){
    UNUSED(ast);

    assert(symbol_table);
    struct mcc_semantic_check *check = malloc(sizeof(*check));
    if (!check){
        return NULL;
    }

    check->status = MCC_SEMANTIC_CHECK_OK;
    check->type = MCC_SEMANTIC_CHECK_MULTIPLE_VARIABLE_DECLARATIONS;
    check->error_buffer = NULL;

    struct mcc_symbol_table_scope *scope = symbol_table->head;
    struct mcc_symbol_table_row *function_row = scope->head;

    if(function_row->child_scope){
        check_scope_for_multiple_variable_declaration(function_row->child_scope, check);
    }

    while(function_row->next_row){
        function_row = function_row->next_row;
        if(function_row->child_scope){
            check_scope_for_multiple_variable_declaration(function_row->child_scope, check);
        }
    }

    return check;
}
// -------------------------------------------------------------- No use of undeclared variables

// generate messaage
static void generate_error_msg_undeclared_variable(const char *name, struct mcc_ast_node node,
                                                       struct mcc_semantic_check *check)
{
    assert(check);

    if(check->error_buffer){
        return;
    }
    int size = 50 + strlen(name);
    char* error_msg = (char *)malloc( sizeof(char) * size);
    snprintf(error_msg, size, "'%s' undeclared (first use in this function).", name);
    write_error_message_to_check(check, node, error_msg);
    check->status = MCC_SEMANTIC_CHECK_FAIL;
    free(error_msg);
}

// callback for expression of variable type concerning the check of undeclared variables
static void cb_use_undeclared_variable(struct mcc_ast_expression *expression, void *data)
{
    assert(expression);
    assert(data);

    struct mcc_semantic_check *check = data;
    struct mcc_symbol_table_row *row = expression->variable_row;
    char* name = expression->identifier->identifier_name;

    struct mcc_symbol_table_row *upward_declaration = mcc_symbol_table_check_upwards_for_declaration(name, row);

    if(!upward_declaration){
        generate_error_msg_undeclared_variable(name, expression->node, check);
    }
}

// callback for expression of array type concerning the check of undeclared variables
static void cb_use_undeclared_array(struct mcc_ast_expression *expression, void *data)
{
    assert(expression);
    assert(data);

    struct mcc_semantic_check *check = data;
    struct mcc_symbol_table_row *row = expression->array_row;
    char* name = expression->array_identifier->identifier_name;

    struct mcc_symbol_table_row *upward_declaration = mcc_symbol_table_check_upwards_for_declaration(name, row);

    if(!upward_declaration){
        generate_error_msg_undeclared_variable(name, expression->node, check);
    }
}

// callback for assignment of variable type concerning the check of undeclared variables
static void cb_use_undeclared_variable_assignment(struct mcc_ast_assignment *assignment, void *data)
{
    assert(assignment);
    assert(data);

    struct mcc_semantic_check *check = data;
    struct mcc_symbol_table_row *row = assignment->row;
    char* name = assignment->variable_identifier->identifier_name;

    struct mcc_symbol_table_row *upward_declaration = mcc_symbol_table_check_upwards_for_declaration(name, row);

    if(!upward_declaration){
        generate_error_msg_undeclared_variable(name, assignment->node, check);
    }
}

// callback for assignment of array type concerning the check of undeclared variables
static void cb_use_undeclared_array_assignment(struct mcc_ast_assignment *assignment, void *data)
{
    assert(assignment);
    assert(data);

    struct mcc_semantic_check *check = data;
    struct mcc_symbol_table_row *row = assignment->row;
    char* name = assignment->array_identifier->identifier_name;

    struct mcc_symbol_table_row *upward_declaration = mcc_symbol_table_check_upwards_for_declaration(name, row);

    if(!upward_declaration){
        generate_error_msg_undeclared_variable(name, assignment->node, check);
    }
}

// Setup an AST Visitor for checking undeclared variables.
static struct mcc_ast_visitor use_undeclared_variable_visitor(struct mcc_semantic_check *check)
{

    return (struct mcc_ast_visitor){
            .traversal = MCC_AST_VISIT_DEPTH_FIRST,
            .order = MCC_AST_VISIT_PRE_ORDER,

            .userdata = check,

            .expression_variable = cb_use_undeclared_variable,
            .expression_array_element = cb_use_undeclared_array,

            .variable_assignment = cb_use_undeclared_variable_assignment,
            .array_assignment = cb_use_undeclared_array_assignment,
    };
}

// No use of undeclared variables - the actual check
struct mcc_semantic_check* mcc_semantic_check_run_use_undeclared_variable(struct mcc_ast_program* ast,
                                                                          struct mcc_symbol_table* symbol_table){
    UNUSED(symbol_table);

    assert(ast);
    struct mcc_semantic_check *check = malloc(sizeof(*check));
    if (!check){
        return NULL;
    }

    check->status = MCC_SEMANTIC_CHECK_OK;
    check->type = MCC_SEMANTIC_CHECK_USE_UNDECLARED_VARIABLE;
    check->error_buffer = NULL;

    struct mcc_ast_visitor visitor = use_undeclared_variable_visitor(check);
    mcc_ast_visit(ast, &visitor);
    return check;
}

// -------------------------------------------------------------- No use of the names of the built_in functions in
//                                                                function definitions
struct mcc_semantic_check* mcc_semantic_check_run_define_built_in(struct mcc_ast_program* ast,
                                                                  struct mcc_symbol_table* symbol_table){
    UNUSED(symbol_table);

    struct mcc_semantic_check* check = malloc(sizeof(*check));
    if (!check){
        return NULL;
    }

    check->status = MCC_SEMANTIC_CHECK_OK;
    check->type = MCC_SEMANTIC_CHECK_DEFINE_BUILT_IN;
    check->error_buffer = NULL;

    if (!(ast->function)){
        return check;
    }


    // Check if we encounter built_ins as userdefined functions
    // Since we walk the AST, the first encounter already is an error, the built_ins
    // are only later added to the symbol table

    do {

        if (strcmp(ast->function->identifier->identifier_name,"print")==0){
            write_error_message_to_check(check,ast->function->node,"Multiple definitions of function `print` found."
                                                                   "`print` is reserved for the built_in function.");
            check->status = MCC_SEMANTIC_CHECK_FAIL;
            return check;
        }
        if (strcmp(ast->function->identifier->identifier_name,"print_nl")==0){
            write_error_message_to_check(check,ast->function->node,"Multiple definitions of function `print_nl` found."
                                                                   "`print_nl` is reserved for the built_in function.");
            check->status = MCC_SEMANTIC_CHECK_FAIL;
            return check;
        }
        if (strcmp(ast->function->identifier->identifier_name,"print_int")==0){
            write_error_message_to_check(check,ast->function->node,"Multiple definitions of function `print_int` found."
                                                                   "`print_int` is reserved for the built_in function.");
            check->status = MCC_SEMANTIC_CHECK_FAIL;
            return check;
        }
        if (strcmp(ast->function->identifier->identifier_name,"print_float")==0){
            write_error_message_to_check(check,ast->function->node,"Multiple definitions of function `print_float` found."
                                                                   "`print_float` is reserved for the built_in function.");
            check->status = MCC_SEMANTIC_CHECK_FAIL;
            return check;
        }
        if (strcmp(ast->function->identifier->identifier_name,"read_int")==0){
            write_error_message_to_check(check,ast->function->node,"Multiple definitions of function `read_int` found."
                                                                   "`read_int` is reserved for the built_in function.");
            check->status = MCC_SEMANTIC_CHECK_FAIL;
            return check;
        }
        if (strcmp(ast->function->identifier->identifier_name,"read_float")==0){
            write_error_message_to_check(check,ast->function->node,"Multiple definitions of function `read_float` found."
                                                                   "`read_float` is reserved for the built_in function.");
            check->status = MCC_SEMANTIC_CHECK_FAIL;
            return check;
        }

        ast = ast->next_function;

    } while (ast);

    return check;
}

// ------------------------------------------------------------- Functions: Cleanup

// Delete all checks
void mcc_semantic_check_delete_all_checks(struct mcc_semantic_check_all_checks *checks){


    /*if (checks->error_buffer != NULL){
        free(checks->error_buffer);
    }*/
    if (checks->nonvoid_check != NULL)
    {
        mcc_semantic_check_delete_single_check(checks->nonvoid_check);
    }
    if (checks->main_function != NULL)
    {
        mcc_semantic_check_delete_single_check(checks->main_function);
    }
    if (checks->unknown_function_call != NULL)
    {
        mcc_semantic_check_delete_single_check(checks->unknown_function_call);
    }
    if (checks->multiple_function_definitions != NULL)
    {
        mcc_semantic_check_delete_single_check(checks->multiple_function_definitions);
    }
    if (checks->multiple_variable_declarations != NULL)
    {
        mcc_semantic_check_delete_single_check(checks->multiple_variable_declarations);
    }
    if (checks->use_undeclared_variable != NULL)
    {
        mcc_semantic_check_delete_single_check(checks->use_undeclared_variable);
    }
    if (checks->define_built_in != NULL)
    {
        mcc_semantic_check_delete_single_check(checks->define_built_in);
    }
    if (checks->error_buffer != NULL){
        free(checks->error_buffer);
    }
    if (checks->type_conversion != NULL)
    {
        mcc_semantic_check_delete_single_check(checks->type_conversion);
    }
    free(checks);
    return;
}

// Delete single checks
void mcc_semantic_check_delete_single_check(struct mcc_semantic_check *check){

    if (check == NULL){
        return;
    }

    if (check->error_buffer != NULL){
        free(check->error_buffer);
    }

    free(check);
}

