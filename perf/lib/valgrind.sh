# Measures performance of $TEST_NAMES using `valgrind`.
#
# Depends on:
#   * common.sh
#   * `valgrind`
#   * $TEST_NAMES
#   * $PERF_DIR
#   * $BUILDCFG
#   * $ARCHBITS
#
# Honors:
#   * $START_TIME
measure_with_valgrind() {
	BUILDCFG="debug"
	SCONSFLAGS="--ignore-missing --with-src-boost=3rdparty/boost --with-src-zlib=3rdparty/zlib --with-bin-openssl=3rdparty/openssl --build-cfg=$BUILDCFG --archbits=$ARCHBITS" # --warnlevel=medium --enable-Trace" # --config=force
	build `cat $TEST_NAMES`
	warmup `cat $TEST_NAMES`

	##
	# MEASUREMENT
	#
	progress "VALGRIND-BASED MEASUREMENT ..."
	for TESTNAME in `cat $TEST_NAMES`
	do
		echo "#------------------------------------------------------------------------" >&2
		echo "# Running test $TESTNAME ... ########################" >&2
		RESULT=$PERF_DIR/valgrind/${TESTNAME}-${ARCHBITS}_${BUILDCFG}.valgrind
		mkdir -p `dirname $RESULT`
		touch $RESULT # needed
		( set -x; scons $SCONSFLAGS --run-force --runparams="-x valgrind -x '--leak-check=full' -x '--log-file=$RESULT' -x '--show-leak-kinds=all' -x -v -- -all" $TESTNAME >$PERF_DIR/valgrind/${TESTNAME}-${ARCHBITS}_${BUILDCFG}.log 2>&1) || { echo "Test $TESTNAME with ARCHBITS=$ARCHBITS failed." >&2; exit 1; }
	done
}
