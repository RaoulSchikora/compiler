#include <stdbool.h>
#include "mc_builtins.c"
typedef const char* string;
/* This is a comment */
int main(){
	print("Please enter a number: ");
	print_nl();
	int n;
	n = read_int();
	bool a = 1 <= n;
	bool b = 2 == n;
	b = 3 >= n;
	n = n * 2;
	n = n / 2;
	n = n + 2;
	n = n - 2;
	a && b;
	a || b;
	a != !b;
	int test[3];
	string hi = "Hello";
	float f = -1.0;
	bool c = true;
	if (c)
	{
		while(c){
			c = false;
		}
	}
	if (c){
		f = f * 2;
	}
	else{
		f = f * 3;
	}
	return 0;
}
