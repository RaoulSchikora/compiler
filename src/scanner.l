%option prefix="mcc_parser_"

%option batch
%option bison-bridge
%option bison-locations
%option noinput
%option nounput
%option noyywrap
%option reentrant
%option extra-type="int"
%option yylineno

/* define exclusive state for comments */
%x COMMENT

%{
#include "parser.tab.h"

#define YYSTYPE MCC_PARSER_STYPE
#define YYLTYPE MCC_PARSER_LTYPE

#define YY_USER_ACTION \
	yylloc->first_column = yylloc->last_column; \
	yylloc->last_column += yyleng; \
	yylloc->first_line = yylloc->last_line;

%}

identifier     [a-zA-Z_][a-zA-Z0-9_]*
int_literal    [0-9]+
float_literal  [0-9]+\.[0-9]+
string_literal \"[^"]*\"


%%

%{
    if(yyextra)
        {
            int t = yyextra;
            yyextra = 0;

            // tell scanner that we start counting from 1 and not from 0
            yylloc->last_line = 1;
            yylloc->last_column = 1;

            return t;
        }
%}


{int_literal}     { yylval->TK_INT_LITERAL = atol(yytext); return TK_INT_LITERAL; }

{float_literal}   { yylval->TK_FLOAT_LITERAL = atof(yytext); return TK_FLOAT_LITERAL; }

{string_literal}  { yylval->TK_STRING_LITERAL = strdup(yytext); 
                    for (int i=0; yytext[i]; i++) yylloc->last_line += (yytext[i] == '\n');
                    return TK_STRING_LITERAL; }

"true"    { yylval->TK_BOOL_LITERAL = true; return TK_BOOL_LITERAL; }
"false"   { yylval->TK_BOOL_LITERAL = false; return TK_BOOL_LITERAL; }

"int"     { yylval->TK_TYPE = INT; return TK_TYPE; }
"float"   { yylval->TK_TYPE = FLOAT; return TK_TYPE; }
"bool"    { yylval->TK_TYPE = BOOL; return TK_TYPE; }
"string"  { yylval->TK_TYPE = STRING; return TK_TYPE; }
"void"    { return TK_VOID; }


"+"               { return TK_PLUS; }
"-"               { return TK_MINUS; }
"*"               { return TK_ASTER; }
"/"               { return TK_SLASH; }
"!"               { return TK_EXKLA; }
"="               { return TK_EQ; }

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

"{"               { return TK_CURL_OPEN; }
"}"               { return TK_CURL_CLOSE; }

"if"              { return TK_IF; }
"else"            { return TK_ELSE; }
"while"           { return TK_WHILE; }
"return"          { return TK_RETURN; }

{identifier}      { yylval->TK_IDENTIFIER = strdup(yytext); return TK_IDENTIFIER; }

";"               { return TK_SEMICOLON; }
","               { return TK_COMMA; }

[ \t\r]+          { /* ignore */ }
\n                { yylloc->last_line += 1; yylloc->last_column = 1;}

"/*"              { BEGIN COMMENT; }
<COMMENT>.        { /* ignore comments */ }
<COMMENT>\n       { yylloc->last_line += 1; yylloc->last_column = 1; }
<COMMENT>"*/"     { BEGIN INITIAL; /* get back to normal mode */ }


<<EOF>>           { return TK_END; }

.                 { fprintf(stderr, "invalid character '%c'. I'll ignore it and continue.\n", yytext[0]); }
