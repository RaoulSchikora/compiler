%option prefix="mcc_parser_"

%option batch
%option bison-bridge
%option bison-locations
%option noinput
%option nounput
%option noyywrap
%option reentrant
%option yylineno

%{
#include "parser.tab.h"
#include <stdbool.h>

#define YYSTYPE MCC_PARSER_STYPE
#define YYLTYPE MCC_PARSER_LTYPE

// TODO:
// - track lines
// - take line breaks into account
#define YY_USER_ACTION \
	yylloc->first_column = yylloc->last_column; \
	yylloc->last_column += yyleng;
%}

identifier    [a-zA-Z_][a-zA-Z0-9_]*
int_literal   [0-9]+
float_literal [0-9]+\.[0-9]+

%%

{int_literal}     { yylval->TK_INT_LITERAL = atol(yytext); return TK_INT_LITERAL; }

{float_literal}   { yylval->TK_FLOAT_LITERAL = atof(yytext); return TK_FLOAT_LITERAL; }

"true"    { yylval->TK_BOOL_LITERAL = true; return TK_BOOL_LITERAL; }
"false"   { yylval->TK_BOOL_LITERAL = false; return TK_BOOL_LITERAL; }

"int"     { yylval->TK_TYPE = INT; return TK_TYPE;}
"float"   { yylval->TK_TYPE = FLOAT; return TK_TYPE;}
"bool"    { yylval->TK_TYPE = BOOL; return TK_TYPE;}
"string"  { yylval->TK_TYPE = STRING; return TK_TYPE;}

{identifier}      { yylval->TK_IDENTIFIER = strdup(yytext); return TK_IDENTIFIER; }

"+"               { return TK_PLUS; }
"-"               { return TK_MINUS; }
"*"               { return TK_ASTER; }
"/"               { return TK_SLASH; }
"!"               { return TK_EXKLA; }

"<"               { return TK_LT_SIGN; }
">"               { return TK_GT_SIGN; }
"<="              { return TK_LT_EQ_SIGN; }
">="              { return TK_GT_EQ_SIGN; }

"&&"              { return TK_ANDAND; }
"||"              { return TK_OROR; }
"=="              { return TK_EQEQ; }
"!="              { return TK_EXKLA_EQ; }


"["               { return TK_SQUARE_OPEN; }
"]"               { return TK_SQUARE_CLOSE; }

"("               { return TK_LPARENTH; }
")"               { return TK_RPARENTH; }

[ \t\r\n]+        { /* ignore */ }

"~"               { return TK_TILDE; /*only for UnitTest purpose*/ }

<<EOF>>           { return TK_END; }

.                 { fprintf(stderr, "invalid character '%c'\n", yytext[0]); }
