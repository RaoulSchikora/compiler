#include "mcc/stack_size.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

struct mcc_annotated_ir *mcc_get_function_label(struct mcc_annotated_ir *an_ir)
{
	assert(an_ir);
	while (an_ir && an_ir->row->instr != MCC_IR_INSTR_FUNC_LABEL) {
		an_ir = an_ir->prev;
	}
	return an_ir;
}

struct mcc_annotated_ir *mcc_new_annotated_ir(struct mcc_ir_row *row, int stack_size)
{
	assert(row);
	struct mcc_annotated_ir *ir = malloc(sizeof(*ir));
	if (!ir)
		return NULL;
	ir->stack_size = stack_size;
	ir->stack_position = 0;
	ir->row = row;
	ir->next = NULL;
	ir->prev = NULL;
	return ir;
}

void mcc_delete_annotated_ir(struct mcc_annotated_ir *head)
{
	if (!head)
		return;

	mcc_delete_annotated_ir(head->next);
	free(head);
}

// --------------------------------------------------------------------------------------- Forward declarations

static struct mcc_ir_row *first_line_of_function(struct mcc_ir_row *ir);
static int get_row_size(struct mcc_ir_row *ir);

// --------------------------------------------------------------------------------------- Calc stack size and position

static bool assignment_is_first_occurence(struct mcc_ir_row *first, struct mcc_ir_row *ir)
{
	assert(first);
	assert(ir);
	assert(ir->instr == MCC_IR_INSTR_ASSIGN);

	// Arrays are allocated when they're declared
	if (ir->arg1->type == MCC_IR_TYPE_ARR_ELEM) {
		return false;
	}

	char *id_name = ir->arg1->ident;
	struct mcc_ir_row *head = first;
	while (head != ir) {
		if (head->instr != MCC_IR_INSTR_ASSIGN) {
			head = head->next_row;
			continue;
		}
		if (strcmp(head->arg1->ident, id_name) == 0) {
			return false;
		}
		head = head->next_row;
	}
	return true;
}

static int get_var_size(struct mcc_ir_row *ir)
{
	assert(ir);
	assert(ir->instr == MCC_IR_INSTR_ASSIGN);

	struct mcc_ir_row *first = first_line_of_function(ir);

	if (!assignment_is_first_occurence(first, ir)) {
		return 0;
	}
	if (ir->type->type != MCC_IR_ROW_TYPELESS)
		return DWORD_SIZE;
	return 0;
}

static struct mcc_ir_row *first_line_of_function(struct mcc_ir_row *ir)
{
	assert(ir);
	do {
		if (ir->instr == MCC_IR_INSTR_FUNC_LABEL)
			return ir;
		ir = ir->prev_row;

	} while (ir);
	return NULL;
}

static int get_row_size(struct mcc_ir_row *ir)
{
	assert(ir);

	if (ir->type->type != MCC_IR_ROW_TYPELESS)
		return DWORD_SIZE;
	return 0;
}

static int get_stack_frame_size(struct mcc_ir_row *ir)
{
	assert(ir);

	switch (ir->instr) {
	// Assignment of variables to immediate value or temporary:
	case MCC_IR_INSTR_ASSIGN:
		return get_var_size(ir);

	// Assignment of temporary: Int or Float
	case MCC_IR_INSTR_PLUS:
	case MCC_IR_INSTR_DIVIDE:
	case MCC_IR_INSTR_MINUS:
	case MCC_IR_INSTR_MULTIPLY:
	case MCC_IR_INSTR_NEGATIV:
		return get_row_size(ir);

	// Assignment of temporary: Bool
	case MCC_IR_INSTR_AND:
	case MCC_IR_INSTR_OR:
	case MCC_IR_INSTR_EQUALS:
	case MCC_IR_INSTR_NOTEQUALS:
	case MCC_IR_INSTR_GREATER:
	case MCC_IR_INSTR_GREATEREQ:
	case MCC_IR_INSTR_NOT:
	case MCC_IR_INSTR_SMALLER:
	case MCC_IR_INSTR_SMALLEREQ:
		return DWORD_SIZE;

	case MCC_IR_INSTR_CALL:
		return get_row_size(ir);

	// Size of entire array
	case MCC_IR_INSTR_ARRAY:
		return get_row_size(ir) * ir->type->array_size;

	// Labels: Size 0
	case MCC_IR_INSTR_LABEL:
	case MCC_IR_INSTR_JUMPFALSE:
	case MCC_IR_INSTR_JUMP:
	case MCC_IR_INSTR_FUNC_LABEL:
	// Rest: Also 0
	case MCC_IR_INSTR_UNKNOWN:
	case MCC_IR_INSTR_POP:
	case MCC_IR_INSTR_RETURN:
	case MCC_IR_INSTR_PUSH:
	default:
		return 0;
	}
}

static struct mcc_annotated_ir *add_stack_sizes(struct mcc_ir_row *ir)
{
	assert(ir);
	assert(ir->instr == MCC_IR_INSTR_FUNC_LABEL);

	struct mcc_annotated_ir *head = mcc_new_annotated_ir(ir, 0);
	struct mcc_annotated_ir *first = head;
	struct mcc_annotated_ir *new;
	ir = ir->next_row;

