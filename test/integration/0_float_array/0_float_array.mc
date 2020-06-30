int main(){
	float[21] arr1;
	float[42] arr2;
	float i;
	i = 0.0;
	int i_copy;
	i_copy = 0;
	while(i<21.0){
		arr1[i_copy] = 0.0;
		arr2[2*i_copy] = 0.0;
		arr2[2*i_copy + 1] = 0.0;
		i = i + 1.0;
		i_copy = i_copy + 1;
	}
	print_float(arr1[0]);
	print_nl();
	print_float(arr2[1]);
	print_nl();
	square_array(arr1, arr2);
	print_float(arr1[20]);
	print_nl();
	print_float(arr2[41]);
	print_nl();
	return 0;
}

void square_array(float[21] arr1, float[42] arr2){
	float i;
	i = 0.0;
	int i_copy;
	i_copy = 0;
	while(i<21.0){
		arr1[i_copy] = i*i;
		arr2[2*i_copy] = i*i;
		arr2[2*i_copy + 1] = i*i + 1.0;
		i = i + 1.0;
		i_copy = i_copy + 1;
	}
}
