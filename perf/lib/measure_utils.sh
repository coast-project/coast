#! /bin/sh

##
# FUNCTIONS
#

# Shows the one-size-fits-all usage for performance measurement scripts.
show_usage() {
	${PAGER:-less --quit-if-one-screen} <<EOF
SYNOPSIS
--------
	(1) $0
	(2) $0 TEST...
	(3) $0 --all-tests
	(4) $0 --diff=TEST
	(5) $0 --export [--all-tests | TEST...]

The purpose of this script is to build and run test suites and measure
performance.  The results are saved into files in under ./perf_results
to be able to compare the differences between 32-bit and 64-bit
builds.


USAGE
-----
(1) Measures the predefined set of core tests (SA's mandatory goal).
(2) Measures the given tests.
(3) Measures all available tests (determined dynamically using scons)
(4) Prints a command which would show the performance
    differences of 32/64-bit builds for the given test.
(5) Extracts common values from results and prints them as CSV.


MEASUREMENT METHODS
-------------------
Depending on which tool you're running, the measurement method is:

	with_time	=>	time
	with_perf	=>	perf (linux-tools)
	with_perf_stat	=>	perf stat (linux-tools)
	with_valgrind	=>	Valgrind
	with_massif	=>	Valgrind massif tool


PREREQUISITES
-------------
	* virtual ENV for COAST has to be activated already


OPTIONS
-------
	-a --all-tests	run all tests available (otherwise core only)
	-d --diff=TEST	show command to get performance difference of TEST
	-e --export	export common values results as CSV
	-h --help	show this help and exit


ENVIRONMENT VARIABLES
---------------------
The following env variables are honored (with default value):

	* PERF_DIR		"./perf_results"
	* ALL_ARCHBITS		"64 32"
	* TEST_NAMES		temp file, content depending on options/args
	* TIMES			"20"

TIMES is currently only used by with_time. It defines how many times to run a
test to get a more accurate timing measurement.
EOF
}

# Removes the temporary file containing the test names.
cleanup() {
	rm -f "$TEST_NAMES"
}


# Prints tests to be built and measured.
print_plan() (
	echo "# Tests to be run:" >&2
	sed -e 's/^/#	/' "$TEST_NAMES" >&2
	ntests=$(wc -l < "$TEST_NAMES")
	echo "# ($ntests tests in total)" >&2
)

# Prints progress information and build-related variables being used on STDERR.
progress() {
	cat <<EOF >&2

##############################################################################
# $1
#
# ARCHBITS=$ARCHBITS
# BUILDCFG=$BUILDCFG
# SCONSFLAGS=$SCONSFLAGS
##############################################################################
EOF
}

# Build the tests given as arguments.
build() {
	progress "BUILDING ..."
	log_cmd scons $SCONSFLAGS "$@"
}

# Informas about warmup of test and runs it. Counts and informs about warmup
# failures.
warmup() {
	echo "#------------------------------------------------------------------------" >&2
	echo "# (WARMUP) Running test $1 ... ###################################" >&2
	# shellcheck disable=SC2086
	scons $SCONSFLAGS --run-force $1
	scons_exit_code=$?
	echo "SCons exited with code: $scons_exit_code" >&2
	if [ "$scons_exit_code" -ne "0" ]
	then
		echo "#!!! WARMUP of test $1 failed." >&2
		FAILED_WARMUPS=$((FAILED_WARMUPS + 1))
		return 1
	fi
	:
}

# Measures performance for selected archbits, then exits with appropriate error
# code.
#
# Depends on:
#   * $ALL_ARCHBITS
#   * measure_performance (function)
#   * $FAILED_WARMUPS
#   * $FAILED_MEASUREMENTS
#   * $SUCCESSFUL_MEASUREMENTS
measure_performance_for_selected_archbits() {
	print_plan

	for ARCHBITS in $ALL_ARCHBITS
	do
		measure_performance
	done

	echo "############  MEASUREMENT STATISTICS ##########################" >&2
	echo "#" >&2
	echo "# $FAILED_WARMUPS tests failed during warmup." >&2
	echo "# $FAILED_MEASUREMENTS tests failed during measurement." >&2
	echo "# $SUCCESSFUL_MEASUREMENTS tests have been measured successfully." >&2
	echo "#" >&2
	echo "# Performance measurement done. ################################" >&2

	[ "$FAILED_MEASUREMENTS" -gt 0 ] && exit 1
	[ "$FAILED_WARMUPS"      -gt 0 ] && exit 1
	exit 0
}

# Informs about the given test being measured.
start() {
	echo "#------------------------------------------------------------------------" >&2
	echo "# Measuring test $1 ... ########################" >&2
}

# Informs that the measurement of the given test has been successful and counts
# that.
#
# Depends on:
#   * $SUCCESSFUL_MEASUREMENTS
measurement_successful() {
	echo "# Measured test $1 successfully. ########################" >&2
	SUCCESSFUL_MEASUREMENTS=$((SUCCESSFUL_MEASUREMENTS + 1))
}

# Informs about a test that has failed and counts that.
#
# Depends on:
#   * $1 = test name
#   * $2 = log file for diagnostics
#   * $FAILED_MEASUREMENTS
measurement_failed() {
	echo "#!!! Measurement of test $1 failed." >&2
	echo "#!!! For diagnostics, check the log: $2" >&2
	FAILED_MEASUREMENTS=$((FAILED_MEASUREMENTS + 1))
}

# Logs command (set -x style) and returns its exit code.
log_cmd() (
	set -x
	"$@"
)

##
# INIT
#

set -e # abort on first error

START_TIME=$(date +"%Y-%m-%d %H:%M:%S") # date (suitable for CSV)
DIFF_TEST=                             # a test name
EXPORT=false                           # whether to export existing results as CSV
RUN_ALL_TESTS=false                    # whether all tests available should be run
TEST_NAMES="${TEST_NAMES:-}"           # path to file containing test names to be run
ALL_ARCHBITS="${ALL_ARCHBITS:-64 32}"  # path to file containing test names to be run


##
# Processes options and command line arguments.
#

# check and reorder arguments
set -- $(getopt --unquoted -o hd:e -l help,diff:,export -- "$@")

# process options
while [ -n "$1" ]
do
	case "$1" in
	-a|--all-tests)
		RUN_ALL_TESTS=true
		shift
		;;
	-h|--help)
		show_usage
		exit 0
		;;
	-d|--diff)
		DIFF_TEST="$2"
		shift 2
		;;
	-e|--export)
		EXPORT=true
		shift
		;;
	--)
		shift
		break
	esac
