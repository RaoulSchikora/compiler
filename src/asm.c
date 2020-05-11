#include "mcc/asm.h"

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "mcc/ir.h"
#include "utils/unused.h"

// ----------------------------------------------------------------------- fake asm
// TODO: delete when asm can be generated
struct mcc_asm *generate_fake()
{
	struct mcc_asm *mcc_asm = malloc(sizeof(*mcc_asm));
	struct mcc_asm_text_section *text = malloc(sizeof(*text));
	mcc_asm->text_section = text;
	struct mcc_asm_declaration *decl = mcc_asm_new_float_declaration("a392", 3.4234, NULL);
	struct mcc_asm_data_section *data = mcc_asm_new_data_section(decl);
	mcc_asm->data_section = data;
	struct mcc_asm_function *func = malloc(sizeof(*func));
	text->function = func;
	func->label = (char*)malloc(7 * sizeof(char));
	snprintf(func->label, 6, "main");
	func->next = NULL;
	struct mcc_asm_assembly_line *line1 = malloc(sizeof(*line1));
	struct mcc_asm_assembly_line *line2 = malloc(sizeof(*line2));
	struct mcc_asm_assembly_line *line3 = malloc(sizeof(*line3));
	struct mcc_asm_assembly_line *line4 = malloc(sizeof(*line4));
	struct mcc_asm_assembly_line *line5 = malloc(sizeof(*line5));
	struct mcc_asm_assembly_line *line6 = malloc(sizeof(*line6));
	func->head = line1;
	struct mcc_asm_operand *op11 = malloc(sizeof(*op11));
	op11->type = MCC_ASM_OPERAND_REGISTER;
	op11->reg = MCC_ASM_EBP;
	line1->first = op11;
	line1->second = NULL;
	line1->opcode = MCC_ASM_PUSHL;
	struct mcc_asm_operand *op21 = malloc(sizeof(*op21));
	op21->type = MCC_ASM_OPERAND_REGISTER;
	op21->reg = MCC_ASM_ESP;
	struct mcc_asm_operand *op22 = malloc(sizeof(*op22));
	op22->type = MCC_ASM_OPERAND_REGISTER;
	op22->reg = MCC_ASM_EBP;
	line2->first = op21;
	line2->second = op22;
	line2->opcode = MCC_ASM_MOVL;
	struct mcc_asm_operand *op31 = malloc(sizeof(*op31));
	op31->type = MCC_ASM_OPERAND_LITERAL;
	op31->literal = 4;
	struct mcc_asm_operand *op32 = malloc(sizeof(*op32));
	op32->type = MCC_ASM_OPERAND_REGISTER;
	op32->reg = MCC_ASM_ESP;
	line3->first = op31;
	line3->second = op32;
	line3->opcode = MCC_ASM_SUBL;
	struct mcc_asm_operand *op41 = malloc(sizeof(*op41));
	op41->type = MCC_ASM_OPERAND_LITERAL;
	op41->literal = 0;
	struct mcc_asm_operand *op42 = malloc(sizeof(*op42));
	op42->type = MCC_ASM_OPERAND_REGISTER;
	op42->reg = MCC_ASM_EAX;
	line4->first = op41;
	line4->second = op42;
	line4->opcode = MCC_ASM_MOVL;
	line5->opcode = MCC_ASM_LEAVE;
	line5->first = NULL;
	line5->second = NULL;
	line6->opcode = MCC_ASM_RETURN;
	line6->first = NULL;
	line6->second = NULL;
	line1->next = line2;
	line2->next = line3;
	line3->next = line4;
	line4->next = line5;
	line5->next = line6;
	line6->next = NULL;
	return mcc_asm;
}
//---------------------------------------------------------------------------------------- Functions: Data structures

struct mcc_asm *mcc_asm_new_asm(struct mcc_asm_data_section *data, struct mcc_asm_text_section *text)
{
	struct mcc_asm *new = malloc(sizeof(*new));
	if (!new)
		return NULL;
	new->data_section = data;
	new->text_section = text;
	return new;
}

struct mcc_asm_data_section *mcc_asm_new_data_section(struct mcc_asm_declaration *head)
{
	struct mcc_asm_data_section *new = malloc(sizeof(*new));
	if (!new)
		return NULL;
	new->head = head;
	return new;
}

struct mcc_asm_text_section *mcc_asm_new_text_section(struct mcc_asm_function *function)
{
	struct mcc_asm_text_section *new = malloc(sizeof(*new));
	if (!new)
		return NULL;
	new->function = function;
	return new;
}

struct mcc_asm_declaration *
mcc_asm_new_float_declaration(char *identifier, float float_value, struct mcc_asm_declaration *next)
{
	struct mcc_asm_declaration *new = malloc(sizeof(*new));
	if (!new)
		return NULL;
	new->identifier = identifier;
	new->float_value = float_value;
	new->next = next;
	new->type = MCC_ASM_DECLARATION_TYPE_FLOAT;
	return new;
}

