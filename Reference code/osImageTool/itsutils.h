/* (C) 2003 XDA Developers
 * Author: Willem Jan Hengeveld <itsme@xs4all.nl>
 * Web: http://www.xda-developers.com/
 *
 * $Header$
 */

#ifdef ITSUTILS_EXPORTS
#define ITSUTILS_API extern "C" __declspec(dllexport)
#else
//#define ITSUTILS_API extern "C" __declspec(dllimport)
#define ITSUTILS_API
#endif

struct IRAPIStream;

ITSUTILS_API HRESULT STDAPICALLTYPE ITGetVersion(
        DWORD cbInput, void *pbInput,
        DWORD *pcbOutput, void **ppbOutput,
        IRAPIStream *pStream);

typedef struct _tagTerminateProcessParams {
	WCHAR wszProcessName[1];
} TerminateProcessParams;

ITSUTILS_API HRESULT STDAPICALLTYPE ITTerminateProcess(
		DWORD cbInput, TerminateProcessParams *pbInput,
		DWORD *pcbOutput, BYTE **ppbOutput,
		IRAPIStream *pStream);


HANDLE GetProcessHandle(WCHAR *wszProcessName);

typedef struct _tagGetProcessHandleParams {
	WCHAR wszProcessName[1];
} GetProcessHandleParams;

typedef struct _tagGetProcessHandleResult {
	HANDLE hProc;
} GetProcessHandleResult;

ITSUTILS_API HRESULT STDAPICALLTYPE ITGetProcessHandle(
		DWORD cbInput, GetProcessHandleParams *pbInput,
		DWORD *pcbOutput, GetProcessHandleResult **ppbOutput,
		IRAPIStream *pStream);


typedef struct _tagReadProcessMemoryParams {
	HANDLE hProcess;
	DWORD dwOffset;
	DWORD nSize;
} ReadProcessMemoryParams;


typedef struct _tagReadProcessMemoryResult {
	DWORD dwNumberOfBytesRead;
	BYTE buffer[1];
} ReadProcessMemoryResult;
ITSUTILS_API HRESULT STDAPICALLTYPE ITReadProcessMemory(
		DWORD cbInput, ReadProcessMemoryParams *pbInput,
		DWORD *pcbOutput, ReadProcessMemoryResult **ppbOutput,
		IRAPIStream *pStream);


typedef struct _tagWriteProcessMemoryParams {
	HANDLE hProcess;
	DWORD dwOffset;
	DWORD nSize;
	BYTE buffer[1];
} WriteProcessMemoryParams;

typedef struct _tagWriteProcessMemoryResult {
	DWORD dwNumberOfBytesWritten;
} WriteProcessMemoryResult;

ITSUTILS_API HRESULT STDAPICALLTYPE ITWriteProcessMemory(
		DWORD cbInput, WriteProcessMemoryParams *pbInput,
		DWORD *pcbOutput, WriteProcessMemoryResult **ppbOutput,
		IRAPIStream *pStream);

//-------------------------------
typedef struct _tagSDCardInfoParams {
	DWORD dwDiskNr;
} SDCardInfoParams;

typedef struct _tagSDCardInfoResult {
	DWORD blockSize;
    DWORD totalBlocks;
} SDCardInfoResult;

ITSUTILS_API HRESULT STDAPICALLTYPE ITSDCardInfo(
		DWORD cbInput, SDCardInfoParams *pbInput,
		DWORD *pcbOutput, SDCardInfoResult **ppbOutput,
		IRAPIStream *pStream);


typedef struct _tagReadSDCardParams {
	DWORD dwDiskNr;
	DWORD dwOffset;
	DWORD dwSize;
} ReadSDCardParams;

typedef struct _tagReadSDCardResult {
	DWORD dwNumberOfBytesRead;
    DWORD start;
    DWORD number;
	BYTE buffer[1];
} ReadSDCardResult;

ITSUTILS_API HRESULT STDAPICALLTYPE ITReadSDCard(
		DWORD cbInput, ReadSDCardParams *pbInput,
		DWORD *pcbOutput, ReadSDCardResult **ppbOutput,
		IRAPIStream *pStream);

