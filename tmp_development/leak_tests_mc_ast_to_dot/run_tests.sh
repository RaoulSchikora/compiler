#!/bin/bash
val="valgrind --error-exitcode=3 --leak-check=full --track-origins=yes --show-leak-kinds=all"
print="../../builddir/mc_ast_to_dot"
success=0

# testing the parse of input1 - input3 
$val $print input_test input_test2 input_test3 2> log1.txt > /dev/null
if [ $? -eq 3  ]
	then 
		success=1
		echo "leak check #1 failed"
fi

# testing the parse of input1 - input3 with -f set to first function
$val $print -f test1 input_test input_test2 input_test3 2> log2.txt > /dev/null
if [ $? -eq 3  ]
	then 
		success=1
		echo "leak check #2 failed"
fi

# testing the parse of input1 - input3 with -f set to second function
$val $print -f test2 input_test input_test2 input_test3 2> log3.txt > /dev/null
if [ $? -eq 3  ]
	then 
		success=1
		echo "leak check #3 failed"
fi

# testing the parse of input1 - input3 with -f set to last function
$val $print -f test3 input_test input_test2 input_test3 2> log4.txt > /dev/null
if [ $? -eq 3  ]
	then 
		success=1
		echo "leak check #4 failed"
fi

# testing the parse of erronous input_error
$val $print input_error 2> log5.txt > /dev/null
if [ $? -eq 3  ]
	then 
		success=1
		echo "leak check #5 failed"
fi

if [ $success -eq 0  ]
	then
		echo "All leak checks were successfull"
fi
