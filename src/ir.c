#include "mcc/ir.h"

#include <assert.h>
#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "mcc/ast.h"
#include "mcc/ast_visit.h"
#include "mcc/symbol_table.h"
#include "utils/unused.h"

#define not_zero(x) (x > 0 ? x : 1)

struct ir_generation_userdata {
	struct mcc_ir_row *head;
	struct mcc_ir_row *current;
	bool has_failed;
	unsigned label_counter;
};

/* TODO:

 *    expression                :    DONE (Just update when other expression functions are added)
 *    expression_literal        :    DONE
 *    expression_binary_op      :    DONE
 *    expression_parenth        :    DONE
 *    expression_unary_op       :    DONE
 *    expression_variable       :    DONE
 *    expression_array_element  :    DONE
 *    expression_function_call  :    DONE

 *    literal                   :    DONE
 *    literal_int               :    DONE
 *    literal_float             :    DONE
 *    literal_bool              :    DONE
 *    literal_string            :    DONE

 *    statement                 :    DONE (Just update when other statement functions are added)
 *    statement_if_stmt         :    DONE
 *    statement_if_else_stmt    :    DONE
 *    statement_expression_stmt :    DONE
 *    statement_while           :    DONE
 *    statement_assignment      :    DONE
 *    statement_declaration     :    DONE
 *    statement_return          :    DONE
 *    statement_compound_stmt   :    DONE

 *    compound_statement        :    DONE
 *    program                   :    TODO
 *    function_definition       :    Need handle passed arrays
 *    parameters                :    TODO
 *    arguments                 :    TODO

 *    assignment                :
 *    variable_assignment       :    DONE
 *    array_assignment          :    DONE
 *    declaration               :    DONE
 *    variable_declaration      :    Not needed!
 *    array_declaration         :    DONE

 *    type                      :    Not needed?
 *    identifier                :    Not needed?

*/

//------------------------------------------------------------------------------ Forward declarations: IR datastructures

static struct mcc_ir_arg *arg_from_declaration(struct mcc_ast_declaration *decl, struct ir_generation_userdata *data);
static struct mcc_ir_arg *new_arg_func_label(struct mcc_ast_function_definition *def,
                                             struct ir_generation_userdata *data);
static struct mcc_ir_row *new_row(struct mcc_ir_arg *arg1,
                                  struct mcc_ir_arg *arg2,
                                  enum mcc_ir_instruction instr,
                                  struct ir_generation_userdata *data);
static struct mcc_ir_arg *copy_arg(struct mcc_ir_arg *arg, struct ir_generation_userdata *data);
static struct mcc_ir_arg *new_arg_int(long lit, struct ir_generation_userdata *data);
static struct mcc_ir_arg *new_arg_float(double lit, struct ir_generation_userdata *data);
static struct mcc_ir_arg *new_arg_bool(bool lit, struct ir_generation_userdata *data);
static struct mcc_ir_arg *new_arg_string(char *lit, struct ir_generation_userdata *data);
static struct mcc_ir_arg *new_arg_row(struct mcc_ir_row *row, struct ir_generation_userdata *data);
static struct mcc_ir_arg *new_arg_label(struct ir_generation_userdata *data);
static struct mcc_ir_arg *new_arg_identifier(struct mcc_ast_identifier *ident, struct ir_generation_userdata *data);
static struct mcc_ir_arg *
new_arg_arr_elem(struct mcc_ast_identifier *ident, struct mcc_ir_arg *elem, struct ir_generation_userdata *data);
static void append_row(struct mcc_ir_row *row, struct ir_generation_userdata *data);
static struct mcc_ir_arg *generate_arg_lit(struct mcc_ast_literal *literal, struct ir_generation_userdata *data);

//------------------------------------------------------------------------------ Forward declarations: IR generation

static void generate_ir_comp_statement(struct mcc_ast_compound_statement *cmp_stmt,
                                       struct ir_generation_userdata *data);
