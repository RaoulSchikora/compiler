int main(){
	float a;
	float b;
	a = 13.0;
	b = add_uneven(a);
	if(b == 91.0)
		return 1;
	return 0;
}

float add_uneven(float a){
	if(a <= 0.0){
		return 0.0;
	}
	return a + add_even(a-1.0);
}

float add_even(float a){
	if(a <= 0.0){
		return 0.0;
	}
	return a + add_uneven(a-1.0);
}
	
