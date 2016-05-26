# Measures performance of $TEST_NAMES using `valgrind --tool=massif`.
#
# Depends on:
#   * common.sh
#   * `valgrind --tool=massif`
#   * $TEST_NAMES
#   * $PERF_DIR
#   * $BUILDCFG
#   * $ARCHBITS
#
# Honors:
#   * $START_TIME
measure_with_massif() {
	BUILDCFG="optimized"
	SCONSFLAGS="--ignore-missing --with-src-boost=3rdparty/boost --with-src-zlib=3rdparty/zlib --with-bin-openssl=3rdparty/openssl --build-cfg=$BUILDCFG --archbits=$ARCHBITS" # --warnlevel=medium --enable-Trace" # --config=force
	build `cat $TEST_NAMES`

	##
	# MEASUREMENT
	#
	progress "VALGRIND-MASSIF-BASED MEASUREMENT ..."
	for TESTNAME in `cat $TEST_NAMES`
	do
		echo "#------------------------------------------------------------------------" >&2
		echo "# Running test $TESTNAME ... ########################" >&2
		RESULT=$PERF_DIR/massif/${TESTNAME}-${ARCHBITS}_${BUILDCFG}.massif
        RUNPARAMS="-x valgrind -x '--tool=massif' -x '--time-unit=B' -x '--detailed-freq=1' -x '--massif-out-file=$RESULT'"
		mkdir -p `dirname $RESULT`
		( set -x; scons CXXFLAGS="-fPIC" $SCONSFLAGS --run-force --runparams="${RUNPARAMS} -- -all" $TESTNAME >$PERF_DIR/massif/${TESTNAME}-${ARCHBITS}_${BUILDCFG}.log 2>&1) || { echo "Test $TESTNAME with ARCHBITS=$ARCHBITS failed." >&2; exit 1; }
	done
}