static void generate_ir_statement(struct mcc_ast_statement *stmt, struct ir_generation_userdata *data);
static void generate_ir_assignment(struct mcc_ast_assignment *asgn, struct ir_generation_userdata *data);
static void generate_ir_statememt_while_stmt(struct mcc_ast_statement *stmt, struct ir_generation_userdata *data);
static void generate_ir_statememt_if_stmt(struct mcc_ast_statement *stmt, struct ir_generation_userdata *data);
static void generate_ir_statememt_if_else_stmt(struct mcc_ast_statement *stmt, struct ir_generation_userdata *data);
static void generate_ir_statement_return(struct mcc_ast_statement *stmt, struct ir_generation_userdata *data);
static void generate_ir_program(struct mcc_ast_program *program, struct ir_generation_userdata *data);
static void generate_ir_function_definition(struct mcc_ast_function_definition *def,
                                            struct ir_generation_userdata *data);
static struct mcc_ir_arg *generate_ir_expression(struct mcc_ast_expression *expression,
                                                 struct ir_generation_userdata *data);
static struct mcc_ir_arg *generate_ir_expression_var(struct mcc_ast_expression *expression,
                                                     struct ir_generation_userdata *data);
static struct mcc_ir_arg *generate_ir_expression_unary_op(struct mcc_ast_expression *expression,
                                                          struct ir_generation_userdata *data);
static struct mcc_ir_arg *generate_ir_expression_binary_op(struct mcc_ast_expression *expression,
                                                           struct ir_generation_userdata *data);

//------------------------------------------------------------------------------ IR generation

static struct mcc_ir_arg *generate_ir_expression_binary_op(struct mcc_ast_expression *expression,
                                                           struct ir_generation_userdata *data)
{
	assert(expression->lhs);
	assert(expression->rhs);
	assert(data);
	if (data->has_failed)
		return NULL;

	struct mcc_ir_arg *lhs = generate_ir_expression(expression->lhs, data);
	struct mcc_ir_arg *rhs = generate_ir_expression(expression->rhs, data);

	enum mcc_ir_instruction instr = MCC_IR_INSTR_UNKNOWN;
	switch (expression->op) {
	case MCC_AST_BINARY_OP_ADD:
		instr = MCC_IR_INSTR_PLUS;
		break;
	case MCC_AST_BINARY_OP_SUB:
		instr = MCC_IR_INSTR_MINUS;
		break;
	case MCC_AST_BINARY_OP_MUL:
		instr = MCC_IR_INSTR_MULTIPLY;
		break;
	case MCC_AST_BINARY_OP_DIV:
		instr = MCC_IR_INSTR_DIVIDE;
		break;
	case MCC_AST_BINARY_OP_SMALLER:
		instr = MCC_IR_INSTR_SMALLER;
		break;
	case MCC_AST_BINARY_OP_GREATER:
		instr = MCC_IR_INSTR_GREATER;
		break;
	case MCC_AST_BINARY_OP_SMALLEREQ:
		instr = MCC_IR_INSTR_SMALLEREQ;
		break;
	case MCC_AST_BINARY_OP_GREATEREQ:
		instr = MCC_IR_INSTR_GREATEREQ;
		break;
	case MCC_AST_BINARY_OP_CONJ:
		instr = MCC_IR_INSTR_AND;
		break;
	case MCC_AST_BINARY_OP_DISJ:
		instr = MCC_IR_INSTR_OR;
		break;
	case MCC_AST_BINARY_OP_EQUAL:
		instr = MCC_IR_INSTR_EQUALS;
		break;
	case MCC_AST_BINARY_OP_NOTEQUAL:
		instr = MCC_IR_INSTR_NOTEQUALS;
		break;
	}

	struct mcc_ir_row *row = new_row(lhs, rhs, instr, data);
	append_row(row, data);

	struct mcc_ir_arg *arg = mcc_ir_new_arg(row, data);
	return arg;
}

static struct mcc_ir_arg *generate_ir_expression_unary_op(struct mcc_ast_expression *expression,
                                                          struct ir_generation_userdata *data)
{
	assert(expression->child);
	assert(data);
	if (data->has_failed)
		return NULL;

	struct mcc_ir_arg *child = generate_ir_expression(expression->child, data);
	enum mcc_ir_instruction instr = MCC_IR_INSTR_UNKNOWN;
	switch (expression->u_op) {
	case MCC_AST_UNARY_OP_NEGATIV:
		instr = MCC_IR_INSTR_NEGATIV;
		break;
	case MCC_AST_UNARY_OP_NOT:
		instr = MCC_IR_INSTR_NOT;
		break;
	}

