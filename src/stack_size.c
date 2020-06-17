#include "mcc/stack_size.h"

#include "mcc/asm.h"
#include "mcc/ir.h"
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

struct mcc_annotated_ir *mcc_new_annotated_ir(struct mcc_ir_row *row, int stack_size)
{
	assert(row);
	struct mcc_annotated_ir *ir = malloc(sizeof(*ir));
	if (!ir)
		return NULL;
	// Hold stack size of current IR line. If line is func label, holds stack size of that function
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

static int get_array_type_size(struct mcc_ir_row *ir);
static struct mcc_ir_row *first_line_of_function(struct mcc_ir_row *ir);
static int get_temporary_size(struct mcc_ir_row *ir);
static struct mcc_ir_row *last_line_of_function(struct mcc_ir_row *ir);
static int argument_size(struct mcc_ir_arg *arg, struct mcc_ir_row *ir);

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

// Finds first occurence of variable (not array element) in a function
static struct mcc_ir_row *find_first_occurence(char *identifier, struct mcc_ir_row *ir)
{
	struct mcc_ir_row *first = first_line_of_function(ir);
	ir = first;
	struct mcc_ir_row *last = last_line_of_function(ir);
	while (ir && (ir != last)) {
		if (ir->instr == MCC_IR_INSTR_ASSIGN || ir->instr == MCC_IR_INSTR_ARRAY) {
			if (strcmp(identifier, ir->arg1->ident) == 0) {
				return ir;
			}
		}
		ir = ir->next_row;
	}

	return NULL;
}

// This function returns the size of an argument in the IR line.
// Since semantic consistency is guaranteed, we can infer the required size of an IR line
// by knowing the size of one of its arguments
static int argument_size(struct mcc_ir_arg *arg, struct mcc_ir_row *ir)
{
	assert(arg);
	struct mcc_ir_row *ref = NULL;

	switch (arg->type) {
	// Not on the stack
	case MCC_IR_TYPE_LIT_STRING:
		return STACK_SIZE_STRING;
	case MCC_IR_TYPE_LIT_INT:
		return STACK_SIZE_INT;
	case MCC_IR_TYPE_LIT_FLOAT:
		return STACK_SIZE_FLOAT;
	case MCC_IR_TYPE_LIT_BOOL:
		return STACK_SIZE_BOOL;
	case MCC_IR_TYPE_IDENTIFIER:
		ref = find_first_occurence(arg->ident, ir);
		if (!ref || ref->instr != MCC_IR_INSTR_ASSIGN)
			return 0;
		return argument_size(ref->arg2, ir);
	case MCC_IR_TYPE_ARR_ELEM:
		ref = find_first_occurence(arg->arr_ident, ir);
		if (!ref || ref->instr != MCC_IR_INSTR_ARRAY)
			return 0;
		return get_array_type_size(ref);
	case MCC_IR_TYPE_ROW:
		return get_temporary_size(arg->row);
	default:
		return 0;
	}
}

static int get_var_size(struct mcc_ir_row *ir)
{
	assert(ir);
	assert(ir->instr == MCC_IR_INSTR_ASSIGN);

	struct mcc_ir_row *first = first_line_of_function(ir);

	if (!assignment_is_first_occurence(first, ir)) {
		return 0;
	}
	// "a = x" -> find size of x
	return argument_size(ir->arg2, ir);
}

static struct mcc_ir_row *first_line_of_function(struct mcc_ir_row *ir)
{
	assert(ir);
	if (ir->instr == MCC_IR_INSTR_FUNC_LABEL)
		return ir;

