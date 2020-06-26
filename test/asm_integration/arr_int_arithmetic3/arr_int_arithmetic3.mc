int main(){

	int [3] b;
	int [5] a;
	b[0] = 1;
	b[1] = 2;
	b[2] = 13;
	a[ b[1-1]*2 + b[1] ] = b[2];
	return a[4];
}
