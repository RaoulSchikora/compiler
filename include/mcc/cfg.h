// Control Flow Graph (CFG)
//
// Here we define the CFG data structure.
// It corresponds to a directed cyclic binary graph. Obtaining a CFG means obtaining the basic block that is at the root
// of the graph. 
// In order to make traversing easier, each node also contains a pointer to the next basic block, inferred from the
// order they appear in the IR. This essentially enables traversing the CFG as if it was a linked list.

#ifndef MCC_CFG_H
#define MCC_CFG_H

#include "mcc/ir.h"

//---------------------------------------------------------------------------------------- Data structure: CFG

struct mcc_basic_block {
	struct mcc_ir_row *leader;
	struct mcc_basic_block *child_left;
	struct mcc_basic_block *child_right;
	struct mcc_basic_block *next;
};

//---------------------------------------------------------------------------------------- Functions: CFG

// Gives the cfg as directed tree
struct mcc_basic_block *mcc_cfg_generate(struct mcc_ir_row *ir);

// Restrict the CFG to just one function. Makes use of the linked list wrapper
struct mcc_basic_block *mcc_cfg_limit_to_function(char *function_identifier, struct mcc_basic_block *cfg);

//---------------------------------------------------------------------------------------- Functions: Set up datastructs

struct mcc_basic_block *mcc_cfg_new_basic_block(struct mcc_ir_row *leader,
                                                struct mcc_basic_block *child_left,
                                                struct mcc_basic_block *child_right);

// Delete CFG and contained IR
void mcc_delete_cfg_and_ir(struct mcc_basic_block *head);

#endif // MCC_CFG_H
