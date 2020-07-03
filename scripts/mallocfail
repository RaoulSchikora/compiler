#!/bin/bash

function print_usage(){
		echo ""
		echo "Usage: ./mallocfail <repetitions> <program> <argument to program>..."
		echo ""
		echo "Test <program> for error handling capabilities, by repeatedly running gdb <repetitions> times and 
letting individual mallocs fail systematically."
		echo "If an error arises, gdb will ask if you want to quit. Hit 'n' and have a look at the call stack."
		echo "If no SIGSEV or similar errors arise, gdb will quit and try again."
		echo ""
		echo "Makes use of the mallocfail library: https://github.com/ralight/mallocfail."
		echo "Assumes that the library is installed at /usr/local/lib/mallocfail.so"
		echo ""
		echo "Note: <program> should be compiled with the debug option for gdb."
		exit
}

if [[ "$1" == "-h" ]]
	then
		print_usage
fi

# Clear previous run
if [[ -a mallocfail_hashes ]]
	then
		rm mallocfail_hashes
fi

x=$1
# Execute gdb
while [[ $x -ge 0 ]]
do 
	gdb -ex run -ex quit --args env LD_PRELOAD=/usr/local/lib/mallocfail.so $2 $3;
	echo $x
	x=$(( $x - 1 ))
done

# Remove the list of hashes to clear the state for the next run
if [[ -a mallocfail_hashes ]]
	then
		rm mallocfail_hashes
fi
