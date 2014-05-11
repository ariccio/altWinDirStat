// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:


// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <string>
#include <future>
#include <vector>
#include <mutex>
#include <cmath>


#ifdef _DEBUG
#define TRACING
#endif


#ifdef _DEBUG
#ifdef TRACING
#include <afx.h>

#define new DEBUG_NEW
#endif

#endif

#ifndef _WINDOWS_
#include <windows.h>
#endif
// TODO: reference additional headers your program requires here


#define VERYUNSECURE

#ifdef VERYUNSECURE //temp required for release mode, else "error C4996: 'wcscpy': This function or variable may be unsafe. Consider using wcscpy_s instead."
#define _CRT_SECURE_NO_WARNINGS
#endif
