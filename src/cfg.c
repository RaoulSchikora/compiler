#include "mcc/cfg.h"

#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

//---------------------------------------------------------------------------------------- Annotate IR with leaders

// Annotate IR to determine leaders
struct annotated_ir {
	bool is_leader;
	struct annotated_ir *next;
	struct annotated_ir *prev;
	struct mcc_ir_row *row;
};

static struct annotated_ir *new_annotated_ir(struct mcc_ir_row *row, bool is_leader)
{
	assert(row);
	struct annotated_ir *ir = malloc(sizeof(*ir));
	if (!ir)
		return NULL;
	ir->is_leader = is_leader;
	ir->row = row;
	ir->next = NULL;
	ir->prev = NULL;
	return ir;
}

static void delete_annotated_ir(struct annotated_ir *head)
{
	if (!head)
		return;

	delete_annotated_ir(head->next);
	free(head);
}

static bool is_leader(enum mcc_ir_instruction current, enum mcc_ir_instruction previous)
{
	switch (current) {
	case MCC_IR_INSTR_LABEL:
	case MCC_IR_INSTR_FUNC_LABEL:
		return true;
	default:
		break;
	}
	switch (previous) {
	case MCC_IR_INSTR_JUMP:
	case MCC_IR_INSTR_JUMPFALSE:
	case MCC_IR_INSTR_RETURN:
		return true;
	default:
		break;
	}
	return false;
}

static struct annotated_ir *annotate_ir(struct mcc_ir_row *head)
{
	assert(head);
	struct annotated_ir *head_an = new_annotated_ir(head, true);
	if (!head_an)
		return NULL;
	struct annotated_ir *first_an = head_an;
	struct annotated_ir *temp;
	head = head->next_row;

	while (head) {
		if (is_leader(head->instr, head_an->row->instr)) {
			temp = new_annotated_ir(head, true);
		} else {
			temp = new_annotated_ir(head, false);
		}
		if (!temp) {
			delete_annotated_ir(first_an);
			return NULL;
		}
		head_an->next = temp;
		temp->prev = head_an;
		head_an = temp;
		head = head->next_row;
		continue;
	}
	return first_an;
}

//---------------------------------------------------------------------------------------- Functions: CFG

// Truncate IR before the next leader
static void truncate_ir_in_BB(struct mcc_ir_row *head)
{
	assert(head);
	struct mcc_ir_row *previous = head;
	head = head->next_row;
	while (head) {
		if (is_leader(head->instr, previous->instr)) {
			previous->next_row = NULL;
			head->prev_row = NULL;
		}
		previous = head;
		head = head->next_row;
	}
}

// Truncate IR in entire CFG
static void truncate_ir(struct mcc_basic_block *head)
{
	if (!head)
		return;
	truncate_ir(head->next);
	truncate_ir_in_BB(head->leader);
}

static struct mcc_ir_row *get_last_row(struct mcc_basic_block *head)
{
	assert(head);
	struct mcc_ir_row *ir_head = head->leader;
	struct mcc_ir_row *previous = ir_head;
	ir_head = ir_head->next_row;
	while (ir_head) {
		if (is_leader(ir_head->instr, previous->instr)) {
			break;
		}
		previous = ir_head;
		ir_head = ir_head->next_row;
	}
	return previous;
}

static bool row_is_target_label(struct mcc_ir_row *row, unsigned label)
{
	assert(row);
	if (row->instr == MCC_IR_INSTR_LABEL) {
		if (row->arg1->label == label) {
			return true;
		}
	}
	return false;
}

static bool jump_target_is_in_bb(struct mcc_ir_row *jump_row, struct mcc_basic_block *block)
{
	struct mcc_ir_row *head = block->leader;
	int target_label;
	if (jump_row->instr == MCC_IR_INSTR_JUMP) {
		target_label = jump_row->arg1->label;
	} else if (jump_row->instr == MCC_IR_INSTR_JUMPFALSE) {
		target_label = jump_row->arg2->label;
	} else {
		return false;
	}

	if (row_is_target_label(head, target_label))
		return true;

	return false;
}

static struct mcc_basic_block *get_bb_jump_target(struct mcc_ir_row *jump_row, struct mcc_basic_block *head)
{
	if (!head)
		return NULL;
	if (jump_target_is_in_bb(jump_row, head))
		return head;
	struct mcc_basic_block *next = get_bb_jump_target(jump_row, head->next);
	if (next)
		return next;
	return NULL;
}

// Set children for one basic block
static void set_children(struct mcc_basic_block *head, struct mcc_basic_block *first)
{
	assert(head);
	assert(first);
	struct mcc_ir_row *last_row = get_last_row(head);

	switch (last_row->instr) {
	case MCC_IR_INSTR_JUMP:
		head->child_left = NULL;
		head->child_right = get_bb_jump_target(last_row, first);
		return;
	case MCC_IR_INSTR_JUMPFALSE:
		// After the jump, the next IR line is given from the linear IR
		if (head->next) {
			head->child_left = head->next;
		} else {
			head->child_left = NULL;
		}
		head->child_right = get_bb_jump_target(last_row, first);
		return;
	case MCC_IR_INSTR_RETURN:
		head->child_left = NULL;
		head->child_right = NULL;
		return;
	default:
		if (head->next) {
			head->child_right = head->next;
		} else {
			head->child_right = NULL;
		}
		return;
	}
}

