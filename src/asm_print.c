#include "mcc/asm_print.h"
#include "mcc/asm.h"

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils/unused.h"

//---------------------------------------------------------------------------------------- Functions: Print ASM

void mcc_asm_print_dot_begin(FILE *out)
{
	UNUSED(out);
}

void mcc_asm_print_dot_end(FILE *out)
{
	UNUSED(out);
}

void mcc_asm_print_dot_asm(FILE *out, struct mcc_asm *head)
{
	UNUSED(out);
	UNUSED(head);
}
