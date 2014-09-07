#include "stdafx.h"
//#include "memoryUsage.h"


void startMemUsage( ) {
	AfxBeginThread( RUNTIME_CLASS( MemoryUsage ), NULL );
	}

IMPLEMENT_DYNCREATE(MemoryUsage, CWinThread)

BEGIN_MESSAGE_MAP(MemoryUsage, CWinThread)
END_MESSAGE_MAP()

BOOL MemoryUsage::InitInstance( ) {
	CWinThread::InitInstance( );
	LoopInfoTrace( );
	return TRUE;
	}

void MemoryUsage::UpdateInfo( ) {
	auto pmc = zeroInitPROCESS_MEMORY_COUNTERS( );
	if ( GetProcessMemoryInfo( GetCurrentProcess( ), &pmc, sizeof( pmc ) ) ) {
		TRACE( _T( "%lu\r\n" ), pmc.WorkingSetSize );
		m_workingSet = pmc.WorkingSetSize;
		}
	}

void MemoryUsage::LoopInfoTrace( ) {
	while ( true ) {
		UpdateInfo( );
		Sleep( 1000 );

		}
	}



//CString CDirstatApp::GetCurrentProcessMemoryInfo( ) {
//	auto workingSetBefore = m_workingSet;
//
//	UpdateMemoryInfo( );
//	auto difference = m_workingSet - workingSetBefore;
//	if ( m_workingSet == workingSetBefore && ( m_MemUsageCache != _T( "" ) ) ) {
//		return m_MemUsageCache;
//		}
//	else if ( abs( difference ) < ( m_workingSet * 0.01 ) && ( m_MemUsageCache != _T( "" ) ) ) {
//		return m_MemUsageCache;
//		}
//	else if ( m_workingSet == 0 ) {
//		return _T( "" );
//		}
//
//	else {
//		CString n = ( _T( "RAM Usage: %s" ), FormatBytes( m_workingSet ) );
//		m_MemUsageCache = n;
//		return n;
//		}
//	}
//
//bool CDirstatApp::UpdateMemoryInfo( ) {
//	if ( !m_psapi.IsSupported( ) ) {
//		return false;
//		}
//
//	auto pmc = zeroInitPROCESS_MEMORY_COUNTERS( );
//	pmc.cb = sizeof( pmc );
//
//	if ( !m_psapi.GetProcessMemoryInfo( GetCurrentProcess( ), &pmc, sizeof( pmc ) ) ) {
//		return false;
//		}
//
//	m_workingSet = pmc.WorkingSetSize;
//
//	bool ret = false;
//	if ( pmc.PageFaultCount > m_pageFaults + 500 ) {
//		ret = true;
//		}
//
//	m_pageFaults = pmc.PageFaultCount;
//
//	return ret;
//	}
