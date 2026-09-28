// version.h - Version number. Used by all resource scripts and by aboutdlg.cpp.
//
// see `file_header_text.txt` for licensing & contact info. If you can't find that file, then assume you're NOT allowed to do whatever you wanted to do.

#pragma once
/*-------------------------------------------------------------------
  This file defines the following:
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  - VN_FILEFLAG_EXE (contains information about RC or debug build)
  - VN_STRING_EXE (contains info about unicode and debug)
  - VN_STRING_DLL (just major.minor.rev.build)
  - VN_FILEVERSION_EXE and VN_PRODVERSION_EXE (only different from
    the DLL version if a development release)
  - VN_FILEVERSION_DLL and VN_PRODVERSION_DLL (numeric representation
    of VN_STRING_DLL)
  - VN_FILEOS_EXE (this depends on Unicode and ANSI!
      For Unicode: VN_FILEOS_EXE == VOS_NT_WINDOWS32
      For ANSI   : VN_FILEOS_EXE == VOS__WINDOWS32 )
  - VN_COPYRIGHTSTRING (copyright to include in the VERSIONINFO)
  - VN_RESOURCEDLL (version of resource DLLs must be the same for DLL
    and EXE at runtime)
  -------------------------------------------------------------------*/

//-------------------------------------------------------------------
// Build categories. Uncomment _one_ line.
//

// altWinDirStat: the version numbers come from build_version.h (stamped by CI from the release tag).
// Release builds report x.y.z.build; debug builds keep the upstream "devel" marker.
#include "build_version.h"
#if defined(_DEBUG)
#define BC_DEVEL			// Development version. The usual setting. File version is 0.0.0.buildno.
#else
#define BC_RELEASE
#endif
//#define BC_RELEASECANDIDATE		// Release candidate. Version number is relevant but not yet official. About-box shows x.y.zrcn. File version is x.y.z.buildno.
//BC_RELEASE			// Set this only during official builds. About-box shows x.y.z. File version is x.y.z.buildno

// A release version must not contain debug information. Raise an error!
#if defined(_DEBUG) && defined(BC_RELEASE)
  #error BC_RELEASE _and_ _DEBUG are defined. This must not happen. Releases contain no debug information.
#endif

// This will not change to often, but the years need to be modified regularly, so it can be in one central place
#define VN_COPYRIGHTSTRING "Original Copyright (C) 2003-2005 Bernhard Seifert, modified by Alexander Riccio 2014-2015 and altWinDirStat contributors. GPL v2."

//-------------------------------------------------------------------
// Version number. Relevant for BC_RELEASECANDIDATE and BC_RELEASE.
//
#define VERNUM_MAJOR		AWDS_VER_MAJOR
#define VERNUM_MINOR		AWDS_VER_MINOR
#define VERNUM_REVISION		AWDS_VER_PATCH
// The following line is automatically incremented by linkcounter.exe.
// Format: #define blank LINKCOUNT blanks decimal
// Reset this to zero only when you increment VERNUM_MAJOR/MINOR/REVISION.

#define LINKCOUNT  AWDS_VER_BUILD

//-------------------------------------------------------------------
// Release candidate number. Relevant for BC_RELEASECANDIDATE.
//
#define VERNUM_CANDIDATE	1



/////////////////////////////////////////////////////////////////////
// Derived values from here. Do not edit.

#define VN_BUILD	LINKCOUNT

#define PPSX(s) #s
#define PPS(s) PPSX(s)

#ifdef _UNICODE
	#define UASPEC "Unicode"
	// OS version is only relevant for the EXE
	#define VN_FILEOS_EXE VOS_NT_WINDOWS32
#else
	#define UASPEC "Ansi"
	// OS version is only relevant for the EXE
	#define VN_FILEOS_EXE VOS__WINDOWS32
#endif

#ifdef _DEBUG
	#define DRSPEC " Debug"
#else
	#define DRSPEC ""
#endif

#define VERVARIANT " (" UASPEC DRSPEC ")"

// This is just major.minor.rev.build always!
#define VN_STRING_DLL	PPS(VERNUM_MAJOR) "." PPS(VERNUM_MINOR) "." PPS(VERNUM_REVISION) "." PPS(VN_BUILD)

#if defined(BC_DEVEL)

	#define VN_MAJOR	0
	#define VN_MINOR	0
	#define VN_REVISION	0
	#define VN_FILEFLAG	0
	#define VN_STRING_DLL	PPS(VERNUM_MAJOR) "." PPS(VERNUM_MINOR) "." PPS(VERNUM_REVISION) "." PPS(VN_BUILD)
	// The variant (debug or not/ unicode or not) is not relevant for resource DLLs, but for EXEs
	#define VN_STRING_EXE	VN_STRING_DLL " devel" VERVARIANT

#elif defined(BC_RELEASECANDIDATE)

	#define VN_MAJOR	VERNUM_MAJOR
	#define VN_MINOR	VERNUM_MINOR
	#define VN_REVISION	VERNUM_REVISION
	#define VN_FILEFLAG	VS_FF_PRERELEASE
	// The variant (debug or not/ unicode or not) is not relevant for resource DLLs, but for EXEs
	#define VN_STRING_EXE	VN_STRING_DLL "rc" PPS(VERNUM_CANDIDATE) VERVARIANT

#elif defined(BC_RELEASE)

	#define VN_MAJOR	VERNUM_MAJOR
	#define VN_MINOR	VERNUM_MINOR
	#define VN_REVISION	VERNUM_REVISION
	#define VN_FILEFLAG	0
	// The variant (debug or not/ unicode or not) is not relevant for resource DLLs, but for EXEs
	#define VN_STRING_EXE	VN_STRING_DLL VERVARIANT

#endif

// EXE files have a different version number in development releases
#define VN_FILEVERSION_EXE VN_MAJOR,VN_MINOR,VN_REVISION,VN_BUILD
#define VN_PRODVERSION_EXE VN_FILEVERSION_EXE
// Resource DLLs need no different version number
#define VN_FILEVERSION_DLL VERNUM_MAJOR,VERNUM_MINOR,VERNUM_REVISION,VN_BUILD
#define VN_PRODVERSION_DLL VN_FILEVERSION_DLL

// Whether debug or not is not relevant for resource DLLs
#ifdef _DEBUG
 #define VN_FILEFLAG_EXE VS_FF_DEBUG | VN_FILEFLAG
#else
 #define VN_FILEFLAG_EXE VN_FILEFLAG
#endif

// ...nothing else.
#undef BC_DEVEL
#undef BC_RELEASECANDIDATE
#undef BC_RELEASE


