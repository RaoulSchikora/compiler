#include "mcc/cfg.h"

#include <assert.h>
#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "mcc/ast.h"
#include "mcc/ast_visit.h"
#include "mcc/ir_print.h"
#include "mcc/symbol_table.h"
#include "utils/unused.h"

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
	truncate_ir(head->child_left);
	truncate_ir(head->child_right);
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

static struct mcc_basic_block *get_bb_jump_target(struct mcc_ir_row *jump_row, struct mcc_basic_block_chain *bc_head)
{
	if (!bc_head)
		return NULL;
	struct mcc_basic_block *head = bc_head->head;
	if (!head)
		return NULL;
	if (jump_target_is_in_bb(jump_row, head))
		return head;
	struct mcc_basic_block *next = get_bb_jump_target(jump_row, bc_head->next);
	if (next)
		return next;
	return NULL;
}

// Set children for one basic block
static void set_children(struct mcc_basic_block_chain *bc_head, struct mcc_basic_block_chain *first)
{
	assert(bc_head);
	assert(first);
	struct mcc_basic_block *head = bc_head->head;
	struct mcc_ir_row *last_row = get_last_row(head);

	switch (last_row->instr) {
	case MCC_IR_INSTR_JUMP:
		head->child_left = NULL;
		head->child_right = get_bb_jump_target(last_row, first);
		return;
	case MCC_IR_INSTR_JUMPFALSE:
		// After the jump, the next IR line is given from the linear IR
		if (bc_head->next) {
			head->child_left = bc_head->next->head;
		} else {
			head->child_left = NULL;
		}
		head->child_right = get_bb_jump_target(last_row, first);
		return;
	default:
		if (bc_head->next) {
			head->child_right = bc_head->next->head;
		} else {
			head->child_right = NULL;
		}
		return;
	}
}

// Transform linear cfg into directed graph
static void sort_cfg(struct mcc_basic_block_chain *head, struct mcc_basic_block_chain *first)
{
	assert(first);
	if (!head)
		return;
	sort_cfg(head->next, first);
	set_children(head, first);
}

// Put all basic block leaders into their own BB. Link them to a single linear chain of BBs
static struct mcc_basic_block_chain *get_linear_bbs(struct annotated_ir *an_ir)
{
	struct mcc_basic_block *bb_first = mcc_cfg_new_basic_block(an_ir->row, NULL, NULL);
	if (!bb_first) {
		return NULL;
	}
	struct mcc_basic_block_chain *bc_head = malloc(sizeof(*bc_head));
	if (!bc_head) {
		return NULL;
	}
	struct mcc_basic_block_chain *bc_first = bc_head;
	bc_head->head = bb_first;
	bc_head->next = NULL;
	an_ir = an_ir->next;

	while (an_ir) {
		if (an_ir->is_leader) {
			struct mcc_basic_block *new = mcc_cfg_new_basic_block(an_ir->row, NULL, NULL);
			if (!new) {
				return NULL;
			}
			struct mcc_basic_block_chain *bc_next = malloc(sizeof(*bc_next));
			if (!bc_next) {
				return NULL;
			}
			bc_next->head = new;
			bc_next->next = NULL;
			bc_head->next = bc_next;
			bc_head = bc_next;
		}
		an_ir = an_ir->next;
	}

	return bc_first;
}

static void delete_blockchain(struct mcc_basic_block_chain *block)
{
	if (!block)
		return;
	delete_blockchain(block->next);
	free(block);
}

struct mcc_basic_block_chain *mcc_cfg_generate_block_chain(struct mcc_ir_row *ir)
{
	UNUSED(ir);
	struct annotated_ir *an_ir = annotate_ir(ir);
	if (!an_ir)
		return NULL;
	struct annotated_ir *an_ir_first = an_ir;

	struct mcc_basic_block_chain *linear_bbs = get_linear_bbs(an_ir_first);
	if (!linear_bbs) {
		delete_annotated_ir(an_ir_first);
		return NULL;
	}

	// Rearrange linear chain into graph
	struct mcc_basic_block_chain *root = linear_bbs;
	sort_cfg(root, root);

	// Truncate IR inside the basic blocks to end before next leader
	truncate_ir(root->head);

	// Cleanup
	delete_annotated_ir(an_ir_first);

	return root;
}

struct mcc_basic_block *mcc_cfg_generate(struct mcc_ir_row *ir)
{
	struct mcc_basic_block_chain *cfg = mcc_cfg_generate_block_chain(ir);
	return cfg->head;
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
	block->child_left = child_left;
	block->child_right = child_right;
	block->leader = leader;
	return block;
}

void mcc_delete_cfg(struct mcc_basic_block *head)
{
	if (!head)
		return;
	mcc_delete_cfg(head->child_left);
	mcc_delete_cfg(head->child_right);
	free(head);
}

void mcc_delete_cfg_and_ir(struct mcc_basic_block *head)
{
	if (!head)
		return;
	mcc_delete_cfg(head->child_left);
	mcc_delete_cfg(head->child_right);
	mcc_ir_delete_ir(head->leader);
}
