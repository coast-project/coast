# Measures performance of $TEST_NAMES using `time`.
#
# Depends on:
#   * common.sh
#   * `time`
#   * $TEST_NAMES
#   * $PERF_DIR
#   * $BUILDCFG
#   * $ARCHBITS
#   * $NTIMES
#
# Honors:
#   * $START_TIME
#   * $TIMES
measure_with_time() {

	SCONSFLAGS="--ignore-missing --with-src-boost=3rdparty/boost --with-src-zlib=3rdparty/zlib --with-bin-openssl=3rdparty/openssl --build-cfg=$BUILDCFG --archbits=$ARCHBITS" # --warnlevel=medium --enable-Trace" # --config=force 
	TIMES=${TIMES:-20} # number of times to run each test

	build `cat $TEST_NAMES`
	warmup `cat $TEST_NAMES`

	##
	# MEASUREMENT
	#
	progress "TIME-BASED MEASUREMENT ..."
	for TESTNAME in `cat $TEST_NAMES`
	do
		echo "#------------------------------------------------------------------------" >&2
		echo "# Running test $TESTNAME ... ########################" >&2
		TIME_RESULT=$PERF_DIR/time/${TESTNAME}-${ARCHBITS}_${BUILDCFG}.csv
		mkdir -p `dirname $TIME_RESULT`

		# write CSV header if file doesn't exist yet
		if [ ! -e $TIME_RESULT ];
		then
			echo "user[s],system[s],real[s],cpu,maxres[KB],avgres[KB],avgunshared[KB],avgunsharedstack[KB],swaps" > $TIME_RESULT
		fi
		echo "# $START_TIME" >> $TIME_RESULT

		# run test n times and log statistics
		( set -x; scons $SCONSFLAGS --run-force --runparams="-x /usr/bin/time -x '-f' -x '%U,%S,%e,%P,%M,%t,%D,%p,%W' -x '--output=$TIME_RESULT' -x '--append' -x $NTIMES -x $TIMES -- -all" $TESTNAME >$PERF_DIR/time/${TESTNAME}-${ARCHBITS}_${BUILDCFG}.log 2>&1) || { echo "Test $TESTNAME with ARCHBITS=$ARCHBITS failed." >&2; exit 1; }
	done
}