	struct mcc_ir_row *row = new_row(child, NULL, instr, data);
	append_row(row, data);
	struct mcc_ir_arg *arg = mcc_ir_new_arg(row, data);
	return arg;
}

static struct mcc_ir_arg *generate_ir_expression_var(struct mcc_ast_expression *expression,
                                                     struct ir_generation_userdata *data)
{
	assert(expression->identifier);
	assert(data);
	if (data->has_failed)
		return NULL;

	struct mcc_ir_arg *arg = mcc_ir_new_arg(expression->identifier, data);
	return arg;
}

static void generate_ir_arguments(struct mcc_ast_arguments *arguments, struct ir_generation_userdata *data)
{
	assert(data);
	assert(arguments);
	if(data->has_failed)
		return;

	if(arguments->next_arguments){
		generate_ir_arguments(arguments->next_arguments, data);
	}
	if(!arguments->is_empty){
		struct mcc_ir_arg *arg_push = generate_ir_expression(arguments->expression, data);
		struct mcc_ir_row *row = new_row(arg_push, NULL, MCC_IR_INSTR_PUSH, data);
		append_row(row, data);
	}
}

static struct mcc_ir_arg *generate_ir_expression_func_call(struct mcc_ast_expression *expression,
                                                           struct ir_generation_userdata *data)
{
	assert(expression->function_identifier);
	assert(data);
	if(data->has_failed)
		return NULL;

	generate_ir_arguments(expression->arguments, data);

	struct mcc_ir_arg *arg = mcc_ir_new_arg(expression->function_identifier, data);
	struct mcc_ir_row *row = new_row(arg, NULL, MCC_IR_INSTR_CALL, data);
	append_row(row, data);
	return mcc_ir_new_arg(row, data);
}

static struct mcc_ir_arg *generate_ir_expression(struct mcc_ast_expression *expression,
                                                 struct ir_generation_userdata *data)
{
	assert(expression);
	assert(data);
	if (data->has_failed)
		return NULL;

	struct mcc_ir_arg *arg = NULL;

	switch (expression->type) {
	case MCC_AST_EXPRESSION_TYPE_LITERAL:
		arg = generate_arg_lit(expression->literal, data);
		break;
	case MCC_AST_EXPRESSION_TYPE_BINARY_OP:
		arg = generate_ir_expression_binary_op(expression, data);
		break;
	case MCC_AST_EXPRESSION_TYPE_PARENTH:
		arg = generate_ir_expression(expression->expression, data);
		break;
	case MCC_AST_EXPRESSION_TYPE_UNARY_OP:
		arg = generate_ir_expression_unary_op(expression, data);
		break;
	case MCC_AST_EXPRESSION_TYPE_VARIABLE:
		arg = generate_ir_expression_var(expression, data);
		break;
	case MCC_AST_EXPRESSION_TYPE_ARRAY_ELEMENT:
		arg = new_arg_arr_elem(expression->array_identifier, generate_ir_expression(expression->index, data),
		                       data);
		break;
	case MCC_AST_EXPRESSION_TYPE_FUNCTION_CALL:
		arg = generate_ir_expression_func_call(expression, data);
		break;
	}
	return arg;
}

static void generate_ir_comp_statement(struct mcc_ast_compound_statement *cmp_stmt, struct ir_generation_userdata *data)
{
	if (data->has_failed)
		return;

	while (cmp_stmt) {
		if (!cmp_stmt->is_empty)
			generate_ir_statement(cmp_stmt->statement, data);
		cmp_stmt = cmp_stmt->next_compound_statement;
	}
	UNUSED(cmp_stmt);
	UNUSED(data);
}

static void generate_ir_assignment(struct mcc_ast_assignment *asgn, struct ir_generation_userdata *data)
{
	assert(asgn);
	assert(data);
	if (data->has_failed)
		return;

	struct mcc_ir_arg *identifier = NULL, *exp = NULL;
	struct mcc_ir_row *row = NULL;
	if (asgn->assignment_type == MCC_AST_ASSIGNMENT_TYPE_VARIABLE) {
		identifier = mcc_ir_new_arg(asgn->variable_identifier, data);
		exp = generate_ir_expression(asgn->variable_assigned_value, data);
		row = new_row(identifier, exp, MCC_IR_INSTR_ASSIGN, data);
	} else {
		struct mcc_ir_arg *index = generate_ir_expression(asgn->array_index, data);
		identifier = new_arg_arr_elem(asgn->array_identifier, index, data);
		exp = generate_ir_expression(asgn->array_assigned_value, data);
		row = new_row(identifier, exp, MCC_IR_INSTR_ASSIGN, data);
	}
	append_row(row, data);
}

