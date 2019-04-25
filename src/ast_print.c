#include "mcc/ast_print.h"

#include <assert.h>

#include "mcc/ast_visit.h"

const char *mcc_ast_print_binary_op(enum mcc_ast_binary_op op)
{
	switch (op) {
	case MCC_AST_BINARY_OP_ADD:
		return "+";
	case MCC_AST_BINARY_OP_SUB:
		return "-";
	case MCC_AST_BINARY_OP_MUL:
		return "*";
	case MCC_AST_BINARY_OP_DIV:
		return "/";
	case MCC_AST_BINARY_OP_SMALLER:
		return "<";
	case MCC_AST_BINARY_OP_GREATER:
		return ">";
	case MCC_AST_BINARY_OP_SMALLEREQ:
		return "<=";
	case MCC_AST_BINARY_OP_GREATEREQ:
		return ">=";
	case MCC_AST_BINARY_OP_CONJ:
		return "&&";
	case MCC_AST_BINARY_OP_DISJ:
		return "||";
	case MCC_AST_BINARY_OP_EQUAL:
		return "==";
	case MCC_AST_BINARY_OP_NOTEQUAL:
		return "!=";
	}

	return "unknown op";
}

const char *mcc_ast_print_unary_op(enum mcc_ast_unary_op u_op)
{

	switch (u_op) {
	case MCC_AST_UNARY_OP_NEGATIV:
		return "- ( )";
	case MCC_AST_UNARY_OP_NOT:
		return "!";
	}

	return "unknown u_op";
}

// ---------------------------------------------------------------- DOT Printer

#define LABEL_SIZE 64

static void print_dot_begin(FILE *out)
{
	assert(out);

	fprintf(out, "digraph \"AST\" {\n"
	             "\tnodesep=0.6\n");
}

static void print_dot_end(FILE *out)
{
	assert(out);

	fprintf(out, "}\n");
}

static void print_dot_node(FILE *out, const void *node, const char *label)
{
	assert(out);
	assert(node);
	assert(label);

	fprintf(out, "\t\"%p\" [shape=box, label=\"%s\"];\n", node, label);
}

static void print_dot_edge(FILE *out, const void *src_node, const void *dst_node, const char *label)
{
	assert(out);
	assert(src_node);
	assert(dst_node);
	assert(label);

	fprintf(out, "\t\"%p\" -> \"%p\" [label=\"%s\"];\n", src_node, dst_node, label);
}

static void print_dot_expression_literal(struct mcc_ast_expression *expression, void *data)
{
	assert(expression);
	assert(data);

	FILE *out = data;
	print_dot_node(out, expression, "expr: lit");
	print_dot_edge(out, expression, expression->literal, "literal");
}

static void print_dot_expression_binary_op(struct mcc_ast_expression *expression, void *data)
{
	assert(expression);
	assert(data);

	char label[LABEL_SIZE] = {0};
	snprintf(label, sizeof(label), "expr: %s", mcc_ast_print_binary_op(expression->op));

	FILE *out = data;
	print_dot_node(out, expression, label);
	print_dot_edge(out, expression, expression->lhs, "lhs");
	print_dot_edge(out, expression, expression->rhs, "rhs");
}

static void print_dot_expression_parenth(struct mcc_ast_expression *expression, void *data)
{
	assert(expression);
	assert(data);

	FILE *out = data;
	print_dot_node(out, expression, "( )");
	print_dot_edge(out, expression, expression->expression, "expression");
}

static void print_dot_expression_unary_op(struct mcc_ast_expression *expression, void *data)
{
	assert(expression);
	assert(data);

	char label[LABEL_SIZE] = {0};
	snprintf(label, sizeof(label), "expr: %s", mcc_ast_print_unary_op(expression->op));

	FILE *out = data;
	print_dot_node(out, expression, label);
	print_dot_edge(out, expression, expression->child, "child");
}

