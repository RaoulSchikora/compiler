//
// Created by raoul on 07.03.20.
// This module provides basic printing infrastructure for the symbol_table. It uses nested HTML tables in DOT format.
// It can be visualized via GraphViz
//

#ifndef MCC_SYMBOL_TABLE_PRINT_H
#define MCC_SYMBOL_TABLE_PRINT_H

#include <stdio.h>

#include "mcc/symbol_table.h"

void mcc_symbol_table_print_dot(struct mcc_symbol_table *table, void *data);

#endif //MCC_SYMBOL_TABLE_PRINT_H
