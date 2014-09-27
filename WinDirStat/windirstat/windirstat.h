// windirstat.h	- Main header for the windirstat application
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

#ifndef WINDIRSTAT_H
#define WINDIRSTAT_H
#else
#error ass
#endif

#pragma once

#include "stdafx.h"
//#include "resource.h" 
//#include "globalhelpers.h"
//#include "options.h"
//#include "dirstatdoc.h"
//#include "selectdrivesdlg.h"
//#include "aboutdlg.h"
//#include "graphview.h"
//#include "memoryUsage.h"

class CMainFrame;
class CDirstatApp;

// Frequently used "globals"
CMainFrame*   GetMainFrame( );
CDirstatApp*  GetApp( );
CMyImageList* GetMyImageList( );

// CDirstatApp. The MFC application object. Knows about RAM Usage, Mount points, Help files and the CMyImageList.
class CDirstatApp : public CWinApp {
public:
	CDirstatApp( ) : m_workingSet( 0 ), m_pageFaults( 0 ), m_lastPeriodicalRamUsageUpdate( GetTickCount64( ) ), m_altEncryptionColor( GetAlternativeColor( RGB( 0x00, 0x80, 0x00 ), _T( "AltEncryptionColor" ) ) ) { }
	virtual BOOL InitInstance                  ( ) override;
	virtual INT ExitInstance                   ( ) override;


	void DoContextHelp                         ( _In_ DWORD topic                          ) const;
	void PeriodicalUpdateRamUsage              (                                           );
	void ReReadMountPoints                     (                                           );
	void RestartApplication                    (                                           );
	void UpdateRamUsage                        (                                           );
	
	bool IsMountPoint                          ( _In_ CString path                         ) const;
	bool IsJunctionPoint                       ( _In_ CString path, _In_ DWORD fAttributes ) const;
	bool b_PeriodicalUpdateRamUsage( );
	
	COLORREF AltEncryptionColor                ( );		   // Coloring of encrypted items
_Success_( SUCCEEDED( return ) ) HRESULT GetCurrentProcessMemoryInfo        ( _Out_writes_z_( strSize ) PWSTR psz_formatted_usage, _In_range_( 20, 64 ) rsize_t strSize );

	_Must_inspect_result_ _Success_( return != NULL ) CMyImageList *GetMyImageList               ( );

protected:
	_Success_( return == true ) bool UpdateMemoryInfo                      (                                                                    );

	// Get the alternative color from Explorer configuration
	_Success_( return != clrDefault ) COLORREF GetAlternativeColor               ( _In_ COLORREF clrDefault, _In_z_ PCTSTR which );
	virtual BOOL OnIdle                        ( _In_ LONG lCount                        ) override;		// This is, where scanning is done.

	CSingleDocTemplate*       m_pDocTemplate;                   // MFC voodoo.
	CMountPoints              m_mountPoints;                    // Mount point information
	CMyImageList              m_myImageList;                    // Out central image list
	SIZE_T                  m_workingSet;					    // Current working set (RAM usage)
	LONGLONG                  m_pageFaults;					    // Page faults so far (unused)
	unsigned long long        m_lastPeriodicalRamUsageUpdate;	// Tick count
	COLORREF                  m_altEncryptionColor;			    // Coloring of encrypted items
	DECLARE_MESSAGE_MAP()
	afx_msg void OnFileOpen();
	afx_msg void OnAppAbout();

	//CString m_MemUsageCache;


	};


// $Log$
// Revision 1.13  2004/12/19 10:52:39  bseifert
// Minor fixes.
//
// Revision 1.12  2004/11/28 14:40:06  assarbad
// - Extended CFileFindWDS to replace a global function
// - Now packing/unpacking the file attributes. This even spares a call to find encrypted/compressed files.
//
// Revision 1.11  2004/11/25 11:58:52  assarbad
// - Minor fixes (odd behavior of coloring in ANSI version, caching of the GetCompressedFileSize API)
//   for details see the changelog.txt
//
// Revision 1.10  2004/11/14 08:49:06  bseifert
// Date/Time/Number formatting now uses User-Locale. New option to force old behavior.
//
// Revision 1.9  2004/11/12 22:14:16  bseifert
// Eliminated CLR_NONE. Minor corrections.
//
// Revision 1.8  2004/11/10 01:03:00  assarbad
// - Style cleaning of the alternative coloring code for compressed/encrypted items
//
// Revision 1.7  2004/11/08 00:46:26  assarbad
// - Added feature to distinguish compressed and encrypted files/folders by color as in the Windows 2000/XP explorer.
//   Same rules apply. (Green = encrypted / Blue = compressed)
//
// Revision 1.6  2004/11/05 16:53:08  assarbad
// Added Date and History tag where appropriate.
//