	while (ir) {
		int size = get_stack_frame_size(ir);
		new = mcc_new_annotated_ir(ir, size);
		if (!new) {
			mcc_delete_annotated_ir(first);
			return NULL;
		}
		// If size == 0, we basically copy the previous line's stack_position to correctly reference later
		// variables
		new->prev = head;
		head->next = new;
		head = new;

		ir = ir->next_row;
	}
	return first;
}

static int get_frame_size_of_function(struct mcc_annotated_ir *head)
{
	assert(head);
	assert(head->row->instr == MCC_IR_INSTR_FUNC_LABEL);

	int frame_size = 0;
	head = head->next;
	while (head && head->row->instr != MCC_IR_INSTR_FUNC_LABEL) {
		frame_size = frame_size + head->stack_size;
		head = head->next;
	}
	return frame_size;
}

int mcc_get_array_base_stack_loc(struct mcc_annotated_ir *an_ir, struct mcc_ir_arg *array_base)
{
	assert(array_base);
	assert(an_ir);
	assert(an_ir->row->arg1 == array_base || an_ir->row->arg2 == array_base);

	an_ir = mcc_get_function_label(an_ir);
	while (an_ir) {
		if (an_ir->row->instr == MCC_IR_INSTR_ARRAY) {
			if (strcmp(an_ir->row->arg1->ident, array_base->arr_ident) == 0) {
				return an_ir->stack_position;
			}
		}
		an_ir = an_ir->next;
	}
	return 0;
}

int mcc_get_array_element_stack_loc(struct mcc_annotated_ir *an_ir, struct mcc_ir_arg *array_element)
{
	assert(array_element);
	assert(an_ir);
	assert(an_ir->row->arg1 == array_element || an_ir->row->arg2 == array_element);

	// Array index is not int literal -> computed during runtime
	if (array_element->index->type != MCC_IR_TYPE_LIT_INT) {
		return 0;
	}

	an_ir = mcc_get_function_label(an_ir);
	while (an_ir) {
		if (an_ir->row->instr == MCC_IR_INSTR_ARRAY) {
			if (strcmp(an_ir->row->arg1->ident, array_element->arr_ident) == 0) {
				int array_pos = an_ir->stack_position;
				int element_pos =
				    array_pos + (array_element->index->lit_int) * get_row_size(an_ir->row);
				return element_pos;
			}
		}
		an_ir = an_ir->next;
	}
	return 0;
}

int lookup_var_loc(struct mcc_annotated_ir *an_ir, struct mcc_annotated_ir *var)
{
	assert(an_ir);
	assert(var);
	assert(var->row->instr == MCC_IR_INSTR_ASSIGN);

	an_ir = mcc_get_function_label(an_ir);
	an_ir = an_ir->next;

	while (an_ir && (an_ir->row->instr != MCC_IR_INSTR_FUNC_LABEL)) {
		if (an_ir->row->instr != MCC_IR_INSTR_ASSIGN) {
			an_ir = an_ir->next;
			continue;
		}
		if (strcmp(an_ir->row->arg1->ident, var->row->arg1->ident) == 0) {
			return an_ir->stack_position;
		}
		an_ir = an_ir->next;
	}
	return 0;
}

static void add_stack_positions(struct mcc_annotated_ir *head)
{
	assert(head);
	assert(head->row->instr == MCC_IR_INSTR_FUNC_LABEL);

	struct mcc_annotated_ir *func = head;
	head->stack_size = get_frame_size_of_function(head);
	head = head->next;
	int current_position = 0;
	int pop_counter = DWORD_SIZE;

	while (head) {
		// Function label
		if (head->row->instr == MCC_IR_INSTR_FUNC_LABEL) {
			head->stack_size = get_frame_size_of_function(head);
			current_position = 0;
			pop_counter = DWORD_SIZE;
			func = head;
			head = head->next;
			continue;
		}

		// Variables
		if (head->row->instr == MCC_IR_INSTR_ASSIGN) {
			if (head->row->arg1->type == MCC_IR_TYPE_ARR_ELEM) {
				head->stack_position = mcc_get_array_element_stack_loc(head, head->row->arg1);
			} else if (!assignment_is_first_occurence(func->row, head->row)) {
				head->stack_position = lookup_var_loc(func, head);
			} else {
				current_position = current_position - head->stack_size;
				head->stack_position = current_position;
			}
			head = head->next;
			continue;
		}
		// Arrays
		if (head->row->instr == MCC_IR_INSTR_ARRAY) {
			current_position = current_position - get_row_size(head->row) * head->row->type->array_size;
			head->stack_position = current_position;
			head = head->next;
			continue;
		}
		// Pop (Located on previous stack)
		if (head->row->instr == MCC_IR_INSTR_POP) {
			pop_counter += 4;
			head->stack_position = pop_counter;
			current_position = current_position - head->stack_size;
			head = head->next;
			continue;
		}

		// Rest
		current_position = current_position - head->stack_size;
		head->stack_position = current_position;
		head = head->next;
	}
}

struct mcc_annotated_ir *mcc_annotate_ir(struct mcc_ir_row *ir)
{
	assert(ir);
	assert(ir->instr == MCC_IR_INSTR_FUNC_LABEL);

	struct mcc_annotated_ir *an_head = add_stack_sizes(ir);
	if (!an_head)
		return NULL;
	add_stack_positions(an_head);
	return an_head;
}
