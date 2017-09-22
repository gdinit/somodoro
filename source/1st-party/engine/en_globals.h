/* en_globals.h */
// ===================================80 chars=================================|

#ifndef EN_GLOBALS_H
#define EN_GLOBALS_H

#include "en_essentials.h"

#include <string>

struct Settings {
	// App Window
	int32 CfgWindowResX, CfgWindowResY;
	bool inGameOverlay = false;
	bool debugPrintToConsole = false;
	bool debugPrintToConsoleFPS = false;
	int32 currentScreenSizeWidth = -9999;
	int32 currentScreenSizeHeight = -9999;
	float ballSpeed = -9999.f;
};

// EN_GLOBALS_H
#endif

// ===================================80 chars=================================|
/* EOF */

