// Stack Size
//
// Annotate the IR with the needed stack sizes per Line of IR
// Also provides a suggested position on the stack for each variable that is introduced

#ifndef MCC_STACK_SIZE_H
#define MCC_STACK_SIZE_H

#include "mcc/ir.h"

// TODO: Name prefixing for public functions

// --------------------------------------------------------------------------------------- Data structure

// Annotate IR to determine stack size of each IR line
struct annotated_ir {
	int stack_size;
	int stack_position;
	struct annotated_ir *next;
	struct annotated_ir *prev;
	struct mcc_ir_row *row;
};

struct annotated_ir *new_annotated_ir(struct mcc_ir_row *row, int stack_size);

void delete_annotated_ir(struct annotated_ir *head);

struct annotated_ir *annotate_ir(struct mcc_ir_row *ir);

#endif