// Transform linear cfg into directed graph
static void sort_cfg(struct mcc_basic_block *head, struct mcc_basic_block *first)
{
	assert(first);
	if (!head)
		return;
	sort_cfg(head->next, first);
	set_children(head, first);
}

static void mcc_delete_cfg(struct mcc_basic_block *head)
{
	if (!head)
		return;
	mcc_delete_cfg(head->next);
	free(head);
}

// Put all basic block leaders into their own BB. Link them to a single linear chain of BBs with the "next" field
static struct mcc_basic_block *get_basic_blocks(struct annotated_ir *an_ir)
{
	struct mcc_basic_block *bb_first = mcc_cfg_new_basic_block(an_ir->row, NULL, NULL);
	if (!bb_first) {
		return NULL;
	}

	struct mcc_basic_block *bb_head = bb_first;
	an_ir = an_ir->next;

	while (an_ir) {
		if (an_ir->is_leader) {
			struct mcc_basic_block *new = mcc_cfg_new_basic_block(an_ir->row, NULL, NULL);
			if (!new) {
				mcc_delete_cfg(bb_first);
				return NULL;
			}
			bb_head->next = new;
			bb_head = new;
		}
		an_ir = an_ir->next;
	}

	return bb_first;
}

struct mcc_basic_block *mcc_cfg_generate(struct mcc_ir_row *ir)
{
	struct annotated_ir *an_ir = annotate_ir(ir);
	if (!an_ir)
		return NULL;
	struct annotated_ir *an_ir_first = an_ir;

	struct mcc_basic_block *basic_blocks = get_basic_blocks(an_ir_first);
	if (!basic_blocks) {
		delete_annotated_ir(an_ir_first);
		return NULL;
	}

	// Rearrange linear chain into graph, by setting the child nodes
	struct mcc_basic_block *root = basic_blocks;
	sort_cfg(root, root);

	// Truncate IR inside the basic blocks to end before next leader
	truncate_ir(root);

	// Cleanup
	delete_annotated_ir(an_ir_first);

	return root;
}

static void remove_all_bbs_above(struct mcc_basic_block *first, struct mcc_basic_block *head)
{
	assert(first);
	assert(head);
	if (first == head) {
		return;
	}
	struct mcc_basic_block *previous = first;
	while (previous) {
		if (previous->next == head) {
			previous->next = NULL;
			mcc_delete_cfg_and_ir(first);
			return;
		}
		previous = previous->next;
	}
}

static void remove_cfg_after_next_function_label(struct mcc_basic_block *head)
{
	assert(head);
	struct mcc_basic_block *previous = head;
	head = previous->next;
	while (head) {
		if (head->leader->arg1) {
			if (head->leader->arg1->type == MCC_IR_TYPE_FUNC_LABEL) {
				previous->next = NULL;
				mcc_delete_cfg_and_ir(head);
				break;
			}
		}
		previous = previous->next;
		head = head->next;
	}
}

struct mcc_basic_block *mcc_cfg_limit_to_function(char *function_identifier, struct mcc_basic_block *cfg_first)
{
	assert(function_identifier);
	assert(cfg_first);
	struct mcc_basic_block *first = cfg_first;
	while (cfg_first) {
		if (cfg_first->leader->arg1) {
			if (cfg_first->leader->arg1->type == MCC_IR_TYPE_FUNC_LABEL) {
				// Right function
				if (strcmp(cfg_first->leader->arg1->func_label, function_identifier) == 0) {
					remove_all_bbs_above(first, cfg_first);
					remove_cfg_after_next_function_label(cfg_first);
					break;
				}
			}
		}
		cfg_first = cfg_first->next;
	}
	if (!cfg_first) {
		mcc_delete_cfg_and_ir(first);
		return NULL;
	}
	return cfg_first;
}

//---------------------------------------------------------------------------------------- Functions: Set up
// datastructs

struct mcc_basic_block *mcc_cfg_new_basic_block(struct mcc_ir_row *leader,
                                                struct mcc_basic_block *child_left,
                                                struct mcc_basic_block *child_right)
{
	assert(leader);
	struct mcc_basic_block *block = malloc(sizeof(*block));
	if (!block)
		return NULL;
	block->next = NULL;
	block->child_left = child_left;
	block->child_right = child_right;
	block->leader = leader;
	return block;
}

void mcc_delete_cfg_and_ir(struct mcc_basic_block *head)
{
	if (!head)
		return;
	mcc_delete_cfg_and_ir(head->next);
	mcc_ir_delete_ir(head->leader);
	free(head);
}
