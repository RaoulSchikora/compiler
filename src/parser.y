%define api.prefix {mcc_parser_}

%define api.pure full
%lex-param   {void *scanner}
%parse-param {void *scanner} {struct mcc_ast_expression** result}

%define parse.trace
%define parse.error verbose

// Enabling printer of the state of the parser for debugging purposes:


//%printer { fprintf (yyo, "Semantic string value:'%s'", $$); } <char*>
//%printer { fprintf (yyo, "Semantic float value:'%ld'", $$); } <long>
//%printer { fprintf (yyo, "Semantic int value:'%f'", $$); } <double>



%code requires {
#include "mcc/parser.h"
}

%{
#include <string.h>

int mcc_parser_lex();
void mcc_parser_error();

#define loc(ast_node, ast_sloc) \
	(ast_node)->node.sloc.start_col = (ast_sloc).first_column;

%}

%define api.value.type union
%define api.token.prefix {TK_}

%locations

%token END 0 "EOF"

%token <long>   INT_LITERAL   "integer literal"
%token <double> FLOAT_LITERAL "float literal"
%token <char*> 	IDENTIFIER    "identifier"
%token <bool>   BOOL_LITERAL  "bool literal"

%token <enum mcc_ast_types> TYPE "type"

%token LPARENTH "("
%token RPARENTH ")"

%token TILDE "~"

%token PLUS  "+"
%token MINUS "-"
%token ASTER "*"
%token SLASH "/"
%token SQUARE_OPEN "["
%token SQUARE_CLOSE "]"
%token EXKLA "!"

%token LT_SIGN "<"
%token GT_SIGN ">"
%token LT_EQ_SIGN "<="
%token GT_EQ_SIGN ">="

%token ANDAND "&&"
%token OROR "||"
%token EQEQ "=="
%token EXKLA_EQ "!="

%left ANDAND OROR
%left LT_SIGN GT_SIGN LT_EQ_SIGN GT_EQ_SIGN EQEQ EXKLA_EQ
%left PLUS MINUS
%left ASTER SLASH

%type <struct mcc_ast_expression *> expression
%type <struct mcc_ast_literal *> literal
%type <struct mcc_ast_variable_declaration *> variable_declaration

%start toplevel

%%

toplevel : TILDE unit_test TILDE {}
         ;

unit_test  : expression { *result = $1; }
	   | variable_declaration { *result = $1 }
	   ;

expression : literal                      { $$ = mcc_ast_new_expression_literal($1);                              loc($$, @1); }
           | expression PLUS  expression  { $$ = mcc_ast_new_expression_binary_op(MCC_AST_BINARY_OP_ADD, $1, $3); loc($$, @1); }
           | expression MINUS expression  { $$ = mcc_ast_new_expression_binary_op(MCC_AST_BINARY_OP_SUB, $1, $3); loc($$, @1); }
           | expression ASTER expression  { $$ = mcc_ast_new_expression_binary_op(MCC_AST_BINARY_OP_MUL, $1, $3); loc($$, @1); }
           | expression SLASH expression  { $$ = mcc_ast_new_expression_binary_op(MCC_AST_BINARY_OP_DIV, $1, $3); loc($$, @1); }
           | expression LT_SIGN expression { $$ = mcc_ast_new_expression_binary_op(MCC_AST_BINARY_OP_SMALLER, $1, $3); loc($$, @1); }
           | expression GT_SIGN expression { $$ = mcc_ast_new_expression_binary_op(MCC_AST_BINARY_OP_GREATER, $1, $3); loc($$, @1); }
           | expression LT_EQ_SIGN expression { $$ = mcc_ast_new_expression_binary_op(MCC_AST_BINARY_OP_SMALLEREQ, $1, $3); loc($$, @1); }
           | expression GT_EQ_SIGN expression { $$ = mcc_ast_new_expression_binary_op(MCC_AST_BINARY_OP_GREATEREQ, $1, $3); loc($$, @1); }
           | expression ANDAND expression { $$ = mcc_ast_new_expression_binary_op(MCC_AST_BINARY_OP_CONJ, $1, $3);loc($$, @1); }
	   | expression OROR expression { $$ = mcc_ast_new_expression_binary_op(MCC_AST_BINARY_OP_DISJ, $1, $3);  loc($$, @1); }
	   | expression EQEQ expression { $$ = mcc_ast_new_expression_binary_op(MCC_AST_BINARY_OP_EQUAL, $1, $3); loc($$, @1); }
	   | expression EXKLA_EQ expression { $$ = mcc_ast_new_expression_binary_op(MCC_AST_BINARY_OP_NOTEQUAL, $1, $3); loc($$, @1); }
           | LPARENTH expression RPARENTH { $$ = mcc_ast_new_expression_parenth($2);                              loc($$, @1); }
           | MINUS expression 		  { $$ = mcc_ast_new_expression_unary_op(MCC_AST_UNARY_OP_NEGATIV, $2);	  loc($$, @1); }
           | EXKLA expression 		  { $$ = mcc_ast_new_expression_unary_op(MCC_AST_UNARY_OP_NOT, $2);     loc($$, @1); }
           | IDENTIFIER			  { $$ = mcc_ast_new_expression_variable($1);				  loc($$, @1); }
           | IDENTIFIER SQUARE_OPEN expression SQUARE_CLOSE {$$ = mcc_ast_new_expression_array_element($1,$3);    loc($$, @1); }
           ;

variable_declaration : TYPE IDENTIFIER { $$ = mcc_ast_new_variable_declaration($1,$2); loc ($$, @1);}
	    	     ;

literal : INT_LITERAL   { $$ = mcc_ast_new_literal_int($1);   loc($$, @1); }
        | FLOAT_LITERAL { $$ = mcc_ast_new_literal_float($1); loc($$, @1); }
        | BOOL_LITERAL  { $$ = mcc_ast_new_literal_bool($1);  loc($$, @1); }
        ;

%%

#include <assert.h>

#include "scanner.h"
#include "utils/unused.h"

// Enabling verbose debugging that shows state of the parser:

/*
#ifdef YYDEBUG
  yydebug = 1;
#endif
*/




void mcc_parser_error(struct MCC_PARSER_LTYPE *yylloc, yyscan_t *scanner, const char *msg)
{
	// TODO
	UNUSED(yylloc);
	UNUSED(scanner);
	UNUSED(msg);
}

struct mcc_parser_result mcc_parse_string(const char *input)
{
	assert(input);

	FILE *in = fmemopen((void *)input, strlen(input), "r");
	if (!in) {
		return (struct mcc_parser_result){
		    .status = MCC_PARSER_STATUS_UNABLE_TO_OPEN_STREAM,
		};
	}

	struct mcc_parser_result result = mcc_parse_file(in);

	fclose(in);

	return result;
}

struct mcc_parser_result mcc_parse_file(FILE *input)
{
	assert(input);

	yyscan_t scanner;
	mcc_parser_lex_init(&scanner);
	mcc_parser_set_in(input, scanner);

	struct mcc_parser_result result = {
	    .status = MCC_PARSER_STATUS_OK,
	};

	if (yyparse(scanner, &result.expression) != 0) {
		result.status = MCC_PARSER_STATUS_UNKNOWN_ERROR;
	}

	mcc_parser_lex_destroy(scanner);

	return result;
}
