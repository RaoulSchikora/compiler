#include "utils/length_of_int.h"

// returns the length of the given int
int length_of_int(int num)
{
	if (num == 0)
		return 1;
	if (num <= 0)
		return floor(log10((-1) * num)) + 2;
	return floor(log10(num)) + 1;
}