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


static struct mcc_ir_row *get_fake_ir_line(){
	struct mcc_ir_row *head = malloc(sizeof(*head));
	if(!head)
		return NULL;

	struct mcc_ir_arg *arg1 = malloc(sizeof(*arg1));
	struct mcc_ir_arg *arg2 = malloc(sizeof(*arg2));
	if(!arg1 || !arg2){
		free(arg1);
		free(arg2);
		free(head);
		return NULL;
	}

	arg1->type = MCC_IR_TYPE_VAR;
	arg2->type = MCC_IR_TYPE_VAR;

	char *str1 = malloc(sizeof(char) * 5);
	char *str2 = malloc(sizeof(char) * 5);
	if(!str1 || !str2){
		free(arg1);
		free(arg2);
		free(head);
		free(str1);
		free(str2);
		return NULL;
	}
	snprintf(str1, 5, "test");
	snprintf(str2, 5, "var2");
	arg1->var = str1;
	arg2->var  = str2;
	head->instr = MCC_IR_INSTR_JUMPFALSE;
	head->row_no = 0;
	head->next_row = NULL;
	head->prev_row = NULL;
	head->arg1 = arg1;
	head->arg2 = arg2;
	return head;
}

static struct mcc_ir_row *get_fake_ir(){
	struct mcc_ir_row *head = get_fake_ir_line();
	struct mcc_ir_row *next = get_fake_ir_line();
	head->next_row = next;
	next->prev_row = head;
	return head;
}

struct mcc_ir_row *mcc_ir_generate(struct mcc_ast_program *ast, struct mcc_symbol_table *table){
	UNUSED(ast);
	UNUSED(table);

	// Return fake IR for testing purpose
	return get_fake_ir();
}

static void mcc_ir_delete_ir_arg(struct mcc_ir_arg *arg){
	if(!arg)
		return;
	if(arg->type == MCC_IR_TYPE_VAR){
		free(arg->var);
	}
	free(arg);
}

void mcc_ir_delete_ir_row(struct mcc_ir_row *row){
	if(!row)
		return;
	mcc_ir_delete_ir_arg(row->arg1);
	mcc_ir_delete_ir_arg(row->arg2);
	free(row);
}

void mcc_ir_delete_ir(struct mcc_ir_row *head){
	struct mcc_ir_row *temp = NULL;
	while (head->next_row) {
		head = head->next_row;
	}
	do {
		temp = head->prev_row;
		mcc_ir_delete_ir_row(head);
		head = temp;
	} while (temp);
}