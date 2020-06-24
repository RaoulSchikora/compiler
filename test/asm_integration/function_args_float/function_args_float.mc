float times2(float f){
float f2;
f2 = 2.0 * f;
return f2;
}

int main(){
float f;
f = 2.2;
float f2;
f2 = times2(f);
if(f2 == 4.4){
	return 1;
} else {
	return 0;
}
}
