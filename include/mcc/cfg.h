#ifndef MCC_CFG_H
#define MCC_CFG_H

#include "mcc/ir.h"

//---------------------------------------------------------------------------------------- Data structure: CFG

struct mcc_basic_block {
	struct mcc_ir_row *leader;
	struct mcc_basic_block *child_left;
	struct mcc_basic_block *child_right;
	struct mcc_basic_block *parent_left;
	struct mcc_basic_block *parent_right;
};

//---------------------------------------------------------------------------------------- Functions: CFG

struct mcc_basic_block *mcc_cfg_generate(struct mcc_ir_row *ir);

void mcc_cfg_print(struct mcc_basic_block *block);

//---------------------------------------------------------------------------------------- Functions: Set up datastructs

struct mcc_basic_block *mcc_cfg_new_basic_block(struct mcc_ir_row *leader,
                                                struct mcc_basic_block *child_left,
                                                struct mcc_basic_block *child_right,
                                                struct mcc_basic_block *parent_left,
                                                struct mcc_basic_block *parent_right);

void mcc_delete_cfg(struct mcc_basic_block *head);

#endif // MCC_CFG_H

