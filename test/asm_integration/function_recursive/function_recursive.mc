int main(){
	int a;
	a = 2;
	int depth;
	depth = 10;
	int b;
	b = sum(a, depth);
	if(b == 20){
		return 1;
	} 
	return 0;
}

int sum(int a, int depth){
	if (depth == 0){
		return 0;
	}
	return a + sum(a, depth - 1);
}
