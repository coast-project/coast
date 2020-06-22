/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "TableCompressor.h"

#include "Registry.h"
#include "StringStream.h"
#include "Tracer.h"

// ------------------- TableCompressor ---------------------------------------------
RegisterCompressor(TableCompressor);
RegisterAliasSecurityItem(tc, TableCompressor);

TableCompressor::TableCompressor(const char *name) : Compressor(name), fMap(Anything::ArrayMarker(), coast::storage::Global()) {
	StatTrace(TableCompressor.Ctor, "Name: <" << NotNull(name) << ">", coast::storage::Current());
}

TableCompressor::~TableCompressor() {
	StatTrace(TableCompressor.Dtor, "Name: <" << fName << ">", coast::storage::Current());
}

bool TableCompressor::Init(ROAnything config) {
	StartTrace(TableCompressor.Init);
	SubTraceAny(Config, config, "Config: ");

	ROAnything tableCompressorConfig;
	if (config.LookupPath(tableCompressorConfig, "SecurityModule.TableCompressor")) {
		MakeTable(tableCompressorConfig["Key2UriMap"], "Key2UriMap", config);
		MakeReverseTable(fMap["Key2UriMap"], "Key2UriMap", "Uri2KeyMap");
		MakeTable(tableCompressorConfig["Val2UriMap"], "Val2UriMap", config);
		MakeReverseTable(fMap["Val2UriMap"], "Val2UriMap", "Uri2ValMap");
		fMap["ValMapTags"] = tableCompressorConfig["ValMapTags"].DeepClone(coast::storage::Global());
		return true;
	}
	return false;
}

void TableCompressor::DoCompress(String &scrambledText, const Anything &dataIn) {
	StartTrace(TableCompressor.DoCompress);

	ROAnything keyTable(GetKey2UriMap());
	ROAnything valTable(GetVal2UriMap());
	ROAnything valMapTags(GetValMapTags());
	TraceAny(valTable, "value mappings");
	TraceAny(valMapTags, "value map tags");
	Anything dataOut;

	//	 compress it by short keys in table
	TraceAny(dataIn, "data to compress");
	long dataSz = dataIn.GetSize();
	for (long i = 0; i < dataSz; ++i) {
		const char *slotname = dataIn.SlotName(i);
		if (slotname != 0) {
			if (keyTable.IsDefined(slotname)) {
				Trace("compressing defined slot [" << slotname << "]");
				const char *extSlotname = keyTable[slotname].AsCharPtr();
				String intVal = dataIn[i].AsCharPtr();	// in cases we are interested in it is always a string
				// map some well defined entities e.g. role, page, action
				// to shorter names
				if (intVal.Length() > 0) {
					if (valMapTags.Contains(slotname)) {
						if (valTable.IsDefined(intVal)) {
							dataOut[extSlotname] = valTable[intVal].AsCharPtr();
						} else {
							dataOut[extSlotname] = dataIn[i];
						}
					} else {
						dataOut[extSlotname] = dataIn[i];
					}
					Trace("mapped [" << slotname << "] to [" << extSlotname << "] value [" << dataOut[extSlotname].AsString()
									 << "]");
				}
			} else {
				dataOut[slotname] = dataIn[i];
				Trace("adding undefined slot [" << slotname << "] value [" << dataOut[slotname].AsString() << "]");
			}
		} else {
			Trace("appending unnamed entry with value [" << dataIn[i].AsString() << "]");
			dataOut.Append(dataIn[i]);
		}
	}
	TraceAny(dataOut, "compressed output");
	OStringStream os(&scrambledText);
	dataOut.PrintOn(os, false);
}

bool TableCompressor::DoExpand(Anything &dataOut, const String &scrambledText) {
	StartTrace(TableCompressor.DoExpand);
	IStringStream is(scrambledText);
	ROAnything keyTable(GetUri2KeyMap());
	ROAnything valTable(GetUri2ValMap());
	ROAnything valMapTags(GetValMapTags());
	Anything dataIn;
	Trace(scrambledText);
	TraceAny(keyTable, "KeyTable: ");

	if (dataIn.Import(is)) {
		// expand it to long internal keys
		long dataSz = dataIn.GetSize();
		for (long i = 0; i < dataSz; ++i) {
			const char *slotname = dataIn.SlotName(i);
			if (slotname != 0) {
				if (keyTable.IsDefined(slotname)) {
					const char *intSlotname = keyTable[slotname].AsCharPtr();
					const char *compVal = dataIn[i].AsCharPtr();

					// expand some well known values also
					// e.g. role, page action names
					if (valMapTags.Contains(intSlotname)) {
						if (valTable.IsDefined(compVal)) {	// is the value really compressed
							dataOut[intSlotname] = valTable[compVal].AsCharPtr();
						} else {
							dataOut[intSlotname] = dataIn[i];
						}
					} else {
						dataOut[intSlotname] = dataIn[i];
					}
				} else {
					dataOut[slotname] = dataIn[i];
				}
			} else {
				dataOut.Append(dataIn[i]);
			}
		}
	}
	return true;
}

