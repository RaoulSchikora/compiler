#ifndef MCC_UTILS_PRINT_STRING_H
#define MCC_UTILS_PRINT_STRING_H

#include <stdbool.h>
#include <stdio.h>

// Prints null-terminated string to filehandle out and escapes "\", "\n", "\t" to "\\", "\\n", "\\t"
// If doubly_escaped is set to true, the backslashes are escaped twice: "\","\n","\t" become "\\\\", ""\\\\n", "\\\\t"
void mcc_print_string_literal(FILE *out, const char *string, bool doubly_escaped);

#endif // MCC_UTILS_PRINT_STRING_H