static void generate_ir_statememt_while_stmt(struct mcc_ast_statement *stmt, struct ir_generation_userdata *data)
{
	if (data->has_failed)
		return;

	// L0
	struct mcc_ir_arg *l0 = new_arg_label(data);
	struct mcc_ir_row *label_row = new_row(l0, NULL, MCC_IR_INSTR_LABEL, data);
	append_row(label_row, data);

	// Condition
	struct mcc_ir_arg *cond = generate_ir_expression(stmt->if_condition, data);

	// Jumpfalse L1
	struct mcc_ir_arg *l1 = new_arg_label(data);
	struct mcc_ir_row *jumpfalse = new_row(cond, l1, MCC_IR_INSTR_JUMPFALSE, data);
	append_row(jumpfalse, data);

	// On true
	generate_ir_statement(stmt->while_on_true, data);

	// Jump L0
	struct mcc_ir_row *jump_row = new_row(copy_arg(l0, data), NULL, MCC_IR_INSTR_JUMP, data);
	append_row(jump_row, data);

	// Label L1
	struct mcc_ir_row *label_row_2 = new_row(copy_arg(l1, data), NULL, MCC_IR_INSTR_LABEL, data);
	append_row(label_row_2, data);
}

static void generate_ir_statememt_if_else_stmt(struct mcc_ast_statement *stmt, struct ir_generation_userdata *data)
{
	if (data->has_failed)
		return;

	// Condition
	struct mcc_ir_arg *cond = generate_ir_expression(stmt->if_condition, data);

	// Jumpfalse L1
	struct mcc_ir_arg *l1 = new_arg_label(data);
	struct mcc_ir_row *jumpfalse = new_row(cond, l1, MCC_IR_INSTR_JUMPFALSE, data);
	append_row(jumpfalse, data);

	// If true
	generate_ir_statement(stmt->if_else_on_true, data);

	// Jump L2
	struct mcc_ir_arg *l2 = new_arg_label(data);
	struct mcc_ir_row *jump_row = new_row(l2, NULL, MCC_IR_INSTR_JUMP, data);
	append_row(jump_row, data);

	// Label L1
	struct mcc_ir_row *label_row = new_row(copy_arg(l1, data), NULL, MCC_IR_INSTR_LABEL, data);
	append_row(label_row, data);

	// If false
	generate_ir_statement(stmt->if_else_on_false, data);

	// Label L2
	struct mcc_ir_row *label_row_2 = new_row(copy_arg(l2, data), NULL, MCC_IR_INSTR_LABEL, data);
	append_row(label_row_2, data);
}

static void generate_ir_statememt_if_stmt(struct mcc_ast_statement *stmt, struct ir_generation_userdata *data)
{
	if (data->has_failed)
		return;
	struct mcc_ir_arg *cond = generate_ir_expression(stmt->if_condition, data);
	struct mcc_ir_arg *label = new_arg_label(data);
	struct mcc_ir_row *jumpfalse = new_row(cond, label, MCC_IR_INSTR_JUMPFALSE, data);
	append_row(jumpfalse, data);
	generate_ir_statement(stmt->if_on_true, data);
	struct mcc_ir_row *label_row = new_row(copy_arg(label, data), NULL, MCC_IR_INSTR_LABEL, data);
	append_row(label_row, data);
}

static void generate_ir_statement_return(struct mcc_ast_statement *stmt, struct ir_generation_userdata *data)
{
	assert(stmt);
	assert(data);

	if (stmt->return_value) {
		struct mcc_ir_arg *exp = generate_ir_expression(stmt->return_value, data);
		struct mcc_ir_row *row = new_row(exp, NULL, MCC_IR_INSTR_RETURN, data);
		append_row(row, data);
	} else {
		struct mcc_ir_row *row = new_row(NULL, NULL, MCC_IR_INSTR_RETURN, data);
		append_row(row, data);
	}
}

