#pragma once

#include "stdafx.h"
//#include "globalhelpers.h"

//#include <Windows.h>
//#include <Psapi.h>


void startMemUsage( );

class MemoryUsage : public CWinThread {
	DECLARE_DYNCREATE( MemoryUsage );
protected:
	virtual BOOL InitInstance( );
	LONGLONG m_workingSet;// Current working set (RAM usage)
	std::uint64_t m_lastPeriodicalRamUsageUpdate;// Tick count
	void LoopInfoTrace( );
	void UpdateInfo( );
	//BOOL GetProcessMemoryInfo( _In_ HANDLE Process, _Inout_ PPROCESS_MEMORY_COUNTERS ppsmemCounters, _In_ DWORD cb );

	DECLARE_MESSAGE_MAP()
private:
	CString m_MemUsageCache;
	};