%define api.prefix {mcc_parser_}

%define api.pure full
%lex-param   {void *scanner}
%parse-param {void *scanner} {struct mcc_parser_result* result}

%define parse.trace
%define parse.error verbose

// Enabling printer of the state of the parser for debugging purposes:


%printer { fprintf (yyo, "Semantic string value:'%s'", $$); } <char*>
%printer { fprintf (yyo, "Semantic float value:'%ld'", $$); } <long>
%printer { fprintf (yyo, "Semantic int value:'%f'", $$); } <double>



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
%token <bool>   BOOL_LITERAL  "bool literal"
%token <char*> 	STRING_LITERAL"string literal"


%token <char*> 	IDENTIFIER    "identifier"

%token <enum mcc_ast_types> TYPE "type"
%token VOID "void"

%token LPARENTH "("
%token RPARENTH ")"

%token CURL_OPEN "{"
%token CURL_CLOSE "}"

%token TILDE "~"

%token PLUS  "+"
%token MINUS "-"
%token ASTER "*"
%token SLASH "/"
%token SQUARE_OPEN "["
%token SQUARE_CLOSE "]"
%token EXKLA "!"
%token EQ "="

%token LT_SIGN "<"
%token GT_SIGN ">"
%token LT_EQ_SIGN "<="

%token GT_EQ_SIGN ">="

%token ANDAND "&&"
%token OROR "||"
%token EQEQ "=="
%token EXKLA_EQ "!="

%token IF "if"
%token ELSE "else"
%token WHILE "while"
%token RETURN "return"

%token SEMICOLON ";"
%token COMMA ","

%left ANDAND OROR
%left LT_SIGN GT_SIGN LT_EQ_SIGN GT_EQ_SIGN EQEQ EXKLA_EQ
%left PLUS MINUS
%left ASTER SLASH

%type <struct mcc_ast_expression *> expression
%type <struct mcc_ast_literal *> literal
%type <struct mcc_ast_declaration *> declaration
%type <struct mcc_ast_assignment *> assignment
%type <struct mcc_ast_statement *> statement
%type <struct mcc_ast_compound_statement *> compound_statement
%type <struct mcc_ast_compound_statement *> statements
%type <struct mcc_ast_program *> program
%type <struct mcc_ast_function_definition *> function_def
%type <struct mcc_ast_parameters *> parameters
%type <struct mcc_ast_arguments *> arguments
%type <struct mcc_ast_program *> function_defs

%destructor { mcc_ast_delete($$); } expression
%destructor { mcc_ast_delete($$); } statement
%destructor { mcc_ast_delete($$); } assignment
%destructor { mcc_ast_delete($$); } declaration
%destructor { mcc_ast_delete($$); } literal

%start toplevel

%%

toplevel        : TILDE unit_test TILDE
                | program
                ;

unit_test       : expression { result->entry_point = MCC_PARSER_ENTRY_POINT_EXPRESSION; result->expression = $1;  }
                | declaration { result->entry_point = MCC_PARSER_ENTRY_POINT_DECLARATION; result->declaration = $1;}
                | assignment { result->entry_point = MCC_PARSER_ENTRY_POINT_ASSIGNMENT; result->assignment = $1;}
                | statement { result->entry_point = MCC_PARSER_ENTRY_POINT_STATEMENT; result->statement = $1;}
                | compound_statement { result->entry_point = MCC_PARSER_ENTRY_POINT_COMPOUND_STATEMENT; result->compound_statement = $1;}
                | function_def { result->entry_point = MCC_PARSER_ENTRY_POINT_FUNCTION_DEFINITION; result->function_definition = $1;}
                | parameters {  result->entry_point = MCC_PARSER_ENTRY_POINT_PARAMETERS; result->parameters = $1; }
                | arguments { result->entry_point = MCC_PARSER_ENTRY_POINT_ARGUMENTS; result->arguments = $1; }
                | program { result->entry_point = MCC_PARSER_ENTRY_POINT_PROGRAM; result->program = $1; }
                ;