void TableCompressor::MakeTable(ROAnything baseState, const char *tag, ROAnything config) {
	StartTrace1(TableCompressor.MakeTable, "tag: " << tag);
	Anything state = baseState.DeepClone();
	TraceAny(state, "State: ");
	if (tag != 0) {
		long sz = state.GetSize(), i;
		Anything map;
		Anything aSlot;
		const char *slotname = 0;

		for (i = sz - 1; i >= 0; --i) {
			aSlot = state[i];
			if (aSlot.GetType() == AnyArrayType) {
				ExpandConfig(i, state, config);
			}
		}

		for (i = 0; i < sz; ++i) {
			aSlot = state[i];
			if (aSlot.GetType() == AnyCharPtrType) {
				slotname = aSlot.AsCharPtr("");
			}
			if (slotname != 0) {
				map[slotname] = CalcKey(i);
			}
			slotname = 0;
		}
		fMap[tag] = map;
	}
}

void TableCompressor::MakeReverseTable(ROAnything state, const char *tag, const char *reverseTag) {
	StartTrace1(TableCompressor.MakeTable, "tag: <" << tag << "> reverseTag: <" << reverseTag << ">");
	TraceAny(state, "State: ");

	if ((tag != 0) && (reverseTag != 0)) {
		long sz = state.GetSize();
		Anything revKeyTable;
		const char *slotname;

		for (long i = 0; i < sz; ++i) {
			slotname = state.SlotName(i);
			revKeyTable[state[i].AsCharPtr("")] = slotname;
		}

		fMap[reverseTag] = revKeyTable;
	}
}

void TableCompressor::ExpandConfig(long index, Anything &state, ROAnything config) {
	StartTrace(TableCompressor.ExpandConfig);
	Trace("index: " << index);
	TraceAny(state, "State: ");
	TraceAny(config, "Config: ");

	String tag(state[index].SlotName(0L));
	ROAnything toExpand;

	Trace("Tag: <" << tag << ">");
	if (tag == "Expand" && config.LookupPath(toExpand, state[index]["Expand"].AsCharPtr(""))) {
		state.Remove(index);
		InstallConfig(index, state, toExpand);
	}
}

void TableCompressor::InstallConfig(long index, Anything &state, ROAnything part) {
	StartTrace(TableCompressor.InstallConfig);
	Trace("index: " << index);
	TraceAny(state, "State: ");
	TraceAny(part, "Part: ");
	long sz = part.GetSize();
	const char *slotname = 0;
	for (long i = 0; i < sz; ++i) {
		slotname = part.SlotName(i);
		if (slotname != 0) {
			state[index] = slotname;
			++index;
			InstallConfig(index, state, part[i]);
		} else if (part[i].GetType() == AnyCharPtrType) {
			state[index] = part[i].AsCharPtr();
			++index;
		}
		slotname = 0;
	}
}

String TableCompressor::CalcKey(long index) {
	StartTrace1(TableCompressor.CalcKey, "index: " << index);
	String key;
	if ((index / 52) > 0) {
		key << CalcKey((index / 52) - 1);
	}
	key << KeyAt(index % 52);
	Trace("key: <" << key << ">");
	return key;
}

char TableCompressor::KeyAt(int index) {
	StartTrace1(TableCompressor.KeyAt, "index: " << (long)index);
	Assert((0 <= index) && (index < 52));
	if (index < 0) {
		index = 0;
	}
	if (index >= 52) {
		index = 51;
	}
	if (index < 26) {
		return (index + 97);
	}
	return (index + 65 - 26);
}

ROAnything TableCompressor::GetMap(const char *tag) const {
	ROAnything a;
	ROAnything(fMap).LookupPath(a, tag);
	return a;
}

ROAnything TableCompressor::GetKey2UriMap() const {
	return GetMap("Key2UriMap");
}

ROAnything TableCompressor::GetVal2UriMap() const {
	return GetMap("Val2UriMap");
}

ROAnything TableCompressor::GetUri2KeyMap() const {
	return GetMap("Uri2KeyMap");
}

ROAnything TableCompressor::GetUri2ValMap() const {
	return GetMap("Uri2ValMap");
}

ROAnything TableCompressor::GetValMapTags() const {
	return GetMap("ValMapTags");
}
