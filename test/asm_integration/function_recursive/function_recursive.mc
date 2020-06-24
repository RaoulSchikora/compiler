int main(){
	int a;
	a = 2;
	int depth;
	depth = 10;
	int b;
	b = rec_func(a, depth);
	if(b == 20){
		return 1;
	} else {
		return 0;
	}
}

int rec_func(int result, int depth){
	if (depth >= 0){
		result = result + rec_func(result, depth - 1);
	}
	return result;
}