expression      : literal                      { $$ = mcc_ast_new_expression_literal($1);                              loc($$, @1); }
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
                | IDENTIFIER LPARENTH arguments RPARENTH        { $$ = mcc_ast_new_expression_function_call(mcc_ast_new_identifier($1), $3); loc($$,@1); }
                ;

arguments       : expression { $$ = mcc_ast_new_arguments(false, $1, NULL); loc($$,@1); }
                | expression COMMA arguments { $$ = mcc_ast_new_arguments(false, $1, $3); loc($$,@1);}
                | %empty { $$ = mcc_ast_new_arguments(true, NULL, NULL); }
                ;


assignment 	    :	IDENTIFIER EQ expression { $$ = mcc_ast_new_variable_assignment ($1, $3); loc($$,@1); }
                |	IDENTIFIER SQUARE_OPEN expression SQUARE_CLOSE EQ expression { $$ = mcc_ast_new_array_assignment ($1, $3, $6); loc($$, @1);}
                ;

declaration     : TYPE IDENTIFIER { $$ = mcc_ast_new_variable_declaration($1,$2); loc ($$, @1);}
                | TYPE SQUARE_OPEN INT_LITERAL SQUARE_CLOSE IDENTIFIER { $$ = mcc_ast_new_array_declaration($1, mcc_ast_new_literal_int($3), $5); loc($$, @1);}
                ;


statement       : IF LPARENTH expression RPARENTH statement 		            { $$ = mcc_ast_new_statement_if_stmt( $3, $5); 	     loc($$, @1);}
                | IF LPARENTH expression RPARENTH statement ELSE statement    { $$ = mcc_ast_new_statement_if_else_stmt( $3, $5, $7); loc($$, @1);}
                | expression SEMICOLON 				                        { $$ = mcc_ast_new_statement_expression( $1); 	     loc($$, @1);}
                | WHILE LPARENTH expression RPARENTH statement 	            { $$ = mcc_ast_new_statement_while( $3, $5); 	     loc($$, @1);}
                | assignment SEMICOLON				                        { $$ = mcc_ast_new_statement_assignment($1);	     loc($$, @1);}
                | declaration SEMICOLON				                        { $$ = mcc_ast_new_statement_declaration($1);	     loc($$, @1);}
                | RETURN SEMICOLON                                          { $$ = mcc_ast_new_statement_return(true, NULL);           loc($$, @1);}
                | RETURN expression SEMICOLON                               { $$ = mcc_ast_new_statement_return(false, $2);             loc($$, @1);}
                ;

statements      : statement statements  { $$ = mcc_ast_new_compound_stmt(false, $1, $2); loc($$,@1); }
                | statement             { $$ = mcc_ast_new_compound_stmt(false, $1, NULL); loc($$,@1); }
                ;


compound_statement  :   CURL_OPEN statements CURL_CLOSE { $$ = $2; loc($$,@1); }
                    |   CURL_OPEN CURL_CLOSE            { $$ = mcc_ast_new_compound_stmt(true,NULL,NULL); loc($$,@1); }
                    ;

literal         : INT_LITERAL    { $$ = mcc_ast_new_literal_int($1);   loc($$, @1); }
                | FLOAT_LITERAL  { $$ = mcc_ast_new_literal_float($1); loc($$, @1); }
                | BOOL_LITERAL   { $$ = mcc_ast_new_literal_bool($1);  loc($$, @1); }
                | STRING_LITERAL { $$ = mcc_ast_new_literal_string($1); free($1); loc($$, @1);}
                ;

parameters      : declaration                    { $$ = mcc_ast_new_parameters(false, $1, NULL ); loc($$,@1); }
                | declaration COMMA parameters   { $$ = mcc_ast_new_parameters(false, $1, $3 ); loc($$,@1); }
                | %empty { $$ = mcc_ast_new_parameters(true, NULL, NULL); }
                ;

function_def    : VOID IDENTIFIER LPARENTH parameters RPARENTH compound_statement    { $$ = mcc_ast_new_void_function_def(mcc_ast_new_identifier($2), $4, $6); loc($$,@1); }
                | TYPE IDENTIFIER LPARENTH parameters RPARENTH compound_statement    { $$ = mcc_ast_new_type_function_def($1, mcc_ast_new_identifier($2), $4, $6); loc($$,@1); }
                ;

