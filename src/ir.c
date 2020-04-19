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

struct ir_generation_userdata {
	struct mcc_ir_row *head;
	struct mcc_ir_row *current;
	bool has_failed;
};

//------------------------------------------------------------------------------ Forward declarations

static struct mcc_ir_row *
mcc_ir_new_row(int row_no, struct mcc_ir_arg *arg1, struct mcc_ir_arg *arg2, enum mcc_ir_instruction instr);
static struct mcc_ir_arg *mcc_ir_new_arg_var(char *var);
static struct mcc_ir_arg *mcc_ir_new_arg_row(struct mcc_ir_row *row);
static void append_row(struct mcc_ir_row *row, struct ir_generation_userdata *data);

//------------------------------------------------------------------------------ Forward declarations, Fake IR

static struct mcc_ir_row *get_fake_ir_line();
static struct mcc_ir_row *get_fake_ir();

//------------------------------------------------------------------------------ Callbacks for visitor that generates IR

static void generate_ir_expression(struct mcc_ast_expression *expression, void *data)
{
	assert(identifier);
	assert(data);
}

static void generate_ir_expression_literal(struct mcc_ast_expression *expression, void *data)
{
	assert(expression);
	assert(data);
}

static void generate_ir_expression_binary_op(struct mcc_ast_expression *expression, void *data)
{
	assert(expression);
	assert(data);
}

static void generate_ir_expression_parenth(struct mcc_ast_expression *expression, void *data)
{
	assert(expression);
	assert(data);
}

static void generate_ir_expression_unary_op(struct mcc_ast_expression *expression, void *data)
{
	assert(expression);
	assert(data);
}

static void generate_ir_expression_array_element(struct mcc_ast_expression *expression, void *data)
{
	assert(expression);
	assert(data);
}

static void generate_ir_expression_variable(struct mcc_ast_expression *expression, void *data)
{
	assert(expression);
	assert(data);
}

static void generate_ir_expression_function_call(struct mcc_ast_expression *expression, void *data)
{
	assert(expression);
	assert(data);
}

static void generate_ir_statememt_if_stmt(struct mcc_ast_statement *statement, void *data)
{
	assert(statement);
	assert(data);
}

static void generate_ir_statement_if_else_stmt(struct mcc_ast_statement *statement, void *data)
{
	assert(statement);
	assert(data);
	// Generate IR for condition (resulting in the last line holding the result of that expression)
	// Generate Jumpfalse condition L1
	// Generate IR for on_true directly afterwards
	// Generate jump L2
	// Label L1
	// Generate IR for on_false
	// Label L2
}

static void generate_ir_statement_expression_stmt(struct mcc_ast_statement *statement, void *data)
{
	assert(statement);
	assert(data);
}

static void generate_ir_statement_while(struct mcc_ast_statement *statement, void *data)
{
	assert(statement);
	assert(data);
}

static void generate_ir_literal(struct mcc_ast_literal *literal, void *data)
{
	assert(literal);
	assert(data);
}

static void generate_ir_literal_int(struct mcc_ast_literal *literal, void *data)
{
	assert(literal);
	assert(data);
}

static void generate_ir_literal_float(struct mcc_ast_literal *literal, void *data)
{
	assert(literal);
	assert(data);
}

static void generate_ir_literal_string(struct mcc_ast_literal *literal, void *data)
{
	assert(literal);
	assert(data);
}

static void generate_ir_literal_bool(struct mcc_ast_literal *literal, void *data)
{
	assert(literal);
	assert(data);
}

static void generate_ir_variable_declaration(struct mcc_ast_declaration *declaration, void *data)
{
	assert(declaration);
	assert(data);
}

static void generate_ir_array_declaration(struct mcc_ast_declaration *declaration, void *data)
{
	assert(declaration);
	assert(data);
}

static void generate_ir_variable_assignment(struct mcc_ast_assignment *assignment, void *data)
{
	assert(assignment);
	assert(data);
}

