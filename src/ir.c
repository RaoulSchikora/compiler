#include "mcc/ir.h"

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "mcc/ast_visit.h"

// clang-format off

#define mcc_ir_new_arg(x, y) _Generic((x), \
    long:                           new_arg_int, \
    double:                         new_arg_float, \
    bool:                           new_arg_bool, \
    char*:                          new_arg_string, \
    struct mcc_ir_row *:            new_arg_row, \
    unsigned:                       new_arg_label, \
    struct mcc_ast_identifier *:    new_arg_identifier \
    )(x,y)

// clang-format on

struct ir_generation_userdata {
	struct mcc_ir_row *head;
	struct mcc_ir_row *current;
	bool has_failed;
	unsigned label_counter;
	unsigned tmp_counter;
};

int length_of_int(int num)
{
	if (num == 0)
		return 1;
	if (num <= 0)
		return floor(log10((-1) * num)) + 2;
	return floor(log10(num)) + 1;
}

bool is_binary_instr(struct mcc_ir_row *row)
{
	assert(row);

	switch (row->instr) {
	case MCC_IR_INSTR_PLUS:
	case MCC_IR_INSTR_MINUS:
	case MCC_IR_INSTR_MULTIPLY:
	case MCC_IR_INSTR_DIVIDE:
	case MCC_IR_INSTR_EQUALS:
	case MCC_IR_INSTR_NOTEQUALS:
	case MCC_IR_INSTR_SMALLER:
	case MCC_IR_INSTR_GREATER:
	case MCC_IR_INSTR_SMALLEREQ:
	case MCC_IR_INSTR_GREATEREQ:
	case MCC_IR_INSTR_AND:
	case MCC_IR_INSTR_OR:
		return true;
	default:
		return false;
	}
}

//------------------------------------------------------------------------------ Forward declarations: IR datastructures

static struct mcc_ir_arg *arg_from_declaration(struct mcc_ast_declaration *decl, struct ir_generation_userdata *data);
static struct mcc_ir_row_type *
new_ir_row_type(enum mcc_ir_row_types row_type, signed size, struct ir_generation_userdata *data);
static struct mcc_ir_row_type *typeless(struct ir_generation_userdata *data);
static struct mcc_ir_arg *new_arg_func_label(struct mcc_ast_function_definition *def,
                                             struct ir_generation_userdata *data);
static struct mcc_ir_row *new_row(struct mcc_ir_arg *arg1,
                                  struct mcc_ir_arg *arg2,
                                  enum mcc_ir_instruction instr,
                                  struct mcc_ir_row_type *type,
                                  struct ir_generation_userdata *data);
static struct mcc_ir_row *new_ir_row_array_tmp(struct mcc_ir_arg *index, struct ir_generation_userdata *data);
static struct mcc_ir_row *new_ir_row_float_tmp(double f_value, struct ir_generation_userdata *data);
static struct mcc_ir_arg *copy_arg(struct mcc_ir_arg *arg, struct ir_generation_userdata *data);
static struct mcc_ir_arg *new_arg_int(long lit, struct ir_generation_userdata *data);
static struct mcc_ir_arg *new_arg_float(double lit, struct ir_generation_userdata *data);
static struct mcc_ir_arg *new_arg_bool(bool lit, struct ir_generation_userdata *data);
static struct mcc_ir_arg *new_arg_string(char *lit, struct ir_generation_userdata *data);
static struct mcc_ir_arg *new_arg_row(struct mcc_ir_row *row, struct ir_generation_userdata *data);
static struct mcc_ir_arg *new_arg_label(struct ir_generation_userdata *data);
static struct mcc_ir_arg *new_arg_identifier(struct mcc_ast_identifier *ident, struct ir_generation_userdata *data);
static struct mcc_ir_arg *new_arg_identifier_from_string(char *ident, struct ir_generation_userdata *data);
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

static struct mcc_ir_row_type *
st_row_to_ir_type(struct mcc_symbol_table_row *row, int array_size, struct ir_generation_userdata *data)
{
	assert(row);
	assert(data);
	if (data->has_failed)
		return NULL;

	struct mcc_ir_row_type *type = new_ir_row_type(MCC_IR_ROW_TYPELESS, -1, data);

	switch (row->row_type) {
	case MCC_SYMBOL_TABLE_ROW_TYPE_INT:
		type->type = MCC_IR_ROW_INT;
		type->array_size = array_size;
		break;
	case MCC_SYMBOL_TABLE_ROW_TYPE_FLOAT:
		type->type = MCC_IR_ROW_FLOAT;
		type->array_size = array_size;
		break;
	case MCC_SYMBOL_TABLE_ROW_TYPE_BOOL:
		type->type = MCC_IR_ROW_BOOL;
		type->array_size = array_size;
		break;
	case MCC_SYMBOL_TABLE_ROW_TYPE_STRING:
		type->type = MCC_IR_ROW_STRING;
		type->array_size = array_size;
		break;
	default:
		type->type = MCC_IR_ROW_TYPELESS;
		type->array_size = array_size;
		break;
	}
	return type;
}

