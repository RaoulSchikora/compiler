#include "mcc/asm.h"

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "mcc/ir.h"
#include "utils/unused.h"

//---------------------------------------------------------------------------------------- Forward declarations

static size_t get_stack_frame_size(struct mcc_ir_row *ir);

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

struct mcc_asm_declaration *mcc_asm_new_array_declaration(char *identifier,
                                                          int size,
                                                          enum mcc_asm_declaration_type type,
                                                          struct mcc_asm_declaration *next)
{
	struct mcc_asm_declaration *new = malloc(sizeof(*new));
	if (!new)
		return NULL;
	new->identifier = identifier;
	new->array_size = size;
	new->next = next;
	new->type = type;
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
	new->ebp_offset = 0;
	new->pos_list = NULL;
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
	new->offset = 0;
	return new;
}
struct mcc_asm_operand *mcc_asm_new_literal_operand(int literal)
{
	struct mcc_asm_operand *new = malloc(sizeof(*new));
	if (!new)
		return NULL;
	new->type = MCC_ASM_OPERAND_LITERAL;
	new->literal = literal;
	new->offset = 0;
	return new;
}

struct mcc_asm_operand *mcc_asm_new_register_operand(enum mcc_asm_register reg, int offset)
{
	struct mcc_asm_operand *new = malloc(sizeof(*new));
	if (!new)
		return NULL;
	new->type = MCC_ASM_OPERAND_REGISTER;
	new->reg = reg;
	new->offset = offset;
	return new;
}

struct mcc_asm_operand *mcc_asm_new_data_operand(struct mcc_asm_declaration *decl)
{
	struct mcc_asm_operand *new = malloc(sizeof(*new));
	if (!new)
		return NULL;
	new->type = MCC_ASM_OPERAND_DATA;
	new->decl = decl;
	new->offset = 0;
	return new;
}

struct mcc_asm_pos_list *mcc_asm_new_pos_ident(struct mcc_ast_identifier *ident, int offset)
{
	struct mcc_asm_pos_list *new = malloc(sizeof(*new));
	if (!new)
		return NULL;
	new->type = MCC_ASM_POS_IDENT;
	new->ident = ident;
	new->pos = offset;
	new->next_pos = NULL;
	return new;
}

struct mcc_asm_pos_list *mcc_asm_new_pos_row(struct mcc_ir_row *row, int offset)
{
	struct mcc_asm_pos_list *new = malloc(sizeof(*new));
	if (!new)
		return NULL;
	new->type = MCC_ASM_POS_ROW;
	new->row = row;
	new->pos = offset;
	new->next_pos = NULL;
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
	mcc_asm_delete_pos_list(function->pos_list);
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

void mcc_asm_delete_pos_list(struct mcc_asm_pos_list *list)
{
	if (!list)
		return;
	mcc_asm_delete_pos_list(list->next_pos);
	free(list);
}

//---------------------------------------------------------------------------------------- Functions: ASM generation

static void append_pos(struct mcc_asm_pos_list *first, struct mcc_asm_pos_list *new)
{
	assert(new);
	assert(first);

	while (first->next_pos) {
		first = first->next_pos;
	}
	first->next_pos = new;
}

static void append_row(struct mcc_asm_function *func, struct mcc_ir_row *row)
{
	assert(row);
	assert(func);

	struct mcc_asm_pos_list *new = mcc_asm_new_pos_row(row, func->ebp_offset);
	if (!new) {
		return;
	}
	if(func->pos_list){
		append_pos(func->pos_list, new);
	} else {
		func->pos_list = new;
	}
}

static void append_ident(struct mcc_asm_function *func, struct mcc_ast_identifier *ident)
{
	assert(ident);
	assert(func);

	struct mcc_asm_pos_list *new = mcc_asm_new_pos_ident(ident, func->ebp_offset);
	if (!new) {
		return;
	}
	if(func->pos_list){
		append_pos(func->pos_list, new);
	} else {
		func->pos_list = new;
	}
}

static struct mcc_asm_pos_list *get_pos_row(struct mcc_asm_pos_list *list, struct mcc_ir_row *row)
{
	assert(row);
	if (!list){
		return NULL;
	}