static void generate_ir_array_assignment(struct mcc_ast_assignment *assignment, void *data)
{
	assert(assignment);
	assert(data);
}

static void generate_ir_assignment(struct mcc_ast_assignment *assignment, void *data)
{
	assert(assignment);
	assert(data);
}

static void generate_ir_declaration(struct mcc_ast_declaration *declaration, void *data)
{
	assert(declaration);
	assert(data);
}

static void generate_ir_type(struct mcc_ast_type *type, void *data)
{
	assert(type);
	assert(data);
}

static void generate_ir_expression_identifier(struct mcc_ast_identifier *identifier, void *data)
{
	assert(identifier);
	assert(data);
}

static void generate_ir_statement(struct mcc_ast_statement *statement, void *data)
{
	assert(statement);
	assert(data);
}

static void generate_ir_statement_declaration(struct mcc_ast_statement *statement, void *data)
{
	assert(statement);
	assert(data);
}

static void generate_ir_statement_assignment(struct mcc_ast_statement *statement, void *data)
{
	assert(statement);
	assert(data);
}

static void generate_ir_statement_return(struct mcc_ast_statement *statement, void *data)
{
	assert(statement);
	assert(data);
}

static void generate_ir_statement_compound_statement(struct mcc_ast_statement *statement, void *data)
{
	assert(statement);
	assert(data);
}

static void generate_ir_compound_statement(struct mcc_ast_compound_statement *compound_statement, void *data)
{
	assert(compound_statement);
	assert(data);
}

static void generate_ir_program(struct mcc_ast_program *program, void *data)
{
	assert(program);
	assert(data);
	struct ir_generation_userdata *userdata = data;
	append_row(get_fake_ir(), userdata);
}

static void generate_ir_parameters(struct mcc_ast_parameters *parameters, void *data)
{
	assert(parameters);
	assert(data);
}

static void generate_ir_arguments(struct mcc_ast_arguments *arguments, void *data)
{
	assert(arguments);
	assert(data);
}

static void generate_ir_function_definition(struct mcc_ast_function_definition *function_definition, void *data)
{
	assert(function_definition);
	assert(data);
}

//---------------------------------------------------------------------------------------- Generate Fake IR for testing

static struct mcc_ir_row *get_fake_ir_line()
{
	struct mcc_ir_row *head = malloc(sizeof(*head));
	if (!head)
		return NULL;

	struct mcc_ir_arg *arg1 = malloc(sizeof(*arg1));
	struct mcc_ir_arg *arg2 = malloc(sizeof(*arg2));
	if (!arg1 || !arg2) {
		free(arg1);
		free(arg2);
		free(head);
		return NULL;
	}

	arg1->type = MCC_IR_TYPE_VAR;
	arg2->type = MCC_IR_TYPE_VAR;

	char *str1 = malloc(sizeof(char) * 5);
	char *str2 = malloc(sizeof(char) * 5);
	if (!str1 || !str2) {
		free(arg1);
		free(arg2);
		free(head);
		free(str1);
		free(str2);
		return NULL;
	}
	snprintf(str1, 5, "test");
	snprintf(str2, 5, "var2");
	arg1->var = str1;
	arg2->var = str2;
	head->instr = MCC_IR_INSTR_JUMPFALSE;
	head->row_no = 0;
	head->next_row = NULL;
	head->prev_row = NULL;
	head->arg1 = arg1;
	head->arg2 = arg2;
	return head;
}

static struct mcc_ir_row *get_fake_ir()
{
	struct mcc_ir_row *head = get_fake_ir_line();
	struct mcc_ir_row *next = get_fake_ir_line();
	if (!head || !next)
		return NULL;
	head->next_row = next;
	next->prev_row = head;
	return head;
}

//---------------------------------------------------------------------------------------- Generate IR datastructures

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

