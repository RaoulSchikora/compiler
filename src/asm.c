#include "mcc/asm.h"

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "mcc/ir.h"
#include "mcc/stack_size.h"
#include "utils/length_of_int.h"

#define EPSILON 1e-06;

//---------------------------------------------------------------------------------------- Function prototypes

static int count_pushes(struct mcc_annotated_ir *an_ir);

// Create register operands more easily
static struct mcc_asm_operand *dl(struct mcc_asm_data *data);
static struct mcc_asm_operand *eax(struct mcc_asm_data *data);
static struct mcc_asm_operand *ebp(int offset, struct mcc_asm_data *data);
static struct mcc_asm_operand *ebx(struct mcc_asm_data *data);
static struct mcc_asm_operand *ecx(struct mcc_asm_data *data);
static struct mcc_asm_operand *edx(struct mcc_asm_data *data);
static struct mcc_asm_operand *esp(struct mcc_asm_data *data);

// Find Stuff
static struct mcc_asm_operand *find_float_identifier(struct mcc_annotated_ir *an_ir, struct mcc_asm_data *data);
static struct mcc_annotated_ir *find_next_function(struct mcc_annotated_ir *an_ir);
static struct mcc_asm_operand *find_string_identifier(struct mcc_annotated_ir *an_ir, struct mcc_asm_data *data);
static int get_row_offset(struct mcc_annotated_ir *an_ir, struct mcc_ir_row *row);
static char *get_tmp_ident(char *id);
static struct mcc_annotated_ir *
get_array_element_declaration(struct mcc_annotated_ir *an_ir, struct mcc_ir_arg *arg, struct mcc_asm_data *data);

// Check stuff
static bool is_float(struct mcc_ir_arg *arg, struct mcc_annotated_ir *an_ir, struct mcc_asm_data *data);
static bool is_in_data_section(char *ident, struct mcc_asm_data *data);
static bool is_proper_prefix(char *prefix, char *string);
static bool arg_is_local_array(struct mcc_annotated_ir *an_ir, struct mcc_ir_arg *arg);
static bool array_is_reference(struct mcc_annotated_ir *an_ir, struct mcc_ir_arg *arg, struct mcc_asm_data *data);

// Generate assembly
static void generate_asm_from_ir(struct mcc_annotated_ir *an_ir, struct mcc_asm_data *data);
static void
generate_arithm_float_op(struct mcc_annotated_ir *an_ir, enum mcc_asm_opcode opcode, struct mcc_asm_data *data);
static void
generate_arithm_int_op(struct mcc_annotated_ir *an_ir, enum mcc_asm_opcode opcode, struct mcc_asm_data *data);
static void generate_assign_row_ident(struct mcc_annotated_ir *an_ir, struct mcc_asm_data *data);
static void generate_call(struct mcc_annotated_ir *an_ir, struct mcc_asm_data *data);
static void generate_cmp(struct mcc_annotated_ir *an_ir, enum mcc_asm_opcode opcode, struct mcc_asm_data *data);
static void generate_cmp_op_float(struct mcc_annotated_ir *an_ir, struct mcc_asm_data *data);
static void generate_cmp_op_int(struct mcc_annotated_ir *an_ir, struct mcc_asm_data *data);
static void generate_div(struct mcc_annotated_ir *an_ir, struct mcc_asm_data *data);
static void generate_float_assign(struct mcc_annotated_ir *an_ir, struct mcc_asm_data *data);
static void
generate_function_body(struct mcc_asm_function *function, struct mcc_annotated_ir *an_ir, struct mcc_asm_data *data);
static void generate_instr_assign(struct mcc_annotated_ir *an_ir, struct mcc_asm_data *data);
static void generate_jumpfalse(enum mcc_asm_opcode opcode, struct mcc_annotated_ir *an_ir, struct mcc_asm_data *data);
static void generate_minus(struct mcc_annotated_ir *an_ir, struct mcc_asm_data *data);
static void generate_mult(struct mcc_annotated_ir *an_ir, struct mcc_asm_data *data);
static void generate_neg_float(struct mcc_annotated_ir *an_ir, struct mcc_asm_data *data);
static void generate_negative(struct mcc_annotated_ir *an_ir, struct mcc_asm_data *data);
static void generate_plus(struct mcc_annotated_ir *an_ir, struct mcc_asm_data *data);
static void generate_pop(struct mcc_annotated_ir *an_ir, struct mcc_asm_data *data);
static void generate_push(struct mcc_annotated_ir *an_ir, struct mcc_asm_data *data);
static void generate_return(struct mcc_annotated_ir *an_ir, struct mcc_asm_data *data);
static void generate_string_assignment(struct mcc_annotated_ir *an_ir, struct mcc_asm_data *data);
static void generate_unary(struct mcc_annotated_ir *an_ir, enum mcc_asm_opcode opcode, struct mcc_asm_data *data);

// Generate the sections
static void generate_text_section(struct mcc_asm_text_section *text_section,
                                  struct mcc_annotated_ir *an_ir,
                                  struct mcc_asm_data *data);
static void generate_data_section(struct mcc_asm_data_section *data_section,
                                  struct mcc_annotated_ir *an_ir,
                                  struct mcc_asm_data *data);

// Get IR args as ASM operand
static struct mcc_asm_operand *
arg_to_op(struct mcc_annotated_ir *an_ir, struct mcc_ir_arg *arg, struct mcc_asm_data *data);

// Get array elements as assembly operand
static struct mcc_asm_operand *
get_array_element_operand(struct mcc_annotated_ir *an_ir, struct mcc_ir_arg *arg, struct mcc_asm_data *data);
static int get_identifier_offset(struct mcc_annotated_ir *first, char *ident);
static int get_offset_of(struct mcc_annotated_ir *an_ir, struct mcc_ir_arg *arg);

//---------------------------------------------------------------------------------------- Implementation

