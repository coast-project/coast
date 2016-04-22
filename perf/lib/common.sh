#! /bin/sh

##
# FUNCTIONS
#

is_usage_wanted() {
	[ "$1" = '--help' ] || [ "$1" = '-h' ]
}

show_common_usage() {
	cat <<EOF
Synopsis: $0 [<TESTNAME> ...]

The purpose of this script is to build and run test suites and measure
performance.  The results are saved into files to be able to compare the
differences between 32-bit and 64-bit builds.

Depending on which tool you're running, the measurement method is:

	whith_time	=>	time
	with_perf	=>	perf (linux-tools)
	with_valgrind	=>	Valgrind

Prerequisites for all:

  * virtual ENV for COAST has to be activated already

Usage:
------
This will run all tests of COAST that are known to work:

	$ $0

This is how you can select a set of tests:

	$0 CoastFoundationBaseTest CoastRegexTest

It'll measure the performance of the test suite of both the 64-bit and
the 32-bit build.

The results go into files in the perf_results/ directory.
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

##
# INIT
#

set -e # abort on first error

NTIMES="$(realpath -e `dirname $0`/lib/ntimes)" # absolute path to ntimes utility

PERF_DIR="$PWD/perf_results"
mkdir -p $PERF_DIR

# list of tests to run
ALL_TESTS=`mktemp`

cd `dirname $0`/.. # root directory of COAST

# gets list of all tests
#scons -u --showtargets | grep '^ - .*Test$'| cut -d' ' -f3 > $ALL_TESTS

START_TIME=`date`
