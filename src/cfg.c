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

	struct annotated_ir *temp = head;
	while (head->next) {
		head = head->next;
	}
	while (head->prev) {
		temp = head->prev;
		free(head);
		head = temp;
	}
	free(head);
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
		if (head->instr == MCC_IR_INSTR_LABEL || head->instr == MCC_IR_INSTR_FUNC_LABEL ||
		    head_an->row->instr == MCC_IR_INSTR_JUMP || head_an->row->instr == MCC_IR_INSTR_JUMPFALSE ||
		    head_an->row->instr == MCC_IR_INSTR_RETURN) {
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

struct mcc_cfg *mcc_cfg_generate(struct mcc_ir_row *ir)
{
	UNUSED(ir);
	struct annotated_ir *an_ir = annotate_ir(ir);
	if (!an_ir)
		return NULL;
	mcc_ir_print_table_begin(stdout);

	while (an_ir) {
		if (an_ir->is_leader) {
			printf("IS_LEADER:\n");
		}
		mcc_ir_print_ir_row(stdout, an_ir->row);
		an_ir = an_ir->next;
	}
	mcc_ir_print_table_end(stdout);
        delete_annotated_ir(an_ir);
	return NULL;
}

void mcc_cfg_print(struct mcc_cfg *cfg)
{
	UNUSED(cfg);
}

//---------------------------------------------------------------------------------------- Functions: Set up datastructs

struct mcc_cfg *mcc_cfg_new_cfg(struct mcc_basic_block *head)
{
	assert(head);
	struct mcc_cfg *cfg = malloc(sizeof(*cfg));
	if (!cfg)
		return NULL;
	cfg->head = head;
	return cfg;
}

struct mcc_basic_block *mcc_cfg_new_basic_block(struct mcc_ir_row *leader,
                                                struct mcc_basic_block *child_left,
                                                struct mcc_basic_block *child_right,
                                                struct mcc_basic_block *parent_left,
                                                struct mcc_basic_block *parent_right)
{
	assert(leader);
	assert(child_right);
	assert(child_left);
	assert(parent_left);
	assert(parent_right);
	struct mcc_basic_block *block = malloc(sizeof(*block));
	if (!block)
		return NULL;
	block->child_left = child_left;
	block->child_right = child_right;
	block->parent_left = parent_left;
	block->parent_right = parent_right;
	return block;
}
