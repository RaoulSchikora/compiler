// Stack Size
//
// Annotate the IR with the needed stack sizes per Line of IR
// Also provides a suggested position on the stack for each variable that is introduced

#ifndef MCC_STACK_SIZE_H
#define MCC_STACK_SIZE_H

#include "mcc/ir.h"

// TODO
#define STACK_SIZE_BOOL 4
#define STACK_SIZE_FLOAT 4
#define STACK_SIZE_STRING 0
#define STACK_SIZE_INT 4

// TODO: Name prefixing for public functions

// --------------------------------------------------------------------------------------- Data structure

// Annotate IR to determine stack size of each IR line
struct mcc_annotated_ir {
	int stack_size;
	int stack_position;
	struct mcc_annotated_ir *next;
	struct mcc_annotated_ir *prev;
	struct mcc_ir_row *row;
};

struct mcc_annotated_ir *mcc_new_annotated_ir(struct mcc_ir_row *row, int stack_size);

void mcc_delete_annotated_ir(struct mcc_annotated_ir *head);

struct mcc_annotated_ir *mcc_annotate_ir(struct mcc_ir_row *ir);

#endif
