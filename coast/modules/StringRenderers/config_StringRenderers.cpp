/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "InitFinisManager.h"

namespace {
	void Init()
	{
		InitFinisManager::IFMTrace(">> StringRenderers::Init\n");
		InitFinisManager::IFMTrace("<< StringRenderers::Init\n");
	}

	void Finis()
	{
		InitFinisManager::IFMTrace(">> StringRenderers::Finis\n");
		InitFinisManager::IFMTrace("<< StringRenderers::Finis\n");
	}
}

extern "C" void __attribute__ ((constructor)) my_StringRenderers_init()
{
	Init();
}
extern "C" void __attribute__ ((destructor)) my_StringRenderers_fini()
{
	Finis();
}
