#!/bin/bash
val="valgrind --error-exitcode=3 --leak-check=full --track-origins=yes --show-leak-kinds=all"
print="../../builddir/mc_ast_to_dot"
success=0
input="int test1(){return a;int a; a = 1; } int test2(){ return a; } void test3(int a){ a =1; return; }"
input_error="int test1(){return a;;int a;} a = 1; } int {test2(){ return a; } void test3(int a){ a =1; return; }"




# testing the parse of input1 - input3 
$val $print input_test input_test2 input_test3 2> log1.txt > /dev/null
if [ $? -eq 3  ]
	then 
		success=1
		echo "leak check #1 failed"
	else
		echo "leak check #1 successfull"
fi

# testing the parse of input1 - input3 with -f set to first function
$val $print -f test1 input_test input_test2 input_test3 2> log2.txt > /dev/null
if [ $? -eq 3  ]
	then 
		success=1
		echo "leak check #2 failed"
	else
		echo "leak check #2 successfull"
fi

# testing the parse of input1 - input3 with -f set to second function
$val $print -f test2 input_test input_test2 input_test3 2> log3.txt > /dev/null
if [ $? -eq 3  ]
	then 
		success=1
		echo "leak check #3 failed"
	else
		echo "leak check #3 successfull"
fi

# testing the parse of input1 - input3 with -f set to last function
$val $print -f test3 input_test input_test2 input_test3 2> log4.txt > /dev/null
if [ $? -eq 3  ]
	then 
		success=1
		echo "leak check #4 failed"
	else
		echo "leak check #4 successfull"
fi

# testing the parse of single input file 
$val $print input_test3 2> log5.txt > /dev/null
if [ $? -eq 3  ]
	then 
		success=1
		echo "leak check #5 failed"
	else
		echo "leak check #5 successfull"
fi

# testing the parse of stdin
$val $print - <<< $input 2>log6.txt > /dev/null
if [ $? -eq 3  ]
	then 
		success=1
		echo "leak check #6 failed"
	else
		echo "leak check #6 successfull"
fi

# testing the parse of stdin with -f set to first function
$val $print -f test1 - <<< $input 2>log7.txt > /dev/null
if [ $? -eq 3  ]
	then 
		success=1
		echo "leak check #7 failed"
	else
		echo "leak check #7 successfull"
fi

# testing the parse of stdin with -f set to second function
$val $print -f test2 - <<< $input 2>log8.txt > /dev/null
if [ $? -eq 3  ]
	then 
		success=1
		echo "leak check #8 failed"
	else
		echo "leak check #8 successfull"
fi

# testing the parse of stdin with -f set to last function
$val $print -f test3 - <<< $input 2>log9.txt > /dev/null
if [ $? -eq 3  ]
	then 
		success=1
		echo "leak check #9 failed"
	else
		echo "leak check #9 successfull"
fi

# testing the parse of erronous input_error from file
$val $print input_error 2> log_error_file.txt > /dev/null
if [ $? -eq 3  ]
	then 
		success=1
		echo "leak check of errornous input via file failed"
	else
		echo "leak check of errornous input via file successfull"
fi

# testing the parse of erronous input_error from stdin
$val $print - <<< $input_error 2> log_error_stdin.txt > /dev/null
if [ $? -eq 3  ]
	then 
		success=1
		echo "leak check of errornous input via stdin failed"
	else
		echo "leak check of errornous input via stdin successfull"
fi

if [ $success -eq 0  ]
	then
		echo "All leak checks were successfull"
fi
