#include "mcc/asm.h"

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "mcc/ir.h"
#include "utils/unused.h"

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
struct mcc_asm_operand *mcc_asm_new_function_operand(char *function_name)
{
	struct mcc_asm_operand *new = malloc(sizeof(*new));
	if (!new)
		return NULL;
	new->type = MCC_ASM_OPERAND_FUNCTION;
	new->func_name = function_name;
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
	struct mcc_asm_operand *print_nl = mcc_asm_new_function_operand("print_nl");
	struct mcc_asm_assembly_line *call = mcc_asm_new_assembly_line(MCC_ASM_CALL, NULL, NULL, NULL);
	if (!print_nl || !call) {
		mcc_asm_delete_assembly_line(call);
		mcc_asm_delete_operand(print_nl);
		return NULL;
	}
	call->first = print_nl;
	// TODO: Implement correctly
	return call;
}

static bool variable_needs_local_space(struct mcc_ir_row *first, struct mcc_ir_row *ir)
{
	assert(first);
	assert(ir);
	switch (ir->instr) {
	case MCC_IR_INSTR_ASSIGN:
		if (ir->arg1->type == MCC_IR_TYPE_ARR_ELEM) {
			return false;
		}
		break;
	default:
		return false;
	}

	char *id_name = ir->arg1->ident->identifier_name;

	struct mcc_ir_row *head = first;
	while (head != ir) {
		if (head->instr != MCC_IR_INSTR_ASSIGN) {
			head = head->next_row;
			continue;
		}
		if (strcmp(head->arg1->ident->identifier_name, id_name) == 0) {
			return false;
		}
		head = head->next_row;
	}
	return true;
}

// TODO: Implement missing cases in switch
static size_t get_var_size(struct mcc_ir_row *ir)
{
	assert(ir);
	assert(ir->instr == MCC_IR_INSTR_ASSIGN);

	switch (ir->arg2->type) {
	case MCC_IR_TYPE_LIT_INT:
		return 4;
	case MCC_IR_TYPE_IDENTIFIER:
		return 4;
	default:
		return 0;
	}
}

// TODO: Implement float,bool and string
static size_t get_stack_frame_size(struct mcc_ir_row *ir)
{
	assert(ir);
	assert(ir->instr == MCC_IR_INSTR_FUNC_LABEL);
	struct mcc_ir_row *last_row = last_line_of_function(ir);
	struct mcc_ir_row *first = ir;
	size_t frame_size = 0;

	// First line is function label
	ir = ir->next_row;

	while (ir && (ir != last_row)) {
		if (variable_needs_local_space(first, ir)) {
			frame_size += get_var_size(ir);
			ir = ir->next_row;
		} else {
			ir = ir->next_row;
		}
	}
	return frame_size;
}

static struct mcc_asm_assembly_line *generate_function_args(struct mcc_asm_function *function, struct mcc_ir_row *ir)
{
	assert(function);
	assert(ir);
	assert(ir->instr == MCC_IR_INSTR_FUNC_LABEL);
	size_t frame_size = get_stack_frame_size(ir);
	struct mcc_asm_operand *esp = mcc_asm_new_register_operand(MCC_ASM_ESP);
	struct mcc_asm_operand *size_literal = mcc_asm_new_literal_operand(frame_size);
	struct mcc_asm_assembly_line *sub_size_esp = mcc_asm_new_assembly_line(MCC_ASM_SUBL, NULL, NULL, NULL);
	if (!esp || !size_literal || !sub_size_esp) {
		mcc_asm_delete_assembly_line(sub_size_esp);
		mcc_asm_delete_operand(esp);
		mcc_asm_delete_operand(size_literal);
		return NULL;
	}
	sub_size_esp->first = size_literal;
	sub_size_esp->second = esp;
	return sub_size_esp;
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
	assert(args->first->type == MCC_ASM_OPERAND_LITERAL);
	function->head = prolog;
	prolog = last_asm_line(prolog);
	// If we remove 0 from ESP, we can remove that line
	if (args->first->literal == 0) {
		prolog->next = body;
		mcc_asm_delete_assembly_line(args);
	} else {
		prolog->next = args;
		args = last_asm_line(args);
		args->next = body;
	}
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
	if (!function) {
		mcc_asm_delete_function(function);
		return NULL;
	}
	struct mcc_asm_assembly_line *prolog = generate_function_prolog();
	struct mcc_asm_assembly_line *args = generate_function_args(function, ir);
	struct mcc_asm_assembly_line *body = generate_function_body(function, ir);
	struct mcc_asm_assembly_line *epilog = generate_function_epilog();
	if (!prolog || !body || !args || !epilog) {
		mcc_asm_delete_all_assembly_lines(prolog);
		mcc_asm_delete_all_assembly_lines(args);
		mcc_asm_delete_all_assembly_lines(body);
		mcc_asm_delete_all_assembly_lines(epilog);
		return NULL;
	}
	compose_function_asm(function, prolog, args, body, epilog);
	return function;
}

static struct mcc_ir_row *find_next_function(struct mcc_ir_row *ir)
{
	assert(ir);
	ir = ir->next_row;
	while (ir && ir->instr != MCC_IR_INSTR_FUNC_LABEL) {
		ir = ir->next_row;
	}
	return ir;
}

struct mcc_asm *mcc_asm_generate(struct mcc_ir_row *ir)
{
	struct mcc_asm *assembly = mcc_asm_new_asm(NULL, NULL);
	struct mcc_asm_function *first_function = mcc_asm_generate_function(ir);
	struct mcc_asm_text_section *text_section = mcc_asm_new_text_section(NULL);
	if (!assembly || !first_function || !text_section) {
		mcc_asm_delete_asm(assembly);
		mcc_asm_delete_function(first_function);
		mcc_asm_delete_text_section(text_section);
		return NULL;
	}
	struct mcc_asm_function *latest_function = first_function;
	ir = find_next_function(ir);
	while (ir) {
		struct mcc_asm_function *new_function = mcc_asm_generate_function(ir);
		if (!new_function) {
			mcc_asm_delete_asm(assembly);
			mcc_asm_delete_all_functions(first_function);
			mcc_asm_delete_text_section(text_section);
			return NULL;
		}
		latest_function->next = new_function;
		latest_function = new_function;
		ir = find_next_function(ir);
	}
	text_section->function = first_function;
	assembly->text_section = text_section;
	assembly->data_section = NULL;
	return assembly;
}
