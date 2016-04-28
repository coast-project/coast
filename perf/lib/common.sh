#! /bin/sh

##
# FUNCTIONS
#

is_usage_wanted() {
	[ "$1" = '--help' ] || [ "$1" = '-h' ]
}

show_common_usage() {
	cat <<EOF
Synopsis: (1) $0
          (2) $0 TEST...
          (2) $0 --diff=TEST

The purpose of this script is to build and run test suites and measure
performance.  The results are saved into files in the perf_results/ directory
(in CWD) to be able to compare the differences between 32-bit and 64-bit
builds.

Depending on which tool you're running, the measurement method is:

	whith_time	=>	time
	with_perf	=>	perf (linux-tools)
	with_valgrind	=>	Valgrind
	with_massif	=>	Valgrind massif tool

Prerequisites:

  * virtual ENV for COAST has to be activated already

Options:
	-h --help	show this help and exit
	-d --diff=TEST	show command to get performance difference of TEST

Usage:
------
(1) will run all tests known to work on 32-bit and 64-bit.
(2) will run just the given tests.
(3) will print a command which, when executed, will show you the performance
    differences of 32/64-bit builds for the given test.
EOF
}

cleanup() {
	rm -f $ALL_TESTS
}


# Prints tests to be run and gets the confirmation from the user.
get_confirmation() {
	echo "# Tests to be run:  ######################################" >&2
	sed -e 's/^/	/' $ALL_TESTS >&2
	ntests=`wc -l < $ALL_TESTS`
	echo "# ($ntests tests in total)  #############################" >&2
	echo "# Press return to confirm or ^C to abort..." >&2
	read confirmation
}

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

build() {
	progress "BUILDING ..."
	( set -x; scons $SCONSFLAGS "$@" )
}

warmup() {
	progress "WARMING UP ..."
	for TESTNAME in "$@"
	do
		echo "#------------------------------------------------------------------------" >&2
		echo "# (WARMUP) Running test $TESTNAME ... ###################################" >&2
		( set -x; scons $SCONSFLAGS --run-force $TESTNAME)
	done
}

parse_opts() {
	while [ -n "$1" ]
	do
		case "$1" in
		-h|--help)
			show_common_usage
			exit 0;;
		-d|--diff)
			DIFF_TEST=$2
			shift 2
			;;
		--)
			return
		esac
	done
}

##
# INIT
#

set -e # abort on first error

DIFF_TEST=
parse_opts `getopt --unquoted -o hd: -l help,diff: -- "$@"`

NTIMES="$(realpath -e `dirname $0`/lib/ntimes)" # absolute path to ntimes utility

PERF_DIR="$PWD/perf_results"
mkdir -p $PERF_DIR

# list of tests to run
ALL_TESTS=`mktemp`

cat <<EOF > $ALL_TESTS
CoastStorageTest
CoastEBCDICTest
CoastFoundationAnythingOptionalTest
CoastFoundationMiscellaneousTest
CoastFoundationIOTest
CoastFoundationTest
CoastFoundationBaseTest
CoastFoundationTimeTest
CoastMTFoundationTest
CoastSystemFunctionsTest
CoastRegexTest
CoastFoundationPerfTest
EOF

cd `dirname $0`/.. # root directory of COAST

# gets list of all tests
#scons -u --showtargets | grep '^ - .*Test$'| cut -d' ' -f3 > $ALL_TESTS

START_TIME=`date`