static void print_dot_expression_array_element(struct mcc_ast_expression *expression, void *data)
{
	assert(expression);
	assert(data);

	char label[LABEL_SIZE] = {0};
	snprintf(label, sizeof(label), "expr: arr_el");

	FILE *out = data;
	print_dot_node(out, expression, label);
	print_dot_edge(out, expression, expression->index, "index");
	print_dot_edge(out, expression, expression->array_identifier, "id");
}

static void print_dot_expression_variable(struct mcc_ast_expression *expression, void *data)
{
	assert(expression);
	assert(data);

	char label[LABEL_SIZE] = {0};
	snprintf(label, sizeof(label), "expr: var");

	FILE *out = data;
	print_dot_node(out, expression, label);
	print_dot_edge(out, expression, expression->identifier, "id");
}

static void print_dot_expression_function_call(struct mcc_ast_expression *expression, void *data)
{
	assert(expression);
	assert(data);

	char label[LABEL_SIZE] = {0};
	if (expression->arguments->is_empty) {
		snprintf(label, sizeof(label), "expr: funct. call, no args");
	} else {
		snprintf(label, sizeof(label), "expr: funct. call");
	}

	FILE *out = data;
	print_dot_node(out, expression, label);
	print_dot_edge(out, expression, expression->function_identifier, "func id");
	if (!expression->arguments->is_empty) {
		print_dot_edge(out, expression, expression->arguments, "func args");
	}
}

static void print_dot_statememt_if_stmt(struct mcc_ast_statement *statement, void *data)
{
	assert(statement);
	assert(data);

	FILE *out = data;
	print_dot_node(out, statement, "if");
	print_dot_edge(out, statement, statement->if_condition, "cond");
	print_dot_edge(out, statement, statement->if_on_true, "on_true");
}

static void print_dot_statement_if_else_stmt(struct mcc_ast_statement *statement, void *data)
{
	assert(statement);
	assert(data);

	FILE *out = data;
	print_dot_node(out, statement, "if_else");
	print_dot_edge(out, statement, statement->if_else_condition, "cond");
	print_dot_edge(out, statement, statement->if_else_on_true, "on_true");
	print_dot_edge(out, statement, statement->if_else_on_false, "on_false");
}

static void print_dot_statement_expression_stmt(struct mcc_ast_statement *statement, void *data)
{
	assert(statement);
	assert(data);

	FILE *out = data;
	print_dot_node(out, statement, "stmt: exp;");
	print_dot_edge(out, statement, statement->stmt_expression, "expr");
}

static void print_dot_statement_while(struct mcc_ast_statement *statement, void *data)
{
	assert(statement);
	assert(data);

	FILE *out = data;
	print_dot_node(out, statement, "while");
	print_dot_edge(out, statement, statement->if_condition, "cond");
	print_dot_edge(out, statement, statement->if_on_true, "on_true");
}

static void print_dot_literal_int(struct mcc_ast_literal *literal, void *data)
{
	assert(literal);
	assert(data);

	char label[LABEL_SIZE] = {0};
	snprintf(label, sizeof(label), "%ld", literal->i_value);

	FILE *out = data;
	print_dot_node(out, literal, label);
}

static void print_dot_literal_float(struct mcc_ast_literal *literal, void *data)
{
	assert(literal);
	assert(data);

	char label[LABEL_SIZE] = {0};
	snprintf(label, sizeof(label), "%f", literal->f_value);

	FILE *out = data;
	print_dot_node(out, literal, label);
}

static void print_dot_literal_string(struct mcc_ast_literal *literal, void *data)
{
	assert(literal);
	assert(data);

	char label[LABEL_SIZE] = {0};
	snprintf(label, sizeof(label), "%s", literal->string_value);

	FILE *out = data;
	print_dot_node(out, literal, label);
}

