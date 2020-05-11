#include "mcc/asm.h"

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "mcc/ir.h"
#include "utils/unused.h"

//---------------------------------------------------------------------------------------- Functions: Data structures

struct mcc_asm *mcc_asm_new_asm(struct mcc_asm_data_section *data, struct mcc_asm_text_section *text)
{
	struct mcc_asm *new = malloc(sizeof(*new));
	if (!new)
		return NULL;
	new->data_section = data;
	new->text_section = text;
	return new;
}

struct mcc_asm_data_section *mcc_asm_new_data_section(struct mcc_asm_declaration *head)
{
	struct mcc_asm_data_section *new = malloc(sizeof(*new));
	if (!new)
		return NULL;
	new->head = head;
	return new;
}

struct mcc_asm_text_section *mcc_asm_new_text_section(struct mcc_asm_function *function)
{
	struct mcc_asm_text_section *new = malloc(sizeof(*new));
	if (!new)
		return NULL;
	new->function = function;
	return new;
}

struct mcc_asm_declaration *
mcc_asm_new_float_declaration(char *identifier, float float_value, struct mcc_asm_declaration *next)
{
	struct mcc_asm_declaration *new = malloc(sizeof(*new));
	if (!new)
		return NULL;
	new->identifier = identifier;
	new->float_value = float_value;
	new->next = next;
	new->type = MCC_ASM_DECLARATION_TYPE_FLOAT;
	return new;
}

struct mcc_asm_declaration *
mcc_asm_new_db_declaration(char *identifier, char *db_value, struct mcc_asm_declaration *next)
{
	struct mcc_asm_declaration *new = malloc(sizeof(*new));
	if (!new)
		return NULL;
	new->identifier = identifier;
	new->db_value = db_value;
	new->next = next;
	new->type = MCC_ASM_DECLARATION_TYPE_DB;
	return new;
}

struct mcc_asm_function *
mcc_asm_new_function(char *label, struct mcc_asm_assembly_line *head, struct mcc_asm_function *next)
{
	struct mcc_asm_function *new = malloc(sizeof(*new));
	if (!new)
		return NULL;
	new->head = head;
	new->label = label;
	new->next = next;
	return new;
}

struct mcc_asm_assembly_line *mcc_asm_new_assembly_line(enum mcc_asm_opcode opcode,
                                                        struct mcc_asm_operand *first,
                                                        struct mcc_asm_operand *second,
                                                        struct mcc_asm_assembly_line *next)
{
	struct mcc_asm_assembly_line *new = malloc(sizeof(*new));
	if (!new)
		return NULL;
	new->opcode = opcode;
	new->first = first;
	new->second = second;
	new->next = next;
	return new;
}

struct mcc_asm_operand *mcc_asm_new_literal_operand(int literal)
{
	struct mcc_asm_operand *new = malloc(sizeof(*new));
	if (!new)
		return NULL;
	new->type = MCC_ASM_OPERAND_LITERAL;
	new->literal = literal;
	return new;
}

struct mcc_asm_operand *mcc_asm_new_register_operand(enum mcc_asm_register reg)
{
	struct mcc_asm_operand *new = malloc(sizeof(*new));
	if (!new)
		return NULL;
	new->type = MCC_ASM_OPERAND_REGISTER;
	new->reg = reg;
	return new;
}

struct mcc_asm_operand *mcc_asm_new_data_operand(struct mcc_asm_declaration *decl)
{
	struct mcc_asm_operand *new = malloc(sizeof(*new));
	if (!new)
		return NULL;
	new->type = MCC_ASM_OPERAND_DATA;
	new->decl = decl;
	return new;
}

//---------------------------------------------------------------------------------------- Functions: ASM generation

struct mcc_asm *mcc_asm_generate(struct mcc_ir_row *ir)
{
	UNUSED(ir);
	return NULL;
}

void mcc_asm_delete_asm(struct mcc_asm *head)
{
	UNUSED(head);
}