static struct mcc_ir_row_type *
ident_to_ir_type(struct mcc_ir_arg *arg, struct mcc_ast_expression *exp, struct ir_generation_userdata *data)
{
	assert(arg);
	assert(exp);
	assert(data);
	if (data->has_failed)
		return NULL;
	// if argument is dummy for tmp of float declaration return row_type float
	if (strncmp(arg->ident, "$tmp", 4) == 0) {
		return new_ir_row_type(MCC_IR_ROW_FLOAT, -1, data);
	}
	struct mcc_symbol_table_row *row = NULL;
	if (exp->type == MCC_AST_EXPRESSION_TYPE_VARIABLE) {
		row = mcc_symbol_table_check_upwards_for_declaration(arg->ident, exp->variable_row);
	} else if (exp->type == MCC_AST_EXPRESSION_TYPE_ARRAY_ELEMENT) {
		row = mcc_symbol_table_check_upwards_for_declaration(arg->ident, exp->array_row);
	} else if (exp->type == MCC_AST_EXPRESSION_TYPE_FUNCTION_CALL) {
		row = mcc_symbol_table_check_for_function_declaration(arg->ident, exp->function_row);
	}

	return st_row_to_ir_type(row, -1, data);
}

static struct mcc_ir_row_type *
get_type_of_row(struct mcc_ir_arg *arg, struct mcc_ast_expression *exp, struct ir_generation_userdata *data)
{
	assert(arg);
	assert(exp);
	assert(data);
	if (data->has_failed)
		return NULL;

	switch (arg->type) {
	case MCC_IR_TYPE_LIT_INT:
		return new_ir_row_type(MCC_IR_ROW_INT, -1, data);
	case MCC_IR_TYPE_LIT_FLOAT:
		return new_ir_row_type(MCC_IR_ROW_FLOAT, -1, data);
	case MCC_IR_TYPE_LIT_BOOL:
		return new_ir_row_type(MCC_IR_ROW_BOOL, -1, data);
	case MCC_IR_TYPE_LIT_STRING:
		return new_ir_row_type(MCC_IR_ROW_STRING, -1, data);
	case MCC_IR_TYPE_ROW:
		return new_ir_row_type(arg->row->type->type, arg->row->type->array_size, data);
	case MCC_IR_TYPE_LABEL:
		return new_ir_row_type(MCC_IR_ROW_TYPELESS, -1, data);
	case MCC_IR_TYPE_IDENTIFIER:
		return ident_to_ir_type(arg, exp, data);
	case MCC_IR_TYPE_ARR_ELEM:
		return ident_to_ir_type(arg, exp, data);
	case MCC_IR_TYPE_FUNC_LABEL:
		return ident_to_ir_type(arg, exp, data);
	}
	return NULL;
}

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
	struct mcc_ir_row_type *type = NULL;
	switch (expression->op) {
	case MCC_AST_BINARY_OP_ADD:
		instr = MCC_IR_INSTR_PLUS;
		type = get_type_of_row(lhs, expression->lhs, data);
		break;
	case MCC_AST_BINARY_OP_SUB:
		instr = MCC_IR_INSTR_MINUS;
		type = get_type_of_row(lhs, expression->lhs, data);
		break;
	case MCC_AST_BINARY_OP_MUL:
		instr = MCC_IR_INSTR_MULTIPLY;
		type = get_type_of_row(lhs, expression->lhs, data);
		break;
	case MCC_AST_BINARY_OP_DIV:
		instr = MCC_IR_INSTR_DIVIDE;
		type = get_type_of_row(lhs, expression->lhs, data);
		break;
	case MCC_AST_BINARY_OP_SMALLER:
		instr = MCC_IR_INSTR_SMALLER;
		type = new_ir_row_type(MCC_IR_ROW_BOOL, -1, data);
		break;
	case MCC_AST_BINARY_OP_GREATER:
		instr = MCC_IR_INSTR_GREATER;
		type = new_ir_row_type(MCC_IR_ROW_BOOL, -1, data);
		break;
	case MCC_AST_BINARY_OP_SMALLEREQ:
		instr = MCC_IR_INSTR_SMALLEREQ;
		type = new_ir_row_type(MCC_IR_ROW_BOOL, -1, data);
		break;
	case MCC_AST_BINARY_OP_GREATEREQ:
		instr = MCC_IR_INSTR_GREATEREQ;
		type = new_ir_row_type(MCC_IR_ROW_BOOL, -1, data);
		break;
	case MCC_AST_BINARY_OP_CONJ:
		instr = MCC_IR_INSTR_AND;
		type = new_ir_row_type(MCC_IR_ROW_BOOL, -1, data);
		break;
	case MCC_AST_BINARY_OP_DISJ:
		instr = MCC_IR_INSTR_OR;
		type = new_ir_row_type(MCC_IR_ROW_BOOL, -1, data);
		break;
	case MCC_AST_BINARY_OP_EQUAL:
		instr = MCC_IR_INSTR_EQUALS;
		type = new_ir_row_type(MCC_IR_ROW_BOOL, -1, data);
		break;
	case MCC_AST_BINARY_OP_NOTEQUAL:
		instr = MCC_IR_INSTR_NOTEQUALS;
		type = new_ir_row_type(MCC_IR_ROW_BOOL, -1, data);
		break;
	}

	struct mcc_ir_row *row = new_row(lhs, rhs, instr, type, data);
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
	struct mcc_ir_row_type *type = NULL;
	switch (expression->u_op) {
	case MCC_AST_UNARY_OP_NEGATIV:
		instr = MCC_IR_INSTR_NEGATIV;
		type = get_type_of_row(child, expression->child, data);
		break;
	case MCC_AST_UNARY_OP_NOT:
		instr = MCC_IR_INSTR_NOT;
		type = new_ir_row_type(MCC_IR_ROW_BOOL, -1, data);
		break;
	}

	struct mcc_ir_row *row = new_row(child, NULL, instr, type, data);
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
	if (data->has_failed)
		return;

	if (!arguments->is_empty) {
		if (arguments->expression->type == MCC_AST_EXPRESSION_TYPE_LITERAL &&
		    arguments->expression->literal->type == MCC_AST_LITERAL_TYPE_STRING) {
			unsigned size = 5 + length_of_int(data->tmp_counter);
			char tmp[size];
			snprintf(tmp, size, "$tmp%d", data->tmp_counter);
			data->tmp_counter += 1;
			struct mcc_ir_arg *lit = generate_ir_expression(arguments->expression, data);
			struct mcc_ir_row_type *type1 = get_type_of_row(lit, arguments->expression, data);
			struct mcc_ir_row_type *type2 = get_type_of_row(lit, arguments->expression, data);
			struct mcc_ir_arg *ident1 = new_arg_identifier_from_string(tmp, data);
			struct mcc_ir_arg *ident2 = new_arg_identifier_from_string(tmp, data);
			struct mcc_ir_row *row1 = NULL, *row2 = NULL;
			row1 = new_row(ident1, lit, MCC_IR_INSTR_ASSIGN, type1, data);
			append_row(row1, data);
			// recursive call of generate_ir_arguments in order to have all push-instructions following each
			// other without othre instructions in between
			if (arguments->next_arguments) {
				generate_ir_arguments(arguments->next_arguments, data);
			}
			row2 = new_row(ident2, NULL, MCC_IR_INSTR_PUSH, type2, data);
			append_row(row2, data);
		} else {
			struct mcc_ir_arg *arg_push = generate_ir_expression(arguments->expression, data);
			struct mcc_ir_row_type *type = get_type_of_row(arg_push, arguments->expression, data);
			// recursive call of generate_ir_arguments in order to have all push-instructions following each
			// other without othre instructions in between
			if (arguments->next_arguments) {
				generate_ir_arguments(arguments->next_arguments, data);
			}
			struct mcc_ir_row *row = new_row(arg_push, NULL, MCC_IR_INSTR_PUSH, type, data);
			append_row(row, data);
		}
	}
}