static void print_dot_literal_bool(struct mcc_ast_literal *literal, void *data)
{
	assert(literal);
	assert(data);

	FILE *out = data;
	print_dot_node(out, literal, literal->bool_value ? "true" : "false");
}

static void print_dot_variable_declaration(struct mcc_ast_declaration *declaration, void *data)
{
	assert(declaration);
	assert(data);

	FILE *out = data;
	print_dot_node(out, declaration, "decl: var;");
	print_dot_edge(out, declaration, declaration->variable_type, "type");
	print_dot_edge(out, declaration, declaration->variable_identifier, "id");
}

static void print_dot_array_declaration(struct mcc_ast_declaration *declaration, void *data)
{
	assert(declaration);
	assert(data);

	FILE *out = data;
	print_dot_node(out, declaration, "decl: array;");
	print_dot_edge(out, declaration, declaration->array_type, "type");
	print_dot_edge(out, declaration, declaration->array_identifier, "id");
	print_dot_edge(out, declaration, declaration->array_size, "size");
}

static void print_dot_variable_assignment(struct mcc_ast_assignment *assignment, void *data)
{
	assert(assignment);
	assert(data);

	FILE *out = data;
	print_dot_node(out, assignment, "assign: var;");
	print_dot_edge(out, assignment, assignment->variable_assigned_value, "value");
	print_dot_edge(out, assignment, assignment->variable_identifier, "id");
}

static void print_dot_array_assignment(struct mcc_ast_assignment *assignment, void *data)
{
	assert(assignment);
	assert(data);

	FILE *out = data;
	print_dot_node(out, assignment, "assign: array;");
	print_dot_edge(out, assignment, assignment->array_index, "index");
	print_dot_edge(out, assignment, assignment->array_identifier, "id");
	print_dot_edge(out, assignment, assignment->array_assigned_value, "value");
}

static void print_dot_assignment(struct mcc_ast_assignment *assignment, void *data)
{
	assert(assignment);
	assert(data);
	switch (assignment->assignment_type) {
	case MCC_AST_ASSIGNMENT_TYPE_VARIABLE:
		print_dot_variable_assignment(assignment, data);
		break;
	case MCC_AST_ASSIGNMENT_TYPE_ARRAY:
		print_dot_array_assignment(assignment, data);
		break;
	}
}

static void print_dot_declaration(struct mcc_ast_declaration *declaration, void *data)
{
	assert(declaration);
	assert(data);
	switch (declaration->declaration_type) {
	case MCC_AST_DECLARATION_TYPE_VARIABLE:
		print_dot_variable_declaration(declaration, data);
		break;
	case MCC_AST_DECLARATION_TYPE_ARRAY:
		print_dot_array_declaration(declaration, data);
		break;
	}
}

static void print_dot_type(struct mcc_ast_type *type, void *data)
{

	assert(data);
	assert(type);

	FILE *out = data;
	switch (type->type_value) {
	case INT:
		print_dot_node(out, type, "int");
		break;
	case FLOAT:
		print_dot_node(out, type, "float");
		break;
	case BOOL:
		print_dot_node(out, type, "bool");
		break;
	case STRING:
		print_dot_node(out, type, "string");
		break;
	}
}

static void print_dot_expression_identifier(struct mcc_ast_identifier *identifier, void *data)
{
	assert(identifier);
	assert(data);

	char label[LABEL_SIZE] = {0};
	snprintf(label, sizeof(label), "%s", identifier->identifier_name);

	FILE *out = data;
	print_dot_node(out, identifier, label);
}

static void print_dot_statement_declaration(struct mcc_ast_statement *statement, void *data)
{

	assert(statement);
	assert(data);

	FILE *out = data;
	print_dot_node(out, statement, "stmt: decl.");
	print_dot_edge(out, statement, statement->declaration, "decl.");
}

static void print_dot_statement_assignment(struct mcc_ast_statement *statement, void *data)
{

	assert(statement);
	assert(data);

	FILE *out = data;
	print_dot_node(out, statement, "stmt: assign");
	print_dot_edge(out, statement, statement->assignment, "assign");
}

