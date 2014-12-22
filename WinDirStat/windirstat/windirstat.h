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
#include "mountpoints.h"//CMountPoints : m_mountPoints, else we'd have to use PIMPL

class CMainFrame;
class CDirstatApp;
class CMountPoints;


// Frequently used "globals"
CMainFrame*   GetMainFrame( );
CDirstatApp*  GetApp( );

// CDirstatApp. The MFC application object. Knows about RAM Usage, Mount points, Help files and the CMyImageList.
class CDirstatApp : public CWinApp {
public:
	CDirstatApp( ) : m_workingSet( 0 ), m_lastPeriodicalRamUsageUpdate( GetTickCount64( ) ), m_altEncryptionColor( GetAlternativeColor( RGB( 0x00, 0x80, 0x00 ), _T( "AltEncryptionColor" ) ) ) { }
	virtual ~CDirstatApp( ) {
		//delete m_pDocTemplate;
		m_pDocTemplate = { NULL };
		}
	virtual BOOL InitInstance                  ( ) override;
	virtual INT  ExitInstance                   ( ) override;

	void PeriodicalUpdateRamUsage              (                                           );
	void UpdateRamUsage                        (                                           );
	
	_Success_( SUCCEEDED( return ) ) HRESULT GetCurrentProcessMemoryInfo        ( _Out_writes_z_( strSize ) _Pre_writable_size_( strSize ) PWSTR psz_formatted_usage, _In_range_( 50, 64 ) rsize_t strSize );

	CMountPoints              m_mountPoints;                    // Mount point information
protected:
	_Success_( return == true ) bool UpdateMemoryInfo                      (                                                                    );

	_Success_( return != clrDefault ) COLORREF GetAlternativeColor               ( _In_ const COLORREF clrDefault, _In_z_ PCWSTR which );
	virtual BOOL OnIdle                        ( _In_ LONG lCount                        ) override;		// This is, where scanning is done.

	CSingleDocTemplate*       m_pDocTemplate;                   // MFC voodoo.
	
	SIZE_T                    m_workingSet;					    // Current working set (RAM usage)
	unsigned long long        m_lastPeriodicalRamUsageUpdate;	// Tick count
	
public:
	COLORREF                  m_altEncryptionColor;			    // Coloring of encrypted items
	private:
	
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnFileOpen();
	afx_msg void OnAppAbout();

	};