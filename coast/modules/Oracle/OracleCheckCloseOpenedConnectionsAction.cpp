/*
 * Copyright (c) 2009, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "OracleCheckCloseOpenedConnectionsAction.h"

#include "Context.h"
#include "OracleEnvironment.h"
#include "OracleModule.h"

RegisterAction(OracleCheckCloseOpenedConnectionsAction);

bool OracleCheckCloseOpenedConnectionsAction::DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config) {
	StartTrace(OracleCheckCloseOpenedConnectionsAction.DoExecAction);
	OracleModule *pModule = SafeCast(WDModule::FindWDModule("OracleModule"), OracleModule);
	coast::oracle::ConnectionPool *pConnectionPool(0);
	if ((pModule != 0) && ((pConnectionPool = pModule->GetConnectionPool()) != 0)) {
		return pConnectionPool->CheckCloseOpenedConnections(ctx.Lookup("PeriodicActionTimeout", 60L));
	}
	return false;
}
