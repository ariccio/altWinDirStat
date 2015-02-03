// windirstat.h	- Main header for the windirstat application
//
// see `file_header_text.txt` for licensing & contact info.

#pragma once

#include "stdafx.h"

#ifndef WDS_WINDIRSTAT_H
#define WDS_WINDIRSTAT_H

#include "mountpoints.h"//CMountPoints : m_mountPoints, else we'd have to use PIMPL

class CMainFrame;
class CDirstatApp;
class CMountPoints;


// Frequently used "globals"
CMainFrame*   GetMainFrame( );
CDirstatApp*  GetApp( );

// CDirstatApp. The MFC application object. Knows about RAM Usage, Mount points, Help files and the CMyImageList.
class CDirstatApp final : public CWinApp {
public:
	CDirstatApp( );
	virtual ~CDirstatApp( ) final;

	CDirstatApp& operator=( const CDirstatApp& in ) = delete;
	CDirstatApp( const CDirstatApp& in ) = delete;


	virtual BOOL InitInstance                  ( ) override final;
	virtual INT  ExitInstance                  ( ) override final;

	void PeriodicalUpdateRamUsage              (                                           );
	void UpdateRamUsage                        (                                           );
	
	_Success_( SUCCEEDED( return ) )
	HRESULT GetCurrentProcessMemoryInfo        ( _Out_writes_z_( strSize ) _Pre_writable_size_( strSize ) PWSTR psz_formatted_usage, _In_range_( 50, 64 ) const rsize_t strSize );

	
protected:
	_Success_( return == true )
	bool UpdateMemoryInfo                      (                                                                    );

	_Success_( return != clrDefault )
	COLORREF GetAlternativeColor               ( _In_ const COLORREF clrDefault, _In_z_ PCWSTR which );

	virtual BOOL OnIdle                        ( _In_ LONG lCount                        ) override final;		// This was, where scanning was done.

public:
	CMountPoints              m_mountPoints;                    // Mount point information

protected:

	CSingleDocTemplate*       m_pDocTemplate;                   // MFC voodoo.
	
	SIZE_T                    m_workingSet;					    // Current working set (RAM usage)
	unsigned long long        m_lastPeriodicalRamUsageUpdate;	// Tick count
	
public:
	//C4820: 'CDirstatApp' : '4' bytes padding added after data member 'CDirstatApp::m_altEncryptionColor'
	COLORREF                  m_altEncryptionColor;			    // Coloring of encrypted items
	private:
	
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnFileOpen();
	afx_msg void OnAppAbout();

	};

#else

#endif
