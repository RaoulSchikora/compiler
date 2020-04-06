%define api.prefix {mcc_parser_}

%define api.pure full
%lex-param   {void *scanner}
%parse-param {void *scanner} {struct mcc_parser_result* result}

%define parse.trace
%define parse.error verbose

%code requires {
#include "mcc/parser.h"

char *filename; /* current filename here for the lexer */

typedef struct MCC_PARSER_LTYPE {
	int first_line;
	int first_column;
	int last_line;
	int last_column;
	char *filename;
} MCC_PARSER_LTYPE;

# define MCC_PARSER_LTYPE_IS_DECLARED 1 /* alert the parser that we have our own definition */
# define YYLLOC_DEFAULT(Cur, Rhs, N)                      \
do                                                        \
  if (N)                                                  \
    {                                                     \
      (Cur).first_line   = YYRHSLOC(Rhs, 1).first_line;   \
      (Cur).first_column = YYRHSLOC(Rhs, 1).first_column; \
      (Cur).last_line    = YYRHSLOC(Rhs, N).last_line;    \
      (Cur).last_column  = YYRHSLOC(Rhs, N).last_column;  \
      (Cur).filename     = YYRHSLOC(Rhs, 1).filename;     \
    }                                                     \
  else                                                    \
    {                                                     \
      (Cur).first_line   = (Cur).last_line   =            \
        YYRHSLOC(Rhs, 0).last_line;                       \
      (Cur).first_column = (Cur).last_column =            \
        YYRHSLOC(Rhs, 0).last_column;                     \
      (Cur).filename     = NULL;                          \
    }                                                     \
while (0)
}

