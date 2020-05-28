#include "mcc/asm.h"

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "mcc/ir.h"
#include "mcc/stack_size.h"
#include "utils/unused.h"

static int get_identifier_offset(struct mcc_annotated_ir *first, struct mcc_ast_identifier *ident)
{
	assert(first);
	assert(ident);
	assert(first->row->instr == MCC_IR_INSTR_FUNC_LABEL);

	first = first->next;

	while (first && first->row->instr != MCC_IR_INSTR_FUNC_LABEL) {
		if (first->row->instr == MCC_IR_INSTR_ASSIGN) {
			if (strcmp(first->row->arg1->ident->identifier_name, ident->identifier_name) == 0) {
				return first->stack_position;
			}
		}
		first = first->next;
	}
	return 0;
}

static int get_row_offset(struct mcc_annotated_ir *first, struct mcc_ir_row *row)
{
	assert(first);
	assert(first->row->instr == MCC_IR_INSTR_FUNC_LABEL);
	assert(row);

	first = first->next;

	while (first && first->row->instr != MCC_IR_INSTR_FUNC_LABEL) {
		if (first->row == row) {
			return first->stack_position;
		}
		first = first->next;
	}
	return 0;
}

static int get_offset_of(struct mcc_annotated_ir *an_ir, struct mcc_ir_arg *arg)
{
	assert(an_ir);
	assert(arg);
	struct mcc_annotated_ir *first = an_ir;
	struct mcc_annotated_ir *prev = first->prev;
	while (prev) {
		if (prev->row->instr == MCC_IR_INSTR_FUNC_LABEL) {
			first = prev;
			break;
		}
		prev = prev->prev;
	}

	switch (arg->type) {
	case MCC_IR_TYPE_LIT_INT:
	case MCC_IR_TYPE_LIT_FLOAT:
	case MCC_IR_TYPE_LIT_BOOL:
	case MCC_IR_TYPE_LIT_STRING:
	case MCC_IR_TYPE_LABEL:
	case MCC_IR_TYPE_FUNC_LABEL:
		return 0;
	case MCC_IR_TYPE_ARR_ELEM:
		// TODO: CHeck if this function does what we want
		return get_array_element_location(an_ir);
	case MCC_IR_TYPE_IDENTIFIER:
		return get_identifier_offset(first, arg->ident);
	case MCC_IR_TYPE_ROW:
		return get_row_offset(first, arg->row);
	default:
		return 0;
	}
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

struct mcc_asm_function *mcc_asm_new_function(char *label, struct mcc_asm_line *head, struct mcc_asm_function *next)
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

struct mcc_asm_line *mcc_asm_new_line(enum mcc_asm_opcode opcode,
                                      struct mcc_asm_operand *first,
                                      struct mcc_asm_operand *second,
                                      struct mcc_asm_line *next)
{
	struct mcc_asm_line *new = malloc(sizeof(*new));
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

//------------------------------------------------------------------------------------ Functions: Registers

static struct mcc_asm_operand *eax()
{
	return mcc_asm_new_register_operand(MCC_ASM_EAX, 0);
}

static struct mcc_asm_operand *ebx()
{
	return mcc_asm_new_register_operand(MCC_ASM_EBX, 0);
}

static struct mcc_asm_operand *edx()
{
	return mcc_asm_new_register_operand(MCC_ASM_EDX, 0);
}

static struct mcc_asm_operand *dl()
{
	return mcc_asm_new_register_operand(MCC_ASM_DL, 0);
}

static struct mcc_asm_operand *ebp(int offset)
{
	return mcc_asm_new_register_operand(MCC_ASM_EBP, offset);
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
	mcc_asm_delete_all_lines(function->head);
	free(function->label);
	free(function);
}

void mcc_asm_delete_all_lines(struct mcc_asm_line *line)
{
	if (!line)
		return;
	mcc_asm_delete_all_lines(line->next);
	mcc_asm_delete_line(line);
}

void mcc_asm_delete_line(struct mcc_asm_line *line)
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

static struct mcc_asm_line *last_asm_line(struct mcc_asm_line *head)
{
	assert(head);
	while (head->next) {
		head = head->next;
	}
	return head;
}

static void func_append(struct mcc_asm_function *func, struct mcc_asm_line *line)
{
	assert(line);
	assert(func);

	if (!func->head) {
		func->head = line;
		return;
	}
	struct mcc_asm_line *tail = last_asm_line(func->head);
	tail->next = line;
	return;
}

// TODO: Float, String
// TODO: Function needs an_ir to find offset (instead of function)
static struct mcc_asm_operand *arg_to_op(struct mcc_annotated_ir *an_ir, struct mcc_ir_arg *arg)
{
	assert(an_ir);
	assert(arg);

	struct mcc_asm_operand *operand = NULL;
	if (arg->type == MCC_IR_TYPE_LIT_INT) {
		operand = mcc_asm_new_literal_operand(arg->lit_int);
	} else if (arg->type == MCC_IR_TYPE_LIT_BOOL) {
		operand = mcc_asm_new_literal_operand(arg->lit_bool);
	} else if (arg->type == MCC_IR_TYPE_ROW || arg->type == MCC_IR_TYPE_IDENTIFIER) {
		operand = mcc_asm_new_register_operand(MCC_ASM_EBP, get_offset_of(an_ir, arg));
	}

	return operand;
}

static struct mcc_asm_line *generate_instr_assign(struct mcc_asm_function *func, struct mcc_annotated_ir *an_ir)
{
	assert(func);
	assert(an_ir);

	int offset2 = an_ir->stack_position;

	// TODO implement correctly
	struct mcc_asm_line *line1 = NULL;
	if (an_ir->row->arg2->type == MCC_IR_TYPE_LIT_INT || an_ir->row->arg2->type == MCC_IR_TYPE_LIT_BOOL) {
		line1 = mcc_asm_new_line(MCC_ASM_MOVL, arg_to_op(an_ir, an_ir->row->arg2), ebp(offset2), NULL);
	} else if (an_ir->row->arg2->type == MCC_IR_TYPE_ROW) {
		// TODO: Get offset of temporary or identifier (Implement function, type-generic for row or identifier)
		struct mcc_asm_line *line2 =
		    mcc_asm_new_line(MCC_ASM_MOVL, eax(), arg_to_op(an_ir, an_ir->row->arg1), NULL);
		line1 = mcc_asm_new_line(MCC_ASM_MOVL, arg_to_op(an_ir, an_ir->row->arg2), eax(), line2);
	} else {
		line1 = mcc_asm_new_line(MCC_ASM_MOVL, mcc_asm_new_literal_operand((int)9999999), ebp(offset2), NULL);
	}
	// TODO: "a=b"

	return line1;
}

static struct mcc_asm_line *
generate_arithm_op(struct mcc_asm_function *func, struct mcc_annotated_ir *an_ir, enum mcc_asm_opcode opcode)
{
	assert(func);
	assert(an_ir);

	struct mcc_asm_line *line4 = mcc_asm_new_line(MCC_ASM_MOVL, eax(), ebp(an_ir->stack_position), NULL);

	struct mcc_asm_line *line2 = NULL;
	if (opcode == MCC_ASM_IDIVL) {
		struct mcc_asm_line *line3 = mcc_asm_new_line(opcode, ebx(), NULL, line4);
		// line to clear EDX
		struct mcc_asm_line *line2a = mcc_asm_new_line(MCC_ASM_XORL, edx(), edx(), line3);
		line2 = mcc_asm_new_line(MCC_ASM_MOVL, arg_to_op(an_ir, an_ir->row->arg2), ebx(), line2a);
	} else {
		line2 = mcc_asm_new_line(opcode, arg_to_op(an_ir, an_ir->row->arg2), eax(), line4);
	}

	struct mcc_asm_line *line1 = mcc_asm_new_line(MCC_ASM_MOVL, arg_to_op(an_ir, an_ir->row->arg1), eax(), line2);
	return line1;
}

static struct mcc_asm_line *
generate_unary(struct mcc_asm_function *func, struct mcc_annotated_ir *an_ir, enum mcc_asm_opcode opcode)
{
	assert(func);
	assert(an_ir);

	struct mcc_asm_line *line3 = mcc_asm_new_line(MCC_ASM_MOVL, eax(), ebp(an_ir->stack_position), NULL);

	struct mcc_asm_line *line2 = NULL;
	if (opcode == MCC_ASM_XORL) {
		struct mcc_asm_operand *lit_1 = mcc_asm_new_literal_operand((int)1);
		line2 = mcc_asm_new_line(MCC_ASM_XORL, lit_1, eax(), line3);
	} else {
		line2 = mcc_asm_new_line(opcode, eax(), NULL, line3);
	}

	struct mcc_asm_line *line1 = mcc_asm_new_line(MCC_ASM_MOVL, arg_to_op(an_ir, an_ir->row->arg1), eax(), line2);
	return line1;
}

static struct mcc_asm_line *
generate_cmp_op(struct mcc_asm_function *func, struct mcc_annotated_ir *an_ir, enum mcc_asm_opcode opcode)
{
	assert(func);
	assert(an_ir);

	struct mcc_asm_line *line1 = NULL;

	// 4. movl eax -x(ebp)
	struct mcc_asm_line *line4 = mcc_asm_new_line(MCC_ASM_MOVL, eax(), ebp(an_ir->stack_position), NULL);
	// 3. movcc dl eax
	struct mcc_asm_line *line3 = mcc_asm_new_line(MCC_ASM_MOVZBL, dl(), eax(), line4);
	// 2. setcc dl
	struct mcc_asm_line *line2 = mcc_asm_new_line(opcode, dl(), NULL, line3);

	if ((an_ir->row->arg1->type == MCC_IR_TYPE_ROW || an_ir->row->arg1->type == MCC_IR_TYPE_IDENTIFIER) &&
	    (an_ir->row->arg2->type == MCC_IR_TYPE_ROW || an_ir->row->arg2->type == MCC_IR_TYPE_IDENTIFIER)) {
		// 1b. cmp eax and arg2
		struct mcc_asm_line *line1b =
		    mcc_asm_new_line(MCC_ASM_CMPL, arg_to_op(an_ir, an_ir->row->arg2), eax(), line2);
		// 1a. move arg1 in eax
		line1 = mcc_asm_new_line(MCC_ASM_MOVL, arg_to_op(an_ir, an_ir->row->arg1), eax(), line1b);

	} else if (an_ir->row->arg1->type == MCC_IR_TYPE_ROW || an_ir->row->arg1->type == MCC_IR_TYPE_IDENTIFIER) {
		// 1. cmp arg1 arg2
		line1 = mcc_asm_new_line(MCC_ASM_CMPL, arg_to_op(an_ir, an_ir->row->arg2),
		                         arg_to_op(an_ir, an_ir->row->arg1), line2);
	} else {
		// 1b. cmp arg1 arg2
		struct mcc_asm_line *line1b =
		    mcc_asm_new_line(MCC_ASM_CMPL, arg_to_op(an_ir, an_ir->row->arg2), eax(), line2);
		// 1.a mov lit eax
		line1 = mcc_asm_new_line(MCC_ASM_MOVL, arg_to_op(an_ir, an_ir->row->arg1), eax(), line1b);
	}

	return line1;
}

static struct mcc_asm_line *generate_asm_from_ir(struct mcc_asm_function *function, struct mcc_annotated_ir *an_ir)
{
	assert(function);
	assert(an_ir);

	struct mcc_asm_line *line = NULL;

	switch (an_ir->row->instr) {
	case MCC_IR_INSTR_ASSIGN:
		line = generate_instr_assign(function, an_ir);
		break;
	case MCC_IR_INSTR_LABEL:
	case MCC_IR_INSTR_FUNC_LABEL:
	case MCC_IR_INSTR_JUMP:
	case MCC_IR_INSTR_CALL:
	case MCC_IR_INSTR_JUMPFALSE:
	case MCC_IR_INSTR_PUSH:
	case MCC_IR_INSTR_POP:
		break;
	case MCC_IR_INSTR_EQUALS:
		line = generate_cmp_op(function, an_ir, MCC_ASM_SETE);
		break;
	case MCC_IR_INSTR_NOTEQUALS:
		line = generate_cmp_op(function, an_ir, MCC_ASM_SETNE);
		break;
	case MCC_IR_INSTR_SMALLER:
		line = generate_cmp_op(function, an_ir, MCC_ASM_SETL);
		break;
	case MCC_IR_INSTR_GREATER:
		line = generate_cmp_op(function, an_ir, MCC_ASM_SETG);
		break;
	case MCC_IR_INSTR_SMALLEREQ:
		line = generate_cmp_op(function, an_ir, MCC_ASM_SETLE);
		break;
	case MCC_IR_INSTR_GREATEREQ:
		line = generate_cmp_op(function, an_ir, MCC_ASM_SETGE);
		break;
	case MCC_IR_INSTR_AND:
		line = generate_arithm_op(function, an_ir, MCC_ASM_AND);
		break;
	case MCC_IR_INSTR_OR:
		line = generate_arithm_op(function, an_ir, MCC_ASM_OR);
		break;
	case MCC_IR_INSTR_PLUS:
		line = generate_arithm_op(function, an_ir, MCC_ASM_ADDL);
		break;
	case MCC_IR_INSTR_MINUS:
		line = generate_arithm_op(function, an_ir, MCC_ASM_SUBL);
		break;
	case MCC_IR_INSTR_MULTIPLY:
		line = generate_arithm_op(function, an_ir, MCC_ASM_IMULL);
		break;
	case MCC_IR_INSTR_DIVIDE:
		line = generate_arithm_op(function, an_ir, MCC_ASM_IDIVL);
		break;
	case MCC_IR_INSTR_RETURN:
		break;
	// In these cases nothing needs to happen
	case MCC_IR_INSTR_ARRAY_INT:
	case MCC_IR_INSTR_ARRAY_FLOAT:
	case MCC_IR_INSTR_ARRAY_BOOL:
	case MCC_IR_INSTR_ARRAY_STRING:
		break;
	case MCC_IR_INSTR_NEGATIV:
		line = generate_unary(function, an_ir, MCC_ASM_NEGL);
		break;
	case MCC_IR_INSTR_NOT:
		line = generate_unary(function, an_ir, MCC_ASM_XORL);
		break;
	case MCC_IR_INSTR_UNKNOWN:
		break;
	}

	return line;
}

static struct mcc_asm_line *get_fake_asm_line()
{
	struct mcc_asm_operand *print_nl = mcc_asm_new_function_operand("print_nl");
	struct mcc_asm_line *call = mcc_asm_new_line(MCC_ASM_CALL, NULL, NULL, NULL);
	if (!print_nl || !call) {
		mcc_asm_delete_line(call);
		mcc_asm_delete_operand(print_nl);
		return NULL;
	}
	call->first = print_nl;
	return call;
}

// TODO: Implement correctly
static struct mcc_asm_line *generate_function_body(struct mcc_asm_function *function, struct mcc_annotated_ir *an_ir)
{
	assert(function);
	assert(an_ir);
	assert(an_ir->row->instr == MCC_IR_INSTR_FUNC_LABEL);

	an_ir = an_ir->next;
	struct mcc_asm_line *line = NULL;

	// Iterate up to next function
	while (an_ir && an_ir->row->instr != MCC_IR_INSTR_FUNC_LABEL) {

		// TODO: When finalizing, line musn't return NULL. It now returns NULL if the corresponding
		// IR line isn't implemented yet
		line = generate_asm_from_ir(function, an_ir);
		if (line)
			func_append(function, line);

		an_ir = an_ir->next;
	}

	// TODO: Exchange function->head for first asm line
	if (!function->head)
		return get_fake_asm_line();

	// TODO: Simply return the first line of the block of asm code you created. The merging will happen later.
	return function->head;
}

static struct mcc_asm_line *generate_function_args(struct mcc_asm_function *function, struct mcc_annotated_ir *an_ir)
{
	assert(function);
	assert(an_ir);
	assert(an_ir->row->instr == MCC_IR_INSTR_FUNC_LABEL);

	size_t frame_size = an_ir->stack_size;
	struct mcc_asm_operand *esp = mcc_asm_new_register_operand(MCC_ASM_ESP, 0);
	struct mcc_asm_operand *size_literal = mcc_asm_new_literal_operand(frame_size);
	struct mcc_asm_line *sub_size_esp = mcc_asm_new_line(MCC_ASM_SUBL, NULL, NULL, NULL);
	if (!esp || !size_literal || !sub_size_esp) {
		mcc_asm_delete_line(sub_size_esp);
		mcc_asm_delete_operand(esp);
		mcc_asm_delete_operand(size_literal);
		return NULL;
	}
	sub_size_esp->first = size_literal;
	sub_size_esp->second = esp;
	return sub_size_esp;
}

// TODO: Remove eventually
static struct mcc_asm_line *generate_function_epilog()
{
	struct mcc_asm_line *leave = mcc_asm_new_line(MCC_ASM_LEAVE, NULL, NULL, NULL);
	struct mcc_asm_line *ret = mcc_asm_new_line(MCC_ASM_RETURN, NULL, NULL, NULL);
	if (!leave || !ret) {
		mcc_asm_delete_line(leave);
		mcc_asm_delete_line(ret);
		return NULL;
	}
	leave->next = ret;
	return leave;
}

static void compose_function_asm(struct mcc_asm_function *function,
                                 struct mcc_asm_line *prolog,
                                 struct mcc_asm_line *args,
                                 struct mcc_asm_line *body,
                                 struct mcc_asm_line *epilog)
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
		mcc_asm_delete_line(args);
	} else {
		prolog->next = args;
		args = last_asm_line(args);
		args->next = body;
	}
	body = last_asm_line(body);
	body->next = epilog;
}

