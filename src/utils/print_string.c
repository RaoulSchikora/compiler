#include "utils/print_string.h"

#include <string.h>

void mcc_print_string_literal(FILE *out, const char *string, bool doubly_escaped)
{
	int i = 0;
	while (string[i] != '\0') {
		switch (string[i]) {
		case '\n':
			if (doubly_escaped)
				fprintf(out, "\\\\n");
			else
				fprintf(out, "\\n");
			break;
		case '\t':
			if (doubly_escaped)
				fprintf(out, "\\\\t");
			else
				fprintf(out, "\\t");
			break;
		case '\\':
			if (doubly_escaped)
				fprintf(out, "\\\\\\\\");
			else
				fprintf(out, "\\\\");
			break;
		default:
			fprintf(out, "%c", string[i]);
			break;
		}
		i++;
	}
}

