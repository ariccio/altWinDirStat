
#include "stdafx.h"

#ifndef ABOUTDLG_H
#include "aboutdlg.h"
#else
#error 
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////

void StartAboutDialog( ) {
	AfxBeginThread( RUNTIME_CLASS( CAboutThread ), NULL );
	}


/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE( CAboutThread, CWinThread );

BOOL CAboutThread::InitInstance( ) {
	CWinThread::InitInstance( );

	//CAboutDlg dlg;
	//dlg.DoModal( );
	return false;
	}
