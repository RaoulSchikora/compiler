#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "mcc/ast.h"
#include "mcc/ast_print.h"
#include "mcc/parser.h"
#include "mcc/ast_visit.h"


#define BUF_SIZE 1024

enum mcc_ast_to_dot_mode{
    MCC_AST_TO_DOT_MODE_TEST,
    MCC_AST_TO_DOT_MODE_PROGRAM,
};

void print_usage(const char *prg);
char *fileToString(char *filename);
char *stdinToString();
static int readInputAndSetMode(int argc, char *argv[]);

enum mcc_ast_to_dot_mode ast_to_dot_mode;
char* input;


int main(int argc, char *argv[]) {
    // set mode and read input
    if (readInputAndSetMode(argc, argv) == EXIT_FAILURE)
        return EXIT_FAILURE;

    struct mcc_parser_result result;
    struct mcc_parser_result *ptr_result;

    // handle entry point dependend on ast_to_dot_mode
    switch (ast_to_dot_mode) {
        case MCC_AST_TO_DOT_MODE_TEST: ;

            // parsing phase - entry point set as expression. Actual entry point is set while parsing
            result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_EXPRESSION);
            ptr_result = &result;
            if (ptr_result->status != MCC_PARSER_STATUS_OK) {
                return EXIT_FAILURE;
            }

            mcc_ast_print_dot_result(stdout, ptr_result);

            // cleanup
            mcc_ast_delete_result(ptr_result);

            return EXIT_SUCCESS;
        case MCC_AST_TO_DOT_MODE_PROGRAM: ;

            // parsing phase
            result = mcc_parse_string(input, MCC_PARSER_ENTRY_POINT_PROGRAM);
            ptr_result = &result;
            if (ptr_result->status != MCC_PARSER_STATUS_OK) {
                return EXIT_FAILURE;
            }

            mcc_ast_print_dot_result(stdout, ptr_result);

            // cleanup
            mcc_ast_delete_result(ptr_result);

            return EXIT_SUCCESS;
    }
}

void print_usage(const char *prg)
{
    printf("usage: %s [OPTIONS] file...\n\n", prg);
    printf("Utility for printing an abstract syntax tree in the DOT format. The output\n");
    printf("can be visualised using graphviz. Errors are reported on invalid inputs.\n\n");
    printf("Use '-' as input file to read from stdin.\n\n");
    printf("OPTIONS:\n");
    printf("  -h, --help                displays this help message\n");
    printf("  -o, --output <file>       write the output to <file> (defaults to stdout)\n");
    printf("  -f, --function <name>     limit scope to the given function\n");
    printf("  -t, --test                testing purpose\n");
}

// from: https://stackoverflow.com/questions/174531
char *fileToString(char *filename) {
    FILE *f = fopen(filename, "rt");
    assert(f);
    fseek(f, 0, SEEK_END);
    long length = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buffer = (char *) malloc(length + 1);
    buffer[length] = '\0';
    //TODO error handling fread
    fread(buffer, 1, length, f);
    fclose(f);
    return buffer;
}

// from: https://stackoverflow.com/questions/2496668
char *stdinToString(){
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


static int readInputAndSetMode(int argc, char *argv[])
{
    if (argc < 2) {
        // print help
        print_usage(argv[0]);
        return EXIT_FAILURE;
    } else if (argc == 2){
        if (strcmp("-h", argv[1]) == 0 || strcmp("--help", argv[1]) == 0){
            // print help
            print_usage(argv[0]);
            return EXIT_FAILURE;
        } else if (strcmp("-", argv[1]) == 0) {
            // read from stdin
            input = stdinToString();
            ast_to_dot_mode = MCC_AST_TO_DOT_MODE_PROGRAM;
        } else if (strcmp("-t", argv[1]) == 0){
            // read from stdin in testing mode
            input = stdinToString();
            ast_to_dot_mode = MCC_AST_TO_DOT_MODE_TEST;
        } else {
            // read file
            input = fileToString(argv[1]);
            ast_to_dot_mode = MCC_AST_TO_DOT_MODE_PROGRAM;
        }
    } else if (argc == 3) {
        if (strcmp("-h", argv[1]) == 0 || strcmp("--help", argv[1]) == 0){
            // print help
            print_usage(argv[0]);
            return EXIT_FAILURE;
        } else if (strcmp("-t", argv[1]) == 0){
            // read from file in testing mode
            input = fileToString(argv[2]);
            ast_to_dot_mode = MCC_AST_TO_DOT_MODE_TEST;
        } else {
            // print help
            printf("error: unkown option");
            print_usage(argv[0]);
            return EXIT_FAILURE;
        }
    } else {
        // print help
        printf("error: too many options");
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