%{
#include <string.h>

int mcc_parser_lex();
void mcc_parser_error();

#define loc(ast_node, ast_sloc, ast_sloc_last) 			      \
	(ast_node)->node.sloc.start_col = (ast_sloc).first_column;    \
	(ast_node)->node.sloc.start_line = (ast_sloc).first_line;     \
	(ast_node)->node.sloc.end_col = (ast_sloc_last).last_column;  \
	(ast_node)->node.sloc.end_line = (ast_sloc_last).last_line;   \
	(ast_node)->node.sloc.filename = filename;          \

int start_token;

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

%token START_UNIT 1
%token START_PROG 2

// set precedence and associativity
%left ANDAND OROR
%left LT_SIGN GT_SIGN LT_EQ_SIGN GT_EQ_SIGN EQEQ EXKLA_EQ
%left PLUS MINUS
%left ASTER SLASH
%precedence NOT_ELSE
%precedence ELSE

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
%type <struct mcc_ast_identifier *> identifier

%destructor { mcc_ast_delete($$); } expression
%destructor { mcc_ast_delete($$); } statement
%destructor { mcc_ast_delete($$); } assignment
%destructor { mcc_ast_delete($$); } declaration
%destructor { mcc_ast_delete($$); } literal
%destructor { mcc_ast_delete($$); } compound_statement
%destructor { mcc_ast_delete($$); } statements
%destructor { mcc_ast_delete($$); } parameters
%destructor { mcc_ast_delete($$); } function_def
%destructor { mcc_ast_delete($$); } function_defs
%destructor { mcc_ast_delete($$); } program
%destructor { mcc_ast_delete($$); } arguments
%destructor { mcc_ast_delete($$); } identifier
%destructor { free($$); } STRING_LITERAL
%destructor { free($$); } IDENTIFIER



%start toplevel

%%

toplevel	    : START_UNIT unit_test
                    | START_PROG program
                    	{result->entry_point = MCC_PARSER_ENTRY_POINT_PROGRAM; result->program = $2;}
                    ;

unit_test           : expression
			{ result->entry_point = MCC_PARSER_ENTRY_POINT_EXPRESSION; result->expression = $1;}
                    | declaration
                    	{ result->entry_point = MCC_PARSER_ENTRY_POINT_DECLARATION; result->declaration = $1;}
                    | assignment
                    	{ result->entry_point = MCC_PARSER_ENTRY_POINT_ASSIGNMENT; result->assignment = $1;}
                    | statement
                    	{ result->entry_point = MCC_PARSER_ENTRY_POINT_STATEMENT; result->statement = $1;}
                    | compound_statement
                    	{ result->entry_point =
                    		MCC_PARSER_ENTRY_POINT_COMPOUND_STATEMENT; result->compound_statement = $1;}
                    | function_def
                    	{ result->entry_point =
                    		MCC_PARSER_ENTRY_POINT_FUNCTION_DEFINITION; result->function_definition = $1;}
                    | parameters
                    	{ result->entry_point = MCC_PARSER_ENTRY_POINT_PARAMETERS; result->parameters = $1; }
                    | arguments
                    	{ result->entry_point = MCC_PARSER_ENTRY_POINT_ARGUMENTS; result->arguments = $1; }
                    | program
                    	{ result->entry_point = MCC_PARSER_ENTRY_POINT_PROGRAM; result->program = $1; }
                    ;

expression          : literal { $$ = mcc_ast_new_expression_literal($1); 			    loc($$, @1 ,@1);}
                    | expression PLUS expression
                    	{ $$ = mcc_ast_new_expression_binary_op(MCC_AST_BINARY_OP_ADD, $1, $3);     loc($$, @1, @3);}
                    | expression MINUS expression
                    	{ $$ = mcc_ast_new_expression_binary_op(MCC_AST_BINARY_OP_SUB, $1, $3);     loc($$, @1, @3);}
                    | expression ASTER expression
                    	{ $$ = mcc_ast_new_expression_binary_op(MCC_AST_BINARY_OP_MUL, $1, $3);     loc($$, @1, @3);}
                    | expression SLASH expression
                    	{ $$ = mcc_ast_new_expression_binary_op(MCC_AST_BINARY_OP_DIV, $1, $3);     loc($$, @1, @3);}
                    | expression LT_SIGN expression
                    	{ $$ = mcc_ast_new_expression_binary_op(MCC_AST_BINARY_OP_SMALLER, $1, $3); loc($$, @1, @3);}
                    | expression GT_SIGN expression
                    	{ $$ = mcc_ast_new_expression_binary_op(MCC_AST_BINARY_OP_GREATER, $1, $3); loc($$, @1, @3);}
                    | expression LT_EQ_SIGN expression
                    	{ $$ = mcc_ast_new_expression_binary_op(MCC_AST_BINARY_OP_SMALLEREQ,$1,$3); loc($$, @1, @3);}
                    | expression GT_EQ_SIGN expression
                    	{ $$ = mcc_ast_new_expression_binary_op(MCC_AST_BINARY_OP_GREATEREQ,$1,$3); loc($$, @1, @3);}
                    | expression ANDAND expression
                    	{ $$ = mcc_ast_new_expression_binary_op(MCC_AST_BINARY_OP_CONJ, $1, $3);    loc($$, @1, @3);}
                    | expression OROR expression
                    	{ $$ = mcc_ast_new_expression_binary_op(MCC_AST_BINARY_OP_DISJ, $1, $3);    loc($$, @1, @3);}
                    | expression EQEQ expression
                    	{ $$ = mcc_ast_new_expression_binary_op(MCC_AST_BINARY_OP_EQUAL, $1, $3);   loc($$, @1, @3);}
                    | expression EXKLA_EQ expression
                    	{ $$ = mcc_ast_new_expression_binary_op(MCC_AST_BINARY_OP_NOTEQUAL,$1,$3);  loc($$, @1, @3);}
                    | LPARENTH expression RPARENTH { $$ = mcc_ast_new_expression_parenth($2);       loc($$, @1, @3);}
                    | MINUS expression
                    	{ $$ = mcc_ast_new_expression_unary_op(MCC_AST_UNARY_OP_NEGATIV, $2);	    loc($$, @1, @2);}
                    | EXKLA expression
                    	{ $$ = mcc_ast_new_expression_unary_op(MCC_AST_UNARY_OP_NOT, $2);           loc($$, @1, @2);}
                    | identifier { $$ = mcc_ast_new_expression_variable($1); 			    loc($$, @1, @1);}
                    | identifier SQUARE_OPEN expression SQUARE_CLOSE
                    	{ $$ = mcc_ast_new_expression_array_element($1,$3);                         loc($$, @1, @4);}
                    | identifier LPARENTH arguments RPARENTH
                    	{ $$ = mcc_ast_new_expression_function_call($1, $3);loc($$, @1, @4);}
                    ;

arguments           : expression { $$ = mcc_ast_new_arguments(false, $1, NULL); 		    loc($$, @1, @1);}
                    | expression COMMA arguments { $$ = mcc_ast_new_arguments(false, $1, $3); 	    loc($$, @1, @3);}
                    | %empty { $$ = mcc_ast_new_arguments(true, NULL, NULL); }
                    ;


assignment 	    : identifier EQ expression { $$ = mcc_ast_new_variable_assignment ($1, $3);     loc($$, @1, @3);}
                    | identifier SQUARE_OPEN expression SQUARE_CLOSE EQ expression
                    	{ $$ = mcc_ast_new_array_assignment ($1, $3, $6); 			    loc($$, @1, @6);}
                    ;

declaration         : TYPE identifier { $$ = mcc_ast_new_variable_declaration($1,$2); 		    loc($$, @1, @2);}
                    | TYPE SQUARE_OPEN INT_LITERAL SQUARE_CLOSE identifier
                    	{ $$ = mcc_ast_new_array_declaration($1, mcc_ast_new_literal_int($3), $5);  loc($$, @1, @5);}
                    ;

identifier 			: IDENTIFIER {$$ = mcc_ast_new_identifier($1); loc($$,@1,@1);}
					;

statement           : IF LPARENTH expression RPARENTH statement %prec NOT_ELSE
			{ $$ = mcc_ast_new_statement_if_stmt( $3, $5); 	    			    loc($$, @1, @5);}
                    | IF LPARENTH expression RPARENTH statement ELSE statement
                    	{ $$ = mcc_ast_new_statement_if_else_stmt( $3, $5, $7);                     loc($$, @1, @7);}
                    | expression SEMICOLON 	  { $$ = mcc_ast_new_statement_expression( $1);     loc($$, @1, @2);}
                    | WHILE LPARENTH expression RPARENTH statement
                    	{ $$ = mcc_ast_new_statement_while( $3, $5); 	    			    loc($$, @1, @5);}
                    | assignment SEMICOLON { $$ = mcc_ast_new_statement_assignment($1);	    	    loc($$, @1, @2);}
                    | declaration SEMICOLON { $$ = mcc_ast_new_statement_declaration($1);	    loc($$, @1, @2);}
                    | RETURN SEMICOLON { $$ = mcc_ast_new_statement_return(true, NULL);  	    loc($$, @1, @2);}
                    | RETURN expression SEMICOLON { $$ = mcc_ast_new_statement_return(false, $2);   loc($$, @1, @3);}
                    | compound_statement { $$ = mcc_ast_new_statement_compound_stmt($1);   	    loc($$, @1, @1);}
                    ;

statements          : statement statements { $$ = mcc_ast_new_compound_stmt(false, $1, $2); 	    loc($$, @1, @2);}
                    | statement            { $$ = mcc_ast_new_compound_stmt(false, $1, NULL);  	    loc($$, @1, @1);}
                    ;


compound_statement  :   CURL_OPEN statements CURL_CLOSE { $$ = $2; 				    loc($$, @1, @3);}
                    |   CURL_OPEN CURL_CLOSE { $$ = mcc_ast_new_compound_stmt(true,NULL,NULL);      loc($$, @1, @2);}
                    ;

literal             : INT_LITERAL    { $$ = mcc_ast_new_literal_int($1);   			    loc($$, @1, @1);}
                    | FLOAT_LITERAL  { $$ = mcc_ast_new_literal_float($1); 			    loc($$, @1, @1);}
                    | BOOL_LITERAL   { $$ = mcc_ast_new_literal_bool($1);  			    loc($$, @1, @1);}
                    | STRING_LITERAL { $$ = mcc_ast_new_literal_string($1); free($1); 		    loc($$, @1, @1);}
                    ;

parameters          : declaration    { $$ = mcc_ast_new_parameters(false, $1, NULL ); 		    loc($$, @1, @1);}
                    | declaration COMMA parameters   { $$ = mcc_ast_new_parameters(false, $1, $3 ); loc($$, @1, @3);}
                    | %empty { $$ = mcc_ast_new_parameters(true, NULL, NULL); }
                    ;

function_def        : VOID identifier LPARENTH parameters RPARENTH compound_statement
			{ $$ = mcc_ast_new_void_function_def($2, $4, $6);   loc($$, @1, @6);}
                    | TYPE identifier LPARENTH parameters RPARENTH compound_statement
                    	{ $$ = mcc_ast_new_type_function_def($1, $2,$4,$6); loc($$, @1, @6);}
                    ;

function_defs       :   function_def function_defs  { $$ = mcc_ast_new_program($1, $2); 	    loc($$, @1, @2);}
                    |   function_def                { $$ = mcc_ast_new_program($1, NULL); 	    loc($$, @1, @1);}
                    ;

program             :  function_defs { $$ = $1; 						    loc($$, @1, @1);}
                    ;

%%


#include <assert.h>

#include "scanner.h"
#include "utils/unused.h"
#include "mcc/parser.h"

char* buffer;

struct mcc_parser_result mcc_parse_string(const char *input_string, enum mcc_parser_entry_point entry_point)
{
	assert(input_string);

	char* input;


	input = (char*) malloc ((strlen(input_string)+1)*sizeof(char));
	if(input == NULL){
		return (struct mcc_parser_result){
			.status = MCC_PARSER_STATUS_UNKNOWN_ERROR,
		};
	}
	strcpy (input,input_string);

	FILE *in = fmemopen((void *)input, strlen(input), "r");
	if (in == NULL) {
		return (struct mcc_parser_result){
		    .status = MCC_PARSER_STATUS_UNABLE_TO_OPEN_STREAM,
		};
	}

	struct mcc_parser_result result = mcc_parse_file(in,entry_point,"stdin");

	free(input);

	fclose(in);

	return result;
}

struct mcc_parser_result mcc_parse_file(FILE *input, enum mcc_parser_entry_point entry_point,char* name)
{
	assert(input);

	yyscan_t scanner;
	mcc_parser_lex_init(&scanner);
	mcc_parser_set_in(input, scanner);

	if (entry_point != MCC_PARSER_ENTRY_POINT_PROGRAM){
		filename = "<test_suite>";
		start_token = 1;
	} else {
		start_token = 2;
		filename = name;
	}

	struct mcc_parser_result result = {
	    .status = MCC_PARSER_STATUS_OK,
	    .error_buffer = NULL,
	};

	if (yyparse(scanner, &result) != 0) {
		result.status = MCC_PARSER_STATUS_UNKNOWN_ERROR;
		result.error_buffer = (char *)malloc(sizeof(char) * strlen(buffer) + 1);
		strcpy(result.error_buffer, buffer);
		free(buffer);
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

// Writes error message to a variable "buffer" that is allocated on the heap
void mcc_parser_error(struct MCC_PARSER_LTYPE *yylloc, struct mcc_parser_result *result, yyscan_t *scanner,
 												const char *msg)
{
	int size = strlen(msg) + 50 + strlen(yylloc->filename);
	char* str = (char *)malloc( sizeof(char) * size);
	snprintf(str, size, "%s:%d:%d: %s\n", yylloc->filename, yylloc->first_line, yylloc->first_column, msg);

	buffer = (char *)malloc(sizeof(char) * strlen(str) + 1);
	strcpy(buffer, str);

	free(str);

	// scanner needed to get meaningfull msg
	UNUSED(scanner);
	UNUSED(result);
}

