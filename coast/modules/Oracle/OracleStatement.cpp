/*
 * Copyright (c) 2009, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "OracleStatement.h"

#include "OracleEnvironment.h"
#include "OracleException.h"
#include "OracleResultset.h"
#include "AnyIterators.h"
#include "Dbg.h"

OracleStatement::OracleStatement(OracleConnection *pConn, String const &strStmt) :
	fpConnection(pConn), fStmt(strStmt), fStatus(UNPREPARED), fStmtType(Coast_OCI_STMT_UNKNOWN)
{
}

OracleStatement::OracleStatement(OracleConnection *pConn, OCIStmt *phStmt) :
	fpConnection(pConn), fStmthp(phStmt), fStmt(), fStatus(PREPARED), fStmtType(Coast_OCI_STMT_BEGIN)
{
}

OracleStatement::~OracleStatement()
{
	Cleanup();
}

bool OracleStatement::AllocHandle()
{
	StartTrace(OracleStatement.AllocHandle);
	// allocates and returns new statement handle
	String strError(32L);
	bool bSuccess(!fpConnection->checkError(OCIHandleAlloc(fpConnection->getEnvironment().EnvHandle(),
											fStmthp.getVoidAddr(), (ub4) OCI_HTYPE_STMT, (size_t) 0, (dvoid **) 0), strError));
	if (!bSuccess) {
		fErrorMessages.Append(strError);
	}
	return bSuccess;
}

void OracleStatement::Cleanup()
{
	StartTrace(OracleStatement.Cleanup);
	AnyExtensions::Iterator<ROAnything> aAnyIter(fSubStatements);
	ROAnything roaObject;
	while (aAnyIter.Next(roaObject)) {
		OracleStatement *pStmt = SafeCast(roaObject.AsIFAObject(0), OracleStatement);
		if ( pStmt ) {
			delete pStmt;
		}
	}
	fSubStatements = Anything();
	//FIXME: do we need to cleanup sub-statement handle??
	// -> if not, we should call release on the fStmthp before resetting, not to delete it
	fStmthp.reset();
	fStatus = UNPREPARED;
}

OracleStatement::Status OracleStatement::execute(ub4 mode, ub4 iters)
{
	StartTrace(OracleStatement.execute);
	// executes a SQL statement (first row is also fetched only if iters > 0)
	// depending on the query type, we could use 'scrollable cursor mode' ( OCI_STMT_SCROLLABLE_READONLY )
	sword status = OCIStmtExecute(fpConnection->SvcHandle(), getHandle(), fpConnection->ErrorHandle(), iters, 0, NULL,
								  NULL, mode);
	if (status != OCI_SUCCESS) {
		throw OracleException(*fpConnection, status);
	} else {
		// how can we find out about the result type? -> see Prepare
		switch (fStmtType) {
			case Coast_OCI_STMT_SELECT:
				fStatus = RESULT_SET_AVAILABLE;
				break;
			case Coast_OCI_STMT_CREATE:
			case Coast_OCI_STMT_DROP:
			case Coast_OCI_STMT_ALTER:
				//FIXME: is this correct?
				fStatus = UPDATE_COUNT_AVAILABLE;
				break;
			case Coast_OCI_STMT_DELETE:
			case Coast_OCI_STMT_INSERT:
			case Coast_OCI_STMT_UPDATE:
				fStatus = UPDATE_COUNT_AVAILABLE;
				break;
			default:
				//FIXME: check if this makes sense
				fStatus = PREPARED;
		}
	}
	return fStatus;
}

sword OracleStatement::Fetch(ub4 numRows)
{
	StartTrace(OracleStatement.Fetch);
	// fetch another row
	return OCIStmtFetch2(getHandle(), fpConnection->ErrorHandle(), numRows, OCI_FETCH_NEXT, 0, OCI_DEFAULT);
}

unsigned long OracleStatement::getUpdateCount() const
{

	return 0UL;
}

bool OracleStatement::Prepare()
{
	StartTrace(OracleStatement.Prepare);
	// prepare SQL statement for execution
	String strErr(32L);
	bool bSuccess(AllocHandle());
	if (bSuccess) {
		if (!(bSuccess = !fpConnection->checkError(OCIStmtPrepare(getHandle(), fpConnection->ErrorHandle(),
						 (const text *) (const char *) fStmt, (ub4) fStmt.Length(), OCI_NTV_SYNTAX, OCI_DEFAULT), strErr))) {
			fErrorMessages.Append(strErr);
		} else {
			ub2 fncode;
			if (fpConnection->checkError(OCIAttrGet(getHandle(), OCI_HTYPE_STMT, (dvoid *) &fncode, 0,
													OCI_ATTR_STMT_TYPE, fpConnection->ErrorHandle()), strErr)) {
				fErrorMessages.Append(strErr);
				bSuccess = false;
			} else {
				fStmtType = (StmtType) fncode;
				Trace("statement type is " << (long)fncode);
				fStatus = PREPARED;
			}
		}
	}
	return bSuccess;
}

OracleResultset *OracleStatement::getResultset()
{
	StartTrace(OracleStatement.getResultset);
	OracleResultset *pResult(0);
	if (fStatus == RESULT_SET_AVAILABLE) {
		pResult = new OracleResultset(*this);
	} else {
		String strMessage("Error - getResultset failed, no resultset available, current status is ");
		strMessage << (long) fStatus;
		throw OracleException(*fpConnection, strMessage);
	}
	return pResult;
}

OracleResultset *OracleStatement::getCursor(long lColumnIndex)
{
	StartTrace1(OracleStatement.getCursor, "column index: " << lColumnIndex);
	OracleResultset *pResult(0);
	--lColumnIndex;
	if (lColumnIndex >= 0 && lColumnIndex < fBuffer.GetSize()) {
		Anything &desc = fBuffer[lColumnIndex], value;
		SubTraceAny(TraceDesc, desc, "desc at col:" << lColumnIndex)
		SubTrace(TraceColType, "column type is: " << desc["Type"].AsLong() << " indicator: " << desc["Indicator"].AsLong())
		SubTrace(TraceBuf, "buf ptr " << (long) (desc["RawBuf"].AsCharPtr()) << " length: " << (long) * ((ub2 *) desc["EffectiveLength"].AsCharPtr()));
		switch (desc["Type"].AsLong()) {
			case SQLT_RSET:
			case SQLT_CUR: {
				Trace("SQLT_RSET/SQLT_CUR")
				OCIStmt *phStmt = (OCIStmt *) desc["RawBuf"].AsCharPtr();
				Trace("retrieved statement handle pointer &" << (long)phStmt)
				if (phStmt) {
					OracleStatement *pStmt = new OracleStatement(fpConnection, phStmt);
					fSubStatements.Append( pStmt );
					pResult = new OracleResultset(*pStmt);
				}
				break;
			}
			default:
				Trace("wrong type accessed")
				break;
		}
	}
	return pResult;
}

ROAnything OracleStatement::GetOutputDescription()
{
	StartTrace( OracleStatement.GetOutputDescription );

	if (fanyDescription.IsNull()) {
		if (getStatementType() == OCI_STMT_SELECT) {
			OCIError *eh(getConnection()->ErrorHandle());

			OCIParam *mypard;
			ub2 dtype;
			ub4 data_len;
			text *col_name;
			ub4 col_name_len;

			// Request a parameter descriptor for position 1 in the select-list
			ub4 counter = 1;
			sword parm_status = OCIParamGet(getHandle(), OCI_HTYPE_STMT, eh, (void **) &mypard, counter);
			if (parm_status != OCI_SUCCESS) {
				throw OracleException(*getConnection(), parm_status);
			}

			// Loop only if a descriptor was successfully retrieved for
			// current  position, starting at 1

			while (parm_status == OCI_SUCCESS) {
				data_len = 0;
				col_name = 0;
				col_name_len = 0;

				// Retrieve the data type attribute
				parm_status = OCIAttrGet((dvoid *) mypard, OCI_DTYPE_PARAM, (dvoid *) &dtype, 0, OCI_ATTR_DATA_TYPE, eh);
				if (parm_status != OCI_SUCCESS) {
					throw OracleException(*getConnection(), parm_status);
				}

				parm_status = OCIAttrGet((dvoid *) mypard, OCI_DTYPE_PARAM, (dvoid *) &data_len, 0, OCI_ATTR_DISP_SIZE,
										 eh);
				if (parm_status != OCI_SUCCESS) {
					throw OracleException(*getConnection(), parm_status);
				}

				// Retrieve the column name attribute
				parm_status = OCIAttrGet((dvoid *) mypard, OCI_DTYPE_PARAM, (dvoid **) &col_name, (ub4 *) &col_name_len,
										 OCI_ATTR_NAME, eh);
				if (parm_status != OCI_SUCCESS) {
					throw OracleException(*getConnection(), parm_status);
				}

				Anything param;
				param["Name"] = String((char *) col_name, col_name_len);
				param["Type"] = dtype;
				param["Length"] = (int) data_len;
				param["Idx"] = (long) counter;
				fanyDescription.Append(param);

				// increment counter and get next descriptor, if there is one
				++counter;
				parm_status = OCIParamGet(getHandle(), OCI_HTYPE_STMT, eh, (void **) &mypard, counter);
			}
		}
	}
	TraceAny( fanyDescription, "column descriptions (" << fanyDescription.GetSize() << ")" )
	return ROAnything(fanyDescription);
}

bool OracleStatement::DefineOutputArea()
{
	StartTrace(OracleStatement.DefineOutputArea);
	// use fBuffer to allocate output area used by oracle library
	// to store fetched data (binary Anything buffers are allocated and
	// stored within the fBuffer structure... for automatic storage
	// management)

	OCIError *eh = getConnection()->ErrorHandle();
	OCIStmt *pStmthp(getHandle());

	fBuffer = fanyDescription.DeepClone();
	AnyExtensions::Iterator<Anything> aAnyIter(fBuffer);
	Anything anyCol;
	while (aAnyIter.Next(anyCol)) {
		String strColName(anyCol["Name"].AsString());
		Trace("colname@" << aAnyIter.Index() << " [" << strColName << "]");
		long lColIndex = anyCol["Idx"].AsLong();

		long len;
		if (anyCol["Type"].AsLong() == SQLT_DAT) {
			// --- date field
			anyCol["Length"] = 9;
			anyCol["Type"] = SQLT_STR;
			len = anyCol["Length"].AsLong() + 1;
		} else if (anyCol["Type"].AsLong() == SQLT_NUM) {
			anyCol["Length"] = (sword) sizeof(sword);
			anyCol["Type"] = SQLT_INT;
			len = anyCol["Length"].AsLong();
		} else {
			anyCol["Type"] = anyCol["Type"].AsLong();
			len = anyCol["Length"].AsLong() + 1;
		}

		// allocate space for the returned data
		Anything buf = Anything((void *) 0, len);
		anyCol["RawBuf"] = buf;

		// accocate space for NULL indicator
		Anything indicator = Anything((void *) 0, sizeof(OCIInd));
		anyCol["Indicator"] = indicator;

		// allocate space to store effective result size
		Anything effectiveSize = Anything((void *) 0, sizeof(ub2));
		anyCol["EffectiveLength"] = effectiveSize;

		OCIDefine *defHandle = 0;
		sword status = OCIDefineByPos(pStmthp, &defHandle, eh, lColIndex, (void *) buf.AsCharPtr(), len,
									  anyCol["Type"].AsLong(), (dvoid *) indicator.AsCharPtr(), (ub2 *) effectiveSize.AsCharPtr(), 0,
									  OCI_DEFAULT);
		if (status != OCI_SUCCESS) {
			throw OracleException(*getConnection(), status);
		}
	}
	return true;
}

String OracleStatement::getString(long lColumnIndex)
{
	StartTrace1(OracleStatement.getString, "col index: " << lColumnIndex);
	String strColValue(32L);
	// oracle always uses 1-based indexes...
	--lColumnIndex;
	if (lColumnIndex >= 0 && lColumnIndex < fBuffer.GetSize()) {
		Anything &desc = fBuffer[lColumnIndex], value;
		SubTraceAny(TraceDesc, desc, "desc at col:" << lColumnIndex)
		SubTrace(TraceColType, "column type is: " << desc["Type"].AsLong() << " indicator: " << desc["Indicator"].AsLong())
		SubTrace(TraceBuf, "buf ptr " << (long) (desc["RawBuf"].AsCharPtr()) << " length: " << (long) * ((ub2 *) desc["EffectiveLength"].AsCharPtr()));
		switch (desc["Type"].AsLong()) {
			case SQLT_INT:
				Trace("SQLT_INT")
				if (desc["Indicator"].AsLong() == OCI_IND_NULL) {
					value = 0L;
				} else {
					value = (*((sword *) desc["RawBuf"].AsCharPtr()));
				}
				break;
			case SQLT_FLT:
				Trace("SQLT_FLT")
				if (desc["Indicator"].AsLong() == OCI_IND_NULL) {
					value = 0.0f;
				} else {
					value = (*((float *) desc["RawBuf"].AsCharPtr()));
				}
				break;
			case SQLT_STR:
				Trace("SQLT_STR")
				if (desc["Indicator"].AsLong() == OCI_IND_NULL) {
					value = "";
				} else {
					value = String(desc["RawBuf"].AsCharPtr());
				}
				break;
			default:
				SubTraceBuf(TraceBuf, desc["RawBuf"].AsCharPtr(), *((ub2 *) desc["EffectiveLength"].AsCharPtr()));
				if (desc["Indicator"].AsLong() == OCI_IND_NULL) {
					value = "";
				} else {
					value = String(static_cast<void *> (const_cast<char *> (desc["RawBuf"].AsCharPtr())),
								   (long) * (reinterpret_cast<ub2 *> (const_cast<char *> (desc["EffectiveLength"].AsCharPtr()))));
				}
				break;
		}
		strColValue = value.AsString();
	}
	Trace("returning value [" << strColValue << "]")
	return strColValue;
}