typedef struct _tagWriteSDCardParams {
	DWORD dwDiskNr;
	DWORD dwOffset;
	DWORD dwSize;
	BYTE buffer[1];
} WriteSDCardParams;

typedef struct _tagWriteSDCardResult {
	DWORD dwNumberOfBytesWritten;
} WriteSDCardResult;

ITSUTILS_API HRESULT STDAPICALLTYPE ITWriteSDCard(
		DWORD cbInput, WriteSDCardParams *pbInput,
		DWORD *pcbOutput, WriteSDCardResult **ppbOutput,
		IRAPIStream *pStream);

//-------------------------------
typedef struct _tagFlashROMParams {
	DWORD dwOffset;
	DWORD dwSize;
    DWORD dwCheckSum;
	BYTE buffer[1];
} FlashROMParams;

typedef struct _tagFlashROMResult {
	DWORD dwResult;
	DWORD dwReturned;
} FlashROMResult;


//!!!!! this call has not been tested yet, and may destroy your device !!!!!!
ITSUTILS_API HRESULT STDAPICALLTYPE ITFlashROM(
		DWORD cbInput, FlashROMParams *pbInput,
		DWORD *pcbOutput, FlashROMResult **ppbOutput,
		IRAPIStream *pStream);


//-------------------------------
typedef struct _tagGetContextResult {
    DWORD dwProcessId;
    HANDLE hProcess;
    DWORD dwThreadId;
    HANDLE hThread;
    DWORD dwCurrentTrust;
    DWORD dwCallerTrust;
    WCHAR wszCmdLine[1];
} GetContextResult;

ITSUTILS_API HRESULT STDAPICALLTYPE ITGetContext(
		DWORD cbInput, BYTE *pbInput,
		DWORD *pcbOutput, GetContextResult **ppbOutput,
		IRAPIStream *pStream);

typedef struct tagCEPROCESSENTRY { 
    DWORD cntUsage; 
    DWORD dwProcessID; 
    DWORD dwDefaultHeapID; 
    DWORD dwModuleID; 
    DWORD cntThreads; 
    DWORD dwParentProcessID; 
    DWORD dwFlags; 
    WCHAR szExeFile[MAX_PATH]; 
    WCHAR szCmdLine[MAX_PATH]; 
    DWORD dwMemoryBase;
    DWORD dwAccessKey;

    DWORD dwDefaultHeapSize;
    DWORD dwKernelTime;
    DWORD dwUserTime;
} CEPROCESSENTRY; 

typedef struct _tagGetProcessListResult {
    int nEntries;
    CEPROCESSENTRY pe[1];
} GetProcessListResult;

ITSUTILS_API HRESULT STDAPICALLTYPE ITGetProcessList(
		DWORD cbInput, BYTE *pbInput,
		DWORD *pcbOutput, GetProcessListResult **ppbOutput,
		IRAPIStream *pStream);

typedef struct _tagFindNextMemoryBlockParams {
	DWORD dwStartSearchAddress;
} FindNextMemoryBlockParams;

typedef struct _tagFindNextMemoryBlockResult {
	DWORD dwBlockStart;
	DWORD dwBlockLength;
} FindNextMemoryBlockResult;

ITSUTILS_API HRESULT STDAPICALLTYPE ITFindNextMemoryBlock(
		DWORD cbInput, FindNextMemoryBlockParams *pbInput,
		DWORD *pcbOutput, FindNextMemoryBlockResult **ppbOutput,
		IRAPIStream *pStream);

typedef struct _tagGetThreadTImesParams {
	HANDLE hThread;
} GetThreadTimesParams;

typedef struct _tagGetThreadTImesResult {
	FILETIME tCreate;
	FILETIME tExit;
	FILETIME tKernel;
	FILETIME tUser;
} GetThreadTimesResult;
ITSUTILS_API HRESULT STDAPICALLTYPE ITGetThreadTimes(
		DWORD cbInput, GetThreadTimesParams *pbInput,
		DWORD *pcbOutput, GetThreadTimesResult **ppbOutput,
		IRAPIStream *pStream);

