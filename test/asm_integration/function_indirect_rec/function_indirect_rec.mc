int main(){
	int a;
	int b;
	a = 13;
	b = add_uneven(a);
	if(b == 91)
		return 1;
	return 0;
}

int add_uneven(int a){
	if(a <= 0){
		return 0;
	}
	return a + add_even(a-1);
}

int add_even(int a){
	if(a <= 0){
		return 0;
	}
	return a + add_uneven(a-1);
}
	