static struct mcc_ir_arg *mcc_ir_new_arg_row(struct mcc_ir_row *row)
{
	struct mcc_ir_arg *arg = malloc(sizeof(*arg));
	if (!arg)
		return NULL;
	arg->type = MCC_IR_TYPE_ROW;
	arg->row = row;
	return arg;
}

static struct mcc_ir_arg *mcc_ir_new_arg_var(char *var)
{
	struct mcc_ir_arg *arg = malloc(sizeof(*arg));
	if (!arg)
		return NULL;
	arg->type = MCC_IR_TYPE_VAR;
	arg->var = var;
	return arg;
}

static struct mcc_ir_row *
mcc_ir_new_row(int row_no, struct mcc_ir_arg *arg1, struct mcc_ir_arg *arg2, enum mcc_ir_instruction instr)
{
	struct mcc_ir_row *row = malloc(sizeof(*row));
	if (!row)
		return NULL;
	row->row_no = row_no;
	row->arg1 = arg1;
	row->arg2 = arg2;
	row->instr = instr;
	row->next_row = NULL;
	row->prev_row = NULL;
	return row;
}

// Setup an AST Visitor for IR generation
static struct mcc_ast_visitor generate_ir_visitor(void *data)
{
	return (struct mcc_ast_visitor){
	    .traversal = MCC_AST_VISIT_DEPTH_FIRST,
	    .order = MCC_AST_VISIT_PRE_ORDER,

	    .userdata = data,

	    .expression = generate_ir_expression,
	    .expression_literal = generate_ir_expression_literal,
	    .expression_binary_op = generate_ir_expression_binary_op,
	    .expression_parenth = generate_ir_expression_parenth,
	    .expression_unary_op = generate_ir_expression_unary_op,
	    .expression_variable = generate_ir_expression_variable,
	    .expression_array_element = generate_ir_expression_array_element,
	    .expression_function_call = generate_ir_expression_function_call,

	    .literal = generate_ir_literal,
	    .literal_int = generate_ir_literal_int,
	    .literal_float = generate_ir_literal_float,
	    .literal_bool = generate_ir_literal_bool,
	    .literal_string = generate_ir_literal_string,

	    .statement = generate_ir_statement,
	    .statement_if_stmt = generate_ir_statememt_if_stmt,
	    .statement_if_else_stmt = generate_ir_statement_if_else_stmt,
	    .statement_expression_stmt = generate_ir_statement_expression_stmt,
	    .statement_while = generate_ir_statement_while,
	    .statement_assignment = generate_ir_statement_assignment,
	    .statement_declaration = generate_ir_statement_declaration,
	    .statement_return = generate_ir_statement_return,
	    .statement_compound_stmt = generate_ir_statement_compound_statement,

	    .compound_statement = generate_ir_compound_statement,
	    .program = generate_ir_program,
	    .function_definition = generate_ir_function_definition,
	    .parameters = generate_ir_parameters,
	    .arguments = generate_ir_arguments,

	    .assignment = generate_ir_assignment,
	    .variable_assignment = generate_ir_variable_assignment,
	    .array_assignment = generate_ir_array_assignment,
	    .declaration = generate_ir_declaration,
	    .variable_declaration = generate_ir_variable_declaration,
	    .array_declaration = generate_ir_array_declaration,

	    .type = generate_ir_type,
	    .identifier = generate_ir_expression_identifier,
	};
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

	struct mcc_ast_visitor visitor = generate_ir_visitor(data);
	mcc_ast_visit(ast, &visitor);
	if (data->has_failed) {
		mcc_ir_delete_ir(data->head);
		free(data);
		return NULL;
	}
	struct mcc_ir_row *head = data->head;
	free(data);
	return head;
}

//---------------------------------------------------------------------------------------- Cleanup

void mcc_ir_delete_ir_arg(struct mcc_ir_arg *arg)
{
	if (!arg)
		return;
	if (arg->type == MCC_IR_TYPE_VAR) {
		free(arg->var);
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