	do {
		if (list->type == MCC_ASM_POS_ROW && list->row->row_no == row->row_no) {
			return list;
		}
		list = list->next_pos;
	} while (list);
	return NULL;
}

static struct mcc_asm_pos_list *get_pos_ident(struct mcc_asm_pos_list *list, struct mcc_ast_identifier *ident)
{
	assert(ident);
	if (!list){
		return NULL;
	}

	do {
		if (strcmp(list->ident->identifier_name, ident->identifier_name) == 0) {
			return list;
		}
		list = list->next_pos;
	} while (list);
	return NULL;
}

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

static void func_append(struct mcc_asm_function *func, struct mcc_asm_assembly_line *line)
{
	assert(line);
	assert(func);

	if (!func->head) {
		func->head = line;
		return;
	}
	struct mcc_asm_assembly_line *tail = last_asm_line(func->head);
	tail->next = line;
	return;
}

static struct mcc_asm_assembly_line *generate_function_prolog()
{
	struct mcc_asm_operand *ebp = mcc_asm_new_register_operand(MCC_ASM_EBP, 0);
	struct mcc_asm_operand *ebp_2 = mcc_asm_new_register_operand(MCC_ASM_EBP, 0);
	struct mcc_asm_operand *esp = mcc_asm_new_register_operand(MCC_ASM_ESP, 0);
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

static struct mcc_asm_assembly_line *generate_instr_plus(struct mcc_asm_function *function, struct mcc_ir_row *ir)
{
	assert(function);
	assert(ir);
}

static struct mcc_asm_assembly_line *generate_instr_assign(struct mcc_asm_function *function, struct mcc_ir_row *ir)
{
	assert(function);
	assert(ir);

	int offset2;
	struct mcc_asm_pos_list *pos = get_pos_ident(function->pos_list, ir->arg1->ident);
	if (!pos){
		append_ident(function, ir->arg1->ident);
		offset2 = function->ebp_offset;
	} else {
		offset2 = pos->pos;
	}

	// TODO implement correctly
	struct mcc_asm_operand *first = NULL;
	if(ir->arg2->type == MCC_IR_TYPE_LIT_INT){
		first = mcc_asm_new_literal_operand(ir->arg2->lit_int);
	} else if (ir->arg2->type == MCC_IR_TYPE_ROW) {
		struct mcc_asm_pos_list *pos = get_pos_row(function->pos_list, ir->arg2->row);
		int offset1 = 0;
		if(pos){
			offset1 = pos->pos;
		}
		// int offset1 = pos->pos;
		first = mcc_asm_new_register_operand(MCC_ASM_EBP, offset1);
	} else {
		first = mcc_asm_new_literal_operand((long)9999999);
	} 
	// ----
	struct mcc_asm_operand *second = mcc_asm_new_register_operand(MCC_ASM_EBP, offset2);
	struct mcc_asm_assembly_line *line = mcc_asm_new_assembly_line(MCC_ASM_MOVL, first, second, NULL);

	return line;
}

static enum mcc_asm_opcode get_opcode(struct mcc_ir_row *ir)
{
	assert(ir);

	switch (ir->instr)
	{
	case MCC_IR_INSTR_PLUS:
		return MCC_ASM_ADDL;
	case MCC_IR_INSTR_MINUS:
		return MCC_ASM_SUBL;
	
	default:
		return MCC_ASM_ADDL;
	}
}

static struct mcc_asm_assembly_line *generate_binary_op(struct mcc_asm_function *function, struct mcc_ir_row *ir)
{
	assert(function);
	assert(ir);

	append_row(function, ir);

