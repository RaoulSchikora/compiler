#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

int main(void)
{

	union Scope {
		struct {
			int doors;
			int housenumber;
			char *name;
			bool on_fire;
		};
	} Scope;
	Scope.doors = 3;
	Scope.name = "Karl";
	printf("House has %d doors and is called %s\n", Scope.doors, Scope.name);

	return EXIT_SUCCESS;
}