static struct mcc_ir_arg *generate_ir_expression_func_call(struct mcc_ast_expression *expression,
                                                           struct ir_generation_userdata *data)
{
	assert(expression->function_identifier);
	assert(data);
	if (data->has_failed)
		return NULL;

	generate_ir_arguments(expression->arguments, data);

	struct mcc_ir_arg *arg = mcc_ir_new_arg(expression->function_identifier, data);
	struct mcc_ir_row_type *type = get_type_of_row(arg, expression, data);
	struct mcc_ir_row *row = new_row(arg, NULL, MCC_IR_INSTR_CALL, type, data);
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
}

static void generate_ir_assignment(struct mcc_ast_assignment *asgn, struct ir_generation_userdata *data)
{
	assert(asgn);
	assert(data);
	if (data->has_failed)
		return;

	struct mcc_ir_arg *identifier = NULL, *exp = NULL;
	struct mcc_ir_row *row = NULL;
	struct mcc_symbol_table_row *st_row = NULL;
	struct mcc_ir_row_type *type = NULL;
	if (asgn->assignment_type == MCC_AST_ASSIGNMENT_TYPE_VARIABLE) {
		st_row = mcc_symbol_table_check_upwards_for_declaration(asgn->variable_identifier->identifier_name,
		                                                        asgn->row);
		type = st_row_to_ir_type(st_row, -1, data);
		identifier = mcc_ir_new_arg(asgn->variable_identifier, data);
		// if float literal do not generate expression, because it will generate unneseccary extra line
		if (asgn->variable_assigned_value->type == MCC_AST_EXPRESSION_TYPE_LITERAL &&
		    asgn->variable_assigned_value->literal->type == MCC_AST_LITERAL_TYPE_FLOAT) {
			exp = new_arg_float(asgn->variable_assigned_value->literal->f_value, data);
		} else {
			exp = generate_ir_expression(asgn->variable_assigned_value, data);
		}
		row = new_row(identifier, exp, MCC_IR_INSTR_ASSIGN, type, data);
	} else {
		st_row =
		    mcc_symbol_table_check_upwards_for_declaration(asgn->array_identifier->identifier_name, asgn->row);
		type = st_row_to_ir_type(st_row, -1, data);
		struct mcc_ir_arg *index = generate_ir_expression(asgn->array_index, data);
		identifier = new_arg_arr_elem(asgn->array_identifier, index, data);
		// if float literal do not generate expression, because it will generate unneseccary extra line
		if (asgn->array_assigned_value->type == MCC_AST_EXPRESSION_TYPE_LITERAL &&
		    asgn->array_assigned_value->literal->type == MCC_AST_LITERAL_TYPE_FLOAT) {
			exp = new_arg_float(asgn->array_assigned_value->literal->f_value, data);
		} else {
			exp = generate_ir_expression(asgn->array_assigned_value, data);
		}
		row = new_row(identifier, exp, MCC_IR_INSTR_ASSIGN, type, data);
	}
	append_row(row, data);
}

