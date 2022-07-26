/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "StatUtils.h"

#include "Anything.h"
#include "StringStream.h"
#include "SystemLog.h"

StatEvtHandler::StatEvtHandler() {}

StatEvtHandler::~StatEvtHandler() {}

void StatEvtHandler::HandleStatEvt(long evt) {
	DoHandleStatEvt(evt);
}

void StatEvtHandler::Statistic(Anything &statElements) {
	DoStatistic(statElements);
}

long StatEvtHandler::GetTotalRequests() {
	return DoGetTotalRequests();
}

long StatEvtHandler::GetCurrentParallelRequests() {
	return DoGetCurrentParallelRequests();
}

StatGatherer::StatGatherer() {}

StatGatherer::~StatGatherer() {}

void StatGatherer::Statistic(Anything &statElements) {
	DoGetStatistic(statElements);
}

void StatGatherer::PrintStatisticsOnStderr(const String &strName) {
	Anything statistic;
	DoGetStatistic(statistic);
	String strbuf;
	{
		StringStream stream(strbuf);
		if (strName.Length() != 0) {
			stream << "Statistics for [" << strName << "] ";
		}
		statistic.PrintOn(stream) << "\n";
	}
	SystemLog::WriteToStderr(strbuf);
}

StatObserver::StatObserver() {}

StatObserver::~StatObserver() {}

void StatObserver::Register(const String &name, StatGatherer *pGatherer) {
	DoRegisterGatherer(name, pGatherer);
}
