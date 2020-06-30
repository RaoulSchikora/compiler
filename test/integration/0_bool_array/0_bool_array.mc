int main(){
	bool[42] even;
	bool[42] uneven;
	int i;
	i = 0;
	bool b;
	b = true;
	while(i<42){
		even[i] = b;
		uneven[i] = !b;
		b = !b;
		i = i + 1;
	}
	print_bool_array(even);
	print_nl();
	print_bool_array(uneven);
	print_nl();
	bool[42] result;
	xor_arrays(even, uneven, result);
	print_bool_array(result);
	print_nl();
	return 0;
}

void print_bool_array(bool[42] arr){
	int i;
	i = 0;
	while (i<42){
		if(arr[i])
			print_int(1);
		if(!arr[i])
			print_int(0);
		i = i + 1;	
	}
}

void xor_arrays(bool[42] arr1, bool[42] arr2, bool[42] result){
	int i;
	i = 0;
	while(i<42){
		result[i] = (arr1[i] || arr2[i]) && (!arr1[i] || !arr2[i]);
		i = i + 1;
	}
}