	struct mcc_ir_row *prev = ir->prev_row;
	while (prev) {
		if (prev->instr == MCC_IR_INSTR_FUNC_LABEL) {
			return prev;
		}
		prev = prev->prev_row;
	}
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

static int get_temporary_size(struct mcc_ir_row *ir)
{
	assert(ir);

	switch (ir->type->type) {
	case MCC_IR_ROW_TYPELESS:
		return 0;
	case MCC_IR_ROW_BOOL:
		return STACK_SIZE_BOOL;
	case MCC_IR_ROW_INT:
		return STACK_SIZE_BOOL;
	case MCC_IR_ROW_STRING:
		return STACK_SIZE_STRING;
	case MCC_IR_ROW_FLOAT:
		return STACK_SIZE_FLOAT;
	default:
		return 0;
	}
}

static int get_array_type_size(struct mcc_ir_row *ir)
{
	assert(ir);
	assert(ir->instr == MCC_IR_INSTR_ARRAY);

	switch (ir->type->type) {
	case MCC_IR_ROW_BOOL:
		return STACK_SIZE_BOOL * ir->type->array_size;
	case MCC_IR_ROW_FLOAT:
		return STACK_SIZE_FLOAT * ir->type->array_size;
	case MCC_IR_ROW_INT:
		return STACK_SIZE_INT * ir->type->array_size;
	case MCC_IR_ROW_STRING:
		return STACK_SIZE_STRING;
	// Unreached, as per assertion
	default:
		return 0;
	}
}

static int get_array_size(struct mcc_ir_row *ir)
{
	assert(ir);
	assert(ir->instr == MCC_IR_INSTR_ARRAY);
	assert(ir->arg2->type == MCC_IR_TYPE_LIT_INT);

	switch (ir->type->type) {
	case MCC_IR_ROW_BOOL:
		return STACK_SIZE_BOOL * (ir->arg2->lit_int);
	case MCC_IR_ROW_FLOAT:
		return STACK_SIZE_FLOAT * (ir->arg2->lit_int);
	case MCC_IR_ROW_INT:
		return STACK_SIZE_INT * (ir->arg2->lit_int);
	case MCC_IR_ROW_STRING:
		return STACK_SIZE_STRING;
	default:
		return 0;
	}
}

static int get_stack_frame_size(struct mcc_ir_row *ir)
{
	assert(ir);

	switch (ir->instr) {

	// Labels: Size 0
	case MCC_IR_INSTR_LABEL:
	case MCC_IR_INSTR_JUMPFALSE:
	case MCC_IR_INSTR_JUMP:
	case MCC_IR_INSTR_FUNC_LABEL:
		return 0;

	// Assignment of variables to immediate value or temporary:
	case MCC_IR_INSTR_ASSIGN:
		return get_var_size(ir);

	// Assignment of temporary: Int or Float
	case MCC_IR_INSTR_PLUS:
	case MCC_IR_INSTR_DIVIDE:
	case MCC_IR_INSTR_MINUS:
	case MCC_IR_INSTR_MULTIPLY:
	case MCC_IR_INSTR_NEGATIV:
		return get_temporary_size(ir);

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
		return STACK_SIZE_BOOL;

	// TODO
	case MCC_IR_INSTR_POP:
	case MCC_IR_INSTR_PUSH:
		break;

	// TODO
	case MCC_IR_INSTR_CALL:
	case MCC_IR_INSTR_RETURN:
		return 0;

	// Arrays are located on the stack, and have special declaration IR instructions
	case MCC_IR_INSTR_ARRAY:
		return get_array_size(ir);

	case MCC_IR_INSTR_UNKNOWN:
		return 0;
	}
	return 0;
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
	struct mcc_ir_row *last = last_line_of_function(head->row);
	while (head->row != last) {
		frame_size = frame_size + head->stack_size;
		head = head->next;
	}
	return frame_size;
}

static int get_array_base_size(struct mcc_ir_row *ir)
{
	assert(ir);
	switch (ir->type->type) {
	case MCC_IR_ROW_BOOL:
		return STACK_SIZE_BOOL;
	case MCC_IR_ROW_FLOAT:
		return STACK_SIZE_FLOAT;
	case MCC_IR_ROW_INT:
		return STACK_SIZE_INT;
	case MCC_IR_ROW_STRING:
		return STACK_SIZE_STRING;
	default:
		return 0;
	}
}

// TODO mcc prefix
int get_array_element_location(struct mcc_annotated_ir *an_ir)
{
	assert(an_ir);
	struct mcc_annotated_ir *head = an_ir;
	struct mcc_annotated_ir *first = head;
	while (first && first->prev) {
		if (first->prev->row->instr == MCC_IR_INSTR_FUNC_LABEL) {
			first = first->prev;
			break;
		}
		first = first->prev;
	}
	head = first;
	while (head) {
		if (head->row->instr == MCC_IR_INSTR_ARRAY) {
			if (strcmp(head->row->arg1->ident, an_ir->row->arg1->arr_ident) == 0) {
				int array_pos = head->stack_position;
				int element_pos =
				    array_pos - an_ir->row->arg1->index->lit_int * get_array_base_size(head->row);
				return element_pos;
			}
		}
		head = head->next;
	}
	return 0;
}

int lookup_var_loc(struct mcc_annotated_ir *func, struct mcc_annotated_ir *head)
{
	assert(func);
	assert(func->row->instr == MCC_IR_INSTR_FUNC_LABEL);
	assert(head);
	assert(head->row->instr == MCC_IR_INSTR_ASSIGN);
	struct mcc_ir_row *last = last_line_of_function(func->row);
	struct mcc_annotated_ir *var = head;
	head = func;
	while (head && (head->row != last)) {
		if (head->row->instr != MCC_IR_INSTR_ASSIGN) {
			head = head->next;
			continue;
		}
		if (strcmp(head->row->arg1->ident, var->row->arg1->ident) == 0) {
			return head->stack_position;
		}
		head = head->next;
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

	while (head) {
		// Function label
		if (head->row->instr == MCC_IR_INSTR_FUNC_LABEL) {
			head->stack_size = get_frame_size_of_function(head);
			current_position = 0;
			func = head;
			head = head->next;
			continue;
		}
		// Array elements
		if (head->row->instr == MCC_IR_INSTR_ASSIGN) {
			if (head->row->arg1->type == MCC_IR_TYPE_ARR_ELEM) {
				head->stack_position = get_array_element_location(head);
				head = head->next;
				continue;
			}
		}
		// Arrays
		if (head->row->instr == MCC_IR_INSTR_ARRAY){
			current_position = current_position - get_array_base_size(head->row);
			head->stack_position = current_position;
			current_position = current_position - head->stack_size + get_array_base_size(head->row);
			head = head->next;
			continue;
		}
		// Variables
		if (head->row->instr == MCC_IR_INSTR_ASSIGN) {
			if (!assignment_is_first_occurence(func->row, head->row)) {
				head->stack_position = lookup_var_loc(func, head);
			} else {
				current_position = current_position - head->stack_size;
				head->stack_position = current_position;
			}
			head = head->next;
			continue;
		}
		// Rest
		current_position = current_position - head->stack_size;
		head->stack_position = current_position;
		head = head->next;
	}
}

static void rename_strings(struct mcc_annotated_ir *head)
{
	assert(head);
	assert(head->row->instr == MCC_IR_INSTR_FUNC_LABEL);
	struct mcc_annotated_ir *first = head;
	int rename_counter = 0;
	while (head) {
		if (head->row->instr != MCC_IR_INSTR_ASSIGN) {
			head = head->next;
			continue;
		}
		if (head->row->type->type != MCC_IR_ROW_STRING) {
			head = head->next;
			continue;
		}
		if (assignment_is_first_occurence(first->row, head->row)) {
			head = head->next;
			continue;
		}
		// rename_identifier(head->row->arg1->ident->identifier_name, rename_counter);
		rename_counter = rename_counter + 1;
		head = head->next;
	}
}

struct mcc_annotated_ir *mcc_annotate_ir(struct mcc_ir_row *ir)
{
	assert(ir);
	// First IR line should be function label
	assert(ir->instr == MCC_IR_INSTR_FUNC_LABEL);

	struct mcc_annotated_ir *an_head = add_stack_sizes(ir);
	if (!an_head)
		return NULL;
	add_stack_positions(an_head);
	rename_strings(an_head);
	return an_head;
}