function_defs   :   function_def function_defs  { $$ = mcc_ast_new_program($1, $2); loc($$,@1); }
                |   function_def                { $$ = mcc_ast_new_program($1, NULL); loc($$,@1); }
                ;

program         :  function_defs { $$ = $1; loc($$,@1); }
                ;

%%


#include <assert.h>

#include "scanner.h"
#include "utils/unused.h"
#include "mcc/parser.h"

// Enabling verbose debugging that shows state of the parser:


#ifdef YYDEBUG
  yydebug = 1;
#endif





struct mcc_parser_result mcc_parse_string(const char *input_string, enum mcc_parser_entry_point entry_point)
{
	assert(input_string);

	char* input;


	if (entry_point != MCC_PARSER_ENTRY_POINT_PROGRAM){
	    input = mcc_transform_into_unit_test(input_string);
		if(input == NULL){
			return (struct mcc_parser_result){
				.status = MCC_PARSER_STATUS_UNKNOWN_ERROR,
			};
		}
	} else {
		input = (char*) malloc ((strlen(input_string)+1)*sizeof(char));
		if(input == NULL){
			return (struct mcc_parser_result){
				.status = MCC_PARSER_STATUS_UNKNOWN_ERROR,
			};
		}
		strcpy (input,input_string);
	}



	FILE *in = fmemopen((void *)input, strlen(input), "r");
	if (in == NULL) {
		return (struct mcc_parser_result){
		    .status = MCC_PARSER_STATUS_UNABLE_TO_OPEN_STREAM,
		};
	}

	struct mcc_parser_result result = mcc_parse_file(in);

	free(input);

	fclose(in);

	return result;
}

char* mcc_transform_into_unit_test (const char* in) {
  char* out = (char*) malloc((strlen(in)+3)*sizeof(char));
  *out = '~';
  strcpy (out + 1,in);
  *(out + strlen(in)+1) = '~';
  *(out + strlen(in)+2) = '\0';
  return out;
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


	if (yyparse(scanner, &result) != 0) {
		result.status = MCC_PARSER_STATUS_UNKNOWN_ERROR;
	}

	mcc_parser_lex_destroy(scanner);

	return result;
}


void mcc_ast_delete_result(struct mcc_parser_result *result)
{
	assert(result);

	enum mcc_parser_entry_point entry_point = result->entry_point;

	switch(entry_point){
	case MCC_PARSER_ENTRY_POINT_EXPRESSION: ;
		mcc_ast_delete(result->expression);
		break;
	case MCC_PARSER_ENTRY_POINT_STATEMENT: ;
		mcc_ast_delete(result->statement);
		break;
	case MCC_PARSER_ENTRY_POINT_DECLARATION: ;
		mcc_ast_delete(result->declaration);
		break;
	case MCC_PARSER_ENTRY_POINT_ASSIGNMENT: ;
		mcc_ast_delete(result->assignment);
		break;
	case MCC_PARSER_ENTRY_POINT_PROGRAM:
		mcc_ast_delete(result->program);
		break;
    case MCC_PARSER_ENTRY_POINT_FUNCTION_DEFINITION:
        mcc_ast_delete(result->function_definition);
        break;
    case MCC_PARSER_ENTRY_POINT_PARAMETERS:
        mcc_ast_delete(result->parameters);
        break;
    case MCC_PARSER_ENTRY_POINT_ARGUMENTS:
        mcc_ast_delete(result->arguments);
        break;
    case MCC_PARSER_ENTRY_POINT_COMPOUND_STATEMENT:
        mcc_ast_delete(result->compound_statement);
        break;
	}
}


void mcc_parser_error(struct MCC_PARSER_LTYPE *yylloc, yyscan_t *scanner, const char *msg)
{
	// TODO
// 	mcc_parser_lex_destroy(scanner);
//	mcc_ast_delete_result(result);
}

