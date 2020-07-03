// Stack Size
//
// This module annotates the IR with additional info in order to ease the generation of x86 assembly code.
// It provides a wrapper for the IR, and returns a doubly-linked list.
// Each entry contains the size on the hardware stack that the line needs, and if it's possible to determine, the offset
// to the base pointer on the stack.
// Also string definitions are renamed, so that each string can be declared in the data section of the asm code.

#ifndef MCC_STACK_SIZE_H
#define MCC_STACK_SIZE_H

#include "mcc/ir.h"

#define DWORD_SIZE 4

// --------------------------------------------------------------------------------------- Data structure

struct mcc_annotated_ir {
	// Hold stack size (number of bytes needed on the stack) of current IR line.
	// If line is func label, holds stack size of that function
	int stack_size;
	int stack_position;
	struct mcc_annotated_ir *next;
	struct mcc_annotated_ir *prev;
	struct mcc_ir_row *row;
};

struct mcc_annotated_ir *mcc_new_annotated_ir(struct mcc_ir_row *row, int stack_size);

void mcc_delete_annotated_ir(struct mcc_annotated_ir *head);

// Annotate IR to determine stack size of each IR line
struct mcc_annotated_ir *mcc_annotate_ir(struct mcc_ir_row *ir);

int mcc_get_array_element_stack_loc(struct mcc_annotated_ir *an_ir, struct mcc_ir_arg *arg);

int mcc_get_array_base_stack_loc(struct mcc_annotated_ir *an_ir, struct mcc_ir_arg *arg);

int mcc_get_array_base_size(struct mcc_annotated_ir *an_ir, struct mcc_ir_arg *arg);

// Rewind to first IR line of
struct mcc_annotated_ir *mcc_get_function_label(struct mcc_annotated_ir *an_ir);

#endif
