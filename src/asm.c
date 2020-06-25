#include "mcc/asm.h"

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "mcc/ir.h"
#include "mcc/stack_size.h"
#include "utils/unused.h"

#define EPSILON 1e-06;

static int get_identifier_offset(struct mcc_annotated_ir *first, char *ident)
{
	assert(first);
	assert(ident);
	assert(first->row->instr == MCC_IR_INSTR_FUNC_LABEL);

	first = first->next;

	while (first && first->row->instr != MCC_IR_INSTR_FUNC_LABEL) {
		if (first->row->instr == MCC_IR_INSTR_ASSIGN) {
			// if ident is of form $tmp_xx only compare starting from position 1
			if (strncmp(ident, "$tmp", 4) == 0) {
				const char *tmp = &ident[1];
				if (strcmp(first->row->arg1->ident, tmp) == 0) {
					return first->stack_position;
				}
			}
			if (strcmp(first->row->arg1->ident, ident) == 0) {
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
		return mcc_get_array_element_stack_loc(an_ir, arg);
	case MCC_IR_TYPE_IDENTIFIER:
		return get_identifier_offset(first, arg->ident);
	case MCC_IR_TYPE_ROW:
		return get_row_offset(first, arg->row);
	default:
		return 0;
	}
}

//---------------------------------------------------------------------------------------- Functions: Data structures

struct mcc_asm *
mcc_asm_new_asm(struct mcc_asm_data_section *data, struct mcc_asm_text_section *text, struct mcc_asm_error *err)
{
	struct mcc_asm *new = malloc(sizeof(*new));
	if (!new) {
		err->has_failed = true;
		return NULL;
	}
	new->data_section = data;
	new->text_section = text;
	return new;
}

struct mcc_asm_data_section *mcc_asm_new_data_section(struct mcc_asm_declaration *head, struct mcc_asm_error *err)
{
	struct mcc_asm_data_section *new = malloc(sizeof(*new));
	if (!new) {
		err->has_failed = true;
		return NULL;
	}
	new->head = head;
	return new;
}

struct mcc_asm_text_section *mcc_asm_new_text_section(struct mcc_asm_function *function, struct mcc_asm_error *err)
{
	struct mcc_asm_text_section *new = malloc(sizeof(*new));
	if (!new) {
		err->has_failed = true;
		return NULL;
	}
	new->function = function;
	return new;
}

struct mcc_asm_declaration *mcc_asm_new_float_declaration(char *identifier,
                                                          float float_value,
                                                          struct mcc_asm_declaration *next,
                                                          struct mcc_asm_error *err)
{
	int size = strlen(identifier) + 1;
	struct mcc_asm_declaration *new = malloc(sizeof(*new));
	char *id_new = malloc(sizeof(char) * size);
	if (!new || !id_new) {
		err->has_failed = true;
		free(new);
		free(id_new);
		return NULL;
	}
	strncpy(id_new, identifier, size);
	new->identifier = id_new;
	new->float_value = float_value;
	new->next = next;
	new->type = MCC_ASM_DECLARATION_TYPE_FLOAT;
	return new;
}

struct mcc_asm_declaration *mcc_asm_new_string_declaration(char *identifier,
                                                           char *string_value,
                                                           struct mcc_asm_declaration *next,
                                                           struct mcc_asm_error *err)
{
	int size = strlen(identifier) + 1;
	struct mcc_asm_declaration *new = malloc(sizeof(*new));
	char *id_new = malloc(sizeof(char) * size);
	if (!new || !id_new) {
		err->has_failed = true;
		free(new);
		free(id_new);
		return NULL;
	}
	strncpy(id_new, identifier, size);
	new->identifier = id_new;
	new->string_value = string_value;
	new->next = next;
	new->type = MCC_ASM_DECLARATION_TYPE_STRING;
	return new;
}

struct mcc_asm_declaration *mcc_asm_new_array_declaration(char *identifier,
                                                          int size,
                                                          enum mcc_asm_declaration_type type,
                                                          struct mcc_asm_declaration *next,
                                                          struct mcc_asm_error *err)
{
	int string_size = strlen(identifier) + 1;
	struct mcc_asm_declaration *new = malloc(sizeof(*new));
	char *id_new = malloc(sizeof(char) * string_size);
	if (!new || !id_new) {
		err->has_failed = true;
		free(new);
		free(id_new);
		return NULL;
	}
	strncpy(id_new, identifier, string_size);
	new->identifier = id_new;
	new->array_size = size;
	new->next = next;
	new->type = type;
	return new;
}

struct mcc_asm_function *
mcc_asm_new_function(char *label, struct mcc_asm_line *head, struct mcc_asm_function *next, struct mcc_asm_error *err)
{
	int size = strlen(label) + 1;
	struct mcc_asm_function *new = malloc(sizeof(*new));
	char *lab_new = malloc(sizeof(char) * size);
	if (!new || !lab_new) {
		free(new);
		free(lab_new);
		err->has_failed = true;
		return NULL;
	}
	strncpy(lab_new, label, size);
	new->head = head;
	new->label = lab_new;
	new->next = next;
	new->pos_list = NULL;
	return new;
}

struct mcc_asm_line *mcc_asm_new_line(enum mcc_asm_opcode opcode,
                                      struct mcc_asm_operand *first,
                                      struct mcc_asm_operand *second,
                                      struct mcc_asm_line *next,
                                      struct mcc_asm_error *err)
{
	struct mcc_asm_line *new = malloc(sizeof(*new));
	if (!new) {
		err->has_failed = true;
		mcc_asm_delete_operand(first);
		mcc_asm_delete_operand(second);
		mcc_asm_delete_all_lines(next);
		return NULL;
	}
	new->opcode = opcode;
	new->first = first;
	new->second = second;
	new->next = next;
	return new;
}

struct mcc_asm_line *mcc_asm_new_label(enum mcc_asm_opcode opcode, unsigned label, struct mcc_asm_error *err)
{
	struct mcc_asm_line *new = malloc(sizeof(*new));
	if (!new) {
		err->has_failed = true;
		return NULL;
	}
	new->opcode = opcode;
	new->label = label;
	new->next = NULL;
	return new;
}

struct mcc_asm_operand *mcc_asm_new_function_operand(char *function_name, struct mcc_asm_error *err)
{
	int size = strlen(function_name) + 1;
	struct mcc_asm_operand *new = malloc(sizeof(*new));
	char *func_name_new = malloc(sizeof(char) * size);
	if (!new || !func_name_new) {
		free(new);
		free(func_name_new);
		err->has_failed = true;
		return NULL;
	}
	strncpy(func_name_new, function_name, size);
	new->type = MCC_ASM_OPERAND_FUNCTION;
	new->func_name = func_name_new;
	new->offset = 0;
	return new;
}

struct mcc_asm_operand *mcc_asm_new_literal_operand(int literal, struct mcc_asm_error *err)
{
	struct mcc_asm_operand *new = malloc(sizeof(*new));
	if (!new) {
		err->has_failed = true;
		return NULL;
	}
	new->type = MCC_ASM_OPERAND_LITERAL;
	new->literal = literal;
	new->offset = 0;
	return new;
}

struct mcc_asm_operand *mcc_asm_new_register_operand(enum mcc_asm_register reg, int offset, struct mcc_asm_error *err)
{
	struct mcc_asm_operand *new = malloc(sizeof(*new));
	if (!new) {
		err->has_failed = true;
		return NULL;
	}
	new->type = MCC_ASM_OPERAND_REGISTER;
	new->reg = reg;
	new->offset = offset;
	return new;
}

struct mcc_asm_operand *mcc_asm_new_data_operand(struct mcc_asm_declaration *decl, struct mcc_asm_error *err)
{
	struct mcc_asm_operand *new = malloc(sizeof(*new));
	if (!new) {
		err->has_failed = true;
		return NULL;
	}
	new->type = MCC_ASM_OPERAND_DATA;
	new->decl = decl;
	new->offset = 0;
	return new;
}

//------------------------------------------------------------------------------------ Functions: Registers

static struct mcc_asm_operand *eax(struct mcc_asm_error *err)
{
	return mcc_asm_new_register_operand(MCC_ASM_EAX, 0, err);
}

static struct mcc_asm_operand *ebx(struct mcc_asm_error *err)
{
	return mcc_asm_new_register_operand(MCC_ASM_EBX, 0, err);
}

static struct mcc_asm_operand *edx(struct mcc_asm_error *err)
{
	return mcc_asm_new_register_operand(MCC_ASM_EDX, 0, err);
}

static struct mcc_asm_operand *dl(struct mcc_asm_error *err)
{
	return mcc_asm_new_register_operand(MCC_ASM_DL, 0, err);
}

static struct mcc_asm_operand *ebp(int offset, struct mcc_asm_error *err)
{
	return mcc_asm_new_register_operand(MCC_ASM_EBP, offset, err);
}

static struct mcc_asm_operand *esp(struct mcc_asm_error *err)
{
	return mcc_asm_new_register_operand(MCC_ASM_ESP, 0, err);
}

static struct mcc_asm_operand *st(int offset, struct mcc_asm_error *err)
{
	return mcc_asm_new_register_operand(MCC_ASM_ST, offset, err);
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
	if (decl->type == MCC_ASM_DECLARATION_TYPE_STRING) {
		free(decl->identifier);
	}
	if (decl->type == MCC_ASM_DECLARATION_TYPE_ARRAY_FLOAT) {
		free(decl->identifier);
	}
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
	if (line->opcode != MCC_ASM_LABEL && line->opcode != MCC_ASM_JE && line->opcode != MCC_ASM_JNE) {
		mcc_asm_delete_operand(line->first);
		mcc_asm_delete_operand(line->second);
	}
	free(line);
}

void mcc_asm_delete_operand(struct mcc_asm_operand *operand)
{
	if (!operand)
		return;
	if (operand->type == MCC_ASM_OPERAND_FUNCTION) {
		free(operand->func_name);
	}
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

static void append_line(struct mcc_asm_line *head1, struct mcc_asm_line *head2)
{
	assert(head1);
	assert(head2);

	struct mcc_asm_line *line = last_asm_line(head1);
	line->next = head2;
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

// TODO: Implement
static struct mcc_asm_operand *
get_array_element_operand(struct mcc_annotated_ir *an_ir, struct mcc_ir_arg *arg, struct mcc_asm_error *err)
{
	assert(an_ir);
	assert(arg);
	assert(err);
	if (err->has_failed)
		return NULL;
	struct mcc_asm_operand *operand = NULL;
	return operand;
}
// function to check if 'prefix' is a proper prefix of 'string'. It is proper if 'prefix' followed by _x is equal to
// 'string', where x is an arbitrary number
static bool is_proper_prefix(char *prefix, char *string)
{
	assert(prefix);
	assert(string);

	if (strlen(prefix) >= strlen(string)) {
		return false;
	}

	unsigned len_p = strlen(prefix);
	if (strncmp(prefix, string, len_p) != 0) {
		return false;
	}
	if ('_' != string[len_p]) {
		return false;
	}
	for (unsigned i = len_p + 1; i < strlen(string); i++) {
		// check if all characters after the character '_' are digits
		if (string[i] < '0' || string[i] > '9') {
			return false;
		}
	}
	return true;
}

static bool is_in_data_section(char *ident, struct mcc_asm_error *err)
{
	struct mcc_asm_declaration *decl = err->data_section->head;
	bool looking_for_tmp = false;
	// if ident is '$tmpXX' look for declaration starting from position 1
	if (strncmp(ident, "$tmp", 4) == 0) {
		ident = &ident[1];
		looking_for_tmp = true;
	}
	while (decl) {
		if (looking_for_tmp) {
			if (strcmp(decl->identifier, ident) == 0 && decl->type == MCC_ASM_DECLARATION_TYPE_FLOAT) {
				return true;
			}
		} else { // if not looking for tmp: check if ident is proper prefix of decl->identifier
			if (is_proper_prefix(ident, decl->identifier)) {
				return true;
			}
		}
		decl = decl->next;
	}
	return false;
}

static bool is_float(struct mcc_ir_arg *arg, struct mcc_asm_error *err)
{
	assert(arg);

	switch (arg->type) {
	case MCC_IR_TYPE_LIT_INT:
		return false;
	case MCC_IR_TYPE_LIT_FLOAT:
		return true;
	case MCC_IR_TYPE_LIT_BOOL:
		return false;
	case MCC_IR_TYPE_LIT_STRING:
		return false;
	case MCC_IR_TYPE_ROW:
		return (arg->row->type->type == MCC_IR_ROW_FLOAT);
	case MCC_IR_TYPE_IDENTIFIER:
		return is_in_data_section(arg->ident, err);
	case MCC_IR_TYPE_ARR_ELEM:
		// TODO
		return false;

	default:
		return false;
	}
}

// TODO: Float
static struct mcc_asm_operand *
arg_to_op(struct mcc_annotated_ir *an_ir, struct mcc_ir_arg *arg, struct mcc_asm_error *err)
{
	assert(an_ir);
	assert(arg);
	if (err->has_failed)
		return NULL;

	struct mcc_asm_operand *operand = NULL;
	switch (arg->type) {
	case MCC_IR_TYPE_LIT_INT:
		operand = mcc_asm_new_literal_operand(arg->lit_int, err);
		break;
	case MCC_IR_TYPE_LIT_BOOL:
		operand = mcc_asm_new_literal_operand(arg->lit_bool, err);
		break;
	case MCC_IR_TYPE_ROW:
	case MCC_IR_TYPE_IDENTIFIER:
		operand = mcc_asm_new_register_operand(MCC_ASM_EBP, get_offset_of(an_ir, arg), err);
		break;
	// TODO #201: Distinguish between literal and computed index
	case MCC_IR_TYPE_ARR_ELEM:
		operand = get_array_element_operand(an_ir, arg, err);
		break;
	default:
		break;
	}

	return operand;
}

static struct mcc_asm_operand *find_string_identifier(struct mcc_annotated_ir *an_ir, struct mcc_asm_error *err)
{
	assert(an_ir);
	assert(an_ir->row->instr == MCC_IR_INSTR_ASSIGN);
	assert(an_ir->row->arg2->type == MCC_IR_TYPE_LIT_STRING);
	assert(err);
	if (err->has_failed)
		return NULL;

	struct mcc_asm_operand *op = malloc(sizeof(*op));
	if (!op) {
		err->has_failed = true;
		return NULL;
	}

	char *wanted_string = an_ir->row->arg2->lit_string;
	struct mcc_asm_declaration *head = err->data_section->head;
	while (head) {
		if (head->type != MCC_ASM_DECLARATION_TYPE_STRING) {
			head = head->next;
			continue;
		}
		if (strcmp(wanted_string, head->string_value) != 0) {
			head = head->next;
			continue;
		}
		op->decl = head;
		op->type = MCC_ASM_OPERAND_DATA;
		op->offset = 0;
		return op;
	}

	free(op);
	return NULL;
}

static struct mcc_asm_operand *find_float_identifier(struct mcc_annotated_ir *an_ir, struct mcc_asm_error *err)
{
	assert(an_ir);
	assert(an_ir->row->instr == MCC_IR_INSTR_ASSIGN);
	assert(an_ir->row->arg2->type == MCC_IR_TYPE_LIT_FLOAT);
	assert(err);
	if (err->has_failed)
		return NULL;

	struct mcc_asm_operand *op = malloc(sizeof(*op));
	if (!op) {
		err->has_failed = true;
		return NULL;
	}

	double wanted_float = an_ir->row->arg2->lit_float;
	double epsilon = EPSILON;
	struct mcc_asm_declaration *head = err->data_section->head;
	while (head) {
		if (head->type == MCC_ASM_DECLARATION_TYPE_FLOAT &&
		    (fabs(wanted_float - head->float_value) < epsilon) &&
		    (is_proper_prefix(an_ir->row->arg1->ident, head->identifier) ||
		     strcmp(an_ir->row->arg1->ident, head->identifier) == 0)) {
			op->decl = head;
			op->type = MCC_ASM_OPERAND_DATA;
			op->offset = 0;
			return op;
		}
		head = head->next;
	}
	free(op);
	return NULL;
}

static struct mcc_asm_line *generate_string_assignment(struct mcc_annotated_ir *an_ir, struct mcc_asm_error *err)
{
	struct mcc_asm_operand *string_id = find_string_identifier(an_ir, err);
	struct mcc_asm_line *leal = mcc_asm_new_line(MCC_ASM_LEAL, string_id, eax(err), NULL, err);
	struct mcc_asm_line *movl =
	    mcc_asm_new_line(MCC_ASM_MOVL, eax(err), ebp(an_ir->stack_position, err), NULL, err);
	if (err->has_failed || !leal || !movl || !string_id) {
		mcc_asm_delete_line(leal);
		mcc_asm_delete_line(movl);
		return NULL;
	}
	leal->next = movl;
	return leal;
}

static struct mcc_asm_line *generate_assign_row_ident(struct mcc_annotated_ir *an_ir, struct mcc_asm_error *err)
{
	assert(an_ir);
	struct mcc_asm_line *line1 = NULL, *line2 = NULL;
	// TODO FLOATs ?!
	line2 = mcc_asm_new_line(MCC_ASM_MOVL, eax(err), arg_to_op(an_ir, an_ir->row->arg1, err), NULL, err);
	line1 = mcc_asm_new_line(MCC_ASM_MOVL, arg_to_op(an_ir, an_ir->row->arg2, err), eax(err), line2, err);
	if (err->has_failed || !line1 || !line2) {
		mcc_asm_delete_line(line1);
		mcc_asm_delete_line(line2);
		return NULL;
	}
	return line1;
}

static struct mcc_asm_line *generate_float_assign(struct mcc_annotated_ir *an_ir, struct mcc_asm_error *err)
{
	struct mcc_asm_line *line2 =
	    mcc_asm_new_line(MCC_ASM_FSTPS, arg_to_op(an_ir, an_ir->row->arg1, err), NULL, NULL, err);
	struct mcc_asm_line *line1 =
	    mcc_asm_new_line(MCC_ASM_FLDS, find_float_identifier(an_ir, err), NULL, line2, err);
	if (err->has_failed || !line1 || !line2) {
		mcc_asm_delete_line(line1);
		mcc_asm_delete_line(line2);
		return NULL;
	}
	return line1;
}

static struct mcc_asm_line *generate_instr_assign(struct mcc_annotated_ir *an_ir, struct mcc_asm_error *err)
{
	assert(an_ir);
	assert(an_ir->row->instr == MCC_IR_INSTR_ASSIGN);

	if (err->has_failed)
		return NULL;

	int offset2 = an_ir->stack_position;

	// TODO #201, arrays
	struct mcc_asm_line *line1 = NULL;
	switch (an_ir->row->arg2->type) {
	case MCC_IR_TYPE_LIT_INT:
	case MCC_IR_TYPE_LIT_BOOL:
		line1 = mcc_asm_new_line(MCC_ASM_MOVL, arg_to_op(an_ir, an_ir->row->arg2, err), ebp(offset2, err), NULL,
		                         err);
		break;
	case MCC_IR_TYPE_LIT_FLOAT:
		line1 = generate_float_assign(an_ir, err);
		break;
	case MCC_IR_TYPE_ROW:
	case MCC_IR_TYPE_IDENTIFIER:
		line1 = generate_assign_row_ident(an_ir, err);
		break;
	case MCC_IR_TYPE_LIT_STRING:
		line1 = generate_string_assignment(an_ir, err);
		break;
	// TODO: Handle array elements
	case MCC_IR_TYPE_ARR_ELEM:
	default:
		// TODO remove when done. Remider: "(int)an_ir->stack_position" was only chosen to let compare
		// operations of float-integration-test fail (makes no sense at all, so don't get confused :) ...)
		line1 = mcc_asm_new_line(MCC_ASM_MOVL, mcc_asm_new_literal_operand((int)an_ir->stack_position, err),
		                         ebp(offset2, err), NULL, err);
		break;
	}

	return line1;
}

static struct mcc_asm_line *
generate_arithm_int_op(struct mcc_annotated_ir *an_ir, enum mcc_asm_opcode opcode, struct mcc_asm_error *err)
{
	assert(an_ir);
	if (err->has_failed) {
		return NULL;
	}

	struct mcc_asm_line *line1 = NULL, *line2 = NULL, *line2a = NULL, *line2b = NULL, *line3 = NULL;
	line3 = mcc_asm_new_line(MCC_ASM_MOVL, eax(err), ebp(an_ir->stack_position, err), NULL, err);

	if (opcode == MCC_ASM_IDIVL) {
		line2b = mcc_asm_new_line(opcode, ebx(err), NULL, line3, err);
		// line to clear EDX
		line2a = mcc_asm_new_line(MCC_ASM_XORL, edx(err), edx(err), line2b, err);
		line2 = mcc_asm_new_line(MCC_ASM_MOVL, arg_to_op(an_ir, an_ir->row->arg2, err), ebx(err), line2a, err);
		if (err->has_failed) {
			mcc_asm_delete_line(line2a);
			mcc_asm_delete_line(line2b);
			mcc_asm_delete_line(line3);
			return NULL;
		}
	} else {
		line2 = mcc_asm_new_line(opcode, arg_to_op(an_ir, an_ir->row->arg2, err), eax(err), line3, err);
	}
	line1 = mcc_asm_new_line(MCC_ASM_MOVL, arg_to_op(an_ir, an_ir->row->arg1, err), eax(err), line2, err);

	if (err->has_failed) {
		mcc_asm_delete_line(line2);
		mcc_asm_delete_line(line3);
	}

	return line1;
}

static struct mcc_asm_line *
generate_unary(struct mcc_annotated_ir *an_ir, enum mcc_asm_opcode opcode, struct mcc_asm_error *err)
{
	assert(an_ir);
	if (err->has_failed)
		return NULL;

	struct mcc_asm_line *line3 = NULL, *line2 = NULL, *line1 = NULL;
	line3 = mcc_asm_new_line(MCC_ASM_MOVL, eax(err), ebp(an_ir->stack_position, err), NULL, err);

	if (opcode == MCC_ASM_XORL) {
		struct mcc_asm_operand *lit_1 = mcc_asm_new_literal_operand((int)1, err);
		line2 = mcc_asm_new_line(MCC_ASM_XORL, lit_1, eax(err), line3, err);
	} else {
		line2 = mcc_asm_new_line(opcode, eax(err), NULL, line3, err);
	}

	line1 = mcc_asm_new_line(MCC_ASM_MOVL, arg_to_op(an_ir, an_ir->row->arg1, err), eax(err), line2, err);

	if (err->has_failed) {
		mcc_asm_delete_line(line2);
		mcc_asm_delete_line(line3);
	}
	return line1;
}

static struct mcc_asm_line *generate_cmp_op_int(struct mcc_annotated_ir *an_ir, struct mcc_asm_error *err)
{
	assert(an_ir);
	if (err->has_failed)
		return NULL;

	struct mcc_asm_line *line1 = NULL, *line1b = NULL;

	if ((an_ir->row->arg1->type == MCC_IR_TYPE_ROW || an_ir->row->arg1->type == MCC_IR_TYPE_IDENTIFIER ||
	     an_ir->row->arg1->type == MCC_IR_TYPE_ARR_ELEM) &&
	    (an_ir->row->arg2->type == MCC_IR_TYPE_ROW || an_ir->row->arg2->type == MCC_IR_TYPE_IDENTIFIER ||
	     an_ir->row->arg2->type == MCC_IR_TYPE_ARR_ELEM)) {
		// 1b. cmp eax and arg2
		line1b = mcc_asm_new_line(MCC_ASM_CMPL, arg_to_op(an_ir, an_ir->row->arg2, err), eax(err), NULL, err);
		// 1a. move arg1 in eax
		line1 = mcc_asm_new_line(MCC_ASM_MOVL, arg_to_op(an_ir, an_ir->row->arg1, err), eax(err), line1b, err);

	} else if (an_ir->row->arg1->type == MCC_IR_TYPE_ROW || an_ir->row->arg1->type == MCC_IR_TYPE_IDENTIFIER) {
		// 1. cmp arg1 arg2
		line1 = mcc_asm_new_line(MCC_ASM_CMPL, arg_to_op(an_ir, an_ir->row->arg2, err),
		                         arg_to_op(an_ir, an_ir->row->arg1, err), NULL, err);
	} else {
		// 1b. cmp arg1 arg2
		line1b = mcc_asm_new_line(MCC_ASM_CMPL, arg_to_op(an_ir, an_ir->row->arg2, err), eax(err), NULL, err);
		// 1.a mov lit eax
		line1 = mcc_asm_new_line(MCC_ASM_MOVL, arg_to_op(an_ir, an_ir->row->arg1, err), eax(err), line1b, err);
	}

	if (err->has_failed) {
		mcc_asm_delete_line(line1);
		mcc_asm_delete_line(line1b);
		return NULL;
	}

	return line1;
}

static struct mcc_asm_line *generate_return(struct mcc_annotated_ir *an_ir, struct mcc_asm_error *err)
{
	assert(an_ir);
	assert(an_ir->row->instr == MCC_IR_INSTR_RETURN);

	if (err->has_failed)
		return NULL;

	struct mcc_asm_line *ret = mcc_asm_new_line(MCC_ASM_RETURN, NULL, NULL, NULL, err);
	struct mcc_asm_line *leave = mcc_asm_new_line(MCC_ASM_LEAVE, NULL, NULL, ret, err);
	if (err->has_failed) {
		mcc_asm_delete_line(leave);
		mcc_asm_delete_line(ret);
		return NULL;
	}
	if (an_ir->row->arg1) {
		struct mcc_asm_line *line =
		    mcc_asm_new_line(MCC_ASM_MOVL, arg_to_op(an_ir, an_ir->row->arg1, err), eax(err), leave, err);
		if (err->has_failed) {
			mcc_asm_delete_all_lines(leave);
			return NULL;
		}
		return line;
	}
	return leave;
}
static struct mcc_asm_line *
generate_jumpfalse(enum mcc_asm_opcode opcode, struct mcc_annotated_ir *an_ir, struct mcc_asm_error *err)
{
	assert(an_ir);
	if (err->has_failed)
		return NULL;

	unsigned label = opcode == MCC_ASM_JNE ? an_ir->row->arg2->label : an_ir->row->arg1->label;
	struct mcc_asm_line *jump = mcc_asm_new_label(opcode, label, err);
	struct mcc_asm_line *cmp = NULL;
	if (opcode == MCC_ASM_JNE) {
		struct mcc_asm_operand *one = mcc_asm_new_literal_operand(1, err);
		cmp = mcc_asm_new_line(MCC_ASM_CMPL, one, arg_to_op(an_ir, an_ir->row->arg1, err), jump, err);
	} else { // case of MCC_ASM_JE
		cmp = mcc_asm_new_line(MCC_ASM_CMPL, eax(err), eax(err), jump, err);
	}
	if (err->has_failed) {
		mcc_asm_delete_line(cmp);
		mcc_asm_delete_line(jump);
		return NULL;
	}
	return cmp;
}

static struct mcc_asm_line *
generate_arithm_float_op(struct mcc_annotated_ir *an_ir, enum mcc_asm_opcode opcode, struct mcc_asm_error *err)
{
	assert(an_ir);
	struct mcc_asm_line *line1 = NULL, *line2 = NULL, *line3 = NULL, *line4 = NULL;
	line4 = mcc_asm_new_line(MCC_ASM_FSTPS, ebp(an_ir->stack_position, err), NULL, NULL, err);
	line3 = mcc_asm_new_line(opcode, st(0, err), st(1, err), line4, err);
	line2 = mcc_asm_new_line(MCC_ASM_FLDS, arg_to_op(an_ir, an_ir->row->arg1, err), NULL, line3, err);
	line1 = mcc_asm_new_line(MCC_ASM_FLDS, arg_to_op(an_ir, an_ir->row->arg2, err), NULL, line2, err);
	if (err->has_failed) {
		mcc_asm_delete_line(line1);
		mcc_asm_delete_line(line2);
		mcc_asm_delete_line(line3);
		mcc_asm_delete_line(line4);
		return NULL;
	}
	return line1;
}

static struct mcc_asm_line *generate_plus(struct mcc_annotated_ir *an_ir, struct mcc_asm_error *err)
{
	struct mcc_asm_line *line = NULL;
	if (an_ir->row->type->type == MCC_IR_ROW_INT) {
		line = generate_arithm_int_op(an_ir, MCC_ASM_ADDL, err);
	} else if (an_ir->row->type->type == MCC_IR_ROW_FLOAT) {
		line = generate_arithm_float_op(an_ir, MCC_ASM_FADDP, err);
	}
	return line;
}

static struct mcc_asm_line *generate_minus(struct mcc_annotated_ir *an_ir, struct mcc_asm_error *err)
{
	struct mcc_asm_line *line = NULL;
	if (an_ir->row->type->type == MCC_IR_ROW_INT) {
		line = generate_arithm_int_op(an_ir, MCC_ASM_SUBL, err);
	} else if (an_ir->row->type->type == MCC_IR_ROW_FLOAT) {
		line = generate_arithm_float_op(an_ir, MCC_ASM_FSUBP, err);
	}
	return line;
}

static struct mcc_asm_line *generate_cmp_op_float(struct mcc_annotated_ir *an_ir, struct mcc_asm_error *err)
{
	assert(an_ir);
	if (err->has_failed) {
		return NULL;
	}

	struct mcc_asm_line *line1 = NULL, *line2 = NULL, *line3 = NULL, *line4 = NULL;

	line4 = mcc_asm_new_line(MCC_ASM_FSTP, st(0, err), NULL, NULL, err);
	line3 = mcc_asm_new_line(MCC_ASM_FCOMIP, st(1, err), st(0, err), line4, err);
	line2 = mcc_asm_new_line(MCC_ASM_FLDS, arg_to_op(an_ir, an_ir->row->arg1, err), NULL, line3, err);
	line1 = mcc_asm_new_line(MCC_ASM_FLDS, arg_to_op(an_ir, an_ir->row->arg2, err), NULL, line2, err);
	if (err->has_failed) {
		mcc_asm_delete_line(line1);
		mcc_asm_delete_line(line2);
		mcc_asm_delete_line(line3);
		mcc_asm_delete_line(line4);
		return NULL;
	}

	return line1;
}

static struct mcc_asm_line *
generate_cmp(struct mcc_annotated_ir *an_ir, enum mcc_asm_opcode opcode, struct mcc_asm_error *err)
{
	assert(an_ir);
	if (err->has_failed)
		return NULL;

	struct mcc_asm_line *first_lines = NULL, *line2 = NULL, *line3 = NULL, *line4 = NULL;
	// line 1 to line 1c or line 1d (depended on case)
	if (is_float(an_ir->row->arg1, err)) {
		first_lines = generate_cmp_op_float(an_ir, err);
		// use unsigned opcode in case of float:
		if (opcode == MCC_ASM_SETG)
			opcode = MCC_ASM_SETA;
		if (opcode == MCC_ASM_SETGE)
			opcode = MCC_ASM_SETAE;
		if (opcode == MCC_ASM_SETL)
			opcode = MCC_ASM_SETB;
		if (opcode == MCC_ASM_SETLE)
			opcode = MCC_ASM_SETBE;
	} else {
		first_lines = generate_cmp_op_int(an_ir, err);
	}

	// 4. movl eax -x(ebp)
	line4 = mcc_asm_new_line(MCC_ASM_MOVL, eax(err), ebp(an_ir->stack_position, err), NULL, err);
	// 3. movcc dl eax
	line3 = mcc_asm_new_line(MCC_ASM_MOVZBL, dl(err), eax(err), line4, err);
	// 2. setcc dl
	line2 = mcc_asm_new_line(opcode, dl(err), NULL, line3, err);
	if (err->has_failed) {
		mcc_asm_delete_line(line2);
		mcc_asm_delete_line(line3);
		mcc_asm_delete_line(line4);
		return NULL;
	}

	append_line(first_lines, line2);
	return first_lines;
}

static struct mcc_asm_line *generate_mult(struct mcc_annotated_ir *an_ir, struct mcc_asm_error *err)
{
	assert(an_ir);
	struct mcc_asm_line *line = NULL;
	if (an_ir->row->type->type == MCC_IR_ROW_INT) {
		line = generate_arithm_int_op(an_ir, MCC_ASM_IMULL, err);
	} else if (an_ir->row->type->type == MCC_IR_ROW_FLOAT) {
		line = generate_arithm_float_op(an_ir, MCC_ASM_FMULP, err);
	}
	return line;
}

static struct mcc_asm_line *generate_div(struct mcc_annotated_ir *an_ir, struct mcc_asm_error *err)
{
	assert(an_ir);
	struct mcc_asm_line *line = NULL;
	if (an_ir->row->type->type == MCC_IR_ROW_INT) {
		line = generate_arithm_int_op(an_ir, MCC_ASM_IDIVL, err);
	} else if (an_ir->row->type->type == MCC_IR_ROW_FLOAT) {
		line = generate_arithm_float_op(an_ir, MCC_ASM_FDIVP, err);
	}
	return line;
}

static struct mcc_asm_line *generate_call(struct mcc_annotated_ir *an_ir, struct mcc_asm_error *err)
{
	assert(an_ir);
	// if function is void do no move instruction
	struct mcc_asm_line *line1 = NULL, *line2 = NULL;
	struct mcc_asm_operand *func = mcc_asm_new_function_operand(an_ir->row->arg1->func_label, err);
	line1 = mcc_asm_new_line(MCC_ASM_CALLL, NULL, NULL, NULL, err);
	if(an_ir->row->type->type != MCC_IR_ROW_TYPELESS){
		line2 = mcc_asm_new_line(MCC_ASM_MOVL, eax(err), ebp(an_ir->stack_position, err), NULL, err);
		line1->next = line2;
	}
	if (err->has_failed) {
		mcc_asm_delete_line(line1);
		mcc_asm_delete_line(line2);
		return NULL;
	}
	line1->first = func;
	return line1;
}

static struct mcc_asm_line *generate_pop(struct mcc_annotated_ir *an_ir, struct mcc_asm_error *err)
{
	assert(an_ir);
	struct mcc_asm_line *line2 = NULL, *line1 = NULL;
	line2 = mcc_asm_new_line(MCC_ASM_MOVL, eax(err), arg_to_op(an_ir->next, an_ir->next->row->arg1, err), NULL, err);
	line1 = mcc_asm_new_line(MCC_ASM_MOVL, ebp(an_ir->stack_position, err), eax(err), line2, err);
	if(err->has_failed){
		mcc_asm_delete_line(line1);
		mcc_asm_delete_line(line2);
		return NULL;
	}
	return line1;
}

static struct mcc_asm_line *generate_asm_from_ir(struct mcc_annotated_ir *an_ir, struct mcc_asm_error *err)
{
	assert(an_ir);

	if (err->has_failed)
		return NULL;

	struct mcc_asm_line *line = NULL;

	switch (an_ir->row->instr) {
	case MCC_IR_INSTR_ASSIGN:
		line = generate_instr_assign(an_ir, err);
		break;
	case MCC_IR_INSTR_LABEL:
		line = mcc_asm_new_label(MCC_ASM_LABEL, an_ir->row->arg1->label, err);
		break;
	case MCC_IR_INSTR_FUNC_LABEL:
		break;
	case MCC_IR_INSTR_JUMP:
		line = generate_jumpfalse(MCC_ASM_JE, an_ir, err);
		break;
	case MCC_IR_INSTR_CALL:
		line = generate_call(an_ir, err);
		break;
	case MCC_IR_INSTR_JUMPFALSE:
		line = generate_jumpfalse(MCC_ASM_JNE, an_ir, err);
		break;
	case MCC_IR_INSTR_PUSH:
		line = mcc_asm_new_line(MCC_ASM_PUSHL, arg_to_op(an_ir, an_ir->row->arg1, err), NULL, NULL, err);
		break;
	case MCC_IR_INSTR_POP:
		line = generate_pop(an_ir, err);
		break;
	case MCC_IR_INSTR_EQUALS:
		line = generate_cmp(an_ir, MCC_ASM_SETE, err);
		break;
	case MCC_IR_INSTR_NOTEQUALS:
		line = generate_cmp(an_ir, MCC_ASM_SETNE, err);
		break;
	case MCC_IR_INSTR_SMALLER:
		line = generate_cmp(an_ir, MCC_ASM_SETL, err);
		break;
	case MCC_IR_INSTR_GREATER:
		line = generate_cmp(an_ir, MCC_ASM_SETG, err);
		break;
	case MCC_IR_INSTR_SMALLEREQ:
		line = generate_cmp(an_ir, MCC_ASM_SETLE, err);
		break;
	case MCC_IR_INSTR_GREATEREQ:
		line = generate_cmp(an_ir, MCC_ASM_SETGE, err);
		break;
	case MCC_IR_INSTR_AND:
		line = generate_arithm_int_op(an_ir, MCC_ASM_AND, err);
		break;
	case MCC_IR_INSTR_OR:
		line = generate_arithm_int_op(an_ir, MCC_ASM_OR, err);
		break;
	case MCC_IR_INSTR_PLUS:
		line = generate_plus(an_ir, err);
		break;
	case MCC_IR_INSTR_MINUS:
		line = generate_minus(an_ir, err);
		break;
	case MCC_IR_INSTR_MULTIPLY:
		line = generate_mult(an_ir, err);
		break;
	case MCC_IR_INSTR_DIVIDE:
		line = generate_div(an_ir, err);
		break;
	case MCC_IR_INSTR_RETURN:
		line = generate_return(an_ir, err);
		break;
	// In these cases nothing needs to happen
	case MCC_IR_INSTR_ARRAY:
		break;
	case MCC_IR_INSTR_NEGATIV:
		line = generate_unary(an_ir, MCC_ASM_NEGL, err);
		break;
	case MCC_IR_INSTR_NOT:
		line = generate_unary(an_ir, MCC_ASM_XORL, err);
		break;
	case MCC_IR_INSTR_UNKNOWN:
		break;
	}

	return line;
}

// TODO: clearify if we need this function. I (Raoul) think we don't need it :)
static struct mcc_asm_line *get_fake_asm_line(struct mcc_asm_error *err)
{
	struct mcc_asm_operand *print_nl = mcc_asm_new_function_operand("print_nl", err);
	struct mcc_asm_line *call = mcc_asm_new_line(MCC_ASM_CALLL, NULL, NULL, NULL, err);
	if (!print_nl || !call) {
		mcc_asm_delete_line(call);
		mcc_asm_delete_operand(print_nl);
		return NULL;
	}
	call->first = print_nl;
	return call;
}

// TODO: Implement correctly
static struct mcc_asm_line *
generate_function_body(struct mcc_asm_function *function, struct mcc_annotated_ir *an_ir, struct mcc_asm_error *err)
{
	assert(function);
	assert(an_ir);
	assert(an_ir->row->instr == MCC_IR_INSTR_FUNC_LABEL);

	if (err->has_failed)
		return NULL;

	an_ir = an_ir->next;
	struct mcc_asm_line *line = NULL;

	// Iterate up to next function
	while (an_ir && an_ir->row->instr != MCC_IR_INSTR_FUNC_LABEL) {

		// TODO: When finalizing, line musn't return NULL. It now returns NULL if the corresponding
		// IR line isn't implemented yet
		line = generate_asm_from_ir(an_ir, err);
		if (line)
			func_append(function, line);
		if (err->has_failed) {
			// deletion of all lines needs to happen after appending current line, since in error case still
			// a line can be returned
			// TODO: Exchange function->head for first asm line
			mcc_asm_delete_all_lines(function->head);
			return NULL;
		}
		// if pop omit the next assign instruction, since it is already handled with the pop instruction
		if(an_ir->row->instr == MCC_IR_INSTR_POP){
			an_ir = an_ir->next->next;
		} else {
			an_ir = an_ir->next;
		}
	}

	// TODO: Exchange function->head for first asm line
	if (!function->head)
		return get_fake_asm_line(err);

	// TODO: Simply return the first line of the block of asm code you created. The merging will happen later.
	return function->head;
}

static struct mcc_asm_line *generate_function_args(struct mcc_annotated_ir *an_ir, struct mcc_asm_error *err)
{
	assert(an_ir);
	assert(an_ir->row->instr == MCC_IR_INSTR_FUNC_LABEL);

	if (err->has_failed)
		return NULL;

	struct mcc_asm_operand *size_literal = mcc_asm_new_literal_operand(an_ir->stack_size, err);
	struct mcc_asm_line *sub_size_esp = mcc_asm_new_line(MCC_ASM_SUBL, size_literal, esp(err), NULL, err);

	return sub_size_esp;
}

static void compose_function_asm(struct mcc_asm_function *function,
                                 struct mcc_asm_line *prolog,
                                 struct mcc_asm_line *args,
                                 struct mcc_asm_line *body)
{
	assert(prolog);
	assert(body);
	assert(function);
	assert(args->first->type == MCC_ASM_OPERAND_LITERAL);
	function->head = prolog;
	prolog = last_asm_line(prolog);
	prolog->next = args;
	args = last_asm_line(args);
	args->next = body;
}

static struct mcc_asm_line *generate_function_prolog(struct mcc_asm_error *err)
{
	if (err->has_failed)
		return NULL;

	struct mcc_asm_line *mov_ebp_esp = mcc_asm_new_line(MCC_ASM_MOVL, esp(err), ebp(0, err), NULL, err);
	struct mcc_asm_line *push_ebp = mcc_asm_new_line(MCC_ASM_PUSHL, ebp(0, err), NULL, mov_ebp_esp, err);
	if (err->has_failed) {
		mcc_asm_delete_line(push_ebp);
		mcc_asm_delete_line(mov_ebp_esp);
		return NULL;
	}
	return push_ebp;
}

struct mcc_asm_function *mcc_asm_generate_function(struct mcc_annotated_ir *an_ir, struct mcc_asm_error *err)
{
	assert(an_ir->row->instr == MCC_IR_INSTR_FUNC_LABEL);
	assert(an_ir);
	assert(an_ir->row->arg1->type == MCC_IR_TYPE_FUNC_LABEL);

	if (err->has_failed)
		return NULL;

	struct mcc_asm_function *function = mcc_asm_new_function(an_ir->row->arg1->func_label, NULL, NULL, err);
	if (!function) {
		return NULL;
	}
	struct mcc_asm_line *prolog = generate_function_prolog(err);
	struct mcc_asm_line *args = generate_function_args(an_ir, err);
	struct mcc_asm_line *body = generate_function_body(function, an_ir, err);
	if (err->has_failed) {
		mcc_asm_delete_all_lines(prolog);
		mcc_asm_delete_all_lines(args);
		mcc_asm_delete_all_lines(body);
		return NULL;
	}
	compose_function_asm(function, prolog, args, body);
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

static bool generate_text_section(struct mcc_asm_text_section *text_section,
                                  struct mcc_annotated_ir *an_ir,
                                  struct mcc_asm_error *err)
{
	if (err->has_failed)
		return NULL;

	struct mcc_asm_function *first_function = mcc_asm_generate_function(an_ir, err);
	if (!first_function)
		return false;
	struct mcc_asm_function *latest_function = first_function;
	an_ir = find_next_function(an_ir);
	while (an_ir) {
		struct mcc_asm_function *new_function = mcc_asm_generate_function(an_ir, err);
		if (err->has_failed) {
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

static char *get_tmp_ident(char *id)
{
	memmove(id, id + 1, strlen(id));
	char *new = malloc(sizeof(char) * strlen(id) + 1);
	snprintf(new, strlen(id) + 1, "%s", id);
	return new;
}

static char *rename_identifier(char *id, int counter)
{
	assert(id);
	if (strncmp(id, "$tmp", 4) == 0) {
		return get_tmp_ident(id);
	} else {
		int extra_length = 2 + length_of_int(counter);
		int new_length = strlen(id) + extra_length;
		char *new = malloc(sizeof(char) * new_length);
		snprintf(new, new_length, "%s_%d", id, counter);
		counter++;
		return new;
	}
}

static bool generate_data_section(struct mcc_asm_data_section *data_section,
                                  struct mcc_annotated_ir *an_ir,
                                  struct mcc_asm_error *err)
{
	assert(data_section);
	assert(an_ir);
	assert(err);
	if (err->has_failed)
		return false;
	struct mcc_asm_declaration *head = data_section->head;
	int counter = 0;

	// Allocate all declared strings
	while (an_ir) {
		if (an_ir->row->instr != MCC_IR_INSTR_ASSIGN) {
			an_ir = an_ir->next;
			continue;
		}
		assert(an_ir->row->arg2);
		struct mcc_asm_declaration *decl = NULL;
		if (an_ir->row->arg2->type == MCC_IR_TYPE_LIT_STRING) {
			char *string_identifier = rename_identifier(an_ir->row->arg1->ident, counter);
			decl =
			    mcc_asm_new_string_declaration(string_identifier, an_ir->row->arg2->lit_string, NULL, err);
			free(string_identifier);
			counter++;
		} else if (an_ir->row->arg2->type == MCC_IR_TYPE_LIT_FLOAT) {
			char *float_identifier = rename_identifier(an_ir->row->arg1->ident, counter);
			decl = mcc_asm_new_float_declaration(float_identifier, an_ir->row->arg2->lit_float, NULL, err);
			free(float_identifier);
			counter++;
		} else {
			an_ir = an_ir->next;
			continue;
		}
		if (!decl) {
			mcc_asm_delete_all_declarations(head);
			err->has_failed = false;
			return false;
		}
		if (!head) {
			head = decl;
			data_section->head = head;
		} else {
			head->next = decl;
			head = decl;
		}
		an_ir = an_ir->next;
	}

	return true;
}

struct mcc_asm *mcc_asm_generate(struct mcc_ir_row *ir)
{
	struct mcc_asm_error *err = malloc(sizeof(*err));
	if (!err) {
		return NULL;
	}
	err->has_failed = false;
	struct mcc_annotated_ir *an_ir = mcc_annotate_ir(ir);
	struct mcc_asm *assembly = mcc_asm_new_asm(NULL, NULL, err);
	struct mcc_asm_text_section *text_section = mcc_asm_new_text_section(NULL, err);
	struct mcc_asm_data_section *data_section = mcc_asm_new_data_section(NULL, err);
	if (!an_ir || err->has_failed) {
		mcc_asm_delete_asm(assembly);
		mcc_asm_delete_text_section(text_section);
		mcc_asm_delete_data_section(data_section);
		mcc_delete_annotated_ir(an_ir);
		return NULL;
	}
	assembly->data_section = data_section;
	assembly->text_section = text_section;
	err->data_section = data_section;
	err->text_section = text_section;

	bool data_section_generated = generate_data_section(assembly->data_section, an_ir, err);
	bool text_section_generated = generate_text_section(assembly->text_section, an_ir, err);
	if (!text_section_generated || !data_section_generated) {
		mcc_asm_delete_asm(assembly);
		mcc_delete_annotated_ir(an_ir);
	}

	free(err);
	mcc_delete_annotated_ir(an_ir);

	return assembly;
}
