#!/bin/bash

# ------------------------------------------------------------ global variables

options_keep_hashes=false
options_repetitions=30
options_continue=false
options_arguments=""
MALLOCFAIL_LIB_DIR="/usr/local/lib/mallocfail.so"

# ------------------------------------------------------------ functions

print_usage(){
		echo ""
		echo "Usage: run_mallocfail.sh [-r repetitions] [-k] [-c] [-l /path/to/mallocfail.so] <program> <program arg>..."
		echo ""
		echo "Options:"
		echo "    -r: Number of repetitions. Defaults to 30."
		echo "    -k: Keep current state of which mallocs were failed already. Defaults to false."
		echo "    -c: Continue from previous run and skip already tested mallocs. Defaults to false."
		echo "    -l: Path to mallocfail.so. Defaults to /usr/local/lib/mallocfail.so."
		echo "    -h: Print usage info."
		echo ""
		echo "Test <program> for error handling capabilities, by running gdb <repetitions> times and 
letting individual mallocs fail systematically."
		echo "If an error arises, gdb will ask if you want to quit. Hit 'n' and have a look at the call stack."
		echo "If no SIGSEV or similar errors arise, gdb will quit and try again."
		echo ""
		echo "Makes use of the mallocfail library: https://github.com/ralight/mallocfail."
		echo "Assumes that the library is installed at /usr/local/lib/mallocfail.so"
		echo ""
		echo "<program> should be compiled with the debug option for gdb."
		exit
}

parse_options_and_check_args(){
	while getopts "kchr:l:" opt; do
		case ${opt} in
			h ) 
				print_usage
				exit 1
				;;
			k ) 
				options_keep_hashes=true
				;;
			r ) 
				options_repetitions=$OPTARG
				;;
			c ) 
				options_continue=true
				;;
			l ) 
				MALLOCFAIL_LIB_DIR=$OPTARG
				;;
			\? )
				print_usage
				exit 1
				;;
		esac
	done
	shift "$((OPTIND -1))"
	options_arguments="$@"

	if ! [[ 0 -lt $options_repetitions ]]
		then 
			echo "You specified less than 0 repetitions. See run_mallocfail.sh -h"
			exit 1
	fi
}

check_if_executable(){
	if ! [[ -x $1 ]]
		then 
			echo "$1 is not executable or does not exist."
			exit 1
	fi
}

# Credit: https://stackoverflow.com/a/8657833
run_gdb(){
	gdb -q -ex='set confirm on' -ex run -ex quit --args env LD_PRELOAD="$MALLOCFAIL_LIB_DIR" $@
}

gdb_loop(){
	x=$1
	shift
	check_if_executable $1
	while [[ $x -gt 0 ]]
	do 
		run_gdb "$@" 		
		x=$(( $x - 1 ))
		echo " --------------------------------------------------------------- $x runs left"
	done
}

clear_stored_hashes(){
	if [[ -a mallocfail_hashes ]]
		then
			rm mallocfail_hashes
	fi
}

cleanup_if_not(){
	if [[ $1 == "false" ]]
		then
			clear_stored_hashes
	fi
}

# ------------------------------------------------------------ main

parse_options_and_check_args "$@"
check_if_executable $MALLOCFAIL_LIB_DIR
cleanup_if_not "$options_continue"
gdb_loop "$options_repetitions" "$options_arguments"
cleanup_if_not "$options_keep_hashes"
exit 0