static void generate_ir_declaration(struct mcc_ast_declaration *decl, struct ir_generation_userdata *data)
{
	assert(decl);
	assert(data);
	if (data->has_failed || decl->declaration_type == MCC_AST_DECLARATION_TYPE_VARIABLE)
		return;

	struct mcc_ir_arg *arg1 = mcc_ir_new_arg(decl->array_identifier, data);
	struct mcc_ir_arg *arg2 = generate_arg_lit(decl->array_size, data);
	struct mcc_ir_row *row = new_row(arg1, arg2, MCC_IR_INSTR_ARRAY, data);
	append_row(row, data);
}

static void generate_ir_statement(struct mcc_ast_statement *stmt, struct ir_generation_userdata *data)
{
	if (data->has_failed)
		return;
	struct mcc_ir_arg *arg = NULL;
	switch (stmt->type) {
	case MCC_AST_STATEMENT_TYPE_EXPRESSION:
		// TODO: When everything is done: Don't generate IR here (has no effect)
		arg = generate_ir_expression(stmt->stmt_expression, data);
		break;
	case MCC_AST_STATEMENT_TYPE_COMPOUND_STMT:
		generate_ir_comp_statement(stmt->compound_statement, data);
		break;
	case MCC_AST_STATEMENT_TYPE_ASSIGNMENT:
		generate_ir_assignment(stmt->assignment, data);
		break;
	case MCC_AST_STATEMENT_TYPE_DECLARATION:
		generate_ir_declaration(stmt->declaration, data);
		break;
	case MCC_AST_STATEMENT_TYPE_IF_ELSE_STMT:
		generate_ir_statememt_if_else_stmt(stmt, data);
		break;
	case MCC_AST_STATEMENT_TYPE_IF_STMT:
		generate_ir_statememt_if_stmt(stmt, data);
		break;
	case MCC_AST_STATEMENT_TYPE_RETURN:
		generate_ir_statement_return(stmt, data);
		break;
	case MCC_AST_STATEMENT_TYPE_WHILE:
		generate_ir_statememt_while_stmt(stmt, data);
		break;
	default:
		break;
	}
	mcc_ir_delete_ir_arg(arg);
}

static void generate_ir_program(struct mcc_ast_program *program, struct ir_generation_userdata *data)
{
	assert(program);
	assert(data);

	if (data->has_failed)
		return;
	generate_ir_function_definition(program->function, data);
}

static void generate_ir_function_definition(struct mcc_ast_function_definition *def,
                                            struct ir_generation_userdata *data)
{
	if (data->has_failed)
		return;

	// Function Label
	struct mcc_ir_arg *func_label = new_arg_func_label(def, data);
	struct mcc_ir_row *label_row = new_row(func_label, NULL, MCC_IR_INSTR_FUNC_LABEL, data);
	append_row(label_row, data);

	// Pop args and assign them
	struct mcc_ast_parameters *pars = def->parameters;
	struct mcc_ir_row *pop_row;
	struct mcc_ir_row *assign;
	struct mcc_ir_arg *var;
	struct mcc_ir_arg *pop_arg;

	while (pars && !pars->is_empty) {
		// Pop arg
		pop_row = new_row(NULL, NULL, MCC_IR_INSTR_POP, data);
		append_row(pop_row, data);
		pop_arg = new_arg_row(pop_row, data);

		// Assign it
		var = arg_from_declaration(pars->declaration, data);
		assign = new_row(var, pop_arg, MCC_IR_INSTR_ASSIGN, data);
		append_row(assign, data);
		pars = pars->next_parameters;
	}

	// Function body
	generate_ir_comp_statement(def->compound_stmt, data);
}

//---------------------------------------------------------------------------------------- IR datastructures

static struct mcc_ir_arg *arg_from_declaration(struct mcc_ast_declaration *decl, struct ir_generation_userdata *data)
{
	assert(decl);
	switch (decl->declaration_type) {
	case MCC_AST_DECLARATION_TYPE_ARRAY:
		return new_arg_identifier(decl->array_identifier, data);
	case MCC_AST_DECLARATION_TYPE_VARIABLE:
		return new_arg_identifier(decl->variable_identifier, data);
	default:
		return NULL;
	}
}

static struct mcc_ir_arg *generate_arg_lit(struct mcc_ast_literal *literal, struct ir_generation_userdata *data)
{
	assert(literal);

