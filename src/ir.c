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
};

//------------------------------------------------------------------------------ Forward declarations

static struct mcc_ir_row *
mcc_ir_new_row(struct mcc_ir_arg *arg1, struct mcc_ir_arg *arg2, enum mcc_ir_instruction instr);
static struct mcc_ir_arg *mcc_ir_new_arg_lit(char *lit);
static struct mcc_ir_arg *mcc_ir_new_arg_row(struct mcc_ir_row *row);
static void append_row(struct mcc_ir_row *row, struct ir_generation_userdata *data);

//------------------------------------------------------------------------------ Forward declarations, Fake IR

static struct mcc_ir_row *get_fake_ir_line();
static struct mcc_ir_row *get_fake_ir();

//------------------------------------------------------------------------------ Callbacks for visitor that generates IR

static struct mcc_ir_arg *generate_arg_lit(struct mcc_ast_literal *literal)
{
	assert(literal);
	char *buffer;

	if(literal->type == MCC_AST_LITERAL_TYPE_INT){
		size_t size = sizeof(char) * floor(log10(not_zero(literal->i_value)))+1;
		buffer = malloc(size);
		if(!buffer || 0>snprintf(buffer, size, "%ld", literal->i_value)){
			free(buffer);
			return NULL;
		}
	} else if(literal->type == MCC_AST_LITERAL_TYPE_FLOAT){
		size_t size = sizeof(char) * floor(log10(not_zero(literal->f_value)))+8;
		buffer = malloc(size);
		if(!buffer || 0>snprintf(buffer, size, "%f", literal->f_value)){
			free(buffer);
			return NULL;
		}
	} else if(literal->type == MCC_AST_LITERAL_TYPE_BOOL){
		size_t size = sizeof(char) * 6;
		buffer = malloc(size);
		if(!buffer){
			return NULL;
		}
		if(literal->bool_value){
			if(0>snprintf(buffer, size, "true")){
				return NULL;
			}
		} else {
			if(0>snprintf(buffer, size, "false")){
				return NULL;
			}
		}
	} else { // literal->type == MCC_AST_LITERAL_TYPE_STRING
		size_t size = sizeof(char) * strlen(literal->string_value);
		buffer = malloc(size);
		if(!buffer || 0>snprintf(buffer, size, "%s", literal->string_value)){
			return NULL;
		}
	}
	
	struct mcc_ir_arg *arg = mcc_ir_new_arg_lit(buffer);
	return arg;
}

static void generate_ir_expression_literal(struct mcc_ast_expression *expression, void *data)
{
	assert(expression);
	assert(data);
}

static struct mcc_ir_row *generate_ir_expression_binary_op(struct mcc_ast_expression *expression, void *data)
{
	assert(expression->lhs);
	assert(expression->rhs);
	assert(data);

	struct mcc_ir_arg *lhs = NULL, *rhs = NULL;
	if(expression->lhs->type == MCC_AST_EXPRESSION_TYPE_LITERAL){
		lhs = generate_arg_lit(expression->lhs->literal);
	}
	if(expression->rhs->type == MCC_AST_EXPRESSION_TYPE_LITERAL){
		rhs = generate_arg_lit(expression->rhs->literal);
	}
}