#ifndef MAX_PROCESSES 
#define MAX_PROCESSES 32
#endif
typedef struct _tagSummaryInfo {
    HANDLE hProc;
    DWORD tKernel;
    DWORD tUser;
} SummaryInfo;
typedef struct _tagGetProcessUsageListParams {
} GetProcessUsageListParams;
typedef struct _tagGetProcessUsageListResult {
    DWORD tQuery;
    SummaryInfo list[MAX_PROCESSES];
} GetProcessUsageListResult;
ITSUTILS_API HRESULT STDAPICALLTYPE ITGetProcessUsageList(
		DWORD cbInput, GetProcessUsageListParams *pbInput,
		DWORD *pcbOutput, GetProcessUsageListResult **ppbOutput,
		IRAPIStream *pStream);

//----------------debugger------------------
typedef struct _tagDebugEvent {
    DWORD dwProcessId;
    DWORD dwThreadId;
    DWORD dwDebugEventCode;
    char description[1024-3*4];
} DebugEvent;
#define MAX_DEBUG_EVENTS 16
typedef struct _tagDebuggerInfo {
    char magic[4];
    HANDLE evStarted;
    HANDLE evStopped;
    HANDLE evBuffer;
    HANDLE smBuffer;
    DWORD dwThreadResult;
    DWORD dwDebuggedProcessId;

    WCHAR wszExeName[MAX_PATH];
    WCHAR wszCmdLine[MAX_PATH];

    HANDLE hThread;
    DWORD dwThreadId;
 	bool bThreadStarted;
 	bool bThreadTerminated;
    HMODULE hDll;

    DWORD dwMissedEvents;

    DWORD bufsize;
    DWORD bufhead;
    DWORD buftail;
    DWORD bufmaxsize;
    DebugEvent buffer[MAX_DEBUG_EVENTS];
} DebuggerInfo;

typedef struct _tagStartDebuggingProcessParams {
    DWORD dwProcessId;
    WCHAR wszExeName[MAX_PATH];
    WCHAR wszCmdLine[MAX_PATH];
} StartDebuggingProcessParams;
typedef struct _tagStartDebuggingProcessResult {
    DWORD debugger;
} StartDebuggingProcessResult;

ITSUTILS_API HRESULT STDAPICALLTYPE ITStartDebuggingProcess(
		DWORD cbInput, StartDebuggingProcessParams *pbInput,
		DWORD *pcbOutput, StartDebuggingProcessResult **ppbOutput,
		IRAPIStream *pStream);

typedef struct _tagStopDebuggingProcessParams {
    DWORD debugger;
} StopDebuggingProcessParams;
//typedef struct _tagStopDebuggingProcessResult {
//} StopDebuggingProcessResult;
typedef BYTE StopDebuggingProcessResult;

ITSUTILS_API HRESULT STDAPICALLTYPE ITStopDebuggingProcess(
		DWORD cbInput, StopDebuggingProcessParams *pbInput,
		DWORD *pcbOutput, StopDebuggingProcessResult **ppbOutput,
		IRAPIStream *pStream);

typedef struct _tagWaitForDebugEventParams {
    DWORD debugger;
} WaitForDebugEventParams;
typedef struct _tagWaitForDebugEventResult {
    DebugEvent ev;
    DWORD dwMissedEvents;
} WaitForDebugEventResult;

ITSUTILS_API HRESULT STDAPICALLTYPE ITWaitForDebugEvent(
		DWORD cbInput, WaitForDebugEventParams *pbInput,
		DWORD *pcbOutput, WaitForDebugEventResult **ppbOutput,
		IRAPIStream *pStream);

bool GetDebugEvent(DebuggerInfo *di, DebugEvent *ev);
bool ProcessDebugEvent(DebuggerInfo *di, DEBUG_EVENT *sysev);
int ParseDebugEvent(char *buf, int bufsize, DEBUG_EVENT *event);