static bool arg_is_local_array(struct mcc_annotated_ir *an_ir, struct mcc_ir_arg *arg)
{
	if (arg->type != MCC_IR_TYPE_IDENTIFIER) {
		return false;
	}
	an_ir = mcc_get_function_label(an_ir);
	an_ir = an_ir->next;
	while (an_ir && an_ir->row->instr != MCC_IR_INSTR_FUNC_LABEL) {
		if (an_ir->row->instr == MCC_IR_INSTR_ARRAY) {
			if (strcmp(an_ir->row->arg1->ident, arg->ident) == 0)
				return true;
		}
		an_ir = an_ir->next;
	}
	return false;
}

static int get_identifier_offset(struct mcc_annotated_ir *first, char *ident)
{
	assert(first);
	assert(ident);

	first = mcc_get_function_label(first);
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

static int get_row_offset(struct mcc_annotated_ir *an_ir, struct mcc_ir_row *row)
{
	assert(an_ir);
	assert(row);

	an_ir = mcc_get_function_label(an_ir);
	an_ir = an_ir->next;

	while (an_ir && an_ir->row->instr != MCC_IR_INSTR_FUNC_LABEL) {
		if (an_ir->row == row) {
			return an_ir->stack_position;
		}
		an_ir = an_ir->next;
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
	if (arg_is_local_array(an_ir, arg))
		return mcc_get_array_base_stack_loc(an_ir, arg);

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
mcc_asm_new_asm(struct mcc_asm_data_section *data_section, struct mcc_asm_text_section *text, struct mcc_asm_data *data)
{
	struct mcc_asm *new = malloc(sizeof(*new));
	if (!new) {
		data->has_failed = true;
		return NULL;
	}
	new->data_section = data_section;
	new->text_section = text;
	return new;
}

struct mcc_asm_data_section *mcc_asm_new_data_section(struct mcc_asm_declaration *head, struct mcc_asm_data *data)
{
	struct mcc_asm_data_section *new = malloc(sizeof(*new));
	if (!new) {
		data->has_failed = true;
		return NULL;
	}
	new->head = head;
	return new;
}

struct mcc_asm_text_section *mcc_asm_new_text_section(struct mcc_asm_function *function, struct mcc_asm_data *data)
{
	struct mcc_asm_text_section *new = malloc(sizeof(*new));
	if (!new) {
		data->has_failed = true;
		return NULL;
	}
	new->function = function;
	return new;
}

struct mcc_asm_declaration *mcc_asm_new_float_declaration(char *identifier,
                                                          double float_value,
                                                          struct mcc_asm_declaration *next,
                                                          struct mcc_asm_data *data)
{
	int size = strlen(identifier) + 1;
	struct mcc_asm_declaration *new = malloc(sizeof(*new));
	char *id_new = malloc(sizeof(char) * size);
	if (!new || !id_new) {
		data->has_failed = true;
		free(new);
		free(id_new);
		return NULL;
	}
	strcpy(id_new, identifier);
	new->identifier = id_new;
	new->float_value = float_value;
	new->next = next;
	new->type = MCC_ASM_DECLARATION_TYPE_FLOAT;
	return new;
}

struct mcc_asm_declaration *mcc_asm_new_string_declaration(char *identifier,
                                                           char *string_value,
                                                           struct mcc_asm_declaration *next,
                                                           struct mcc_asm_data *data)
{
	int size = strlen(identifier) + 1;
	struct mcc_asm_declaration *new = malloc(sizeof(*new));
	char *id_new = malloc(sizeof(char) * size);
	if (!new || !id_new) {
		data->has_failed = true;
		free(new);
		free(id_new);
		return NULL;
	}
	strcpy(id_new, identifier);
	new->identifier = id_new;
	new->string_value = string_value;
	new->next = next;
	new->type = MCC_ASM_DECLARATION_TYPE_STRING;
	return new;
}

struct mcc_asm_function *
mcc_asm_new_function(char *label, struct mcc_asm_line *head, struct mcc_asm_function *next, struct mcc_asm_data *data)
{
	int size = strlen(label) + 1;
	struct mcc_asm_function *new = malloc(sizeof(*new));
	char *lab_new = malloc(sizeof(char) * size);
	if (!new || !lab_new) {
		free(new);
		free(lab_new);
		data->has_failed = true;
		return NULL;
	}
	strcpy(lab_new, label);
	new->head = head;
	new->label = lab_new;
	new->next = next;
	return new;
}

void mcc_asm_new_line(enum mcc_asm_opcode opcode,
                      struct mcc_asm_operand *first,
                      struct mcc_asm_operand *second,
                      struct mcc_asm_data *data)
{
	if (data->has_failed) {
		return;
	}
	struct mcc_asm_line *new = malloc(sizeof(*new));
	if (!new) {
		data->has_failed = true;
		mcc_asm_delete_operand(first);
		mcc_asm_delete_operand(second);
		return;
	}
	new->opcode = opcode;
	new->first = first;
	new->second = second;
	new->next = NULL;

	data->current->next = new;
	data->current = new;
}

void mcc_asm_new_label(enum mcc_asm_opcode opcode, unsigned label, struct mcc_asm_data *data)
{
	if (data->has_failed) {
		return;
	}
	struct mcc_asm_line *new = malloc(sizeof(*new));
	if (!new) {
		data->has_failed = true;
		return;
	}
	new->opcode = opcode;
	new->label = label;
	new->next = NULL;

	data->current->next = new;
	data->current = new;
}

struct mcc_asm_operand *mcc_asm_new_function_operand(char *function_name, struct mcc_asm_data *data)
{
	int size = strlen(function_name) + 1;
	struct mcc_asm_operand *new = malloc(sizeof(*new));
	char *func_name_new = malloc(sizeof(char) * size);
	if (!new || !func_name_new) {
		free(new);
		free(func_name_new);
		data->has_failed = true;
		return NULL;
	}
	strcpy(func_name_new, function_name);
	new->type = MCC_ASM_OPERAND_FUNCTION;
	new->func_name = func_name_new;
	new->offset = 0;
	return new;
}

struct mcc_asm_operand *mcc_asm_new_literal_operand(int literal, struct mcc_asm_data *data)
{
	struct mcc_asm_operand *new = malloc(sizeof(*new));
	if (!new) {
		data->has_failed = true;
		return NULL;
	}
	new->type = MCC_ASM_OPERAND_LITERAL;
	new->literal = literal;
	new->offset = 0;
	return new;
}

struct mcc_asm_operand *mcc_asm_new_register_operand(enum mcc_asm_register reg, int offset, struct mcc_asm_data *data)
{
	struct mcc_asm_operand *new = malloc(sizeof(*new));
	if (!new) {
		data->has_failed = true;
		return NULL;
	}
	new->type = MCC_ASM_OPERAND_REGISTER;
	new->reg = reg;
	new->offset = offset;
	return new;
}

struct mcc_asm_operand *mcc_asm_new_computed_offset_operand(int offset_initial,
                                                            enum mcc_asm_register offset_base,
                                                            enum mcc_asm_register offset_factor,
                                                            int offset_size,
                                                            struct mcc_asm_data *data)
{
	struct mcc_asm_operand *new = malloc(sizeof(*new));
	if (!new) {
		data->has_failed = true;
		return NULL;
	}
	new->type = MCC_ASM_OPERAND_COMPUTED_OFFSET;
	new->offset_initial = offset_initial;
	new->offset_base = offset_base;
	new->offset_factor = offset_factor;
	new->offset_size = offset_size;
	return new;
}

struct mcc_asm_operand *mcc_asm_new_data_operand(struct mcc_asm_declaration *decl, struct mcc_asm_data *data)
{
	struct mcc_asm_operand *new = malloc(sizeof(*new));
	if (!new) {
		data->has_failed = true;
		return NULL;
	}
	new->type = MCC_ASM_OPERAND_DATA;
	new->decl = decl;
	new->offset = 0;
	return new;
}

//------------------------------------------------------------------------------------ Functions: Registers

static struct mcc_asm_operand *eax(struct mcc_asm_data *data)
{
	return mcc_asm_new_register_operand(MCC_ASM_EAX, 0, data);
}

static struct mcc_asm_operand *ebx(struct mcc_asm_data *data)
{
	return mcc_asm_new_register_operand(MCC_ASM_EBX, 0, data);
}

static struct mcc_asm_operand *ecx(struct mcc_asm_data *data)
{
	return mcc_asm_new_register_operand(MCC_ASM_ECX, 0, data);
}

static struct mcc_asm_operand *edx(struct mcc_asm_data *data)
{
	return mcc_asm_new_register_operand(MCC_ASM_EDX, 0, data);
}

static struct mcc_asm_operand *dl(struct mcc_asm_data *data)
{
	return mcc_asm_new_register_operand(MCC_ASM_DL, 0, data);
}

static struct mcc_asm_operand *ebp(int offset, struct mcc_asm_data *data)
{
	return mcc_asm_new_register_operand(MCC_ASM_EBP, offset, data);
}

static struct mcc_asm_operand *esp(struct mcc_asm_data *data)
{
	return mcc_asm_new_register_operand(MCC_ASM_ESP, 0, data);
}

static struct mcc_asm_operand *st(int offset, struct mcc_asm_data *data)
{
	return mcc_asm_new_register_operand(MCC_ASM_ST, offset, data);
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
	if (decl->type == MCC_ASM_DECLARATION_TYPE_STRING || decl->type == MCC_ASM_DECLARATION_TYPE_FLOAT) {
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

static struct mcc_annotated_ir *
get_array_element_declaration(struct mcc_annotated_ir *an_ir, struct mcc_ir_arg *arg, struct mcc_asm_data *data)
{
	assert(an_ir);
	if (data->has_failed)
		return NULL;
	an_ir = mcc_get_function_label(an_ir);
	an_ir = an_ir->next;
	while (an_ir->row->instr != MCC_IR_INSTR_FUNC_LABEL) {
		if (an_ir->row->instr == MCC_IR_INSTR_ARRAY) {
			if (strcmp(an_ir->row->arg1->ident, arg->ident) == 0) {
				return an_ir;
			}
		}
		if (an_ir->row->instr == MCC_IR_INSTR_ASSIGN) {
			if (strcmp(an_ir->row->arg1->ident, arg->ident) == 0) {
				return an_ir;
			}
		}
		an_ir = an_ir->next;
		continue;
	}
	data->has_failed = true;
	return NULL;
}

static bool array_is_reference(struct mcc_annotated_ir *an_ir, struct mcc_ir_arg *arg, struct mcc_asm_data *data)
{
	assert(an_ir);
	assert(arg);

	an_ir = get_array_element_declaration(an_ir, arg, data);
	if (data->has_failed)
		return false;
	return (an_ir->prev->row->instr == MCC_IR_INSTR_POP);
}

static struct mcc_asm_operand *
get_array_element_operand(struct mcc_annotated_ir *an_ir, struct mcc_ir_arg *arg, struct mcc_asm_data *data)
{
	assert(an_ir);
	assert(arg);
	assert(arg->type == MCC_IR_TYPE_ARR_ELEM);
	assert(data);
	if (data->has_failed)
		return NULL;

	int index_offset;
	int offset;
	bool is_reference = array_is_reference(an_ir, arg, data);

	if (is_reference)
		offset = get_identifier_offset(an_ir, arg->arr_ident);

	switch (arg->index->type) {
	case MCC_IR_TYPE_LIT_INT:
		index_offset = arg->index->lit_int;
		mcc_asm_new_line(MCC_ASM_MOVL, mcc_asm_new_literal_operand(index_offset, data), ebx(data), data);
		break;
	case MCC_IR_TYPE_IDENTIFIER:
		index_offset = get_identifier_offset(an_ir, arg->index->ident);
		mcc_asm_new_line(MCC_ASM_MOVL, ebp(index_offset, data), ebx(data), data);
		break;
	case MCC_IR_TYPE_ROW:
		index_offset = get_row_offset(an_ir, arg->index->row);
		mcc_asm_new_line(MCC_ASM_MOVL, ebp(index_offset, data), ebx(data), data);
		break;
	default:
		data->has_failed = true;
		return NULL;
	}

	if (is_reference) {
		mcc_asm_new_line(MCC_ASM_MOVL, ebp(offset, data), ecx(data), data);
		return mcc_asm_new_computed_offset_operand(0, MCC_ASM_ECX, MCC_ASM_EBX, DWORD_SIZE, data);
	} else {
		return mcc_asm_new_computed_offset_operand(mcc_get_array_base_stack_loc(an_ir, arg), MCC_ASM_EBP,
		                                           MCC_ASM_EBX, DWORD_SIZE, data);
	}
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

static bool is_in_data_section(char *ident, struct mcc_asm_data *data)
{
	struct mcc_asm_declaration *decl = data->data_section->head;
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

static bool is_float(struct mcc_ir_arg *arg, struct mcc_annotated_ir *an_ir, struct mcc_asm_data *data)
{
	assert(arg);
	assert(an_ir);

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
		return is_in_data_section(arg->ident, data);
	case MCC_IR_TYPE_ARR_ELEM:
		an_ir = get_array_element_declaration(an_ir, arg, data);
		return (an_ir->row->type->type == MCC_IR_ROW_FLOAT);
	default:
		return false;
	}
}

static int count_pushes(struct mcc_annotated_ir *an_ir)
{
	assert(an_ir->row->instr == MCC_IR_INSTR_CALL);
	an_ir = an_ir->prev;
	int num_pushes = 0;
	while (an_ir->row->instr == MCC_IR_INSTR_PUSH) {
		num_pushes += 1;
		an_ir = an_ir->prev;
	}
	return num_pushes;
}

static struct mcc_asm_operand *
arg_to_op(struct mcc_annotated_ir *an_ir, struct mcc_ir_arg *arg, struct mcc_asm_data *data)
{
	assert(an_ir);
	assert(arg);
	if (data->has_failed)
		return NULL;

	struct mcc_asm_operand *operand = NULL;
	switch (arg->type) {
	case MCC_IR_TYPE_LIT_INT:
		operand = mcc_asm_new_literal_operand(arg->lit_int, data);
		break;
	case MCC_IR_TYPE_LIT_BOOL:
		operand = mcc_asm_new_literal_operand(arg->lit_bool, data);
		break;
	case MCC_IR_TYPE_ROW:
	case MCC_IR_TYPE_IDENTIFIER:
		operand = mcc_asm_new_register_operand(MCC_ASM_EBP, get_offset_of(an_ir, arg), data);
		break;
	case MCC_IR_TYPE_ARR_ELEM:
		operand = get_array_element_operand(an_ir, arg, data);
		break;
	default:
		break;
	}

	return operand;
}

static struct mcc_asm_operand *find_string_identifier(struct mcc_annotated_ir *an_ir, struct mcc_asm_data *data)
{
	assert(an_ir);
	assert(an_ir->row->instr == MCC_IR_INSTR_ASSIGN);
	assert(an_ir->row->arg2->type == MCC_IR_TYPE_LIT_STRING);
	assert(data);
	if (data->has_failed)
		return NULL;

	struct mcc_asm_operand *op = malloc(sizeof(*op));
	if (!op) {
		data->has_failed = true;
		return NULL;
	}

	char *wanted_string = an_ir->row->arg2->lit_string;
	struct mcc_asm_declaration *head = data->data_section->head;
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

static struct mcc_asm_operand *find_float_identifier(struct mcc_annotated_ir *an_ir, struct mcc_asm_data *data)
{
	assert(an_ir);
	assert(an_ir->row->instr == MCC_IR_INSTR_ASSIGN);
	assert(an_ir->row->arg2->type == MCC_IR_TYPE_LIT_FLOAT);
	assert(data);
	if (data->has_failed)
		return NULL;

	struct mcc_asm_operand *op = malloc(sizeof(*op));
	if (!op) {
		data->has_failed = true;
		return NULL;
	}

	double wanted_float = an_ir->row->arg2->lit_float;
	double epsilon = EPSILON;
	struct mcc_asm_declaration *head = data->data_section->head;
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

static void generate_string_assignment(struct mcc_annotated_ir *an_ir, struct mcc_asm_data *data)
{
	struct mcc_asm_operand *string_id = find_string_identifier(an_ir, data);
	mcc_asm_new_line(MCC_ASM_LEAL, string_id, eax(data), data);
	mcc_asm_new_line(MCC_ASM_MOVL, eax(data), ebp(an_ir->stack_position, data), data);
}

static void generate_assign_row_ident(struct mcc_annotated_ir *an_ir, struct mcc_asm_data *data)
{
	assert(an_ir);
	mcc_asm_new_line(MCC_ASM_MOVL, arg_to_op(an_ir, an_ir->row->arg2, data), eax(data), data);
	mcc_asm_new_line(MCC_ASM_MOVL, eax(data), arg_to_op(an_ir, an_ir->row->arg1, data), data);
}

static void generate_float_assign(struct mcc_annotated_ir *an_ir, struct mcc_asm_data *data)
{
	mcc_asm_new_line(MCC_ASM_FLDS, find_float_identifier(an_ir, data), NULL, data);
	mcc_asm_new_line(MCC_ASM_FSTPS, arg_to_op(an_ir, an_ir->row->arg1, data), NULL, data);
}

static void generate_instr_assign(struct mcc_annotated_ir *an_ir, struct mcc_asm_data *data)
{
	assert(an_ir);
	assert(an_ir->row->instr == MCC_IR_INSTR_ASSIGN);

	if (data->has_failed)
		return;

	switch (an_ir->row->arg2->type) {
	case MCC_IR_TYPE_LIT_INT:
	case MCC_IR_TYPE_LIT_BOOL:
		mcc_asm_new_line(MCC_ASM_MOVL, arg_to_op(an_ir, an_ir->row->arg2, data),
		                 arg_to_op(an_ir, an_ir->row->arg1, data), data);
		break;
	case MCC_IR_TYPE_LIT_FLOAT:
		generate_float_assign(an_ir, data);
		break;
	case MCC_IR_TYPE_ROW:
	case MCC_IR_TYPE_IDENTIFIER:
	case MCC_IR_TYPE_ARR_ELEM:
		generate_assign_row_ident(an_ir, data);
		break;
	case MCC_IR_TYPE_LIT_STRING:
		generate_string_assignment(an_ir, data);
		break;
	default:
		break;
	}
}

static void
generate_arithm_int_op(struct mcc_annotated_ir *an_ir, enum mcc_asm_opcode opcode, struct mcc_asm_data *data)
{
	assert(an_ir);
	if (data->has_failed) {
		return;
	}

	mcc_asm_new_line(MCC_ASM_MOVL, arg_to_op(an_ir, an_ir->row->arg1, data), eax(data), data);

	if (opcode == MCC_ASM_IDIVL) {
		mcc_asm_new_line(MCC_ASM_MOVL, arg_to_op(an_ir, an_ir->row->arg2, data), ebx(data), data);
		// line to clear EDX
		mcc_asm_new_line(MCC_ASM_XORL, edx(data), edx(data), data);
		mcc_asm_new_line(opcode, ebx(data), NULL, data);
	} else {
		mcc_asm_new_line(opcode, arg_to_op(an_ir, an_ir->row->arg2, data), eax(data), data);
	}
	mcc_asm_new_line(MCC_ASM_MOVL, eax(data), ebp(an_ir->stack_position, data), data);
}

static void generate_unary(struct mcc_annotated_ir *an_ir, enum mcc_asm_opcode opcode, struct mcc_asm_data *data)
{
	assert(an_ir);
	if (data->has_failed)
		return;

	mcc_asm_new_line(MCC_ASM_MOVL, arg_to_op(an_ir, an_ir->row->arg1, data), eax(data), data);
	if (opcode == MCC_ASM_XORL) {
		struct mcc_asm_operand *lit_1 = mcc_asm_new_literal_operand((int)1, data);
		mcc_asm_new_line(MCC_ASM_XORL, lit_1, eax(data), data);
	} else {
		mcc_asm_new_line(opcode, eax(data), NULL, data);
	}
	mcc_asm_new_line(MCC_ASM_MOVL, eax(data), ebp(an_ir->stack_position, data), data);
}

static void generate_cmp_op_int(struct mcc_annotated_ir *an_ir, struct mcc_asm_data *data)
{
	assert(an_ir);
	if (data->has_failed)
		return;

	if ((an_ir->row->arg1->type == MCC_IR_TYPE_ROW || an_ir->row->arg1->type == MCC_IR_TYPE_IDENTIFIER ||
	     an_ir->row->arg1->type == MCC_IR_TYPE_ARR_ELEM) &&
	    (an_ir->row->arg2->type == MCC_IR_TYPE_ROW || an_ir->row->arg2->type == MCC_IR_TYPE_IDENTIFIER ||
	     an_ir->row->arg2->type == MCC_IR_TYPE_ARR_ELEM)) {
		// 1a. move arg1 in eax
		mcc_asm_new_line(MCC_ASM_MOVL, arg_to_op(an_ir, an_ir->row->arg1, data), eax(data), data);
		// 1b. cmp eax and arg2
		mcc_asm_new_line(MCC_ASM_CMPL, arg_to_op(an_ir, an_ir->row->arg2, data), eax(data), data);

	} else if (an_ir->row->arg1->type == MCC_IR_TYPE_ROW || an_ir->row->arg1->type == MCC_IR_TYPE_IDENTIFIER) {
		// 1. cmp arg1 arg2
		mcc_asm_new_line(MCC_ASM_CMPL, arg_to_op(an_ir, an_ir->row->arg2, data),
		                 arg_to_op(an_ir, an_ir->row->arg1, data), data);
	} else {
		// 1.a mov lit eax
		mcc_asm_new_line(MCC_ASM_MOVL, arg_to_op(an_ir, an_ir->row->arg1, data), eax(data), data);
		// 1b. cmp arg1 arg2
		mcc_asm_new_line(MCC_ASM_CMPL, arg_to_op(an_ir, an_ir->row->arg2, data), eax(data), data);
	}
}

static void generate_return(struct mcc_annotated_ir *an_ir, struct mcc_asm_data *data)
{
	assert(an_ir);
	assert(an_ir->row->instr == MCC_IR_INSTR_RETURN);

	if (data->has_failed)
		return;

	if (an_ir->row->arg1) {
		if (an_ir->row->type->type == MCC_IR_ROW_FLOAT) {
			mcc_asm_new_line(MCC_ASM_FLDS, arg_to_op(an_ir, an_ir->row->arg1, data), NULL, data);
		} else {
			mcc_asm_new_line(MCC_ASM_MOVL, arg_to_op(an_ir, an_ir->row->arg1, data), eax(data), data);
		}
	}
	// pop ebx
	an_ir = mcc_get_function_label(an_ir);
	if (strcmp(an_ir->row->arg1->func_label, "main") != 0) {
		mcc_asm_new_line(MCC_ASM_POPL, ebx(data), NULL, data);
	}
	mcc_asm_new_line(MCC_ASM_LEAVE, NULL, NULL, data);
	mcc_asm_new_line(MCC_ASM_RETURN, NULL, NULL, data);
}

static void generate_push(struct mcc_annotated_ir *an_ir, struct mcc_asm_data *data)
{
	assert(an_ir);
	assert(an_ir->row->arg1);
	if (arg_is_local_array(an_ir, an_ir->row->arg1)) {
		mcc_asm_new_line(MCC_ASM_LEAL, arg_to_op(an_ir, an_ir->row->arg1, data), eax(data), data);
		mcc_asm_new_line(MCC_ASM_PUSHL, eax(data), NULL, data);
		return;
	}
	assert(an_ir->row->instr == MCC_IR_INSTR_PUSH);
	mcc_asm_new_line(MCC_ASM_PUSHL, arg_to_op(an_ir, an_ir->row->arg1, data), NULL, data);
}

static void generate_jumpfalse(enum mcc_asm_opcode opcode, struct mcc_annotated_ir *an_ir, struct mcc_asm_data *data)
{
	assert(an_ir);
	if (data->has_failed)
		return;

	if (opcode == MCC_ASM_JNE) {
		struct mcc_asm_operand *one = mcc_asm_new_literal_operand(1, data);
		mcc_asm_new_line(MCC_ASM_MOVL, one, eax(data), data);
		mcc_asm_new_line(MCC_ASM_CMPL, arg_to_op(an_ir, an_ir->row->arg1, data), eax(data), data);
	} else { // case of MCC_ASM_JE
		mcc_asm_new_line(MCC_ASM_CMPL, eax(data), eax(data), data);
	}

	unsigned label = opcode == MCC_ASM_JNE ? an_ir->row->arg2->label : an_ir->row->arg1->label;
	mcc_asm_new_label(opcode, label, data);
}

static void
generate_arithm_float_op(struct mcc_annotated_ir *an_ir, enum mcc_asm_opcode opcode, struct mcc_asm_data *data)
{
	assert(an_ir);
	mcc_asm_new_line(MCC_ASM_FLDS, arg_to_op(an_ir, an_ir->row->arg2, data), NULL, data);
	mcc_asm_new_line(MCC_ASM_FLDS, arg_to_op(an_ir, an_ir->row->arg1, data), NULL, data);
	mcc_asm_new_line(opcode, st(0, data), st(1, data), data);
	mcc_asm_new_line(MCC_ASM_FSTPS, ebp(an_ir->stack_position, data), NULL, data);
}

static void generate_plus(struct mcc_annotated_ir *an_ir, struct mcc_asm_data *data)
{
	if (an_ir->row->type->type == MCC_IR_ROW_INT) {
		generate_arithm_int_op(an_ir, MCC_ASM_ADDL, data);
	} else if (an_ir->row->type->type == MCC_IR_ROW_FLOAT) {
		generate_arithm_float_op(an_ir, MCC_ASM_FADDP, data);
	}
}

static void generate_minus(struct mcc_annotated_ir *an_ir, struct mcc_asm_data *data)
{
	if (an_ir->row->type->type == MCC_IR_ROW_INT) {
		generate_arithm_int_op(an_ir, MCC_ASM_SUBL, data);
	} else if (an_ir->row->type->type == MCC_IR_ROW_FLOAT) {
		generate_arithm_float_op(an_ir, MCC_ASM_FSUBP, data);
	}
}

static void generate_cmp_op_float(struct mcc_annotated_ir *an_ir, struct mcc_asm_data *data)
{
	assert(an_ir);
	if (data->has_failed) {
		return;
	}

	mcc_asm_new_line(MCC_ASM_FLDS, arg_to_op(an_ir, an_ir->row->arg2, data), NULL, data);
	mcc_asm_new_line(MCC_ASM_FLDS, arg_to_op(an_ir, an_ir->row->arg1, data), NULL, data);
	mcc_asm_new_line(MCC_ASM_FCOMIP, st(1, data), st(0, data), data);
	mcc_asm_new_line(MCC_ASM_FSTP, st(0, data), NULL, data);
}

static void generate_cmp(struct mcc_annotated_ir *an_ir, enum mcc_asm_opcode opcode, struct mcc_asm_data *data)
{
	assert(an_ir);
	if (data->has_failed)
		return;

	// line 1 to line 1c or line 1d (depended on case)
	if (is_float(an_ir->row->arg1, an_ir, data)) {
		generate_cmp_op_float(an_ir, data);
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
		generate_cmp_op_int(an_ir, data);
	}

	// 2. setcc dl
	mcc_asm_new_line(opcode, dl(data), NULL, data);
	// 3. movcc dl eax
	mcc_asm_new_line(MCC_ASM_MOVZBL, dl(data), eax(data), data);
	// 4. movl eax -x(ebp)
	mcc_asm_new_line(MCC_ASM_MOVL, eax(data), ebp(an_ir->stack_position, data), data);
}

static void generate_mult(struct mcc_annotated_ir *an_ir, struct mcc_asm_data *data)
{
	assert(an_ir);
	if (an_ir->row->type->type == MCC_IR_ROW_INT) {
		generate_arithm_int_op(an_ir, MCC_ASM_IMULL, data);
	} else if (an_ir->row->type->type == MCC_IR_ROW_FLOAT) {
		generate_arithm_float_op(an_ir, MCC_ASM_FMULP, data);
	}
}

static void generate_div(struct mcc_annotated_ir *an_ir, struct mcc_asm_data *data)
{
	assert(an_ir);
	if (an_ir->row->type->type == MCC_IR_ROW_INT) {
		generate_arithm_int_op(an_ir, MCC_ASM_IDIVL, data);
	} else if (an_ir->row->type->type == MCC_IR_ROW_FLOAT) {
		generate_arithm_float_op(an_ir, MCC_ASM_FDIVP, data);
	}
}

static void generate_call(struct mcc_annotated_ir *an_ir, struct mcc_asm_data *data)
{
	assert(an_ir);
	struct mcc_asm_operand *func = mcc_asm_new_function_operand(an_ir->row->arg1->func_label, data);
	mcc_asm_new_line(MCC_ASM_CALLL, func, NULL, data);
	// count pushes before call and add to esp afterwards
	int literal = count_pushes(an_ir);
	if (literal != 0) {
		mcc_asm_new_line(MCC_ASM_ADDL, mcc_asm_new_literal_operand(literal * 4, data), esp(data), data);
	}
	// if function is void do no move instruction
	if (an_ir->row->type->type != MCC_IR_ROW_TYPELESS) {
		if (an_ir->row->type->type == MCC_IR_ROW_FLOAT) {
			mcc_asm_new_line(MCC_ASM_FSTPS, ebp(an_ir->stack_position, data), NULL, data);
		} else {
			mcc_asm_new_line(MCC_ASM_MOVL, eax(data), ebp(an_ir->stack_position, data), data);
		}
	}
}

static void generate_pop(struct mcc_annotated_ir *an_ir, struct mcc_asm_data *data)
{
	assert(an_ir);
	assert(an_ir->row->instr == MCC_IR_INSTR_POP);
	mcc_asm_new_line(MCC_ASM_MOVL, ebp(an_ir->stack_position, data), eax(data), data);
	mcc_asm_new_line(MCC_ASM_MOVL, eax(data), arg_to_op(an_ir->next, an_ir->next->row->arg1, data), data);
}

static void generate_neg_float(struct mcc_annotated_ir *an_ir, struct mcc_asm_data *data)
{
	assert(an_ir);
	mcc_asm_new_line(MCC_ASM_FLDS, arg_to_op(an_ir, an_ir->row->arg1, data), NULL, data);
	mcc_asm_new_line(MCC_ASM_FCHS, NULL, NULL, data);
	mcc_asm_new_line(MCC_ASM_FSTPS, ebp(an_ir->stack_position, data), NULL, data);
}

static void generate_negative(struct mcc_annotated_ir *an_ir, struct mcc_asm_data *data)
{
	assert(an_ir);
	if (an_ir->row->type->type == MCC_IR_ROW_INT) {
		generate_unary(an_ir, MCC_ASM_NEGL, data);
	} else if (an_ir->row->type->type == MCC_IR_ROW_FLOAT) {
		generate_neg_float(an_ir, data);
	}
}

static void generate_asm_from_ir(struct mcc_annotated_ir *an_ir, struct mcc_asm_data *data)
{
	assert(an_ir);

	if (data->has_failed)
		return;

	switch (an_ir->row->instr) {
	case MCC_IR_INSTR_ASSIGN:
		generate_instr_assign(an_ir, data);
		break;
	case MCC_IR_INSTR_LABEL:
		mcc_asm_new_label(MCC_ASM_LABEL, an_ir->row->arg1->label, data);
		break;
	case MCC_IR_INSTR_FUNC_LABEL:
		break;
	case MCC_IR_INSTR_JUMP:
		generate_jumpfalse(MCC_ASM_JE, an_ir, data);
		break;
	case MCC_IR_INSTR_CALL:
		generate_call(an_ir, data);
		break;
	case MCC_IR_INSTR_JUMPFALSE:
		generate_jumpfalse(MCC_ASM_JNE, an_ir, data);
		break;
	case MCC_IR_INSTR_PUSH:
		generate_push(an_ir, data);
		break;
	case MCC_IR_INSTR_POP:
		generate_pop(an_ir, data);
		break;
	case MCC_IR_INSTR_EQUALS:
		generate_cmp(an_ir, MCC_ASM_SETE, data);
		break;
	case MCC_IR_INSTR_NOTEQUALS:
		generate_cmp(an_ir, MCC_ASM_SETNE, data);
		break;
	case MCC_IR_INSTR_SMALLER:
		generate_cmp(an_ir, MCC_ASM_SETL, data);
		break;
	case MCC_IR_INSTR_GREATER:
		generate_cmp(an_ir, MCC_ASM_SETG, data);
		break;
	case MCC_IR_INSTR_SMALLEREQ:
		generate_cmp(an_ir, MCC_ASM_SETLE, data);
		break;
	case MCC_IR_INSTR_GREATEREQ:
		generate_cmp(an_ir, MCC_ASM_SETGE, data);
		break;
	case MCC_IR_INSTR_AND:
		generate_arithm_int_op(an_ir, MCC_ASM_AND, data);
		break;
	case MCC_IR_INSTR_OR:
		generate_arithm_int_op(an_ir, MCC_ASM_OR, data);
		break;
	case MCC_IR_INSTR_PLUS:
		generate_plus(an_ir, data);
		break;
	case MCC_IR_INSTR_MINUS:
		generate_minus(an_ir, data);
		break;
	case MCC_IR_INSTR_MULTIPLY:
		generate_mult(an_ir, data);
		break;
	case MCC_IR_INSTR_DIVIDE:
		generate_div(an_ir, data);
		break;
	case MCC_IR_INSTR_RETURN:
		generate_return(an_ir, data);
		break;
	// In these cases nothing needs to happen
	case MCC_IR_INSTR_ARRAY:
		break;
	case MCC_IR_INSTR_NEGATIV:
		generate_negative(an_ir, data);
		break;
	case MCC_IR_INSTR_NOT:
		generate_unary(an_ir, MCC_ASM_XORL, data);
		break;
	case MCC_IR_INSTR_UNKNOWN:
		break;
	}
}

static void
generate_function_body(struct mcc_asm_function *function, struct mcc_annotated_ir *an_ir, struct mcc_asm_data *data)
{
	assert(function);
	assert(an_ir);
	assert(an_ir->row->instr == MCC_IR_INSTR_FUNC_LABEL);

	if (data->has_failed)
		return;

	an_ir = an_ir->next;

	// Iterate up to next function
	while (an_ir && an_ir->row->instr != MCC_IR_INSTR_FUNC_LABEL) {

		generate_asm_from_ir(an_ir, data);
		if (data->has_failed) {
			return;
		}
		// if pop, omit the next assign instruction, since it is already handled with the pop instruction
		if (an_ir->row->instr == MCC_IR_INSTR_POP) {
			an_ir = an_ir->next->next;
		} else {
			an_ir = an_ir->next;
		}
	}
}

struct mcc_asm_function *mcc_asm_generate_function(struct mcc_annotated_ir *an_ir, struct mcc_asm_data *data)
{
	assert(an_ir->row->instr == MCC_IR_INSTR_FUNC_LABEL);
	assert(an_ir);
	assert(an_ir->row->arg1->type == MCC_IR_TYPE_FUNC_LABEL);

	if (data->has_failed)
		return NULL;

	struct mcc_asm_function *function = mcc_asm_new_function(an_ir->row->arg1->func_label, NULL, NULL, data);
	if (!function) {
		data->has_failed = true;
		return NULL;
	}

	// Prolog
	struct mcc_asm_line *push_ebp = malloc(sizeof *push_ebp);
	if (!push_ebp) {
		data->has_failed = true;
		mcc_asm_delete_function(function);
		return NULL;
	}
	push_ebp->opcode = MCC_ASM_PUSHL;
	push_ebp->first = ebp(0, data);
	push_ebp->second = NULL;
	data->current = push_ebp;
	mcc_asm_new_line(MCC_ASM_MOVL, esp(data), ebp(0, data), data);
	// Func args
	struct mcc_asm_operand *size_literal = mcc_asm_new_literal_operand(an_ir->stack_size, data);
	mcc_asm_new_line(MCC_ASM_SUBL, size_literal, esp(data), data);
	// store ebx except for main
	if (strcmp(an_ir->row->arg1->func_label, "main") != 0) {
		mcc_asm_new_line(MCC_ASM_PUSHL, ebx(data), NULL, data);
	}

	// Function body
	generate_function_body(function, an_ir, data);

	if (data->has_failed) {
		mcc_asm_delete_all_lines(push_ebp);
		mcc_asm_delete_function(function);
		return NULL;
	}
	function->head = push_ebp;

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

static void generate_text_section(struct mcc_asm_text_section *text_section,
                                  struct mcc_annotated_ir *an_ir,
                                  struct mcc_asm_data *data)
{
	if (data->has_failed)
		return;

	struct mcc_asm_function *first_function = mcc_asm_generate_function(an_ir, data);
	if (!first_function) {
		data->has_failed = true;
		return;
	}
	struct mcc_asm_function *latest_function = first_function;
	an_ir = find_next_function(an_ir);
	while (an_ir) {
		struct mcc_asm_function *new_function = mcc_asm_generate_function(an_ir, data);
		if (data->has_failed) {
			mcc_asm_delete_all_functions(first_function);
			return;
		}
		latest_function->next = new_function;
		latest_function = new_function;
		an_ir = find_next_function(an_ir);
	}
	text_section->function = first_function;
	return;
}

static char *get_tmp_ident(char *id)
{
	// TODO: Check if memmove necessary. Shouldn't it be sizeof(char) * (strlen(id) +1). (Bracket nesting) 
	memmove(id, id + 1, strlen(id));
	char *new = malloc(sizeof(char) * strlen(id) + 1);
	if (!new)
		return NULL;
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
		if (!new)
			return NULL;
		snprintf(new, new_length, "%s_%d", id, counter);
		counter++;
		return new;
	}
}

static void generate_data_section(struct mcc_asm_data_section *data_section,
                                  struct mcc_annotated_ir *an_ir,
                                  struct mcc_asm_data *data)
{
	assert(data_section);
	assert(an_ir);
	assert(data);
	if (data->has_failed)
		return;
	struct mcc_asm_declaration *head = data_section->head;
	int counter = 0;

	// Allocate all declared strings
	while (an_ir) {
		if (an_ir->row->instr != MCC_IR_INSTR_ASSIGN) {
			an_ir = an_ir->next;
			continue;
		}
		struct mcc_asm_declaration *decl = NULL;
		if (an_ir->row->arg2->type == MCC_IR_TYPE_LIT_STRING) {
			char *string_identifier = rename_identifier(an_ir->row->arg1->ident, counter);
			decl =
			    mcc_asm_new_string_declaration(string_identifier, an_ir->row->arg2->lit_string, NULL, data);
			free(string_identifier);
			counter++;
		} else if (an_ir->row->arg2->type == MCC_IR_TYPE_LIT_FLOAT) {
			char *float_identifier = rename_identifier(an_ir->row->arg1->ident, counter);
			decl = mcc_asm_new_float_declaration(float_identifier, an_ir->row->arg2->lit_float, NULL, data);
			free(float_identifier);
			counter++;
		} else {
			an_ir = an_ir->next;
			continue;
		}
		if (!decl) {
			mcc_asm_delete_all_declarations(head);
			data->has_failed = false;
			return;
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

	return;
}

struct mcc_asm *mcc_asm_generate(struct mcc_ir_row *ir)
{
	struct mcc_asm_data *data = malloc(sizeof(*data));
	if (!data) {
		return NULL;
	}
	data->has_failed = false;
	struct mcc_annotated_ir *an_ir = mcc_annotate_ir(ir);
	struct mcc_asm *assembly = mcc_asm_new_asm(NULL, NULL, data);
	struct mcc_asm_text_section *text_section = mcc_asm_new_text_section(NULL, data);
	struct mcc_asm_data_section *data_section = mcc_asm_new_data_section(NULL, data);
	if (!an_ir || data->has_failed) {
		mcc_asm_delete_asm(assembly);
		mcc_asm_delete_text_section(text_section);
		mcc_asm_delete_data_section(data_section);
		mcc_delete_annotated_ir(an_ir);
		return NULL;
	}
	assembly->data_section = data_section;
	assembly->text_section = text_section;
	data->data_section = data_section;

	generate_data_section(assembly->data_section, an_ir, data);
	generate_text_section(assembly->text_section, an_ir, data);
	if (data->has_failed) {
		mcc_asm_delete_asm(assembly);
		mcc_delete_annotated_ir(an_ir);
	}

	free(data);
	mcc_delete_annotated_ir(an_ir);

	return assembly;
}
