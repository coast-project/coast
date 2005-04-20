/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _SybaseModule_H
#define _SybaseModule_H

//---- WDModule include -------------------------------------------------
#include "config_sybasect.h"
#include "WDModule.h"

class SybCTPoolManager;

//---- SybaseModule ----------------------------------------------------------
//: comment action
//	Structure of config:
//<PRE>	{
//		/SybaseServerName1 {
//			/ParallelQueries	long	defines number of parallel sql queries which can be issued, default 5
//		}
//      ...
//	}</PRE>
class EXPORTDECL_SYBASECT SybaseModule : public WDModule
{
public:
	//--- constructors
	SybaseModule(const char *name);
	~SybaseModule();

	//:implementers should initialize module using config
	virtual bool Init(const Anything &config);
	//:implementers should terminate module expecting destruction
	virtual bool Finis();

	// function to get workerpool for a server
	SybCTPoolManager *GetPoolManager(const char *pServerName);

//    //:initializes module after termination for reinitialization; default uses Init; check if this applies
//    virtual bool ResetInit(const Anything &config);
//    //:terminates module for reinitialization; default uses Finis; check if this applies
//    virtual bool ResetFinis(const Anything &config);

private:
	Anything	fWorkerPools;
	bool		fHasDAImpls;
	bool		fHasNewDAImpls;
};

#endif
