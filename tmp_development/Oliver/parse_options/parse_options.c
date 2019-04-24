#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct options {
  bool output_specified;
  char *output;
  bool scope_limited;
  char *function;
  bool file_input;
  char *inputs;
};

void print_struct(struct options option);
int parse_options(int argc, char *argv[], struct options *option);
void print_usage(void);
char* copy_args_into_array(int number, char *argv[]);

int main(int argc, char *argv[]) {

  struct options option = {
      .output_specified = false,
      .output = NULL,
      .scope_limited = false,
      .function = NULL,
      .file_input = false,
      .inputs = NULL,
  };

  parse_options(argc, argv, &option);
  print_struct(option);

  return EXIT_SUCCESS;
}

int parse_options(int argc, char *argv[], struct options *option) {

	int opt;
	while ((opt = getopt(argc, argv, "ho:f:")) != -1){
		switch(opt) {
			case 'o':
				option->output_specified = true;
				option->output = optarg;
				break;
			case 'h':
				print_usage();
				return 0;
				break;
			case 'f':
				option->scope_limited = true;
				option->function = optarg;
				break;
			default:
				print_usage();
				return 1;
				break;
		}
	}
	return 0;
}

void print_struct(struct options option) {

  printf("output_specified: %d\n", option.output_specified);
  printf("output: %s\n", option.output);
  printf("scope_limited: %d\n", option.scope_limited);
  printf("function: %s\n", option.function);
  printf("file_input: %d\n", option.file_input);
  printf("inputs: %s\n", option.inputs);
  return;
}

void print_usage(void) {
  printf("usage: %s [OPTIONS] file...\n\n", "mc_ast_to_dot");
  printf("Utility for printing an abstract syntax tree in the DOT format. "
         "The output\n");
  printf("can be visualised using graphviz. Errors are reported on invalid "
         "inputs.\n\n");
  printf("Use '-' as input file to read from stdin.\n\n");
  printf("OPTIONS:\n");
  printf("  -h, --help                displays this help message\n");
  printf("  -o, --output <file>       write the output to <file> (defaults "
         "to stdout)\n");
  printf("  -f, --function <name>     limit scope to the given function\n");
  return;
}

char* copy_args_into_array(int number, char *argv[]){
	return NULL;
}