	int offset;
	struct mcc_asm_operand *first = NULL;
	if(ir->arg1->type == MCC_IR_TYPE_LIT_INT){
		first = mcc_asm_new_literal_operand(ir->arg1->lit_int);
	} else if (ir->arg1->type == MCC_IR_TYPE_ROW){
		struct mcc_asm_pos_list *pos = get_pos_row(function->pos_list, ir->arg1->row);
		offset = pos->pos;
		first = mcc_asm_new_register_operand(MCC_ASM_EBP, offset);		
	}
	struct mcc_asm_operand *second = NULL;
	if(ir->arg2->type == MCC_IR_TYPE_LIT_INT){
		second = mcc_asm_new_literal_operand(ir->arg2->lit_int);
	} else if (ir->arg2->type == MCC_IR_TYPE_ROW){
		struct mcc_asm_pos_list *pos = get_pos_row(function->pos_list, ir->arg2->row);
		offset = pos->pos;
		second = mcc_asm_new_register_operand(MCC_ASM_EBP, offset);		
	}
	struct mcc_asm_operand *eax1 = mcc_asm_new_register_operand(MCC_ASM_EAX, 0);
	struct mcc_asm_operand *eax2 = mcc_asm_new_register_operand(MCC_ASM_EAX, 0);
	struct mcc_asm_operand *eax3 = mcc_asm_new_register_operand(MCC_ASM_EAX, 0);
	struct mcc_asm_operand *ebp = mcc_asm_new_register_operand(MCC_ASM_EBP, function->ebp_offset);
	struct mcc_asm_assembly_line *third_line = mcc_asm_new_assembly_line(MCC_ASM_MOVL, eax3, ebp, NULL);
	enum mcc_asm_opcode opcode = get_opcode(ir);
	struct mcc_asm_assembly_line *second_line = mcc_asm_new_assembly_line(opcode, second, eax2, third_line);
	struct mcc_asm_assembly_line *first_line = mcc_asm_new_assembly_line(MCC_ASM_MOVL, first, eax1, second_line);
	return first_line;
}

static struct mcc_asm_assembly_line *generate_ir_row(struct mcc_asm_function *function, struct mcc_ir_row *ir)
{
	assert(function);
	assert(ir);

	struct mcc_asm_assembly_line *line = NULL;

	switch (ir->instr) {
	case MCC_IR_INSTR_ASSIGN:
		function->ebp_offset -= 4;
		line = generate_instr_assign(function, ir);
		break;
	case MCC_IR_INSTR_LABEL:
		break;
	case MCC_IR_INSTR_FUNC_LABEL:
		return NULL;
	case MCC_IR_INSTR_JUMP:
		break;
	case MCC_IR_INSTR_CALL:
		break;
	case MCC_IR_INSTR_JUMPFALSE:
		break;
	case MCC_IR_INSTR_PUSH:
		break;
	case MCC_IR_INSTR_POP:
		break;
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
		function->ebp_offset -= 4;
		line = generate_binary_op(function, ir);
		break;
	case MCC_IR_INSTR_RETURN:
		break;
	case MCC_IR_INSTR_ARRAY_INT:
		break;
	case MCC_IR_INSTR_ARRAY_FLOAT:
		break;
	case MCC_IR_INSTR_ARRAY_BOOL:
		break;
	case MCC_IR_INSTR_ARRAY_STRING:
		break;
	case MCC_IR_INSTR_NEGATIV:
		break;
	case MCC_IR_INSTR_NOT:
		break;
	case MCC_IR_INSTR_UNKNOWN:
		break;
	default:
		break;
	}

	return line;
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