	struct mcc_ir_arg *arg = NULL;

	switch (literal->type) {
	case MCC_AST_LITERAL_TYPE_INT:
		arg = mcc_ir_new_arg(literal->i_value, data);
		break;
	case MCC_AST_LITERAL_TYPE_FLOAT:
		arg = mcc_ir_new_arg(literal->f_value, data);
		break;
	case MCC_AST_LITERAL_TYPE_BOOL:
		arg = mcc_ir_new_arg(literal->bool_value, data);
		break;
	case MCC_AST_LITERAL_TYPE_STRING:
		arg = mcc_ir_new_arg(literal->string_value, data);
		break;
	}

	if (!arg) {
		data->has_failed = true;
	}

	return arg;
}

static void append_row(struct mcc_ir_row *row, struct ir_generation_userdata *data)
{
	assert(data);
	struct ir_generation_userdata *userdata = data;
	if (!row) {
		userdata->has_failed = true;
		return;
	}
	if (!data->head) {
		data->head = row;
		data->current = row;
		return;
	}
	data->current->next_row = row;
	row->prev_row = data->current;
	data->current = row;
}

static struct mcc_ir_arg *copy_label_arg(struct mcc_ir_arg *arg, struct ir_generation_userdata *data)
{
	struct mcc_ir_arg *new = malloc(sizeof(*new));
	if (!new) {
		data->has_failed = true;
		return NULL;
	}
	new->type = MCC_IR_TYPE_LABEL;
	new->label = arg->label;
	return new;
}

static struct mcc_ir_arg *copy_arg(struct mcc_ir_arg *arg, struct ir_generation_userdata *data)
{
	switch (arg->type) {
	case MCC_IR_TYPE_LIT_INT:
		return new_arg_int(arg->lit_int, data);
	case MCC_IR_TYPE_LIT_BOOL:
		return new_arg_bool(arg->lit_bool, data);
	case MCC_IR_TYPE_LIT_FLOAT:
		return new_arg_float(arg->lit_float, data);
	case MCC_IR_TYPE_LIT_STRING:
		return new_arg_string(arg->lit_string, data);
	case MCC_IR_TYPE_IDENTIFIER:
		return new_arg_identifier(arg->ident, data);
	case MCC_IR_TYPE_LABEL:
		return copy_label_arg(arg, data);
	case MCC_IR_TYPE_ROW:
		return new_arg_row(arg->row, data);
	default:
		return NULL;
	}
}

static struct mcc_ir_arg *new_arg_func_label(struct mcc_ast_function_definition *def,
                                             struct ir_generation_userdata *data)
{
	struct mcc_ir_arg *arg = malloc(sizeof(*arg));
	if (!arg) {
		data->has_failed = true;
		return NULL;
	}
	arg->type = MCC_IR_TYPE_FUNC_LABEL;
	arg->func_label = strdup(def->identifier->identifier_name);
	return arg;
}

static struct mcc_ir_arg *new_arg_row(struct mcc_ir_row *row, struct ir_generation_userdata *data)
{
	struct mcc_ir_arg *arg = malloc(sizeof(*arg));
	if (!arg) {
		data->has_failed = true;
		return NULL;
	}
	arg->type = MCC_IR_TYPE_ROW;
	arg->row = row;
	return arg;
}

static struct mcc_ir_arg *new_arg_int(long lit, struct ir_generation_userdata *data)
{
	struct mcc_ir_arg *arg = malloc(sizeof(*arg));
	if (!arg) {
		data->has_failed = true;
		return NULL;
	}
	arg->type = MCC_IR_TYPE_LIT_INT;
	arg->lit_int = lit;
	return arg;
}

static struct mcc_ir_arg *new_arg_float(double lit, struct ir_generation_userdata *data)
{
	struct mcc_ir_arg *arg = malloc(sizeof(*arg));
	if (!arg) {
		data->has_failed = true;
		return NULL;
	}
	arg->type = MCC_IR_TYPE_LIT_FLOAT;
	arg->lit_float = lit;
	return arg;
}

static struct mcc_ir_arg *new_arg_bool(bool lit, struct ir_generation_userdata *data)
{
	struct mcc_ir_arg *arg = malloc(sizeof(*arg));
	if (!arg) {
		data->has_failed = true;
		return NULL;
	}
	arg->type = MCC_IR_TYPE_LIT_BOOL;
	arg->lit_bool = lit;
	return arg;
}

