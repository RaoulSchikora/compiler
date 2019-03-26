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

#define YYSTYPE MCC_PARSER_STYPE
#define YYLTYPE MCC_PARSER_LTYPE

// TODO:
// - track lines
// - take line breaks into account
#define YY_USER_ACTION \
	yylloc->first_column = yylloc->last_column; \
	yylloc->last_column += yyleng;
%}

int_literal     [0-9]+
float_literal   [0-9]+\.[0-9]+
bool_literal    "true"|"false"
string_literal  /"[^"]*"/

%%

{int_literal}     { yylval->TK_INT_LITERAL = atol(yytext); return TK_INT_LITERAL; }

{float_literal}   { yylval->TK_FLOAT_LITERAL = atof(yytext); return TK_FLOAT_LITERAL; }




"bool"            { return TK_TYPE_BOOL; }
"int"             { return TK_TYPE_INT; }
"float"           { return TK_TYPE_STRING; }
"void"            { return TK_TYPE_VOID; }

"true"            { return TK_BOOL_TRUE; }
"false"           { return TK_BOOK_FALSE; }

"if"              { return TK_IF; }
"else"            { return TK_ELSE; }
"while"           { return TK_WHILE;}
"return"          { return TK_RETURN; }


"+"               { return TK_PLUS; }
"-"               { return TK_MINUS; }
"*"               { return TK_ASTER; }
"/"               { return TK_SLASH; }

"<"               { return TK_LESS_THAN; }
">"               { return TK_GREATER_THAN; }
"<="              { return TK_LESS_EQUAL; }
">="              { return TK_GREATER_EQUAL; }

"!"               { return TK_LOGICAL_NEGATION; }
"&&"              { return TK_LOGICAL_AND; }
"||"              { return TK_LOGICAL_OR; }
"!="              { return TK_NOT_EQUAL; }
"=="              { return TK_EQUAL; }

"("               { return TK_LPARENTH; }
")"               { return TK_RPARENTH; }
"{"               { return TK_L_CURL_PARENTH; }
"}"               { return TK_R_CURL_PARENTH; }
"["               { return TK_L_SQUARE_PARENTH; }
"]"               { return TK_R_SQUARE_PARENTH; }

";"               { return TK_SEMICOLON; }

[ \t\r\n]+        { /* ignore */ }

<<EOF>>           { return TK_END; }

.                 { fprintf(stderr, "invalid character '%c'\n", yytext[0]); }
