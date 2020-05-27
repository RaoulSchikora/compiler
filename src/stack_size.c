#include "mcc/stack_size.h"

#include "mcc/asm.h"
#include "mcc/ir.h"
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

struct annotated_ir *new_annotated_ir(struct mcc_ir_row *row, int stack_size)
{
	assert(row);
	struct annotated_ir *ir = malloc(sizeof(*ir));
	if (!ir)
		return NULL;
	ir->stack_size = stack_size;
	ir->row = row;
	ir->next = NULL;
	ir->prev = NULL;
	return ir;
}

void delete_annotated_ir(struct annotated_ir *head)
{
	if (!head)
		return;

	delete_annotated_ir(head->next);
	free(head);
}

// --------------------------------------------------------------------------------------- Forward declarations

static struct mcc_ir_row *first_line_of_function(struct mcc_ir_row *ir);

// --------------------------------------------------------------------------------------- Calc stack size and position

// DONE
static bool assignment_is_first_occurence(struct mcc_ir_row *first, struct mcc_ir_row *ir)
{
	assert(first);
	assert(ir);
	assert(ir->instr == MCC_IR_INSTR_ASSIGN);

	// Arrays are allocated when they're declared
	if (ir->arg1->type == MCC_IR_TYPE_ARR_ELEM) {
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

// This function returns the size of an argument in the IR line.
// Since semantic consistency is guaranteed, we can infer the required size of an IR line
// by knowing the size of one of its arguments
static size_t argument_size(struct mcc_ir_arg *arg)
{
	assert(arg);

	switch (arg->type) {
	// Not on the stack
	case MCC_IR_TYPE_LIT_STRING:
		return 0;
	case MCC_IR_TYPE_LIT_INT:
		return 4;
	// TODO
	case MCC_IR_TYPE_LIT_FLOAT:
		return 0;
	// TODO: Bool = 4 bytes?
	case MCC_IR_TYPE_LIT_BOOL:
		return 4;
	// TODO: "a = b" -> Find b and get its size
	case MCC_IR_TYPE_IDENTIFIER:
		return 0;
	// TODO: Find out size of the array element
	case MCC_IR_TYPE_ARR_ELEM:
		return 0;
	// TODO:"a = t1" -> Find out size of temporary
	case MCC_IR_TYPE_ROW:
		return 0;
	default:
		return 0;
	}
}

static size_t get_var_size(struct mcc_ir_row *ir)
{
	assert(ir);
	assert(ir->instr == MCC_IR_INSTR_ASSIGN);

	struct mcc_ir_row *first = first_line_of_function(ir);

	if (!assignment_is_first_occurence(first, ir)) {
		return 0;
	}
	// "a = x" -> find size of x
	return argument_size(ir->arg2);
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

// Done
static size_t get_temporary_size(struct mcc_ir_row *ir)
{
	assert(ir);
	assert(ir->arg1);
	return argument_size(ir->arg1);
}

// TODO
static size_t get_array_size(struct mcc_ir_row *ir)
{
	assert(ir);
	assert(ir->instr == MCC_IR_INSTR_ARRAY_BOOL || MCC_ASM_DECLARATION_TYPE_ARRAY_FLOAT ||
	       MCC_ASM_DECLARATION_TYPE_ARRAY_INT || MCC_ASM_DECLARATION_TYPE_ARRAY_STRING);
	assert(ir->arg2->type == MCC_IR_TYPE_LIT_INT);

	switch (ir->instr) {
	case MCC_IR_INSTR_ARRAY_BOOL:
		return 4 * (ir->arg2->lit_int);
	// Update 4 to the size of floats TODO
	case MCC_IR_INSTR_ARRAY_FLOAT:
		return 4 * (ir->arg2->lit_int);
	case MCC_IR_INSTR_ARRAY_INT:
		return 4 * (ir->arg2->lit_int);
	// Not on the stack? TODO
	case MCC_IR_INSTR_ARRAY_STRING:
		break;
	// Unreached, as per assertion
	default:
		return 0;
	}
}

static size_t get_stack_frame_size(struct mcc_ir_row *ir)
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

	// Assignment of temporary: Bool (TODO: 4 bytes for bool?)
	case MCC_IR_INSTR_AND:
	case MCC_IR_INSTR_OR:
	case MCC_IR_INSTR_EQUALS:
	case MCC_IR_INSTR_NOTEQUALS:
	case MCC_IR_INSTR_GREATER:
	case MCC_IR_INSTR_GREATEREQ:
	case MCC_IR_INSTR_NOT:
	case MCC_IR_INSTR_SMALLER:
	case MCC_IR_INSTR_SMALLEREQ:
		return 4;

	// TODO
	case MCC_IR_INSTR_POP:
	case MCC_IR_INSTR_PUSH:
		break;

	// TODO
	case MCC_IR_INSTR_CALL:
	case MCC_IR_INSTR_RETURN:
		return 0;

	// Arrays are located on the stack, and have special declaration IR instructions
	case MCC_IR_INSTR_ARRAY_BOOL:
	case MCC_IR_INSTR_ARRAY_INT:
	case MCC_IR_INSTR_ARRAY_FLOAT:
	case MCC_IR_INSTR_ARRAY_STRING:
		return get_array_size(ir);

	case MCC_IR_INSTR_UNKNOWN:
		return 0;
	}
	return 0;
}

static struct annotated_ir *add_stack_sizes(struct mcc_ir_row *ir)
{
	assert(ir);
	assert(ir->instr == MCC_IR_INSTR_FUNC_LABEL);

	struct annotated_ir *head = new_annotated_ir(ir, 0);
	struct annotated_ir *first = head;
	struct annotated_ir *new;

	while (ir) {
		size_t size = get_stack_frame_size(ir);
		new = new_annotated_ir(ir, size);
		if (!new) {
			delete_annotated_ir(first);
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

// TODO
static bool add_stack_positions(struct annotated_ir *an_head)
{
	assert(an_head);
	return true;
}

struct annotated_ir *annotate_ir(struct mcc_ir_row *ir)
{
	assert(ir);
	// First IR line should be function label
	assert(ir->instr == MCC_IR_INSTR_FUNC_LABEL);

	struct annotated_ir *an_head = add_stack_sizes(ir);
	if (!an_head)
		return NULL;
	bool successfull = add_stack_positions(an_head);
	if (!successfull) {
		delete_annotated_ir(an_head);
		return NULL;
	}
	return an_head;
}
