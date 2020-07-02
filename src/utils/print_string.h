#ifndef MCC_UTILS_PRINT_STRING_H
#define MCC_UTILS_PRINT_STRING_H

#include <stdio.h>

// Prints null-terminated string to filehandle out and escapes "\", "\n", "\t" to "\\", "\\n", "\\t"
void mcc_print_string_literal(FILE *out, const char *string);

#endif // MCC_UTILS_PRINT_STRING_H
