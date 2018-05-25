// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently
//
// see `file_header_text.txt` for licensing & contact info. If you can't find that file, then assume you're NOT allowed to do whatever you wanted to do. If you can't find that file, then assume you're NOT allowed to do whatever you wanted to do.
#pragma once


#ifndef STDAFX_INCLUDED

#pragma message( "defining `STDAFX_INCLUDED`..." )

#define STDAFX_INCLUDED


//#pragma message( "defining `_HAS_EXCEPTIONS 0`..." )

//#define _HAS_EXCEPTIONS 0

#ifndef VC_EXTRALEAN

#pragma message( "defining `VC_EXTRALEAN`..." )

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#endif

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
//#ifndef WINVER				// Allow use of features specific to Windows 95 and Windows NT 4 or later.
//#define WINVER 0x0400		// Change this to the appropriate value to target Windows 98 and Windows 2000 or later.
//#endif

#ifndef UNICODE

#pragma message( "defining `UNICODE`..." )

#define UNICODE
#endif

#ifndef _UNICODE

#pragma message( "defining `_UNICODE`..." )

#define _UNICODE
#endif

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows NT 4 or later.

#pragma message( "defining `_WIN32_WINNT`..." )

#define _WIN32_WINNT 0x0602	// Change this to the appropriate value to target Windows 98 and Windows 2000 or later.
#else
static_assert( _WIN32_WINNT >= 0x0600, "" );
//static_assert( _WIN32_WINNT >= 0x0601, "" );
//static_assert( _WIN32_WINNT >= 0x0602, "" );
#endif						

#ifndef _WIN32_WINDOWS

#pragma message( "defining `_WIN32_WINDOWS`..." )

#define _WIN32_WINDOWS 0x0602
#endif

#ifndef _WIN32_IE			// Allow use of features specific to IE 4.0 or later.

#pragma message( "defining `_WIN32_IE`..." )

#define _WIN32_IE 0x0800	// Change this to the appropriate value to target IE 5.0 or later.
#endif

#ifndef _ATL_CSTRING_EXPLICIT_CONSTRUCTORS

#pragma message( "defining `_ATL_CSTRING_EXPLICIT_CONSTRUCTORS`..." )

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit

#endif


#ifndef _AFX_ALL_WARNINGS

#pragma message( "defining `_AFX_ALL_WARNINGS`..." )
// turns off MFC's hiding of some common and often safely ignored warning messages
#define _AFX_ALL_WARNINGS

#endif


#ifndef _ATL_ALL_WARNINGS

#pragma message( "defining `_ATL_ALL_WARNINGS`..." )

#define _ATL_ALL_WARNINGS

#endif


//_ATL_NO_AUTOMATIC_NAMESPACE: https://msdn.microsoft.com/en-us/library/a477k694.aspx
//A symbol which prevents the default use of namespace as ATL.
#ifndef _ATL_NO_AUTOMATIC_NAMESPACE

#pragma message( "defining `_ATL_NO_AUTOMATIC_NAMESPACE`..." )

#define _ATL_NO_AUTOMATIC_NAMESPACE

#endif



#ifndef _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES

#pragma message( "defining `_CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES`..." )

#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1

#else
#error already defined!
#endif


#ifndef _ATL_ENABLE_PTM_WARNING

#pragma message( "defining `_ATL_ENABLE_PTM_WARNING`..." )

#define _ATL_ENABLE_PTM_WARNING

#endif


//Things that I will eventually get rid of/add to program, but can't safely do so as of yet.

//Debugging defs
//#define DUMP_MEMUSAGE
//#define GRAPH_LAYOUT_DEBUG
//#define EXTENSION_LIST_DEBUG
//#define COLOR_DEBUGGING
//#define SIMD_ACCESS_DEBUGGING
//#define WDS_STRING_ALLOC_DEBUGGING
//#define DISPLAY_FINAL_CITEMBRANCH_SIZE
//#define WDS_OWNERDRAWNLISTITEM_DESTRUCTOR_DEBUG
//#define WDS_SCOPE_GUARD_DEBUGGING
//#define WDS_TYPEVIEW_TREEMAP_MEMORY_USAGE_DEBUGGING


#ifndef DEBUG
#define DISPLAY_FINAL_CITEMBRANCH_SIZE
#endif

#ifdef WDS_STRING_ALLOC_DEBUGGING
#ifndef DEBUG
#error incompatible!
#endif
#endif

#ifdef GRAPH_LAYOUT_DEBUG
#ifndef DEBUG
#error incompatible!
#endif
#endif


