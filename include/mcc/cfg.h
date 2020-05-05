#ifndef MCC_CFG_H
#define MCC_CFG_H

#include "mcc/ir.h"

//---------------------------------------------------------------------------------------- Data structure: CFG

struct mcc_basic_block {
	struct mcc_ir_row *leader;
	struct mcc_basic_block *child_left;
	struct mcc_basic_block *child_right;
};

// To simplify visiting all blocks only once
struct mcc_basic_block_chain {
	struct mcc_basic_block *head;
	struct mcc_basic_block_chain *next;
};

//---------------------------------------------------------------------------------------- Functions: CFG

// Gives the cfg as directed tree
struct mcc_basic_block *mcc_cfg_generate(struct mcc_ir_row *ir);

// Gives the same cfg wrapped in a linked list, where each element is a basic block
struct mcc_basic_block_chain *mcc_cfg_generate_block_chain(struct mcc_ir_row *ir);

//---------------------------------------------------------------------------------------- Functions: Set up datastructs

struct mcc_basic_block *mcc_cfg_new_basic_block(struct mcc_ir_row *leader,
                                                struct mcc_basic_block *child_left,
                                                struct mcc_basic_block *child_right);

// Delete CFG and contained IR
void mcc_delete_cfg_and_ir(struct mcc_basic_block *head);

// Delete all basic blocks and contained IR
void mcc_delete_blockchain_and_ir(struct mcc_basic_block_chain *block);

#endif // MCC_CFG_H

