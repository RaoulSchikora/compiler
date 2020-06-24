int main(){
int a;
a = 3;
int b;
b = 2;
int c;
c = mul(a, b);
if(c == 6){
	return 1;
} else {
	return 0;
}
}

int mul(int a, int b){
int c;
c = a*b;
return c;
}
