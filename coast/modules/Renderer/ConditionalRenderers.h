/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ConditionalRenderers_H
#define _ConditionalRenderers_H

#include "config_renderer.h"
#include "Renderer.h"

// ---- ConditionalRenderer ---------------------------------------------------------
//! <B>This renderer checks the value of a specified slot in the Context and - dependingon the outcome of the test - renders a predefined renderer specification.</B>
/*!
<B>Configuration:</B><PRE>
{
	/ContextCondition	Rendererspec	mandatory, resulting a String used to Lookup the Context. (Lookuped value is usually an integer)
	/True				Rendererspec	optional, if ContextCondition-value evaluates to true, the content of this slot will be rendered
	/False				Rendererspec	optional, if ContextCondition-value evaluates to false, the content of this slot will be rendered
	/Defined			Rendererspec	optional, if ContextCondition-value is fefined in Context, the content of this slot will be rendered
	/Undefined			Rendererspec	optional, if ContextCondition-value not defined in Context, the content of this slot will be rendered
	/Error				Rendererspec	optional, if the renderer received invalid input, the content of this slot will be rendered
}
</PRE>
*/
class EXPORTDECL_RENDERER ConditionalRenderer : public Renderer
{
public:
	ConditionalRenderer(const char *);

	void RenderAll(ostream &reply, Context &c, const ROAnything &config);

protected:
	/*! TestCondition does the actual testing. Result of test is returned via res. Errors are signaled using the value "Error".
		\param context context to work with
		\param res conditions test result */
	virtual void TestCondition(Context &context, const ROAnything &args, String &res);
};

// ---- SwitchRenderer ---------------------------------------------------------
//! <B>The SwitchRenderer provides an indirection depending on some value in the Context</B>
/*!
<B>Configuration:</B><PRE>
{
	/ContextLookupName	Rendererspec	mandatory, resulting a String used to Lookup the Context
	/Case {								mandatory, list of different cases to compare ContextLookupName-value with
		/xxx			Rendererspec	optional, if context.Lookup("ContextLookupName-value") == xxx, the content of this slot will be rendered
		/yyy			Rendererspec	optional, if context.Lookup("ContextLookupName-value") == yyy, the content of this slot will be rendered
		/''				Rendererspec	optional, if context.Lookup("ContextLookupName-value") is empty, the content of this slot will be rendered, Important: you must use single quotes when defining Anythings empty slotname!
	}
	/Default			Rendererspec	optional, for all unspecified conditions (not listed in Case slot), the content of this slot will be rendered
}
</PRE>
*/
class EXPORTDECL_RENDERER SwitchRenderer : public Renderer
{
public:
	SwitchRenderer(const char *);

	void RenderAll(ostream &reply, Context &c, const ROAnything &config);
};

#endif	//not defined _ConditionalRenderers_H