	ir = ir->next_row;
	struct mcc_asm_assembly_line *line = NULL;
	while (ir && ir->instr != MCC_IR_INSTR_FUNC_LABEL) {
		if (ir->instr == MCC_IR_INSTR_ASSIGN || is_binary_instr(ir)) {
			// TODO delete 'if'
			line = generate_ir_row(function, ir);
			if (!line) {
				mcc_asm_delete_assembly_line(call);
				mcc_asm_delete_operand(print_nl);
				return NULL;
			}
			func_append(function, line);
		}
		ir = ir->next_row;
	}
	// TODO: Implement correctly
	if (!function->head) {
		return call;
	}
	mcc_asm_delete_assembly_line(call);
	// TODO: end
	return function->head;
}

static bool variable_needs_local_space(struct mcc_ir_row *first, struct mcc_ir_row *ir)
{
	assert(first);
	assert(ir);

	if(is_binary_instr(ir)){
		return true;
	}

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

	switch (ir->arg2->type) {
	case MCC_IR_TYPE_LIT_INT:
		return 4;
	case MCC_IR_TYPE_IDENTIFIER:
		return 4;
	case MCC_IR_TYPE_ROW:
		return get_var_size(ir->arg2->row);
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
	struct mcc_asm_operand *esp = mcc_asm_new_register_operand(MCC_ASM_ESP, 0);
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

static bool generate_text_section(struct mcc_asm_text_section *text_section, struct mcc_ir_row *ir)
{
	struct mcc_asm_function *first_function = mcc_asm_generate_function(ir);
	if (!first_function)
		return false;
	struct mcc_asm_function *latest_function = first_function;
	ir = find_next_function(ir);
	while (ir) {
		struct mcc_asm_function *new_function = mcc_asm_generate_function(ir);
		if (!new_function) {
			mcc_asm_delete_all_functions(first_function);
			return false;
		}
		latest_function->next = new_function;
		latest_function = new_function;
		ir = find_next_function(ir);
	}
	text_section->function = first_function;
	return true;
}

static enum mcc_asm_declaration_type get_array_type(enum mcc_ir_instruction instr)
{
	switch (instr) {
	case MCC_IR_INSTR_ARRAY_BOOL:
		return MCC_ASM_DECLARATION_TYPE_ARRAY_BOOL;
	case MCC_IR_INSTR_ARRAY_FLOAT:
		return MCC_ASM_DECLARATION_TYPE_ARRAY_FLOAT;
	case MCC_IR_INSTR_ARRAY_INT:
		return MCC_ASM_DECLARATION_TYPE_ARRAY_INT;
	case MCC_IR_INSTR_ARRAY_STRING:
		return MCC_ASM_DECLARATION_TYPE_ARRAY_STRING;
	default:
		// Not reachable case, included to prevent compiler warning
		return MCC_ASM_DECLARATION_TYPE_DB;
	}
}

static bool allocate_arrays(struct mcc_asm_data_section *data_section, struct mcc_ir_row *ir)
{
	assert(data_section);
	assert(ir);

	bool first = true;
	struct mcc_asm_declaration *decl_head;
	while (ir) {
		if (ir->instr == MCC_IR_INSTR_ARRAY_INT || ir->instr == MCC_IR_INSTR_ARRAY_BOOL ||
		    ir->instr == MCC_IR_INSTR_ARRAY_FLOAT || ir->instr == MCC_IR_INSTR_ARRAY_STRING) {
			enum mcc_asm_declaration_type array_type = get_array_type(ir->instr);
			struct mcc_asm_declaration *decl = mcc_asm_new_array_declaration(
			    ir->arg1->arr_ident->identifier_name, ir->arg2->lit_int, array_type, NULL);

			if (!decl) {
				mcc_asm_delete_all_declarations(data_section->head);
				return false;
			}
			if (first) {
				data_section->head = decl;
				decl_head = data_section->head;
				first = false;
			} else {
				decl_head->next = decl;
				decl_head = decl;
			}
		}
		ir = ir->next_row;
	}
	return true;
}

static bool generate_data_section(struct mcc_asm_data_section *data_section, struct mcc_ir_row *ir)
{
	bool arrays_allocated = allocate_arrays(data_section, ir);
	if (!arrays_allocated) {
		return false;
	}
	return true;
}

struct mcc_asm *mcc_asm_generate(struct mcc_ir_row *ir)
{
	struct mcc_asm *assembly = mcc_asm_new_asm(NULL, NULL);
	struct mcc_asm_text_section *text_section = mcc_asm_new_text_section(NULL);
	struct mcc_asm_data_section *data_section = mcc_asm_new_data_section(NULL);
	if (!assembly || !text_section || !data_section) {
		mcc_asm_delete_asm(assembly);
		mcc_asm_delete_text_section(text_section);
		mcc_asm_delete_data_section(data_section);
		return NULL;
	}
	assembly->data_section = data_section;
	assembly->text_section = text_section;

	bool text_section_generated = generate_text_section(assembly->text_section, ir);
	bool data_section_generated = generate_data_section(assembly->data_section, ir);
	if (!text_section_generated || !data_section_generated) {
		mcc_asm_delete_asm(assembly);
	}

	return assembly;
}
