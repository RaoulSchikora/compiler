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
			struct mcc_ast_program* : mc_cleanup_delete_ast, \
                        struct mcc_basic_block*: mc_cleanup_delete_cfg, \
                        struct mcc_basic_block_chain *: mc_cleanup_delete_block_chain \
			)


#define register_cleanup(x) on_exit(clean_func(x),x)

#define UNUSED(x) (void)(x)

// Below we use preprocessor directives to suppress compiler warnings.
// This way the provided cleanup mechanism can still be included and used, even if the given header is not used (and
// thus not included)

#ifdef MCC_AST_H
    void mc_cleanup_delete_error_buffer(int n,void* string){
            UNUSED(n);
            free(string);
    }
#else
    void mc_cleanup_delete_error_buffer(int n, void* string){
        UNUSED(n);
        UNUSED(string);
    }
#endif

#ifdef MCC_SYMBOL_TABLE_H
    void mc_cleanup_delete_table(int n,void* data){
            UNUSED(n);
            mcc_symbol_table_delete_table(data);
    }
#else
    void mc_cleanup_delete_table(int n, void*data){
        UNUSED(n);
        UNUSED(data);
    }
#endif

#ifdef MC_CL_PARSER_INC
    void mc_cleanup_delete_cl_parser(int n, void* data){
            UNUSED(n);
            mc_cl_parser_delete_command_line_parser(data);
    }
#else
    void mc_cleanup_delete_cl_parser(int n, void* data){
            UNUSED(n);
            UNUSED(data);
    }
#endif

#ifdef MCC_PARSER_H
void mc_cleanup_delete_ast(int n, void* data){
	UNUSED(n);
	mcc_ast_delete_program(data);
}
#else
void mc_cleanup_delete_ast(int n, void* data){
	UNUSED(n);
        UNUSED(data);
}
#endif


#ifdef PROJECT_SEMANTIC_CHECKS_H
    void mc_cleanup_delete_check(int n, void* data){
        UNUSED(n);
        mcc_semantic_check_delete_single_check(data);
    }
#else
    void mc_cleanup_delete_check(int n, void* data){
        UNUSED(n);
        UNUSED(data);
    }
#endif

#ifdef MCC_IR_H
    void mc_cleanup_delete_ir(int n, void* data){
            UNUSED(n);
            mcc_ir_delete_ir(data);
    }
#else
    void mc_cleanup_delete_ir(int n, void* data){
            UNUSED(n);
            UNUSED(data);
    }
#endif

#ifdef MCC_CFG_H
    void mc_cleanup_delete_cfg(int n, void* data){
            UNUSED(n);
            mcc_delete_cfg_and_ir(data);
    }

    void mc_cleanup_delete_block_chain(int n, void* data){
            UNUSED(n);
            mcc_delete_blockchain_and_ir(data);
    }
#else
    void mc_cleanup_delete_cfg(int n, void* data){
            UNUSED(n);
            UNUSED(data);
    }

    void mc_cleanup_delete_block_chain(int n, void* data){
            UNUSED(n);
            UNUSED(data);
    }
#endif


#endif // MC_CLEANUP_INC
