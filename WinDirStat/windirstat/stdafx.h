// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently
//
// WinDirStat - Directory Statistics
// Copyright (C) 2003-2004 Bernhard Seifert
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// Author: bseifert@users.sourceforge.net, bseifert@daccord.net
//
// Last modified: $Date$

#ifndef STDAFX_INCLUDED
#define STDAFX_INCLUDED
#else
#error ass
#endif

#pragma once



#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#endif

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
//#ifndef WINVER				// Allow use of features specific to Windows 95 and Windows NT 4 or later.
//#define WINVER 0x0400		// Change this to the appropriate value to target Windows 98 and Windows 2000 or later.
//#endif

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows NT 4 or later.
#define _WIN32_WINNT 0x0600	// Change this to the appropriate value to target Windows 98 and Windows 2000 or later.
#endif						

#ifndef _WIN32_WINDOWS			// Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0600	// Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE			// Allow use of features specific to IE 4.0 or later.
#define _WIN32_IE 0x0800	// Change this to the appropriate value to target IE 5.0 or later.
#endif

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit

// turns off MFC's hiding of some common and often safely ignored warning messages
#define _AFX_ALL_WARNINGS

//From helpmap.h
#define IDH_Treemap 1003

#ifdef _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 
#error
#else
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1
#endif


//Things that I will eventually get rid of/add to program, but can't safely do so as of yet.
//#define DRAW_ICONS
//#define ITEM_DRAW_SUBITEM

//Debugging defs
//#define DUMP_MEMUSAGE
//#define GRAPH_LAYOUT_DEBUG
//#define EXTENSION_LIST_DEBUG
//#define PERF_DEBUG_SLEEP


//#pragma warning(disable:4061) //enumerator 'identifier' in switch of enum 'enumeration' is not explicitly handled by a case label. The enumerate has no associated handler in a switch statement.
//#pragma warning(disable:4062) //The enumerate has no associated handler in a switch statement, and there is no default label.


//these are all in MFC message maps.
#pragma warning(disable:4191) //'operator/operation' : unsafe conversion from 'type of expression' to 'type required'


#pragma warning(disable:4265) //'class' : class has virtual functions, but destructor is not virtual
#pragma warning(disable:4350) //An rvalue cannot be bound to a non-const reference. In previous versions of Visual C++, it was possible to bind an rvalue to a non-const reference in a direct initialization. This code now gives a warning.


#ifndef DUMP_MEMUSAGE
#pragma warning(disable:4820) //'bytes' bytes padding added after construct 'member_name'. The type and order of elements caused the compiler to add padding to the end of a struct
#endif

#pragma warning(disable:4917) //'declarator' : a GUID can only be associated with a class, interface or namespace. A user-defined structure other than class, interface, or namespace cannot have a GUID.
//#pragma warning(disable:4987) //nonstandard extension used: 'throw (...)'

//noisy
#pragma warning(disable:4548) //expression before comma has no effect; expected expression with side-effect

//ANYTHING that inherits from CWND will 
#pragma warning(disable:4625) //A copy constructor was not accessible in a base class, therefore not generated for a derived class. Any attempt to copy an object of this type will cause a compiler error. warn!


#pragma warning(disable:4626) //An assignment operator was not accessible in a base class and was therefore not generated for a derived class. Any attempt to assign objects of this type will cause a compiler error.

#pragma warning(disable:4264) //'virtual_function' : no override available for virtual member function from base 'class'; function is hidden
//#pragma warning(disable:4263) //A class function definition has the same name as a virtual function in a base class but not the same number or type of arguments. This effectively hides the virtual function in the base class.
//#pragma warning(disable:4189) //A variable is declared and initialized but not used.
//#pragma warning(disable:4755) //Conversion rules for arithmetic operations in the comparison mean that one branch cannot be executed in an inlined function. Cast '(nBaseTypeCharLen + ...)' to 'ULONG64' (or similar type of 8 bytes).
//#pragma warning(disable:4280) //'operator –>' was self recursive through type 'type'. Your code incorrectly allows operator–> to call itself.
//#pragma warning(disable:4127) //The controlling expression of an if statement or while loop evaluates to a constant.
//#pragma warning(disable:4365) //'action' : conversion from 'type_1' to 'type_2', signed/unsigned mismatch

#pragma warning(disable:4514) //'function' : unreferenced inline function has been removed
#pragma warning(disable:4710) //The given function was selected for inline expansion, but the compiler did not perform the inlining.
#pragma warning(disable:4711) //function 'function' selected for inline expansion. The compiler performed inlining on the given function, although it was not marked for inlining.

#ifndef _DEBUG
#pragma warning(disable:4555) //expression has no effect; expected expression with side-effect //Happens alot with AfxCheckMemory in release builds.
#endif