static void print_dot_statement_return(struct mcc_ast_statement *statement, void *data)
{
	assert(statement);
	assert(data);

	FILE *out = data;
	if (statement->is_empty_return) {
		print_dot_node(out, statement, "stmt:empty_ret");
	} else {
		print_dot_node(out, statement, "stmt:ret");
		print_dot_edge(out, statement, statement->return_value, "ret_val");
	}
}

static void print_dot_statement_compound_statement(struct mcc_ast_statement *statement, void *data)
{
	assert(statement);
	assert(data);

	FILE *out = data;
	print_dot_node(out, statement, "stmt: comp_stmt");
	print_dot_edge(out, statement, statement->compound_statement, "comp_stmt");
}

static void print_dot_compound_statement(struct mcc_ast_compound_statement *compound_statement, void *data)
{
	assert(compound_statement);
	assert(data);

	FILE *out = data;
	if (compound_statement->is_empty) {
		print_dot_node(out, compound_statement, "empty comp_stmt");
	} else {
		print_dot_node(out, compound_statement, "comp_stmt");
	}
	if (compound_statement->is_empty == false) {
		print_dot_edge(out, compound_statement, compound_statement->statement, "stmt");
	}
	if (compound_statement->has_next_statement == true) {
		print_dot_edge(out, compound_statement, compound_statement->next_compound_statement, "next stmt:");
	}
}

static void print_dot_program(struct mcc_ast_program *program, void *data)
{
	assert(program);
	assert(data);

	FILE *out = data;
	print_dot_node(out, program, "program");
	print_dot_edge(out, program, program->function, "func");
	if (program->has_next_function == true) {
		print_dot_edge(out, program, program->next_function, "next func");
	}
}

static void print_dot_parameters(struct mcc_ast_parameters *parameters, void *data)
{
	assert(parameters);
	assert(data);

	FILE *out = data;
	if (!(parameters->is_empty)) {
		print_dot_edge(out, parameters, parameters->declaration, "decl");
		print_dot_node(out, parameters, "param");
	} else {
		print_dot_node(out, parameters, "empty param");
	}
	if (parameters->has_next_parameter == true) {
		print_dot_edge(out, parameters, parameters->next_parameters, "next param");
	}
}

static void print_dot_arguments(struct mcc_ast_arguments *arguments, void *data)
{
	assert(arguments);
	assert(data);

	FILE *out = data;
	print_dot_node(out, arguments, "args");
	print_dot_edge(out, arguments, arguments->expression, "expr");
	if (arguments->has_next_expression == true) {
		print_dot_edge(out, arguments, arguments->next_arguments, "next arg");
	}
}

static void print_dot_function_definition(struct mcc_ast_function_definition *function_definition, void *data)
{
	assert(function_definition);
	assert(data);

	FILE *out = data;
	print_dot_node(out, function_definition, "func def");
	print_dot_edge(out, function_definition, function_definition->identifier, "id");
	print_dot_edge(out, function_definition, function_definition->parameters, "param");
	print_dot_edge(out, function_definition, function_definition->compound_stmt, "comp_stmt");
	print_dot_edge(out, function_definition, &function_definition->type, "type");
	switch (function_definition->type) {
	case MCC_AST_FUNCTION_TYPE_INT:
		print_dot_node(out, &function_definition->type, "Int");
		break;
	case MCC_AST_FUNCTION_TYPE_FLOAT:
		print_dot_node(out, &function_definition->type, "Float");
		break;
	case MCC_AST_FUNCTION_TYPE_STRING:
		print_dot_node(out, &function_definition->type, "String");
		break;
	case MCC_AST_FUNCTION_TYPE_BOOL:
		print_dot_node(out, &function_definition->type, "Bool");
		break;
	case MCC_AST_FUNCTION_TYPE_VOID:
		print_dot_node(out, &function_definition->type, "Void");
		break;
	}
}