static struct mcc_asm_line *generate_function_prolog()
{
	struct mcc_asm_operand *ebp = mcc_asm_new_register_operand(MCC_ASM_EBP, 0);
	struct mcc_asm_operand *ebp_2 = mcc_asm_new_register_operand(MCC_ASM_EBP, 0);
	struct mcc_asm_operand *esp = mcc_asm_new_register_operand(MCC_ASM_ESP, 0);
	struct mcc_asm_line *push_ebp = mcc_asm_new_line(MCC_ASM_PUSHL, ebp, NULL, NULL);
	struct mcc_asm_line *mov_ebp_esp = mcc_asm_new_line(MCC_ASM_MOVL, esp, ebp_2, NULL);
	if (!ebp || !esp || !push_ebp || !ebp_2 || !mov_ebp_esp) {
		mcc_asm_delete_operand(ebp);
		mcc_asm_delete_operand(ebp_2);
		mcc_asm_delete_operand(esp);
		mcc_asm_delete_line(push_ebp);
		mcc_asm_delete_line(mov_ebp_esp);
		return NULL;
	}
	push_ebp->next = mov_ebp_esp;
	return push_ebp;
}

struct mcc_asm_function *mcc_asm_generate_function(struct mcc_annotated_ir *an_ir)
{
	assert(an_ir->row->instr == MCC_IR_INSTR_FUNC_LABEL);
	assert(an_ir);
	assert(an_ir->row->arg1->type == MCC_IR_TYPE_FUNC_LABEL);