//#pragma comment(lib, "d2d1")
//#pragma comment(lib, "Dwrite")

#pragma warning(push, 3)

#include <afxwin.h>         // MFC Core //MUST BE INCLUDED FIRST!!!!!!!!!!!!!

#include <vector>
#include <memory>
#include <string>
#include <algorithm>
#include <future>
#include <map>
#include <numeric>
#include <cstdint>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include <agents.h>
#include <ppl.h>
#include <afxext.h>         // MFC Extensions
#include <afxdtctl.h>		// MFC IE 4
#include <afxcmn.h>			// MFC Common Controls
#include <afxtempl.h>		// MFC Container classes
#include <afxmt.h>			// MFC Multithreading
#include <atlbase.h>		// USES_CONVERSION, ComPtr<>

#include <windowsx.h>

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#endif
#include <stdlib.h>
//#include <crtdbg.h>

#include <math.h>			// floor(), fmod(), sqrt() etc.
#include <psapi.h>			// PROCESS_MEMORY_INFO

#include <lmcons.h>			// UNLEN
#include <float.h>			// DBL_MAX

//#include <VersionHelpers.h>

//#include <winioctl.h>
#include <strsafe.h>
#include <intsafe.h>
//#include <d2d1.h>
//#include <d2d1helper.h>
//#include <dwrite.h>

#include <iso646.h>
#include <wctype.h>
#pragma warning(pop)



// Headers placed in the common directory (used by windirstat and by setup)
//#include "../common/mdexceptions.h"
//#include "../common/cotaskmem.h"
//#include "../common/commonhelpers.h"
//#include "../common/platform.h"
#include "../common/version.h"

// General purpose headers
//#include "selectobject.h"
//#include "set.h"

#ifndef _INC_STDARG
#include <stdarg.h>
#endif


#define countof(arr) (sizeof(arr)/sizeof((arr)[0]))


#define pi2 1.5707963267948966192
#define RAM_USAGE_UPDATE_INTERVAL 100
#define TREELISTCOLORCOUNT size_t( 8 )

#define PALETTE_BRIGHTNESS double( 0.6 )
#define INDICATORS_NUMBER size_t( 2 )

#define COLORFLAG_DARKER DWORD( 0x01000000 )
#define COLORFLAG_LIGHTER DWORD( 0x02000000 )
#define COLORFLAG_MASK DWORD( 0x03000000 )

#define GRIPPER_RADIUS INT( 8 )

#ifndef PROGRESS_RANGE
#define PROGRESS_RANGE std::uint64_t( 100 )
#endif

#define ITEM_ROW_HEIGHT 20
static_assert( ITEM_ROW_HEIGHT > -1, "Rows need to be a positive size!" );

//static UINT _N_ID_EXTENSION_LIST_CONTROL = 4711;
#define _N_ID_EXTENSION_LIST_CONTROL UINT( 4711 )

//const UINT CXySlider::XY_SETPOS = WM_USER + 100;
//const UINT CXySlider::XY_GETPOS = WM_USER + 101;
#define XY_SETPOS UINT( WM_USER + 100 )
#define XY_GETPOS UINT( WM_USER + 101 )



#ifndef DEBUG
#pragma warning(3:4710) //The given function was selected for inline expansion, but the compiler did not perform the inlining.
#endif


//some generic structures!
#include "datastructures.h"

//helper functions & classes
#include "globalhelpers.h"

//WDS headers (infrequently modified)
#include "mountpoints.h"
//#include "osspecific.h"
#include "myimagelist.h"
//#include "pacman.h"
#include "colorbutton.h"
#include "xyslider.h"
//#include "memoryUsage.h"
#include "Resource.h"
//#include "ModalApiShuttle.h"
#include "ModalShellApi.h"
#include "PageGeneral.h"

#include "treemap.h"
#include "options.h"
//#include "PageTreelist.h"





//WDS headers (Frequently modified)


//#include "PageTreemap.h"
//#include "mainframe.h"
//#include "sortinglistcontrol.h"
//#include "ownerdrawnlistcontrol.h"
//#include "layout.h"
//#include "SelectDrivesDlg.h"
//#include "aboutdlg.h"
#include "windirstat.h"
//#include "Treelistcontrol.h"
//#include "FileFindWDS.h"
#include "item.h"
#include "dirstatdoc.h"

//#include "graphview.h"
//#include "dirstatview.h"
//#include "typeview.h"




// $Log$
// Revision 1.10  2004/11/12 22:14:16  bseifert
// Eliminated CLR_NONE. Minor corrections.
//
// Revision 1.9  2004/11/12 13:19:44  assarbad
// - Minor changes and additions (in preparation for the solution of the "Browse for Folder" problem)
//
// Revision 1.8  2004/11/05 16:53:08  assarbad
// Added Date and History tag where appropriate.
//
