#include "mcc/asm.h"

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "mcc/ir.h"
#include "utils/unused.h"

struct mcc_asm *mcc_asm_generate(struct mcc_ir_row *ir)
{
	UNUSED(ir);
	return NULL;
}

void mcc_asm_delete_asm(struct mcc_asm *head)
{
	UNUSED(head);
}
