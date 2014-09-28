// globalhelpers.h	- Declaration of global helper functions
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

#ifndef GLOBALHELPERS_H
#define GLOBALHELPERS_H
#else
#error ass
#endif

#pragma once
#include "stdafx.h"

struct SExtensionRecord;

template<class T>
INT signum(T x) {
	static_assert( std::is_arithmetic<T>::value, "need an arithmetic datatype!" );
	if ( x < 0 ) {
		return -1;
		}
	else {
		if ( x == 0 ) {
			return 0;
			}
		else {
			return 1;
			}
		}
	//return ( x < 0 ) ? -1 : ( x == 0 ) ? 0 : 1;
	}


// CCoTaskMem<>. Some Windows APIs return memory which must be freed with CoTaskMemFree().
// This template does that in its destructor.
template<class T>
class CCoTaskMem {
	public:
	CCoTaskMem( T lp = 0 ) {
		p = lp;
		}
	CCoTaskMem( const CCoTaskMem<T>& ) {// operator not allowed for CCoTaskMem 
		_ASSERTE( 0 );
		p = 0;
		}
	~CCoTaskMem( ) {
		if ( p ) {
			CoTaskMemFree( p );
			}
		}

	operator T( ) {
		return p;
		}
	T& operator*( ) {
		_ASSERTE( p != NULL );
		return p;
		}
	//The assert on operator& usually indicates a bug.  If this is really
	//what is needed, however, take the address of the p member explicitly.
	T* operator&( ) {
		_ASSERTE( p == NULL );
		return &p;
		}
	T operator->( ) {
		_ASSERTE( p != NULL );
		return p;
		}
	T operator = ( T lp ) {
		if ( p != NULL ) CoTaskMemFree( p ); p = lp; return p;
		}
	T operator=( const CCoTaskMem<T>& lp ) {// operator not allowed for CCoTaskMem 
		_ASSERTE( 0 );
		return p;
		}
	bool operator!( ) {
		return ( p == NULL );
		}

	T p;
	};


//class CMdStringException : public CException {
//public:
//	CMdStringException( _In_ CString pszText ) : m_sText( pszText ) { }
//protected:
//	CString m_sText;
//	};

_Success_( SUCCEEDED( return ) ) HRESULT FormatBytes ( _In_ const std::uint64_t n, _Out_writes_z_( strSize ) PWSTR psz_formatted_bytes, _In_range_( 20, 64 ) rsize_t strSize );


CString GetCOMSPEC                 (                                                    );

CString FormatAttributes           ( _In_ const DWORD              attr                                                                );
CString FormatBytes                ( _In_ const std::uint64_t      n                                                                   );
CString FormatCount                ( _In_ const std::uint32_t      n                                                                   );
CString FormatCount                ( _In_ const std::uint64_t      n                                                                   );
CString FormatDouble               ( _In_ const DOUBLE             d                                                                   );
CString FormatFileTime             ( _In_ const FILETIME&          t                                                                   );

CString FormatVolumeName           ( _In_ const CString            rootPath,    _In_ const CString   volumeName                        );

_Success_( return == 0 ) int CStyle_FormatFileTime( _In_ const FILETIME& t, _Out_writes_z_( strSize ) PWSTR psz_formatted_datetime, rsize_t strSize );
_Success_( return == 0 ) int CStyle_FormatAttributes( _In_ const DWORD attr, _Out_writes_z_( strSize ) PWSTR psz_formatted_attributes, _In_range_( 1, 6 ) rsize_t strSize );

_Success_( SUCCEEDED( return ) ) HRESULT CStyle_FormatDouble( _In_ DOUBLE d, _Out_writes_z_( strSize ) PWSTR psz_formatted_double, _In_range_( 3, 64 ) rsize_t strSize );
_Success_( SUCCEEDED( return ) ) HRESULT CStyle_FormatLongLongHuman( _In_ std::uint64_t n,  _Out_writes_z_( strSize ) PWSTR psz_formatted_LONGLONG_HUMAN, _In_range_( 3, 64 ) rsize_t strSize );

CString MyQueryDosDevice           ( _In_z_ const PCTSTR            drive                                                               );
CString PathFromVolumeName         ( _In_ const CString            name                                                                );

