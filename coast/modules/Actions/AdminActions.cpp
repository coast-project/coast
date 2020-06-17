/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "AdminActions.h"

#include "Anything.h"
#include "Context.h"
#include "Server.h"
#include "Threads.h"
#include "Tracer.h"

//---- ServerManagement ---------------------------------------------------------------
RegisterAction(ServerManagement);

ServerManagement::ServerManagement(const char *name) : Action(name) {}

bool ServerManagement::DoAction(String &action, Context &c) {
	Server *server = c.GetServer();

	if (server != 0) {
		StartTrace(Admin.DoAction);
		TraceAny(c.GetQuery(), "query");

		if (action == "ServerKill") {
			//			server->Abort();

		} else if (action == "ServerShutdown") {
			//			server->PrepareShutdown();

		} else if (action == "ServerReset") {
			//			server->Reset();

		} else if (action == "ConnectionsAllowed") {
			//			Anything roleStore= c.GetRoleStoreGlobal();
			//			roleStore["connectionsAllowed"]= true;	// poor man's simulation of old Coast feature
			//			server->EnableConnections();

		} else if (action == "ConnectionsDisallow") {
			//			Anything roleStore= c.GetRoleStoreGlobal();
			//			roleStore["connectionsAllowed"]= false;	// poor man's simulation of old Coast feature
			//			server->DisableConnections();
		}
	}

	return true;
}
