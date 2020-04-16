#ifndef MC_CLEANUP_INC
#define MC_CLEANUP_INC

#define _GNU_SOURCE

// clang-format off

#define clean_func(x)  _Generic((x), \
			struct mcc_symbol_table * : mc_cleanup_delete_table, \
			struct mc_cl_parser_command_line_parser * : mc_cleanup_delete_cl_parser, \
			struct mcc_semantic_check * : mc_cleanup_delete_check, \
			struct mcc_ir_row * : mc_cleanup_delete_ir, \
			char* :mc_cleanup_delete_error_buffer, \
			struct mcc_ast_program* : mc_cleanup_delete_ast\
			)

#define register_cleanup(x) on_exit(clean_func(x),x)

#define UNUSED(x) (void)(x)


// clang-format on

void mc_cleanup_delete_error_buffer(int n,void* string){
	UNUSED(n);
	free(string);
}

void mc_cleanup_delete_table(int n,void* data){
	UNUSED(n);
	mcc_symbol_table_delete_table(data);
}

void mc_cleanup_delete_cl_parser(int n, void* data){
	UNUSED(n);
	mc_cl_parser_delete_command_line_parser(data);
}

void mc_cleanup_delete_ast(int n, void* data){
	UNUSED(n);
	mcc_ast_delete_program(data);
}

void mc_cleanup_delete_check(int n, void* data){
	UNUSED(n);
	mcc_semantic_check_delete_single_check(data);
}

void mc_cleanup_delete_ir(int n, void* data){
	UNUSED(n);
	mcc_ir_delete_ir(data);
}

#endif // MC_CLEANUP_INC