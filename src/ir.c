#include "mcc/ir.h"

#include <assert.h>
#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "mcc/ast.h"
#include "mcc/ast_visit.h"
#include "mcc/symbol_table.h"
#include "utils/unused.h"


struct mcc_ir_row *mcc_ir_generate(struct mcc_ast_program *ast, struct mcc_symbol_table *table){
	UNUSED(ast);
	UNUSED(table);
	return NULL;
}