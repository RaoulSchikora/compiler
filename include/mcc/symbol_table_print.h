// This module provides basic printing infrastructure for the symbol_table into dot-format.

#ifndef MCC_SYMBOL_TABLE_PRINT_H
#define MCC_SYMBOL_TABLE_PRINT_H

#include <stdio.h>

#include "mcc/symbol_table.h"

void mcc_symbol_table_print(struct mcc_symbol_table *table, void *data);

#endif //MCC_SYMBOL_TABLE_PRINT_H