struct mcc_asm_declaration *
mcc_asm_new_db_declaration(char *identifier, char *db_value, struct mcc_asm_declaration *next)
{
	struct mcc_asm_declaration *new = malloc(sizeof(*new));
	if (!new)
		return NULL;
	new->identifier = identifier;
	new->db_value = db_value;
	new->next = next;
	new->type = MCC_ASM_DECLARATION_TYPE_DB;
	return new;
}

struct mcc_asm_function *
mcc_asm_new_function(char *label, struct mcc_asm_assembly_line *head, struct mcc_asm_function *next)
{
	struct mcc_asm_function *new = malloc(sizeof(*new));
	if (!new)
		return NULL;
	new->head = head;
	new->label = label;
	new->next = next;
	return new;
}

struct mcc_asm_assembly_line *mcc_asm_new_assembly_line(enum mcc_asm_opcode opcode,
                                                        struct mcc_asm_operand *first,
                                                        struct mcc_asm_operand *second,
                                                        struct mcc_asm_assembly_line *next)
{
	struct mcc_asm_assembly_line *new = malloc(sizeof(*new));
	if (!new)
		return NULL;
	new->opcode = opcode;
	new->first = first;
	new->second = second;
	new->next = next;
	return new;
}

struct mcc_asm_operand *mcc_asm_new_literal_operand(int literal)
{
	struct mcc_asm_operand *new = malloc(sizeof(*new));
	if (!new)
		return NULL;
	new->type = MCC_ASM_OPERAND_LITERAL;
	new->literal = literal;
	return new;
}

struct mcc_asm_operand *mcc_asm_new_register_operand(enum mcc_asm_register reg)
{
	struct mcc_asm_operand *new = malloc(sizeof(*new));
	if (!new)
		return NULL;
	new->type = MCC_ASM_OPERAND_REGISTER;
	new->reg = reg;
	return new;
}

struct mcc_asm_operand *mcc_asm_new_data_operand(struct mcc_asm_declaration *decl)
{
	struct mcc_asm_operand *new = malloc(sizeof(*new));
	if (!new)
		return NULL;
	new->type = MCC_ASM_OPERAND_DATA;
	new->decl = decl;
	return new;
}

//------------------------------------------------------------------------------------ Functions: Delete data structures

void mcc_asm_delete_asm(struct mcc_asm *head)
{
	if (!head)
		return;
	mcc_asm_delete_text_section(head->text_section);
	mcc_asm_delete_data_section(head->data_section);
	free(head);
}
void mcc_asm_delete_text_section(struct mcc_asm_text_section *text_section)
{
	if (!text_section)
		return;
	mcc_asm_delete_all_functions(text_section->function);
	free(text_section);
}
void mcc_asm_delete_data_section(struct mcc_asm_data_section *data_section)
{
	if (!data_section)
		return;
	mcc_asm_delete_all_declarations(data_section->head);
	free(data_section);
}

void mcc_asm_delete_all_declarations(struct mcc_asm_declaration *decl)
{
	if (!decl)
		return;
	mcc_asm_delete_all_declarations(decl->next);
	mcc_asm_delete_declaration(decl);
}

void mcc_asm_delete_declaration(struct mcc_asm_declaration *decl)
{
	if (!decl)
		return;
	free(decl);
}

void mcc_asm_delete_all_functions(struct mcc_asm_function *function)
{
	if (!function)
		return;
	mcc_asm_delete_all_functions(function->next);
	mcc_asm_delete_function(function);
}

void mcc_asm_delete_function(struct mcc_asm_function *function)
{
	if (!function)
		return;
	mcc_asm_delete_all_assembly_lines(function->head);
	free(function->label);
	free(function);
}

void mcc_asm_delete_all_assembly_lines(struct mcc_asm_assembly_line *line)
{
	if (!line)
		return;
	mcc_asm_delete_all_assembly_lines(line->next);
	mcc_asm_delete_assembly_line(line);
}

void mcc_asm_delete_assembly_line(struct mcc_asm_assembly_line *line)
{
	if (!line)
		return;
	mcc_asm_delete_operand(line->first);
	mcc_asm_delete_operand(line->second);
	free(line);
}
void mcc_asm_delete_operand(struct mcc_asm_operand *operand)
{
	if (!operand)
		return;
	free(operand);
}

//---------------------------------------------------------------------------------------- Functions: Dummy ASM line

static struct mcc_asm_assembly_line *get_dummy_line()
{
	struct mcc_asm_assembly_line *line = mcc_asm_new_assembly_line(MCC_ASM_PUSHL, NULL, NULL, NULL);
	return line;
}

//---------------------------------------------------------------------------------------- Functions: ASM generation

static struct mcc_ir_row *last_line_of_function(struct mcc_ir_row *ir)
{
	assert(ir->instr == MCC_IR_INSTR_FUNC_LABEL);
	assert(ir);
	struct mcc_ir_row *next = ir->next_row;
	while (next) {
		if (next->instr == MCC_IR_INSTR_FUNC_LABEL) {
			return ir;
		}
		ir = ir->next_row;
		next = next->next_row;
	}
	return ir;
}

