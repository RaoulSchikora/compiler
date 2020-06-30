int main(){
	float[42] arr;
	int i;
	i = 0;
	float i_f;
	i_f = 0.0;
	while (i<42){
		arr[i] = i_f*i_f;
		i = i + 1;
		i_f = i_f + 1.0;
	}
	i = i - 1;
	while(arr[i] > 399.999){
		print_float(arr[i]);
		print_nl();
		i = i - 1;
	}
	arr[i] = arr[i] + 40.0;
	arr[i+1] = arr[i] - arr[i-1];
	print_float(arr[i]);
	print_nl();
	print_float(arr[i+1]);
	print_nl();
	print_float(20.0*20.0 / arr[10] * arr[4]);
	print_nl();
	return 0;
}
