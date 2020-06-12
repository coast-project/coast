/*
 * Copyright (c) 2010, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */
#include "MimeHeaderResultMapper.h"
#include "MIMEHeader.h"
#include "HTTPProcessor.h"
#include "Tracer.h"
#include <istream>

RegisterResultMapper(MimeHeaderResultMapper);

namespace {
	const char *normalizeKey = "Normalize";
}

bool MimeHeaderResultMapper::DoPutStream(const char *key, std::istream &is, Context &ctx, ROAnything script) {
	StartTrace1(MimeHeaderResultMapper.DoPutStream, "key [" << NotNull(key) << "]");
	coast::urlutils::NormalizeTag shift = static_cast<coast::urlutils::NormalizeTag>(Lookup(normalizeKey, static_cast<long>(coast::urlutils::eUpshift)));
	MIMEHeader mh(shift);
	try {
		if (mh.ParseHeaders(is) && is.good()) {
			Anything header(mh.GetHeaderInfo());
			TraceAny(header, "header");
			return DoPutAny(key, header, ctx, script);
		}
	} catch (MIMEHeader::StreamNotGoodException &e) {
	} catch (MIMEHeader::LineSizeExceededException &e) {
		coast::http::PutErrorMessageIntoContext(ctx, 413, String(e.what()).Append(" => check setting of [LineSizeLimit]"), e.fLine, "MimeHeaderResultMapper::DoPutStream");
	}
	return false;
}