static void generate_ir_statememt_while_stmt(struct mcc_ast_statement *stmt, struct ir_generation_userdata *data)
{
	if (data->has_failed)
		return;

	// L0
	struct mcc_ir_arg *l0 = new_arg_label(data);
	struct mcc_ir_row *label_row = new_row(l0, NULL, MCC_IR_INSTR_LABEL, typeless(data), data);
	append_row(label_row, data);

	// Condition
	struct mcc_ir_arg *cond = generate_ir_expression(stmt->if_condition, data);

	// Jumpfalse L1
	struct mcc_ir_arg *l1 = new_arg_label(data);
	struct mcc_ir_row *jumpfalse = new_row(cond, l1, MCC_IR_INSTR_JUMPFALSE, typeless(data), data);
	append_row(jumpfalse, data);

	// On true
	generate_ir_statement(stmt->while_on_true, data);

	// Jump L0
	struct mcc_ir_row *jump_row = new_row(copy_arg(l0, data), NULL, MCC_IR_INSTR_JUMP, typeless(data), data);
	append_row(jump_row, data);

	// Label L1
	struct mcc_ir_row *label_row_2 = new_row(copy_arg(l1, data), NULL, MCC_IR_INSTR_LABEL, typeless(data), data);
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
	struct mcc_ir_row *jumpfalse = new_row(cond, l1, MCC_IR_INSTR_JUMPFALSE, typeless(data), data);
	append_row(jumpfalse, data);

	// If true
	generate_ir_statement(stmt->if_else_on_true, data);

	struct mcc_ir_arg *l2 = new_arg_label(data);
	bool if_ends_on_return = false;
	if (data->current->instr != MCC_IR_INSTR_RETURN) {
		// Jump L2
		struct mcc_ir_row *jump_row = new_row(l2, NULL, MCC_IR_INSTR_JUMP, typeless(data), data);
		append_row(jump_row, data);
	} else {
		if_ends_on_return = true;
	}

	// Label L1
	struct mcc_ir_row *label_row = new_row(copy_arg(l1, data), NULL, MCC_IR_INSTR_LABEL, typeless(data), data);
	append_row(label_row, data);

	// If false
	generate_ir_statement(stmt->if_else_on_false, data);

	if (data->current->instr != MCC_IR_INSTR_RETURN || !if_ends_on_return) {
		// Label L2
		struct mcc_ir_row *label_row_2 =
		    new_row(copy_arg(l2, data), NULL, MCC_IR_INSTR_LABEL, typeless(data), data);
		append_row(label_row_2, data);
	} else {
		mcc_ir_delete_ir_arg(l2);
	}
}

static void generate_ir_statememt_if_stmt(struct mcc_ast_statement *stmt, struct ir_generation_userdata *data)
{
	if (data->has_failed)
		return;
	struct mcc_ir_arg *cond = generate_ir_expression(stmt->if_condition, data);
	struct mcc_ir_arg *label = new_arg_label(data);
	struct mcc_ir_row *jumpfalse = new_row(cond, label, MCC_IR_INSTR_JUMPFALSE, typeless(data), data);
	append_row(jumpfalse, data);
	generate_ir_statement(stmt->if_on_true, data);
	struct mcc_ir_row *label_row = new_row(copy_arg(label, data), NULL, MCC_IR_INSTR_LABEL, typeless(data), data);
	append_row(label_row, data);
}

static void generate_ir_statement_return(struct mcc_ast_statement *stmt, struct ir_generation_userdata *data)
{
	assert(stmt);
	assert(data);

	if (data->current->instr == MCC_IR_INSTR_RETURN) {
		return;
	}

	if (stmt->return_value) {
		struct mcc_ir_arg *exp = generate_ir_expression(stmt->return_value, data);
		struct mcc_ir_row_type *type = get_type_of_row(exp, stmt->return_value, data);
		struct mcc_ir_row *row = new_row(exp, NULL, MCC_IR_INSTR_RETURN, type, data);
		append_row(row, data);
	} else {
		struct mcc_ir_row *row = new_row(NULL, NULL, MCC_IR_INSTR_RETURN, typeless(data), data);
		append_row(row, data);
	}
}