static void generate_ir_expression(struct mcc_ast_expression *expression, void *data)
{
	assert(expression);
	assert(data);

	struct mcc_ir_row *row = NULL;

	switch(expression->type){
	case MCC_AST_EXPRESSION_TYPE_LITERAL:
		break;
	case MCC_AST_EXPRESSION_TYPE_BINARY_OP:
		row = generate_ir_expression_binary_op(expression, data);
		break;
	case MCC_AST_EXPRESSION_TYPE_PARENTH:
		break;
	case MCC_AST_EXPRESSION_TYPE_UNARY_OP:
		break;
	case MCC_AST_EXPRESSION_TYPE_VARIABLE:
		break;
	case MCC_AST_EXPRESSION_TYPE_ARRAY_ELEMENT:
		break;
	case MCC_AST_EXPRESSION_TYPE_FUNCTION_CALL:
		break;
	}
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
	append_row(get_fake_ir(program->function->identifier->identifier_name), userdata);
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

static struct mcc_ir_row *get_fake_ir_line(char *name)
{
	size_t size = strlen(name) +1;
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

	arg1->type = MCC_IR_TYPE_LIT;
	arg2->type = MCC_IR_TYPE_LIT;

	char *str1 = malloc(sizeof(char) * size);
	char *str2 = malloc(sizeof(char) * 5);
	if (!str1 || !str2) {
		free(arg1);
		free(arg2);
		free(head);
		free(str1);
		free(str2);
		return NULL;
	}
	snprintf(str1, size, "%s", name);
	snprintf(str2, 5, "var2");
	arg1->lit = str1;
	arg2->lit = str2;
	head->instr = MCC_IR_INSTR_JUMPFALSE;
	head->row_no = 0;
	head->next_row = NULL;
	head->prev_row = NULL;
	head->arg1 = arg1;
	head->arg2 = arg2;
	return head;
}

static struct mcc_ir_row *get_fake_ir(char *name)
{
	struct mcc_ir_row *head = get_fake_ir_line(name);
	if (!head)
		return NULL;
	head->next_row = NULL;
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

static struct mcc_ir_arg *mcc_ir_new_arg_lit(char *lit)
{
	struct mcc_ir_arg *arg = malloc(sizeof(*arg));
	if (!arg)
		return NULL;
	arg->type = MCC_IR_TYPE_LIT;
	arg->lit = lit;
	return arg;
}

static struct mcc_ir_row *
mcc_ir_new_row(struct mcc_ir_arg *arg1, struct mcc_ir_arg *arg2, enum mcc_ir_instruction instr)
{
	struct mcc_ir_row *row = malloc(sizeof(*row));
	if (!row)
		return NULL;
	row->row_no = 0;
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

struct mcc_ir_row *mcc_ir_generate_entry_point(struct mcc_parser_result *result,
                                               struct mcc_symbol_table *table,
                                               enum mcc_parser_entry_point entry_point)
{
	assert(table);

	struct ir_generation_userdata *data = malloc(sizeof(*data));
	if (!data)
		return NULL;
	data->head = NULL;
	data->has_failed = false;
	data->current = NULL;

	struct mcc_ast_visitor visitor = generate_ir_visitor(data);

	switch (entry_point) {
	case MCC_PARSER_ENTRY_POINT_EXPRESSION:
		mcc_ast_visit(result->expression, &visitor);
		break;
	case MCC_PARSER_ENTRY_POINT_PROGRAM:
		mcc_ast_visit(result->program, &visitor);
		break;
	case MCC_PARSER_ENTRY_POINT_DECLARATION:
		mcc_ast_visit(result->declaration, &visitor);
		break;
	case MCC_PARSER_ENTRY_POINT_ASSIGNMENT:
		mcc_ast_visit(result->assignment, &visitor);
		break;
	case MCC_PARSER_ENTRY_POINT_STATEMENT:
		mcc_ast_visit(result->statement, &visitor);
		break;
	case MCC_PARSER_ENTRY_POINT_FUNCTION_DEFINITION:
		mcc_ast_visit(result->function_definition, &visitor);
		break;
	case MCC_PARSER_ENTRY_POINT_ARGUMENTS:
		mcc_ast_visit(result->arguments, &visitor);
		break;
	case MCC_PARSER_ENTRY_POINT_COMPOUND_STATEMENT:
		mcc_ast_visit(result->compound_statement, &visitor);
		break;
	case MCC_PARSER_ENTRY_POINT_PARAMETERS:
		mcc_ast_visit(result->parameters, &visitor);
	}

	if (data->has_failed) {
		mcc_ir_delete_ir(data->head);
		free(data);
		return NULL;
	}
	struct mcc_ir_row *head = data->head;
	free(data);
	number_rows(head);
	return head;
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
	number_rows(head);
	return head;
}

//---------------------------------------------------------------------------------------- Cleanup

void mcc_ir_delete_ir_arg(struct mcc_ir_arg *arg)
{
	if (!arg)
		return;
	if (arg->type == MCC_IR_TYPE_LIT) {
		free(arg->lit);
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
