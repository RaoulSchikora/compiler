#ifndef MC_CLEANUP_INC
#define MC_CLEANUP_INC

// clang-format off

#define clean_func(x)  _Generic((x), \
			struct mcc_symbol_table * : mcc_delete_table, \
			struct mc_cl_parser_command_line_parser * : mcc_delete_cl_parser, \
			struct mcc_semantic_check * : mcc_delete_check, \
			struct mcc_ir_row * : mcc_delete_ir, \
			char* :mcc_delete_error_buffer, \
			struct mcc_ast_program* : mcc_delete_ast\
			)

#define register_cleanup(x) on_exit(clean_func(x),x)

#define UNUSED(x) (void)(x)


// clang-format on

void mcc_delete_error_buffer(int n,void* string){
	UNUSED(n);
	free(string);
}

void mcc_delete_table(int n,void* data){
	UNUSED(n);
	mcc_symbol_table_delete_table(data);
}

void mcc_delete_cl_parser(int n, void* data){
	UNUSED(n);
	mc_cl_parser_delete_command_line_parser(data);
}

void mcc_delete_ast(int n, void* data){
	UNUSED(n);
	mcc_ast_delete_program(data);
}

void mcc_delete_check(int n, void* data){
	UNUSED(n);
	mcc_semantic_check_delete_single_check(data);
}

void mcc_delete_ir(int n, void* data){
	UNUSED(n);
	mcc_ir_delete_ir(data);
}

#endif // MC_CLEANUP_INC