// Setup an AST Visitor for printing.
static struct mcc_ast_visitor print_dot_visitor(FILE *out)
{
	assert(out);

	return (struct mcc_ast_visitor){
	    .traversal = MCC_AST_VISIT_DEPTH_FIRST,
	    .order = MCC_AST_VISIT_PRE_ORDER,

	    .userdata = out,

	    .expression_literal = print_dot_expression_literal,
	    .expression_binary_op = print_dot_expression_binary_op,
	    .expression_parenth = print_dot_expression_parenth,
	    .expression_unary_op = print_dot_expression_unary_op,
	    .expression_variable = print_dot_expression_variable,
	    .expression_array_element = print_dot_expression_array_element,
	    .expression_function_call = print_dot_expression_function_call,

	    .literal_int = print_dot_literal_int,
	    .literal_float = print_dot_literal_float,
	    .literal_bool = print_dot_literal_bool,
	    .literal_string = print_dot_literal_string,

	    .statement_if_stmt = print_dot_statememt_if_stmt,
	    .statement_if_else_stmt = print_dot_statement_if_else_stmt,
	    .statement_expression_stmt = print_dot_statement_expression_stmt,
	    .statement_while = print_dot_statement_while,
	    .statement_assignment = print_dot_statement_assignment,
	    .statement_declaration = print_dot_statement_declaration,
	    .statement_return = print_dot_statement_return,
	    .statement_compound_stmt = print_dot_statement_compound_statement,

	    .compound_statement = print_dot_compound_statement,
	    .program = print_dot_program,
	    .function_definition = print_dot_function_definition,
	    .parameters = print_dot_parameters,
	    .arguments = print_dot_arguments,

	    .assignment = print_dot_assignment,
	    .variable_assignment = print_dot_variable_assignment,
	    .array_assignment = print_dot_array_assignment,
	    .declaration = print_dot_declaration,
	    .variable_declaration = print_dot_variable_declaration,
	    .array_declaration = print_dot_array_declaration,

	    .type = print_dot_type,
	    .identifier = print_dot_expression_identifier,
	};
}

void mcc_ast_print_dot_expression(FILE *out, struct mcc_ast_expression *expression)
{
	assert(out);
	assert(expression);

	print_dot_begin(out);

	struct mcc_ast_visitor visitor = print_dot_visitor(out);
	mcc_ast_visit(expression, &visitor);

	print_dot_end(out);
}

void mcc_ast_print_dot_statement(FILE *out, struct mcc_ast_statement *statement)
{
	assert(out);
	assert(statement);

	print_dot_begin(out);

	struct mcc_ast_visitor visitor = print_dot_visitor(out);
	mcc_ast_visit(statement, &visitor);

	print_dot_end(out);
}

void mcc_ast_print_dot_declaration(FILE *out, struct mcc_ast_declaration *declaration)
{
	assert(out);
	assert(declaration);

	print_dot_begin(out);

	struct mcc_ast_visitor visitor = print_dot_visitor(out);
	mcc_ast_visit(declaration, &visitor);

	print_dot_end(out);
}

void mcc_ast_print_dot_assignment(FILE *out, struct mcc_ast_assignment *assignment)
{
	assert(out);
	assert(assignment);

	print_dot_begin(out);

	struct mcc_ast_visitor visitor = print_dot_visitor(out);
	mcc_ast_visit(assignment, &visitor);

	print_dot_end(out);
}

void mcc_ast_print_dot_literal(FILE *out, struct mcc_ast_literal *literal)
{
	assert(out);
	assert(literal);

	print_dot_begin(out);

	struct mcc_ast_visitor visitor = print_dot_visitor(out);
	mcc_ast_visit(literal, &visitor);

	print_dot_end(out);
}

