// windirstat.h	- Main header for the windirstat application
//
// see `file_header_text.txt` for licensing & contact info. If you can't find that file, then assume you're NOT allowed to do whatever you wanted to do.

#pragma once

#include "stdafx.h"

#ifndef WDS_WINDIRSTAT_H
#define WDS_WINDIRSTAT_H

WDS_FILE_INCLUDE_MESSAGE

#include "mountpoints.h"//CMountPoints : m_mountPoints, else we'd have to use PIMPL

class CMainFrame;
class CDirstatApp;
class CMountPoints;


// Frequently used "globals"
CMainFrame*  GetMainFrame( );
CDirstatApp* GetApp( );

// CDirstatApp. The MFC application object. Knows about RAM Usage, Mount points, Help files and the CMyImageList.
class CDirstatApp final : public CWinApp {
public:
	CDirstatApp( ) noexcept;
	virtual ~CDirstatApp( ) final;

	CDirstatApp& operator=( const CDirstatApp& in ) = delete;
	CDirstatApp( const CDirstatApp& in ) = delete;


	virtual BOOL InitInstance                  ( ) override final;
	virtual INT  ExitInstance                  ( ) override final;

	//The compiler will automatically inline if /Ob2 is on, so we'll ask anyways.
	void PeriodicalUpdateRamUsage( ) noexcept {
		if ( ::GetTickCount64( ) - m_lastPeriodicalRamUsageUpdate > RAM_USAGE_UPDATE_INTERVAL ) {
			UpdateRamUsage( );
			m_lastPeriodicalRamUsageUpdate = ::GetTickCount64( );
			}
		}

	//The compiler will automatically inline if /Ob2 is on, so we'll ask anyways.
	void UpdateRamUsage( ) noexcept {
		CWinThread::OnIdle( 0 );
		}
	
	_Success_( SUCCEEDED( return ) )
	HRESULT GetCurrentProcessMemoryInfo        ( _Out_writes_z_( strSize ) _Pre_writable_size_( strSize ) PWSTR psz_formatted_usage, _In_range_( 50, 64 ) const rsize_t strSize ) noexcept;

	
protected:
	//_Success_( return == true )
	//bool UpdateMemoryInfo                      (                                                                    ) noexcept;

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
	CMainFrame*               m_frameptr;
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnFileOpen();
	afx_msg void OnFileOpenLight();
	afx_msg void OnAppAbout();

	};

#else

#endif
