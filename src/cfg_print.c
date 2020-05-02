#include "mcc/cfg_print.h"
#include "mcc/cfg.h"
#include "mcc/ir.h"
#include "mcc/ir_print.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void mcc_cfg_print_dot_begin(FILE *out)
{
	fprintf(out, "%s", "digraph A {\n");
}

void mcc_cfg_print_dot_end(FILE *out)
{
	fprintf(out, "%s", "}\n");
}

void mcc_cfg_print_dot_cfg(FILE *out, struct mcc_basic_block *head)
{
	mcc_cfg_print_dot_begin(out);
	mcc_cfg_print_dot_bb(out, head);
	mcc_cfg_print_dot_end(out);
}

void mcc_cfg_print_dot_bb(FILE *out, struct mcc_basic_block *block)
{
	fprintf(out, "%p [shape=record label=\"{\n", (void *)block);
	mcc_cfg_print_dot_ir(out, block->leader);
	fprintf(out, "}\n\"];\n");
	if (block->child_left) {
		fprintf(out, "%p -> %p;\n", (void *)block, (void *)block->child_left);
	}
	if (block->child_right) {
		fprintf(out, "%p -> %p;\n", (void *)block, (void *)block->child_right);
	}
}

void mcc_cfg_print_dot_ir_row(FILE *out, struct mcc_ir_row *leader)
{
	char *row = mcc_ir_print_ir_row_to_string(leader);
	if (leader->next_row) {
		fprintf(out, "{%s}|\n", row);
	} else {
		fprintf(out, "{%s}\n", row);
	}
	free(row);
}

void mcc_cfg_print_dot_ir(FILE *out, struct mcc_ir_row *leader)
{
	while (leader) {
		mcc_cfg_print_dot_ir_row(out, leader);
		leader = leader->next_row;
	}
}
