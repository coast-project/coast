# Measures performance of $ALL_TESTS using `perf`.
#
# Depends on:
#   * common.sh
#   * `perf`
#   * $TEST_NAMES
#   * $PERF_DIR
#   * $BUILDCFG
#   * $ARCHBITS
measure_with_perf() {
	SCONSFLAGS="--ignore-missing --with-src-boost=3rdparty/boost --with-src-zlib=3rdparty/zlib --with-bin-openssl=3rdparty/openssl --build-cfg=$BUILDCFG --archbits=$ARCHBITS" # --warnlevel=medium --enable-Trace" # --config=force 
	build `cat $TEST_NAMES`
	warmup `cat $TEST_NAMES`

	##
	# MEASUREMENT
	#
	progress "PERF-BASED MEASUREMENT ..."
	for TESTNAME in `cat $TEST_NAMES`
	do
		echo "#------------------------------------------------------------------------" >&2
		echo "# Running test $TESTNAME ... ########################" >&2
		RESULT=$PERF_DIR/perf/${TESTNAME}-${ARCHBITS}_${BUILDCFG}.perf
		mkdir -p `dirname $RESULT`
		( set -x; scons $SCONSFLAGS --run-force --runparams="-x perf -x record -x -F99 -x -g -x '-o$RESULT' -- -all" $TESTNAME >$PERF_DIR/perf/${TESTNAME}-${ARCHBITS}_${BUILDCFG}.log 2>&1) || { echo "Test $TESTNAME with ARCHBITS=$ARCHBITS failed." >&2; exit 1; }
	done
}
