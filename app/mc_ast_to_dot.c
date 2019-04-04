#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "mcc/ast.h"
#include "mcc/ast_print.h"
#include "mcc/parser.h"

#define BUF_SIZE 1024

enum mcc_ast_to_dot_mode{
    MCC_AST_TO_DOT_MODE_EXPRESSION,
    MCC_AST_TO_DOT_MODE_STATEMENT,
//    MCC_AST_TO_DOT_MODE_PROGRAM,
//    MCC_AST_TO_DOT_MODE_VARIABLE_DECLARATION,
};

void print_usage(const char *prg)
{
	printf("usage: %s <FILE>\n\n", prg);
	printf("  <FILE>        Input filepath or - for stdin\n");
}

// from: https://stackoverflow.com/questions/174531/how-to-read-the-content-of-a-file-to-a-string-in-c
char *fileToString(char *filename) {
    FILE *f = fopen(filename, "rt");
    assert(f);
    fseek(f, 0, SEEK_END);
    long length = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buffer = (char *) malloc(length + 1);
    buffer[length] = '\0';
    fread(buffer, 1, length, f);
    fclose(f);
    return buffer;
}

//from: https://stackoverflow.com/questions/2496668/how-to-read-the-standard-input-into-string-variable-until-eof-in-c
char* stdinToString(){
    char buffer[BUF_SIZE];
    size_t contentSize = 1; // includes NULL
    /* Preallocate space. */
    char *content = malloc(sizeof(char) * BUF_SIZE);
    if(content == NULL)
    {
        perror("Failed to allocate content");
        exit(1);
    }
    content[0] = '\0'; // make null-terminated
    while(fgets(buffer, BUF_SIZE, stdin))
    {
        char *old = content;
        contentSize += strlen(buffer);
        content = realloc(content, contentSize);
        if(content == NULL)
        {
            perror("Failed to reallocate content");
            free(old);
            exit(2);
        }
        strcat(content, buffer);
    }

    if(ferror(stdin))
    {
        free(content);
        perror("Error reading from stdin.");
        exit(3);
    }

    return content;
}

int main(int argc, char *argv[])
{

	enum mcc_ast_to_dot_mode ast_to_dot_mode;
	char* input;

	if (argc < 2) {
		print_usage(argv[0]);
		return EXIT_FAILURE;
	}
	if (argc == 2){
		if (strcmp("-", argv[1]) == 0) {
			input = stdinToString();
			ast_to_dot_mode = MCC_AST_TO_DOT_MODE_EXPRESSION;
		} else if (strcmp("-h", argv[1]) == 0 || strcmp("--help", argv[1]) == 0){
	        print_usage(argv[0]);
	        return EXIT_FAILURE;
	    } else if (strcmp("-e", argv[1]) == 0){
			input = stdinToString();
			ast_to_dot_mode = MCC_AST_TO_DOT_MODE_EXPRESSION;
		} else if (strcmp("-v", argv[1]) == 0){
//			input = stdinToString();
//			ast_to_dot_mode = MCC_AST_TO_DOT_MODE_VARIABLE_DECLARATION;
		} else if (strcmp("-s", argv[1]) == 0){
            input = stdinToString();
            ast_to_dot_mode = MCC_AST_TO_DOT_MODE_STATEMENT;
		}else {
//            input = fileToString(argv[1]);
//            ast_to_dot_mode = MCC_AST_TO_DOT_MODE_PROGRAM;
            }
    } else if (argc == 3) {
        if (strcmp("-h", argv[1]) == 0 || strcmp("--help", argv[1]) == 0){
            print_usage(argv[0]);
            return EXIT_FAILURE;
        } else if (strcmp("-e", argv[1]) == 0){
            input = fileToString(argv[2]);
            ast_to_dot_mode = MCC_AST_TO_DOT_MODE_EXPRESSION;
        } else if (strcmp("-v", argv[1]) == 0){
//            input = fileToString(argv[2]);
//            ast_to_dot_mode = MCC_AST_TO_DOT_MODE_VARIABLE_DECLARATION;
        } else if (strcmp("-s", argv[1]) == 0){
            input = fileToString(argv[2]);
            ast_to_dot_mode = MCC_AST_TO_DOT_MODE_STATEMENT;
        } else {
            print_usage(argv[0]);
            return EXIT_FAILURE;
            }
    } else {
	    input = "fail";
	    printf(input);
        print_usage(argv[0]);
        return EXIT_FAILURE;
	}

	switch (ast_to_dot_mode) {
    // MCC_PASER_ENTRY_POINT_EXPRESSION
    case MCC_AST_TO_DOT_MODE_EXPRESSION:;
        struct mcc_ast_expression *expr = NULL;

        // parsing phase
        {
            struct mcc_parser_result result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_EXPRESSION);
            if (result.status != MCC_PARSER_STATUS_OK) {
                return EXIT_FAILURE;
            }
            expr = result.expression;
        }

        mcc_ast_print_dot(stdout, expr);

        // cleanup
        mcc_ast_delete(expr);

        return EXIT_SUCCESS;
    // MCC_PASER_ENTRY_POINT_STATEMENT
    case MCC_AST_TO_DOT_MODE_STATEMENT: ;

        struct mcc_ast_statement *stmt = NULL;

        // parsing phase
        {
            struct mcc_parser_result result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_STATEMENT);
            if (result.status != MCC_PARSER_STATUS_OK) {
                return EXIT_FAILURE;
            }
            stmt = result.statement;
        }

        mcc_ast_print_dot(stdout, stmt);

        // cleanup
        mcc_ast_delete(stmt);

        return EXIT_SUCCESS;
    }

	    //TODO: below
//	case MCC_AST_TO_DOT_MODE_VARIABLE_DECLARATION: ;
//	    struct mcc_ast_variable_declaration *variable = NULL;
//
//	    {
//	        struct mcc_parser_result result = mcc_parse_file(in,MCC_PARSER_ENTRY_POINT_VARIABLE_DECLARATION);
//	        fclose(in);
//	        if (result.status != MCC_PARSER_STATUS_OK) {
//	            return EXIT_FAILURE;
//	        }
//	        variable = result.variable_declaration;
//	    }
//
//	    mcc_ast_print_dot(stdout, variable);
//
//        // cleanup
//        mcc_ast_delete(variable);
//
//        return EXIT_SUCCESS;
//
//	// MCC_PARSER_ENTRY_POINT_PROGRAN
//	case MCC_AST_TO_DOT_MODE_PROGRAM: ;
//		struct mcc_ast_program *program = NULL;
//
//		{
//			struct mcc_parser_result result = mcc_parse_file(in,MCC_PARSER_ENTRY_POINT_PROGRAM);
//			fclose(in);
//			if (result.status != MCC_PARSER_STATUS_OK) {
//				return EXIT_FAILURE;
//			}
//			program = result.program;
//		}
//
//		mcc_ast_print_dot(stdout, program);
//
//		// cleanup
//		mcc_ast_delete(program);
//
//		return EXIT_SUCCESS;
//	}
}
