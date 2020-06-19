int main(){

	int b;
	int [5] a;
	b = 1;
	a[2*b+1] = 3;
	a[2*2 + 4 - 4] = 2;
	a[2] = a[3] - a[4];
	return a[2];
}
