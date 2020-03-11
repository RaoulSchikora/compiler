#include <stdio.h>
#include <stdlib.h>

void test ( int* a  ){
	*a = 3;
	return;
}

int main  (int argc, char* argv[]){
	int a = 4;
	printf("%d\n",a);
	test(&a);
	printf("%d\n",a);
}