//#pragma warning(disable:4265) //'class' : class has virtual functions, but destructor is not virtual
//#pragma warning(disable:4987) //nonstandard extension used: 'throw (...)'
//#pragma warning(disable:4548) //expression before comma has no effect; expected expression with side-effect
//#pragma warning(disable:4625) //A copy constructor was not accessible in a base class, therefore not generated for a derived class. Any attempt to copy an object of this type will cause a compiler error. warn!
//#pragma warning(disable:4626) //An assignment operator was not accessible in a base class and was therefore not generated for a derived class. Any attempt to assign objects of this type will cause a compiler error.
//#pragma warning(disable:4189) //A variable is declared and initialized but not used.
//#pragma warning(disable:4755) //Conversion rules for arithmetic operations in the comparison mean that one branch cannot be executed in an inlined function. Cast '(nBaseTypeCharLen + ...)' to 'ULONG64' (or similar type of 8 bytes).
//#pragma warning(disable:4280) //'operator –>' was self recursive through type 'type'. Your code incorrectly allows operator–> to call itself.
//#pragma warning(disable:4127) //The controlling expression of an if statement or while loop evaluates to a constant.
//#pragma warning(disable:4365) //'action' : conversion from 'type_1' to 'type_2', signed/unsigned mismatch
//#pragma warning(disable:4710) //The given function was selected for inline expansion, but the compiler did not perform the inlining.


//these are all in MFC message maps.
#pragma warning(disable:4191) //'operator/operation' : unsafe conversion from 'type of expression' to 'type required'

#ifndef DUMP_MEMUSAGE
#pragma warning(disable:4820) //'bytes' bytes padding added after construct 'member_name'. The type and order of elements caused the compiler to add padding to the end of a struct
#endif

#pragma warning(disable:4917) //'declarator' : a GUID can only be associated with a class, interface or namespace. A user-defined structure other than class, interface, or namespace cannot have a GUID.
#pragma warning(disable:4264) //'virtual_function' : no override available for virtual member function from base 'class'; function is hidden
#pragma warning(disable:4263) //A class function definition has the same name as a virtual function in a base class but not the same number or type of arguments. This effectively hides the virtual function in the base class.


//Comment this out for insanely slow compilation!
#pragma warning(disable:4514) //'function' : unreferenced inline function has been removed
#pragma warning(disable:4711) //function 'function' selected for inline expansion. The compiler performed inlining on the given function, although it was not marked for inlining.

#ifndef _DEBUG
#pragma warning(disable:4555) //expression has no effect; expected expression with side-effect //Happens alot with AfxCheckMemory in release builds.
#endif

//#pragma comment(lib, "d2d1")
//#pragma comment(lib, "Dwrite")

#pragma warning(push, 3)

#ifdef DUMP_MEMUSAGE
#pragma warning(disable:4820) //'bytes' bytes padding added after construct 'member_name'. The type and order of elements caused the compiler to add padding to the end of a struct
#endif



#pragma warning(disable:4530)//C++ exception handler used, but unwind semantics are not enabled.
#pragma warning(disable:4555) //expression has no effect; expected expression with side-effect //Happens alot in WTL.
#pragma warning(disable:4302)//'type cast' : truncation from 'LPCTSTR' to 'WORD'


//These are ALL in STL
#pragma warning(disable:4350) //An rvalue cannot be bound to a non-const reference. In previous versions of Visual C++, it was possible to bind an rvalue to a non-const reference in a direct initialization. This code now gives a warning.
#pragma warning(disable:4061) //enumerator 'identifier' in switch of enum 'enumeration' is not explicitly handled by a case label. The enumerate has no associated handler in a switch statement.
#pragma warning(disable:4062) //The enumerate has no associated handler in a switch statement, and there is no default label.



#pragma warning(disable:4710) //The given function was selected for inline expansion, but the compiler did not perform the inlining.

#include <afxwin.h>         // MFC Core //MUST BE INCLUDED FIRST!!!!!!!!!!!!!

// Add support for ATL/WTL
//#define _WTL_FORWARD_DECLARE_CSTRING

#ifndef _WTL_NO_AUTOMATIC_NAMESPACE

#pragma message( "defining `_WTL_NO_AUTOMATIC_NAMESPACE`..." )

#define _WTL_NO_AUTOMATIC_NAMESPACE //_ATL_NO_AUTOMATIC_NAMESPACE

#endif

#include <atlbase.h>        // base ATL classes

#ifdef _AFX

#ifndef _WTL_NO_CSTRING

#pragma message( "defining `_WTL_NO_CSTRING`..." )

#define _WTL_NO_CSTRING 1

#endif // _WTL_NO_CSTRING

#pragma message( "defining `_CSTRING_NS`..." )

#define _CSTRING_NS

#endif // _AFX

