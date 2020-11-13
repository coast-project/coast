/*
 * TLSEntry.h
 *
 *  Created on: Jun 19, 2020
 *      Author: m1huber
 */

#ifndef TLSENTRY_H_
#define TLSENTRY_H_

#include "SystemAPI.h"

namespace coast {
	namespace threading {
		template <typename ValueType>
		struct TLSEntry {
			typedef ValueType *ValueTypePtr;
			TLSEntry(THREADKEY key, ValueTypePtr value) : fKey(key), fOldValue(ValueTypePtr()) {
				GETTLSDATA(fKey, fOldValue, ValueType);
				(void)SETTLSDATA(fKey, value);
			}
			~TLSEntry() { (void)SETTLSDATA(fKey, fOldValue); }
			THREADKEY fKey;
			ValueTypePtr fOldValue;
		};
	}  // namespace threading
}  // namespace coast

#endif /* TLSENTRY_H_ */