static struct mcc_asm_assembly_line *last_asm_line(struct mcc_asm_assembly_line *head)
{
	assert(head);
	while (head->next) {
		head = head->next;
	}
	return head;
}

static struct mcc_asm_assembly_line *generate_function_prolog()
{
	struct mcc_asm_operand *ebp = mcc_asm_new_register_operand(MCC_ASM_EBP);
	struct mcc_asm_operand *ebp_2 = mcc_asm_new_register_operand(MCC_ASM_EBP);
	struct mcc_asm_operand *esp = mcc_asm_new_register_operand(MCC_ASM_ESP);
	struct mcc_asm_assembly_line *push_ebp = mcc_asm_new_assembly_line(MCC_ASM_PUSHL, ebp, NULL, NULL);
	struct mcc_asm_assembly_line *mov_ebp_esp = mcc_asm_new_assembly_line(MCC_ASM_MOVL, esp, ebp_2, NULL);
	if (!ebp || !esp || !push_ebp || !ebp_2 || !mov_ebp_esp) {
		mcc_asm_delete_operand(ebp);
		mcc_asm_delete_operand(ebp_2);
		mcc_asm_delete_operand(esp);
		mcc_asm_delete_assembly_line(push_ebp);
		mcc_asm_delete_assembly_line(mov_ebp_esp);
		return NULL;
	}
	push_ebp->next = mov_ebp_esp;
	return push_ebp;
}

static struct mcc_asm_assembly_line *generate_function_body(struct mcc_asm_function *function, struct mcc_ir_row *ir)
{
	assert(function);
	assert(ir);
	assert(ir->instr == MCC_IR_INSTR_FUNC_LABEL);
	// TODO: Implement correctly
	return get_dummy_line();
}

static struct mcc_asm_assembly_line *generate_function_args(struct mcc_asm_function *function, struct mcc_ir_row *ir)
{
	assert(function);
	assert(ir);
	assert(ir->instr == MCC_IR_INSTR_FUNC_LABEL);
	// TODO: Implement correctly
	return get_dummy_line();
}

static struct mcc_asm_assembly_line *generate_function_epilog()
{
	struct mcc_asm_assembly_line *leave = mcc_asm_new_assembly_line(MCC_ASM_LEAVE, NULL, NULL, NULL);
	struct mcc_asm_assembly_line *ret = mcc_asm_new_assembly_line(MCC_ASM_RETURN, NULL, NULL, NULL);
	if (!leave || !ret) {
		mcc_asm_delete_assembly_line(leave);
		mcc_asm_delete_assembly_line(ret);
		return NULL;
	}
	leave->next = ret;
	return leave;
}

static void compose_function_asm(struct mcc_asm_function *function,
                                 struct mcc_asm_assembly_line *prolog,
                                 struct mcc_asm_assembly_line *args,
                                 struct mcc_asm_assembly_line *body,
                                 struct mcc_asm_assembly_line *epilog)
{
	assert(prolog);
	assert(body);
	assert(epilog);
	assert(function);
	function->head = prolog;
	prolog = last_asm_line(prolog);
	prolog->next = args;
	args = last_asm_line(args);
	args->next = body;
	body = last_asm_line(body);
	body->next = epilog;
}

struct mcc_asm_function *mcc_asm_generate_function(struct mcc_ir_row *ir)
{
	assert(ir->instr == MCC_IR_INSTR_FUNC_LABEL);
	assert(ir);
	assert(ir->arg1->type == MCC_IR_TYPE_FUNC_LABEL);

	char *label = strdup(ir->arg1->func_label);
	if (!label)
		return NULL;
	struct mcc_asm_function *function = mcc_asm_new_function(label, NULL, NULL);
	struct mcc_asm_assembly_line *prolog = generate_function_prolog();
	struct mcc_asm_assembly_line *args = generate_function_args(function, ir);
	struct mcc_asm_assembly_line *body = generate_function_body(function, ir);
	struct mcc_asm_assembly_line *epilog = generate_function_epilog();
	if (!function || !prolog || !body || !args || !epilog) {
		mcc_asm_delete_function(function);
		mcc_asm_delete_all_assembly_lines(prolog);
		mcc_asm_delete_all_assembly_lines(args);
		mcc_asm_delete_all_assembly_lines(body);
		mcc_asm_delete_all_assembly_lines(epilog);
		return NULL;
	}
	compose_function_asm(function, prolog, args, body, epilog);
	return function;
}

struct mcc_asm *mcc_asm_generate(struct mcc_ir_row *ir)
{
	struct mcc_asm *assembly = mcc_asm_new_asm(NULL, NULL);
	struct mcc_asm_function *function = mcc_asm_generate_function(ir);
	struct mcc_asm_text_section *text_section = mcc_asm_new_text_section(NULL);
	if (!assembly || !function || !text_section) {
		mcc_asm_delete_asm(assembly);
		mcc_asm_delete_function(function);
		mcc_asm_delete_text_section(text_section);
		return NULL;
	}
	text_section->function = function;
	assembly->text_section = text_section;
	assembly->data_section = NULL;
	return assembly;
}

