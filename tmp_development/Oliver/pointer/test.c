#include <stdio.h>
#include <stdlib.h>

void* test ( void **data  ){
	return data;
}

int main  (int argc, char* argv[]){
	void *a = (void*) malloc(sizeof(void*));
	test(a);
}