done

# process other arguments
if [ -n "$1" ]
then
	if $RUN_ALL_TESTS
	then
		echo "Option --all-tests is incompatible with test names given as arguments." >&2
		exit 1
	fi

	if [ -n "$TEST_NAMES" ]
	then
		echo "Warning: Shadowing \$TEST_NAMES environment variable." >&2
		echo "Using set of tests provided on command line instead." >&2
	fi


	# fill given test names into file
	TEST_NAMES=$(mktemp)
	for test_name in "$@"
	do
		echo "$test_name" >> "$TEST_NAMES"
	done
fi

# absolute path to ntimes utility
NTIMES="$(realpath -e "$(dirname "$0")"/lib/ntimes)"

PERF_DIR="${PERF_DIR:-"$PWD/perf_results"}"
mkdir -p "$PERF_DIR"

# go to root directory of COAST
cd "$(dirname "$0")"; until [ -e SConstruct ]; do cd ..; done

# determine list of tests if none have been passed
if [ -z "$TEST_NAMES" ]
then
	TEST_NAMES=$(mktemp)

	if $RUN_ALL_TESTS
	then
		# get list of ALL tests available
		echo "Getting list of all available tests ..." >&2
		scons -u --showtargets | grep '^ - .*Test$'| cut -d' ' -f3 > "$TEST_NAMES"
	else
		# use set of core tests
		echo "Using pre-defined set of core tests ..." >&2
		cat perf/lib/core_tests.txt > "$TEST_NAMES"
	fi
fi

# performance measurement statistics
FAILED_WARMUPS=0
FAILED_MEASUREMENTS=0
SUCCESSFUL_MEASUREMENTS=0