static void generate_ir_declaration(struct mcc_ast_declaration *decl, struct ir_generation_userdata *data)
{
	assert(decl);
	assert(data);

	struct mcc_ir_arg *arg1 = NULL, *arg2 = NULL;
	struct mcc_ir_row *row = NULL;
	// Only arrays and floats need an extra IR line for declaration
	if (data->has_failed ||
	    (decl->declaration_type == MCC_AST_DECLARATION_TYPE_VARIABLE && decl->variable_type->type_value != FLOAT)) {
		return;
	} else if (decl->variable_type->type_value == FLOAT) {
		arg1 = mcc_ir_new_arg(decl->variable_identifier, data);
		arg2 = mcc_ir_new_arg((double)0.0, data);
		row = new_row(arg1, arg2, MCC_IR_INSTR_ASSIGN, new_ir_row_type(MCC_IR_ROW_FLOAT, -1, data), data);
	} else {
		arg1 = mcc_ir_new_arg(decl->array_identifier, data);
		arg2 = generate_arg_lit(decl->array_size, data);

		struct mcc_ir_row_type *type = st_row_to_ir_type(decl->row, (int)decl->array_size->i_value, data);
		switch (decl->array_type->type_value) {
		case INT:
			row = new_row(arg1, arg2, MCC_IR_INSTR_ARRAY, type, data);
			break;
		case FLOAT:
			row = new_row(arg1, arg2, MCC_IR_INSTR_ARRAY, type, data);
			break;
		case STRING:
			row = new_row(arg1, arg2, MCC_IR_INSTR_ARRAY, type, data);
			break;
		case BOOL:
			row = new_row(arg1, arg2, MCC_IR_INSTR_ARRAY, type, data);
			break;
		case VOID:
			mcc_ir_delete_ir_arg(arg1);
			mcc_ir_delete_ir_arg(arg2);
			data->has_failed = true;
			return;
		}
	}

	if (!arg1 || !arg2 || !row) {
		mcc_ir_delete_ir_arg(arg1);
		mcc_ir_delete_ir_arg(arg2);
		mcc_ir_delete_ir_row(row);
		data->has_failed = true;
		return;
	}

	append_row(row, data);
}

