/*
 */
alpha = [a-zA-Z_]
alpha_num = [a-zA-Z0-9_]
digit = [0-9]
identifier = alpha (alpha_num)*

%%



%%

main()
{
  yylex();
}


