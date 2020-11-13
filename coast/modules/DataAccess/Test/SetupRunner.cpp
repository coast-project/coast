/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "ConfigMapperTest.h"
#include "DataAccessImplTest.h"
#include "DataAccessTest.h"
#include "DataMapperTest.h"
#include "LoopBackDAImplTest.h"
#include "MapperTest.h"
#include "NameUsingOutputMapperTest.h"
#include "ParameterMapperTest.h"
#include "ReadFileDAImplTest.h"
#include "RegExpFilterFieldsParameterMapperTest.h"
#include "RegExpFilterFieldsResultMapperTest.h"
#include "RegExpSearchReplaceResultMapperTest.h"
#include "RenderedKeyResultMapperTest.h"
#include "RendererMapperTest.h"
#include "ResultMapperTest.h"
#include "SlotnameOutputMapperTest.h"
#include "StreamingAnythingMapperTest.h"
#include "TestRunner.h"
#include "WriteFileDAImplTest.h"

void setupRunner(TestRunner &runner) {
	ADD_SUITE(runner, DataAccessTest);
	ADD_SUITE(runner, DataAccessImplTest);
	ADD_SUITE(runner, MapperTest);
	ADD_SUITE(runner, ParameterMapperTest);
	ADD_SUITE(runner, ResultMapperTest);
	ADD_SUITE(runner, DataMapperTest);
	ADD_SUITE(runner, RendererMapperTest);
	ADD_SUITE(runner, LoopBackDAImplTest);
	ADD_SUITE(runner, NameUsingOutputMapperTest);
	ADD_SUITE(runner, ConfiguredActionTest);
	ADD_SUITE(runner, SlotnameOutputMapperTest);
	ADD_SUITE(runner, ReadFileDAImplTest);
	ADD_SUITE(runner, WriteFileDAImplTest);
	ADD_SUITE(runner, ConfigMapperTest);
	ADD_SUITE(runner, StreamingAnythingMapperTest);
	ADD_SUITE(runner, RegExpFilterFieldsResultMapperTest);
	ADD_SUITE(runner, RegExpSearchReplaceResultMapperTest);
	ADD_SUITE(runner, RenderedKeyResultMapperTest);
	ADD_SUITE(runner, RegExpFilterFieldsParameterMapperTest);
}  // setupRunner