static void generate_ir_statement(struct mcc_ast_statement *stmt, struct ir_generation_userdata *data)
{
	if (data->has_failed)
		return;
	struct mcc_ir_arg *arg = NULL;
	switch (stmt->type) {
	case MCC_AST_STATEMENT_TYPE_EXPRESSION:
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
	struct mcc_ir_row *label_row = new_row(func_label, NULL, MCC_IR_INSTR_FUNC_LABEL, typeless(data), data);
	append_row(label_row, data);

	// Pop args and assign them
	struct mcc_ast_parameters *pars = def->parameters;

	while (pars && !pars->is_empty) {
		int size = -1;
		if (pars->declaration->declaration_type == MCC_AST_DECLARATION_TYPE_ARRAY) {
			size = (int)pars->declaration->array_size->i_value;
		}
		// Pop arg
		struct mcc_ir_row_type *type1 = st_row_to_ir_type(pars->declaration->row, size, data);
		struct mcc_ir_row *pop_row = new_row(NULL, NULL, MCC_IR_INSTR_POP, type1, data);
		append_row(pop_row, data);
		struct mcc_ir_arg *pop_arg = new_arg_row(pop_row, data);

		// Assign it
		struct mcc_ir_row_type *type2 = st_row_to_ir_type(pars->declaration->row, size, data);
		struct mcc_ir_arg *var = arg_from_declaration(pars->declaration, data);
		struct mcc_ir_row *assign = new_row(var, pop_arg, MCC_IR_INSTR_ASSIGN, type2, data);
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
	struct mcc_ir_row *row = NULL;

	switch (literal->type) {
	case MCC_AST_LITERAL_TYPE_INT:
		arg = mcc_ir_new_arg(literal->i_value, data);
		break;
	case MCC_AST_LITERAL_TYPE_FLOAT:
		row = new_ir_row_float_tmp(literal->f_value, data);
		arg = new_arg_identifier_from_string(row->arg1->ident, data);
		break;
	case MCC_AST_LITERAL_TYPE_BOOL:
		arg = mcc_ir_new_arg(literal->bool_value, data);
		break;
	case MCC_AST_LITERAL_TYPE_STRING:
		arg = mcc_ir_new_arg(literal->string_value, data);
		break;
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
		return new_arg_identifier_from_string(arg->ident, data);
	case MCC_IR_TYPE_LABEL:
		return copy_label_arg(arg, data);
	case MCC_IR_TYPE_ROW:
		return new_arg_row(arg->row, data);
	default:
		return NULL;
	}
}

static struct mcc_ir_row_type *
new_ir_row_type(enum mcc_ir_row_types row_type, signed size, struct ir_generation_userdata *data)
{
	struct mcc_ir_row_type *type = malloc(sizeof(*type));
	if (!type) {
		data->has_failed = true;
		return NULL;
	}
	type->type = row_type;
	type->array_size = size;
	return type;
}

static struct mcc_ir_row_type *typeless(struct ir_generation_userdata *data)
{
	assert(data);
	if (data->has_failed)
		return NULL;

	return new_ir_row_type(MCC_IR_ROW_TYPELESS, -1, data);
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
	char *string = malloc(sizeof(char) * (strlen(lit) + 1));
	if (!arg || !string) {
		data->has_failed = true;
		free(arg);
		free(string);
		return NULL;
	}
	strcpy(string, lit);
	arg->type = MCC_IR_TYPE_LIT_STRING;
	arg->lit_string = string;
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
	char *str = malloc(sizeof(char) * (strlen(ident->identifier_name) + 1));
	if (!arg || !str) {
		data->has_failed = true;
		free(arg);
		free(str);
		return NULL;
	}
	strcpy(str, ident->identifier_name);
	arg->type = MCC_IR_TYPE_IDENTIFIER;
	arg->ident = str;
	return arg;
}

static struct mcc_ir_arg *new_arg_identifier_from_string(char *ident, struct ir_generation_userdata *data)
{
	struct mcc_ir_arg *arg = malloc(sizeof(*arg));
	char *str = strdup(ident);
	if (!arg || !str) {
		data->has_failed = true;
		free(arg);
		free(str);
		return NULL;
	}
	arg->type = MCC_IR_TYPE_IDENTIFIER;
	arg->ident = str;
	return arg;
}

static struct mcc_ir_arg *
new_arg_arr_elem(struct mcc_ast_identifier *ident, struct mcc_ir_arg *index, struct ir_generation_userdata *data)
{
	struct mcc_ir_arg *arg = malloc(sizeof(*arg));
	char *str = malloc(sizeof(char) * (strlen(ident->identifier_name) + 1));
	if (!arg || !str) {
		data->has_failed = true;
		free(arg);
		free(str);
		return NULL;
	}
	strcpy(str, ident->identifier_name);
	arg->type = MCC_IR_TYPE_ARR_ELEM;
	arg->arr_ident = str;
	if (index->type == MCC_IR_TYPE_ARR_ELEM) {
		struct mcc_ir_row *row = new_ir_row_array_tmp(index, data);
		arg->index = new_arg_identifier_from_string(row->arg1->ident, data);
	} else {
		arg->index = index;
	}
	return arg;
}

static struct mcc_ir_row *new_row(struct mcc_ir_arg *arg1,
                                  struct mcc_ir_arg *arg2,
                                  enum mcc_ir_instruction instr,
                                  struct mcc_ir_row_type *type,
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
	row->type = type;
	row->next_row = NULL;
	row->prev_row = NULL;
	return row;
}

static struct mcc_ir_row *new_ir_row_array_tmp(struct mcc_ir_arg *index, struct ir_generation_userdata *data)
{
	unsigned size = 4 + length_of_int(data->tmp_counter) + 1;
	char *ident = malloc(sizeof(char) * size);
	if (!ident) {
		data->has_failed = true;
		return NULL;
	}
	snprintf(ident, size, "$tmp%d", data->tmp_counter);
	data->tmp_counter++;
	struct mcc_ir_arg *arg1 = new_arg_identifier_from_string(ident, data);
	free(ident);
	struct mcc_ir_arg *arg2 = index;
	// is always of type int because it is only used when index of array element is again array element
	struct mcc_ir_row_type *type = new_ir_row_type(MCC_IR_ROW_INT, -1, data);
	struct mcc_ir_row *row = new_row(arg1, arg2, MCC_IR_INSTR_ASSIGN, type, data);
	append_row(row, data);
	return row;
}

static struct mcc_ir_row *new_ir_row_float_tmp(double f_value, struct ir_generation_userdata *data)
{
	unsigned size = 4 + length_of_int(data->tmp_counter) + 1;
	char *ident = malloc(sizeof(char) * size);
	if (!ident) {
		data->has_failed = true;
		return NULL;
	}
	snprintf(ident, size, "$tmp%d", data->tmp_counter);
	data->tmp_counter++;
	struct mcc_ir_arg *arg1 = new_arg_identifier_from_string(ident, data);
	free(ident);
	struct mcc_ir_arg *arg2 = new_arg_float(f_value, data);
	struct mcc_ir_row_type *type = new_ir_row_type(MCC_IR_ROW_FLOAT, -1, data);
	struct mcc_ir_row *row = new_row(arg1, arg2, MCC_IR_INSTR_ASSIGN, type, data);
	append_row(row, data);
	return row;
}

static void number_rows(struct mcc_ir_row *head)
{
	if (!head)
		return;
	int i = 0;
	do {
		switch (head->instr) {
		case MCC_IR_INSTR_AND:
		case MCC_IR_INSTR_OR:
		case MCC_IR_INSTR_PLUS:
		case MCC_IR_INSTR_DIVIDE:
		case MCC_IR_INSTR_MINUS:
		case MCC_IR_INSTR_NEGATIV:
		case MCC_IR_INSTR_GREATER:
		case MCC_IR_INSTR_MULTIPLY:
		case MCC_IR_INSTR_SMALLER:
		case MCC_IR_INSTR_SMALLEREQ:
		case MCC_IR_INSTR_GREATEREQ:
		case MCC_IR_INSTR_EQUALS:
		case MCC_IR_INSTR_NOTEQUALS:
		case MCC_IR_INSTR_NOT:
		case MCC_IR_INSTR_CALL:
		case MCC_IR_INSTR_POP:

			head->row_no = i;
			i = i + 1;
			break;
		default:
			break;
		}
		head = head->next_row;
	} while (head);
}

// --------------------------------------------------------------------------------------- Variable shadowing

// struct for user data concerning variable shadowing
struct renaming_userdata {
	struct ir_generation_userdata *ir_data;
	int num;
	struct mcc_ast_identifier *ident;
};

static void rename_row(struct mcc_symbol_table_row *row, struct renaming_userdata *data)
{
	assert(row);
	assert(data);

	size_t size = 3 + length_of_int(data->num);
	row->name = (char *)realloc(row->name, size);
	if (!row->name) {
		data->ir_data->has_failed = true;
		return;
	}
	snprintf(row->name, size, "$r%d", data->num);
}

static void rename_ident(struct mcc_ast_identifier *ident, struct renaming_userdata *data)
{
	assert(ident);
	assert(data);

	size_t size = 3 + length_of_int(data->num);
	ident->identifier_name = (char *)realloc(ident->identifier_name, size);
	if (!ident->identifier_name) {
		data->ir_data->has_failed = true;
		return;
	}
	snprintf(ident->identifier_name, size, "$r%d", data->num);
}

static void cb_rename_ident(struct mcc_ast_identifier *ident, void *data)
{
	assert(ident);
	assert(data);
	struct renaming_userdata *re_data = data;
	if (strcmp(ident->identifier_name, re_data->ident->identifier_name) == 0) {
		rename_ident(ident, re_data);
	}
}

// Setup an AST Visitor for visiting declarations
static struct mcc_ast_visitor rename_ident_visitor(struct renaming_userdata *data)
{
	return (struct mcc_ast_visitor){
	    .traversal = MCC_AST_VISIT_DEPTH_FIRST,
	    .order = MCC_AST_VISIT_PRE_ORDER,

	    .userdata = data,

	    .identifier = cb_rename_ident,
	};
}

// callback of the modifying visitor. Checks if compound statement is a declaration, if so checks in symbol table if it
// shadows a variable. If it does, all variables in the subtree of that compound statement become renamed
static void cb_variable_shadowing(struct mcc_ast_compound_statement *comp_stmt, void *data)
{
	assert(data);
	assert(comp_stmt);
	struct renaming_userdata *re_data = data;
	if (re_data->ir_data->has_failed)
		return;
	if (comp_stmt->is_empty || comp_stmt->statement->type != MCC_AST_STATEMENT_TYPE_DECLARATION)
		return;
	struct mcc_ast_declaration *decl = comp_stmt->statement->declaration;
	struct mcc_symbol_table_row *row = decl->row;
	struct mcc_symbol_table_row *prev = NULL;
	// get previous row in symbol table
	if (row->prev_row) {
		prev = row->prev_row;
	} else if (row->scope->parent_row) {
		if (row->scope->parent_row->row_structure == MCC_SYMBOL_TABLE_ROW_STRUCTURE_FUNCTION) {
			return;
		} else {
			prev = row->scope->parent_row;
		}
	}
	// get identifier of declaration
	struct mcc_ast_identifier *ident = NULL;
	switch (decl->declaration_type) {
	case MCC_AST_DECLARATION_TYPE_VARIABLE:
		ident = decl->variable_identifier;
		break;
	case MCC_AST_DECLARATION_TYPE_ARRAY:
		ident = decl->array_identifier;
		break;
	}
	// check if a row with same name exists upwards in the symbol table, if yes rename
	prev = mcc_symbol_table_check_upwards_for_declaration(ident->identifier_name, prev);
	if (prev) {
		re_data->ident = ident;
		if (comp_stmt->next_compound_statement) {
			struct mcc_ast_visitor visitor = rename_ident_visitor(data);
			mcc_ast_visit(comp_stmt->next_compound_statement, &visitor);
		}
		rename_ident(ident, re_data);
		rename_row(row, re_data);
		re_data->num += 1;
	}
}

// --------------------------------------------------------------------------------------- append empty return

static void append_empty_return(struct mcc_ast_compound_statement *comp_stmt, struct renaming_userdata *re_data)
{
	assert(comp_stmt);
	assert(re_data);
	if (re_data->ir_data->has_failed) {
		return;
	}
	struct mcc_ast_statement *stmt = malloc(sizeof(*stmt));
	if (!stmt) {
		re_data->ir_data->has_failed = true;
		return;
	}
	stmt->type = MCC_AST_STATEMENT_TYPE_RETURN;
	stmt->is_empty_return = true;
	stmt->return_value = NULL;
	struct mcc_ast_compound_statement *new_comp_stmt = malloc(sizeof(*new_comp_stmt));
	if (!new_comp_stmt) {
		re_data->ir_data->has_failed = true;
		return;
	}
	new_comp_stmt->statement = stmt;
	new_comp_stmt->is_empty = false;
	new_comp_stmt->has_next_statement = false;
	new_comp_stmt->next_compound_statement = NULL;
	comp_stmt->has_next_statement = true;
	comp_stmt->next_compound_statement = new_comp_stmt;
}

// callback to add an empty return statement to void functions where no empty return statement is present at the end of
// an execution path.
static void cb_add_return(struct mcc_ast_function_definition *def, void *data)
{
	assert(def);
	assert(data);
	struct renaming_userdata *re_data = data;
	if (re_data->ir_data->has_failed)
		return;

	struct mcc_ast_compound_statement *comp_stmt = def->compound_stmt;
	struct mcc_ast_compound_statement *prev_stmt = comp_stmt;
	do {
		if (!comp_stmt->is_empty && comp_stmt->statement->type == MCC_AST_STATEMENT_TYPE_RETURN) {
			return;
		}
		prev_stmt = comp_stmt;
		comp_stmt = comp_stmt->next_compound_statement;
	} while (comp_stmt);
	if (def->type == VOID) {
		append_empty_return(prev_stmt, data);
	}
}

// --------------------------------------------------------------------------------------- generate IR

// Setup an AST Visitor for visiting compound statements with a statement of type declaration to ensure variable
// shadowing and visit function definitions to add returns in void functions where needed
static struct mcc_ast_visitor modifying_visitor(struct renaming_userdata *data)
{
	return (struct mcc_ast_visitor){
	    .traversal = MCC_AST_VISIT_DEPTH_FIRST,
	    .order = MCC_AST_VISIT_POST_ORDER,

	    .userdata = data,

	    .compound_statement = cb_variable_shadowing,
	    .function_definition = cb_add_return,
	};
}

static void modify_ast(struct mcc_ast_program *ast, struct ir_generation_userdata *ir_data)
{
	assert(ast);
	assert(ir_data);
	if (ir_data->has_failed)
		return;

	struct renaming_userdata *re_data = malloc(sizeof(*re_data));
	if (!re_data) {
		ir_data->has_failed = true;
		return;
	}
	re_data->ir_data = ir_data;
	re_data->num = 0;
	struct mcc_ast_visitor visitor = modifying_visitor(re_data);
	mcc_ast_visit(ast, &visitor);
	free(re_data);
}

struct mcc_ir_row *mcc_ir_generate(struct mcc_ast_program *ast)
{
	struct ir_generation_userdata *data = malloc(sizeof(*data));
	if (!data)
		return NULL;
	data->head = NULL;
	data->has_failed = false;
	data->current = NULL;
	data->label_counter = 0;
	data->tmp_counter = 0;

	// remove all built_ins before creating the IR
	ast = mcc_ast_remove_built_ins(ast);

	// Add return statements for void functions and enforce variable shadowing
	modify_ast(ast, data);
	if (data->has_failed) {
		free(data);
		return NULL;
	}

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

	// Set row numbers (used for naming temporaries in IR) for the visual representation
	number_rows(head);
	return head;
}

//---------------------------------------------------------------------------------------- Cleanup

void mcc_ir_delete_ir_arg(struct mcc_ir_arg *arg)
{
	if (!arg)
		return;
	if (arg->type == MCC_IR_TYPE_FUNC_LABEL) {
		free(arg->func_label);
	}
	if (arg->type == MCC_IR_TYPE_ARR_ELEM) {
		mcc_ir_delete_ir_arg(arg->index);
	}
	if (arg->type == MCC_IR_TYPE_LIT_STRING) {
		free(arg->lit_string);
	}
	if (arg->type == MCC_IR_TYPE_IDENTIFIER) {
		free(arg->ident);
	}
	if (arg->type == MCC_IR_TYPE_ARR_ELEM) {
		free(arg->arr_ident);
	}
	free(arg);
}

void mcc_ir_delete_ir_row_type(struct mcc_ir_row_type *type)
{
	if (!type)
		return;
	free(type);
}

void mcc_ir_delete_ir_row(struct mcc_ir_row *row)
{
	if (!row)
		return;
	mcc_ir_delete_ir_arg(row->arg1);
	mcc_ir_delete_ir_arg(row->arg2);
	mcc_ir_delete_ir_row_type(row->type);
	free(row);
}

void mcc_ir_delete_ir(struct mcc_ir_row *head)
{
	if (!head)
		return;
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