static struct mcc_ir_arg *new_arg_string(char *lit, struct ir_generation_userdata *data)
{
	struct mcc_ir_arg *arg = malloc(sizeof(*arg));
	if (!arg) {
		data->has_failed = true;
		return NULL;
	}
	arg->type = MCC_IR_TYPE_LIT_STRING;
	arg->lit_string = lit;
	return arg;
}

static struct mcc_ir_arg *new_arg_label(struct ir_generation_userdata *data)
{
	struct mcc_ir_arg *arg = malloc(sizeof(*arg));
	if (!arg) {
		data->has_failed = true;
		return NULL;
	}
	arg->type = MCC_IR_TYPE_LABEL;
	arg->label = data->label_counter;
	data->label_counter = data->label_counter + 1;
	return arg;
}

static struct mcc_ir_arg *new_arg_identifier(struct mcc_ast_identifier *ident, struct ir_generation_userdata *data)
{
	struct mcc_ir_arg *arg = malloc(sizeof(*arg));
	if (!arg) {
		data->has_failed = true;
		return NULL;
	}
	arg->type = MCC_IR_TYPE_IDENTIFIER;
	arg->ident = ident;
	return arg;
}

static struct mcc_ir_arg *
new_arg_arr_elem(struct mcc_ast_identifier *ident, struct mcc_ir_arg *index, struct ir_generation_userdata *data)
{
	struct mcc_ir_arg *arg = malloc(sizeof(*arg));
	if (!arg) {
		data->has_failed = true;
		return NULL;
	}
	arg->type = MCC_IR_TYPE_ARR_ELEM;
	arg->arr_ident = ident;
	arg->index = index;
	return arg;
}

static struct mcc_ir_row *new_row(struct mcc_ir_arg *arg1,
                                  struct mcc_ir_arg *arg2,
                                  enum mcc_ir_instruction instr,
                                  struct ir_generation_userdata *data)
{
	struct mcc_ir_row *row = malloc(sizeof(*row));
	if (!row) {
		data->has_failed = true;
		return NULL;
	}
	row->row_no = 0;
	row->arg1 = arg1;
	row->arg2 = arg2;
	row->instr = instr;
	row->next_row = NULL;
	row->prev_row = NULL;
	return row;
}

static void number_rows(struct mcc_ir_row *head)
{
	if (!head)
		return;
	int i = 0;
	do {
		head->row_no = i;
		i = i + 1;
		head = head->next_row;
	} while (head);
}

struct mcc_ir_row *mcc_ir_generate(struct mcc_ast_program *ast, struct mcc_symbol_table *table)
{
	UNUSED(ast);
	UNUSED(table);

	struct ir_generation_userdata *data = malloc(sizeof(*data));
	if (!data)
		return NULL;
	data->head = NULL;
	data->has_failed = false;
	data->current = NULL;
	data->label_counter = 0;

	while (ast) {
		generate_ir_program(ast, data);
		ast = ast->next_function;
	}
	if (data->has_failed) {
		mcc_ir_delete_ir(data->head);
		free(data);
		return NULL;
	}
	struct mcc_ir_row *head = data->head;
	free(data);

	// Set row numbers for the visual representation
	number_rows(head);
	return head;
}

//---------------------------------------------------------------------------------------- Cleanup

void mcc_ir_delete_ir_arg(struct mcc_ir_arg *arg)
{
	if (!arg)
		return;
	if (arg->type == MCC_IR_TYPE_LIT_STRING) {
		free(arg->lit_string);
	}
	if (arg->type == MCC_IR_TYPE_FUNC_LABEL) {
		free(arg->func_label);
	}
	free(arg);
}

void mcc_ir_delete_ir_row(struct mcc_ir_row *row)
{
	if (!row)
		return;
	mcc_ir_delete_ir_arg(row->arg1);
	mcc_ir_delete_ir_arg(row->arg2);
	free(row);
}

void mcc_ir_delete_ir(struct mcc_ir_row *head)
{
	struct mcc_ir_row *temp = NULL;
	while (head->next_row) {
		head = head->next_row;
	}
	do {
		temp = head->prev_row;
		mcc_ir_delete_ir_row(head);
		head = temp;
	} while (temp);
}
