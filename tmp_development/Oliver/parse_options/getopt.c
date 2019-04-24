#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

struct options{
	bool to_file;
	char *output;
	bool print_help;
	bool limited_scope;
	char *function;
};

struct arguments{
	int size;
	char **args;
};

int parse_options(struct options *options, int argc, char *argv[]);
void print_struct(struct options *options);
void print_usage(void);
struct arguments *parse_arguments(int argc, char *argv[]);
void print_arguments(struct arguments *arguments);

int main(int argc, char *argv[]){

	struct options options = {
		.to_file = false,
		.output = NULL,
		.print_help = false,
		.limited_scope = false,
		.function = NULL
	};

	parse_options(&options,argc,argv);
	print_struct(&options);
	if(options.print_help == true){
		print_usage();
	}

	struct arguments *args = parse_arguments(argc,argv);
	print_arguments(args);
	free(args);

	return EXIT_SUCCESS;
}

void print_arguments(struct arguments *arguments){
	int i = 0;
	while(arguments->size >	i){
		printf("Arg #%d:\t%s\n",i+1,*(arguments->args + i));
		i++;
	}
			 
}


struct arguments *parse_arguments(int argc, char *argv[]){
	int i = optind;
	char **args = malloc (sizeof(char*) * (argc-optind));

	while(i < argc){
		*(args + i - optind) = argv[i];
		i++;
	}
	struct arguments *arguments = malloc (sizeof(arguments));
	if (arguments == NULL){
		perror("malloc");
	}
	arguments->args = args;
	arguments->size = argc-optind;
	return arguments;
}

void print_usage(void){
	printf("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n\n");
	printf("\t\t\t USAGE\n\n");
	printf("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n");
}

int parse_options(struct options *options, int argc, char *argv[])
{
	int c;
	while ((c = getopt(argc,argv,"o:hf:")) != -1){
		switch(c){
			case 'o':
				options->to_file = true;
				options->output = optarg;
				break;
			case 'h':
				options->print_help = true;
				break;
			case 'f':
				options->limited_scope = true;
				options->function = optarg;
				break;
			default:
				options->print_help = true;
				break;
		}
	}	

	return 0;	
}

void print_struct(struct options *options)
{
	printf("to_file: \t\t %d\n",options->to_file);
	printf("output:\t\t\t %s\n",options->output);
	printf("print_help: \t\t %d\n",options->print_help);
	printf("limited_scope: \t\t %d\n",options->limited_scope);
	printf("function: \t\t %s\n",options->function);
}
