mcc_parser_result:
	- return value is pointer to allocated mcc_parser_result struct
fileToString:
	- return value is pointer to allocated string
StdinToString:
	- return value is pointer to allocated string
parse_options:
	- return value is pointer to allocated mc_ast_to_dot_options struct
parse_arguments:
	- return value is pointer to allocated mc_ast_to_dot_arguments struct
	- inside this struct the member args is allocated by malloc as well
parse_command_line:
	- return value is pointer to allocated mc_ast_to_dot_command_line_parser_struct
	- inside this struct the members options and arguments are allocated, as well as
	  arguments->args
limit_result_to_function_scope:
	- returns mcc_parser_result struct by value
