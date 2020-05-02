#ifndef MCC_CFG_H
#define MCC_CFG_H

#include "mcc/ir.h"

//---------------------------------------------------------------------------------------- Data structure: CFG

struct mcc_basic_block {
	struct mcc_ir_row *leader;
	struct mcc_basic_block *child_left;
	struct mcc_basic_block *child_right;
};

//---------------------------------------------------------------------------------------- Functions: CFG

struct mcc_basic_block *mcc_cfg_generate(struct mcc_ir_row *ir);

//---------------------------------------------------------------------------------------- Functions: Set up datastructs

struct mcc_basic_block *mcc_cfg_new_basic_block(struct mcc_ir_row *leader,
                                                struct mcc_basic_block *child_left,
                                                struct mcc_basic_block *child_right);

// Delete only CFG datastructure and not contained IR
void mcc_delete_cfg(struct mcc_basic_block *head);

// Delete CFG and contained IR
void mcc_delete_cfg_and_ir(struct mcc_basic_block *head);

#endif // MCC_CFG_H