void mcc_ast_print_dot_type(FILE *out, struct mcc_ast_type *type)
{
	assert(out);
	assert(type);

	print_dot_begin(out);

	struct mcc_ast_visitor visitor = print_dot_visitor(out);
	mcc_ast_visit(type, &visitor);

	print_dot_end(out);
}

void mcc_ast_print_dot_identifier(FILE *out, struct mcc_ast_identifier *identifier)
{
	assert(out);
	assert(identifier);

	print_dot_begin(out);

	struct mcc_ast_visitor visitor = print_dot_visitor(out);
	mcc_ast_visit(identifier, &visitor);

	print_dot_end(out);
}

void mcc_ast_print_dot_result(FILE *out, struct mcc_parser_result *result)
{
	assert(out);
	assert(result);

	enum mcc_parser_entry_point entry_point = result->entry_point;

	switch (entry_point) {
	case MCC_PARSER_ENTRY_POINT_EXPRESSION:;
		mcc_ast_print_dot(out, result->expression);
		break;
	case MCC_PARSER_ENTRY_POINT_STATEMENT:;
		mcc_ast_print_dot(out, result->statement);
		break;
	case MCC_PARSER_ENTRY_POINT_DECLARATION:;
		mcc_ast_print_dot(out, result->declaration);
		break;
	case MCC_PARSER_ENTRY_POINT_ASSIGNMENT:;
		mcc_ast_print_dot(out, result->assignment);
		break;
	case MCC_PARSER_ENTRY_POINT_PROGRAM:
		mcc_ast_print_dot(out, result->program);
		break;
	case MCC_PARSER_ENTRY_POINT_FUNCTION_DEFINITION:
		mcc_ast_print_dot(out, result->function_definition);
		break;
	case MCC_PARSER_ENTRY_POINT_PARAMETERS:
		mcc_ast_print_dot(out, result->parameters);
		break;
	case MCC_PARSER_ENTRY_POINT_ARGUMENTS:
		mcc_ast_print_dot(out, result->arguments);
		break;
	case MCC_PARSER_ENTRY_POINT_COMPOUND_STATEMENT:
		mcc_ast_print_dot(out, result->compound_statement);
		break;
	}
}

void mcc_ast_print_dot_compound_statement(FILE *out, struct mcc_ast_compound_statement *compound_statement)
{
	assert(out);
	assert(compound_statement);

	print_dot_begin(out);
	struct mcc_ast_visitor visitor = print_dot_visitor(out);
	mcc_ast_visit(compound_statement, &visitor);
	print_dot_end(out);
}

void mcc_ast_print_dot_program(FILE *out, struct mcc_ast_program *program)
{
	assert(out);
	assert(program);

	print_dot_begin(out);
	struct mcc_ast_visitor visitor = print_dot_visitor(out);
	mcc_ast_visit(program, &visitor);
	print_dot_end(out);
}

void mcc_ast_print_dot_function_definition(FILE *out, struct mcc_ast_function_definition *function_definition)
{
	assert(out);
	assert(function_definition);

	print_dot_begin(out);
	struct mcc_ast_visitor visitor = print_dot_visitor(out);
	mcc_ast_visit(function_definition, &visitor);
	print_dot_end(out);
}

void mcc_ast_print_dot_parameters(FILE *out, struct mcc_ast_parameters *parameters)
{
	assert(out);
	assert(parameters);

	print_dot_begin(out);
	struct mcc_ast_visitor visitor = print_dot_visitor(out);
	mcc_ast_visit(parameters, &visitor);
	print_dot_end(out);
}

void mcc_ast_print_dot_arguments(FILE *out, struct mcc_ast_arguments *arguments)
{
	assert(out);
	assert(arguments);

	print_dot_begin(out);
	struct mcc_ast_visitor visitor = print_dot_visitor(out);
	mcc_ast_visit(arguments, &visitor);
	print_dot_end(out);
}
