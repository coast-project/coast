/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "AppBooter.h"
#include "Application.h"
#include "Context.h"
#include "SecurityModule.h"
#include "WDModule.h"

#include <iostream>

#include <unistd.h>

class MyAppBooter : public AppBooter {
	Application *FindApplication(const Anything &config, String &applicationName) {
		applicationName = "UnscrambleUrl";
		return Application::FindApplication(applicationName);
	}
};

class UnscrambleUrl : public Application {
public:
	UnscrambleUrl(const char *n) : Application(n) {}
	/*! @copydoc IFAObject::Clone(Allocator *) const */
	IFAObject *Clone(Allocator *a) const { return new (a) UnscrambleUrl(fName); }

	int GlobalInit(int argc, char *argv[], const Anything &config) {
		if (argc < 2) {
			std::cerr << "Unscramble Coast URL state." << std::endl;
			std::cerr << "To run, use same COAST_ROOT and COAST_PATH as the WD-App which generated the URLs" << std::endl;
			std::cerr << "Usage: " << argv[0] << " [ <url> ]+" << std::endl;
			exit(-1);
		}
		for (int i = 1; i < argc; i++) {
			fUrls.Append(argv[i]);
		}
	}
	int GlobalRun() {
		for (long i = 0; i < fUrls.GetSize(); i++) {
			Anything result;
			String url(fUrls[i].AsString(""));
			SecurityModule::UnscrambleState(result, url);
			std::cout << "Url State: " << url << std::endl;
			std::cout << result;
			std::cout << std::endl << std::flush;
		}
		return 0;
	}

private:
	Anything fUrls;
};
RegisterApplication(UnscrambleUrl);

int main(int argc, const char *argv[]) {
	return MyAppBooter().Run(argc, argv);
}
