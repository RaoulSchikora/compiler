int main(){
	string[6] arr;
	arr[0] = "Don't";
	arr[1] = "worry";
	arr[2] = ",";
	arr[3] = "be";
	arr[4] = "happy";
	arr[5] = "!";
	print_string_array(arr);
	return 0;
}

void print_string_array(string[6] arr){
	int i;
	i = 0;
	while(i<6){
		print(arr[i]);
		if(i != 1 && i < 4)
			print(" ");
		i = i + 1;
	}
	print_nl();
}
