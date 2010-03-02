/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "AuthenticationService.h"

//--- standard modules used ----------------------------------------------------
#include "System.h"
#include "SystemLog.h"
#include "SecurityModule.h"
#include "HTTPProtocolReplyRenderer.h"
#include "AccessManager.h"
#include "Dbg.h"
#include "BasicAuthenticationData.h"

//--- c-modules used -----------------------------------------------------------

//---- AuthenticationService ----------------------------------------------------------------
RegisterServiceHandler(AuthenticationService);

AuthenticationService::AuthenticationService(const char *authenticationServiceHandlerName )
	: ServiceHandler(authenticationServiceHandlerName)
{
	StartTrace(AuthenticationService.AuthenticationService);
}

AuthenticationService::~AuthenticationService()
{
	StartTrace(AuthenticationService.Dtor);
}

void AuthenticationService::DoHandleService( ostream &os, Context &ctx )
{
	StartTrace(AuthenticationService.DoHandleService);
	if ( DoCheck(ctx) ) {
		Trace( "DoCheck succeeded --> Calling ForwardToMainHandler" );
		ForwardToMainHandler(os, ctx);
	} else {
		Trace( "DoCheck failed --> Calling Produce401Response" );
		Produce401Response( os, ctx );
	}
}

bool AuthenticationService::DoCheck( Context &ctx )
{
	StartTrace(AuthenticationService.DoCheck);
	bool ret = false;

	String name;
	String pw;
	GetUserNameAndPw(ctx, name, pw);
	ret = AuthenticateUser(ctx, name, pw);
	Trace( "ret = [" << (long)ret << "]" );
	return ret;
}

void AuthenticationService::GetUserNameAndPw( Context &ctx, String &name, String &pw )
{
	StartTrace(AuthenticationService.GetUserNameAndPw);

	BasicAuthenticationData baDat(ctx.Lookup("header.AUTHORIZATION", ""));
	pw = baDat.GetPassword();
	name = baDat.GetUserName();

}

bool AuthenticationService::AuthenticateUser( Context &ctx, String &name, String &pw )
{
	StartTrace(AuthenticationService.AuthenticateUser);
	bool ret = false;
	if (name.Length()) {
		Trace("name given [" << name << "] pw [" << pw << "]");
		String strAccessMgrName = Lookup("AccessManager", "AccessManager");
		AccessManager *pMgr = AccessManagerModule::GetAccessManager(strAccessMgrName);
		Trace("requested AccessManager [" << strAccessMgrName << "]");
		if (pMgr) {
//			Anything anyUserName;
//			String strContextNameSlot = Lookup("AuthUserNameSlot", "AuthUserName");
//			String strContextPasswordSlot = Lookup("AuthPasswordSlot", "AuthPassword");
//			ctx.GetTmpStore()[strContextNameSlot] = name;
//			ctx.GetTmpStore()[strContextPasswordSlot] = pw;
			String strNewRole;
			ret = pMgr->Validate(name) ;
			if (ret) {
				ret = pMgr->AuthenticateWeak(name, pw, strNewRole);
				Trace( "strNewRole = [" << strNewRole << "] ret = [" << (long) ret << "]" );
			}
			Trace("new Role [" << strNewRole << "] for [" << name << "]");
		} else {
			SYSWARNING(String("requested AccessManager [") << strAccessMgrName << "] not found!");
		}
	}
	Trace( "ret = [" << (long)ret << "]" );
	return ret;
}

void AuthenticationService::ForwardToMainHandler( ostream &os, Context &ctx )
{
	StartTrace(AuthenticationService.ForwardToMainHandler);

	String service = Lookup("Service", "WebAppService");
	Trace("Forward to : " << service );
	ServiceHandler *sh = ServiceHandler::FindServiceHandler( service );
	if (sh) {
		sh->HandleService(os, ctx);
	} else {
		SYSWARNING(String("ServiceHandler [") << service << "] not found!");
	}
}

void AuthenticationService::Produce401Response( ostream &os, Context &ctx )
{
	StartTrace(AuthenticationService.Produce401Response);

	ctx.GetTmpStore()["HTTPStatus"]["ResponseCode"] = 401L;
	Renderer *pRenderer = Renderer::FindRenderer("HTTPProtocolReplyRenderer");
	if (pRenderer) {
		ROAnything roaDummy;
		pRenderer->RenderAll(os, ctx, roaDummy);
	}

	os << "WWW-Authenticate: Basic realm=\"";
	ROAnything realmConfig = ctx.Lookup("Realm");
	if (realmConfig.IsNull()) {
		os << "Coast";
	} else {
		Renderer::Render(os, ctx, realmConfig);
	}
	os << "\"" << ENDL << ENDL;

	ROAnything bodyConfig = ctx.Lookup("401MsgBody");
	if (bodyConfig.IsNull()) {
		os << "<html><h1>Please stay away</h1></html>";
	} else {
		Renderer::Render(os, ctx, bodyConfig);
	}
	ctx.GetTmpStore()["BasicAuthRetCode"] = 0;		// false
}
