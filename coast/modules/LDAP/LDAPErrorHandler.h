/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _LDAPErrorHandler_H
#define _LDAPErrorHandler_H

#include "LDAPAPI.h"
#include "Mapper.h"

//! Handles LDAP Errors
class LDAPErrorHandler
{
public:
	enum eRetryState { eRetry, eIsInRetrySequence, eNoRetry };
	LDAPErrorHandler(Context &ctx, ParameterMapper *getter, ResultMapper *putter, String daName);
	virtual ~LDAPErrorHandler() {}

	// handle session error with message
	virtual void HandleSessionError(LDAP *ldap, String msg);

	// handle session error without disposing of LDAPErrorHandler  object
	static void HandleUnbindError(LDAP *ldap);

	// handle other error (general implementation)
	virtual void HandleError(String msg, Anything args = Anything(), String argDescr = "AdditionalInfo");

	// get last error description of the associated DAImpl
	// (returns false, if none was found)
	virtual bool GetLDAPError(Anything &error);

	ROAnything GetQueryParams();
	ROAnything GetConnectionParams();

	void PutQueryParams(Anything qp);
	void PutConnectionParams(Anything cp);

	//! Set ShouldRetry flag This flag is used when LDAPPooledConnections are used
	virtual void SetRetryState(eRetryState retryState);
	//! Query ShouldRetry flag
	virtual eRetryState GetRetryState();
	//! Set retry state when not in a retry sequence, otherwise don't set the retry state
	//! The two cases need to be distinguished because otherwise the connection handle would
	//! not be released in the case where the retry itself failes.
	virtual void SetShouldRetry();

	//! Clean up LDAPError and LDAPResultSlot in Context's tmp store
	virtual void CleanUp();

	//! Translate eRetryState into string
	String RetryStateAsString(eRetryState retryState);

	//! Is retry state set to eRetry ?
	bool IsRetry();

protected:
	Context &fCtx;
	ParameterMapper *fIn;
	ResultMapper *fOut;
	String fName;
	Anything fQueryParams, fConnectionParams, fErrorSuppressCodes;
	eRetryState fRetryState;

private:
	void PutLDAPError(Anything &error);
	void WriteSysLog(Anything &error, String const &msg);
	String CreateMessageAsString(Anything &error);
};

#endif