	char *label = strdup(an_ir->row->arg1->func_label);
	if (!label)
		return NULL;
	struct mcc_asm_function *function = mcc_asm_new_function(label, NULL, NULL);
	if (!function) {
		mcc_asm_delete_function(function);
		return NULL;
	}
	struct mcc_asm_line *prolog = generate_function_prolog();
	struct mcc_asm_line *args = generate_function_args(function, an_ir);
	struct mcc_asm_line *body = generate_function_body(function, an_ir);
	struct mcc_asm_line *epilog = generate_function_epilog();
	if (!prolog || !body || !args || !epilog) {
		mcc_asm_delete_all_lines(prolog);
		mcc_asm_delete_all_lines(args);
		mcc_asm_delete_all_lines(body);
		mcc_asm_delete_all_lines(epilog);
		return NULL;
	}
	compose_function_asm(function, prolog, args, body, epilog);
	return function;
}

static struct mcc_annotated_ir *find_next_function(struct mcc_annotated_ir *an_ir)
{
	assert(an_ir);
	an_ir = an_ir->next;
	while (an_ir && an_ir->row->instr != MCC_IR_INSTR_FUNC_LABEL) {
		an_ir = an_ir->next;
	}
	return an_ir;
}

static bool generate_text_section(struct mcc_asm_text_section *text_section, struct mcc_annotated_ir *an_ir)
{
	struct mcc_asm_function *first_function = mcc_asm_generate_function(an_ir);
	if (!first_function)
		return false;
	struct mcc_asm_function *latest_function = first_function;
	an_ir = find_next_function(an_ir);
	while (an_ir) {
		struct mcc_asm_function *new_function = mcc_asm_generate_function(an_ir);
		if (!new_function) {
			mcc_asm_delete_all_functions(first_function);
			return false;
		}
		latest_function->next = new_function;
		latest_function = new_function;
		an_ir = find_next_function(an_ir);
	}
	text_section->function = first_function;
	return true;
}

static bool generate_data_section()
{
	return true;
}

struct mcc_asm *mcc_asm_generate(struct mcc_ir_row *ir)
{
	struct mcc_annotated_ir *an_ir = mcc_annotate_ir(ir);
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

	bool text_section_generated = generate_text_section(assembly->text_section, an_ir);
	bool data_section_generated = generate_data_section();
	if (!text_section_generated || !data_section_generated) {
		mcc_asm_delete_asm(assembly);
	}

	return assembly;
}
