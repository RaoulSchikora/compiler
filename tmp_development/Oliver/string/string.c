#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *mcc_transform_into_unit_test(char *unit_test_string)
{

	char *mcc_unit_test_input;
	mcc_unit_test_input = (char *)malloc((strlen(unit_test_string) + 2) * sizeof(char));
	*mcc_unit_test_input = '~';
	strcpy(mcc_unit_test_input + 1, unit_test_string);
	*(mcc_unit_test_input + strlen(unit_test_string) + 1) = '~';
	*(mcc_unit_test_input + strlen(unit_test_string) + 2) = '\0';

	return mcc_unit_test_input;
}

int main(void)
{

	printf("%s\n", "test");
	printf("%s\n", mcc_transform_into_unit_test("test"));

	return EXIT_SUCCESS;
}