CString GetLastErrorAsFormattedMessage( );
//CString GetShellExecuteError( _In_       const UINT u                                           );

CString MyGetFullPathName   ( _In_z_     const PCTSTR relativePath                             );
CString GetAppFileName      ( );

void MyShellExecute         ( _In_opt_       HWND hwnd,         _In_opt_z_       PCTSTR lpOperation, _In_z_ PCTSTR lpFile, _In_opt_z_ PCTSTR lpParameters, _In_opt_z_ PCTSTR lpDirectory, _In_ const INT nShowCmd );


bool DriveExists                   ( _In_ const CString&           path                                                                );
bool GetVolumeName                 ( _In_z_ const PCTSTR            rootPath,    _Out_    CString&  volumeName                        );
bool IsSUBSTedDrive                ( _In_z_ const PCTSTR            drive                                                               );


CString GetParseNameOfMyComputer   (                                                             );
_Success_( !FAILED( return ) ) HRESULT GetPidlOfMyComputer           ( _Inout_    LPITEMIDLIST *ppidl                              );
_Success_( return > 32 ) int ShellExecuteWithAssocDialog   ( _In_ const HWND hwnd,           _In_z_ const PCTSTR filename );


void check8Dot3NameCreationAndNotifyUser( );
void displayWindowsMsgBoxWithError( );

void displayWindowsMsgBoxWithMessage( CString message );

void MyGetDiskFreeSpace        ( _In_z_ const PCTSTR            pszRootPath, _Inout_    std::uint64_t& total, _Inout_ std::uint64_t& unused   );

const LARGE_INTEGER help_QueryPerformanceCounter( );
const LARGE_INTEGER help_QueryPerformanceFrequency( );


LVITEM partInitLVITEM( );
SHELLEXECUTEINFO partInitSEI                     ( );

SHELLEXECUTEINFO        zeroInitSEI                     ( );
WINDOWPLACEMENT         zeroInitWINDOWPLACEMENT         ( );
LVHITTESTINFO           zeroInitLVHITTESTINFO           ( );
HDITEM                  zeroInitHDITEM                  ( );
LVFINDINFO              zeroInitLVFINDINFO              ( );
PROCESS_MEMORY_COUNTERS zeroInitPROCESS_MEMORY_COUNTERS ( );
STARTUPINFO             zeroInitSTARTUPINFO             ( );
PROCESS_INFORMATION     zeroInitPROCESS_INFORMATION     ( );
NMLISTVIEW              zeroInitNMLISTVIEW              ( );
BROWSEINFO              zeroInitBROWSEINFO              ( );
SHFILEOPSTRUCT          zeroInitSHFILEOPSTRUCT          ( );
FILETIME                zeroInitFILETIME                ( );

CString EncodeSelection( _In_ const RADIO radio, _In_ const CString folder, _In_ const CStringArray& drives );

// Whether an item type is a leaf type
inline bool IsLeaf( const ITEMTYPE t ) {
	return ( t == IT_FILE );
	}

void zeroDate( _Out_ FILETIME& in );

CRect BuildCRect( const SRECT& in );

std::vector<COLORREF> GetDefaultPaletteAsVector( );

// $Log$
// Revision 1.15  2004/11/28 14:40:06  assarbad
// - Extended CFileFindWDS to replace a global function
// - Now packing/unpacking the file attributes. This even spares a call to find encrypted/compressed files.
//
// Revision 1.14  2004/11/25 21:13:38  assarbad
// - Implemented "attributes" column in the treelist
// - Adopted width in German dialog
// - Provided German, Russian and English version of IDS_TREECOL_ATTRIBUTES
//
// Revision 1.13  2004/11/25 11:58:52  assarbad
// - Minor fixes (odd behavior of coloring in ANSI version, caching of the GetCompressedFileSize API)
//   for details see the changelog.txt
//
// Revision 1.12  2004/11/12 13:19:44  assarbad
// - Minor changes and additions (in preparation for the solution of the "Browse for Folder" problem)
//
// Revision 1.11  2004/11/07 20:14:30  assarbad
// - Added wrapper for GetCompressedFileSize() so that by default the compressed file size will be shown.
//
// Revision 1.10  2004/11/05 16:53:07  assarbad
// Added Date and History tag where appropriate.
//