#pragma warning(disable:4265) //'class' : class has virtual functions, but destructor is not virtual
#pragma warning(disable:6031) //Return value ignored: 'CoSuspendClassObjects'.
#pragma warning(disable:6387) //error C6387: '_Param_(1)' could be '0':  this does not adhere to the specification for the function 'ATL::CWindow::SetFont'.
#pragma warning(disable:6400) //Using 'lstrcmpiW' to perform a case-insensitive compare to constant string 'static'.  Yields unexpected results in non-English locales.
#pragma warning(disable:6401) //Using 'CompareStringW' in a default locale to perform a case-insensitive compare to constant string '<A>'.  Yields unexpected results in non-English locales.
#pragma warning(disable:26110) //error C26110: Caller failing to hold lock '(& this->m_cslock)->m_cs' before calling function 'ATL::CComCritSecLock<ATL::CComCriticalSection>::Unlock'.
#pragma warning(disable:26165) //Possibly failing to release lock '(& this->m_cslock)->m_cs' in function 'WTL::CStaticDataInitCriticalSectionLock::Lock'.
#pragma warning(disable:26160) //Caller possibly failing to hold lock '(& this->m_cslock)->m_cs' before calling function 
#pragma warning(disable:26167) //Possibly releasing unheld lock '(& this->m_cslock)->m_cs' in function 'WTL::CStaticDataInitCriticalSectionLock::Unlock'.
#pragma warning(disable:28204) //'OnFileOk' : Only one of this override and the one at c:\program files (x86)\windows kits\8.1\include\um\shobjidl.h

#include <atlapp.h>         // base WTL classes
extern WTL::CAppModule _Module;

#include <atlwin.h>         // ATL GUI  classes
//#include <atlframe.h>       // WTL frame window classes
//#include <atlsplit.h>
#include <atlctrls.h>

#pragma warning(disable:4640) //construction of local static object is not thread-safe	

#include <atlctrlx.h>
#include <atlmisc.h>        // ATL utility classes (i.e. CString)
#include <atlcrack.h>       // WTL message map macros
#include <atldlgs.h>
#include <atlddx.h>         //For DDX support


#pragma warning(disable:4702)//unreachable code

#include <vector>
#include <memory>
#include <string>
#include <algorithm>
#include <future>
#include <map>
#include <unordered_map>
#include <numeric>
#include <cstdint>
#include <type_traits>
#include <utility>
#include <tuple>
//#include <iterator>
//#include <PathCch.h>
//#pragma comment(lib, "Pathcch")

#include <afxext.h>         // MFC Extensions
#include <afxdlgs.h>
//#include <afxdtctl.h>		// MFC IE 4
#include <afxdisp.h>
#include <afxcmn.h>			// MFC Common Controls
//#include <afxtempl.h>		// MFC Container classes
//#include <afxmt.h>		// MFC Multithreading


//#include <cvt/wstring>		//for wstring_convert

#include <windowsx.h>
#include <stdlib.h>


//#define _WIN32_WINNT 0x0602	// Change this to the appropriate value to target Windows 98 and Windows 2000 or later.


#include <math.h>			// floor(), fmod(), sqrt() etc.
#include <psapi.h>			// PROCESS_MEMORY_INFO
#include <processthreadsapi.h>// SetProcessMitigationPolicy 

#include <float.h>			// DBL_MAX

#include <strsafe.h>
//#include <intsafe.h>

#include <iso646.h>
#include <wctype.h>

#pragma warning (push)
#pragma warning (disable:4619)  // unreachable code
#include <concurrent_vector.h>
#pragma warning (pop)

#pragma warning(pop)

#pragma warning(disable:4710) //The given function was selected for inline expansion, but the compiler did not perform the inlining.

// Headers placed in the common directory (used by windirstat and by setup)
//#include "../common/version.h"

#ifndef _INC_STDARG
#include <stdarg.h>
#endif


//#ifndef countof
//#define countof(arr) (sizeof(arr)/sizeof((arr)[0]))
//#else
//#error already defiend!
//#endif

//#define pi2 1.5707963267948966192
#define RAM_USAGE_UPDATE_INTERVAL 100
#define TREELISTCOLORCOUNT size_t( 8 )

#define PALETTE_BRIGHTNESS DOUBLE( 0.6 )
//#define INDICATORS_NUMBER size_t( 2 )

//#define ITEM_ROW_HEIGHT 20
//static_assert( ITEM_ROW_HEIGHT > -1, "Rows need to be a positive size!" );

//#define _N_ID_EXTENSION_LIST_CONTROL UINT( 4711 )

#define XY_SETPOS UINT( WM_USER + 100 )
#define XY_GETPOS UINT( WM_USER + 101 )

#define XYSLIDER_CHANGED 0x88 // this is a value, I hope, that is nowhere used as notification code.


#ifdef DEBUG
#define new DEBUG_NEW
#endif

//#ifndef DEBUG
////#pragma warning(3:4710) //The given function was selected for inline expansion, but the compiler did not perform the inlining.
//#endif

//WDS headers (infrequently modified)
#include "Resource.h"


#ifndef WDS_FILE_INCLUDE_MESSAGE

#pragma message( "defining `WDS_FILE_INCLUDE_MESSAGE`..." )

#define WDS_FILE_INCLUDE_MESSAGE __pragma( message( "Including `" __FILE__ "`..." ) )

#endif

#pragma warning(3:4711)

#pragma warning(disable:4711) //function 'function' selected for inline expansion. The compiler performed inlining on the given function, although it was not marked for inlining.

#else
#error ass
#endif
