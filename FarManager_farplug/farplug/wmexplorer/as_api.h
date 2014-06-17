#ifndef _AS_API_H
#define _AS_API_H

#define RAPI(proc) (g_rapi2 ? (session->proc) : (rapi.proc))

#define CHECK_RAPI(condition) { \
  if (!(condition)) { \
    CHECK_COM(RAPI(CeRapiGetError())); \
    CHECK_ADVAPI(RAPI(CeGetLastError())); \
    assert(false); \
  } \
}

typedef HRESULT __stdcall PCeRapiInitEx(RAPIINIT* pRapiInit);
typedef HRESULT __stdcall PCeRapiUninit();
typedef BOOL __stdcall PCeFindAllFiles(LPCWSTR szPath, DWORD dwFlags, LPDWORD lpdwFoundCount, LPLPCE_FIND_DATA ppFindDataArray);
typedef HRESULT __stdcall PCeRapiFreeBuffer(LPVOID Buffer);
typedef HANDLE __stdcall PCeFindFirstFile(LPCWSTR lpFileName, LPCE_FIND_DATA lpFindFileData);
typedef BOOL __stdcall PCeFindNextFile(HANDLE hFindFile, LPCE_FIND_DATA lpFindFileData);
typedef BOOL __stdcall PCeFindClose(HANDLE hFindFile);
typedef HANDLE __stdcall PCeCreateFile(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
typedef BOOL __stdcall PCeCloseHandle(HANDLE hObject);
typedef BOOL __stdcall PCeReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped);
typedef BOOL __stdcall PCeWriteFile(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped);
typedef BOOL __stdcall PCeDeleteFile(LPCWSTR lpFileName);
typedef DWORD __stdcall PCeGetFileAttributes(LPCWSTR lpFileName);
typedef BOOL __stdcall PCeSetFileAttributes(LPCWSTR lpFileName, DWORD dwFileAttributes);
typedef BOOL __stdcall PCeSetFileTime(HANDLE hFile, LPFILETIME lpCreationTime, LPFILETIME lpLastAccessTime, LPFILETIME lpLastWriteTime);
typedef BOOL __stdcall PCeCreateDirectory(LPCWSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes);
typedef BOOL __stdcall PCeRemoveDirectory(LPCWSTR lpPathName);
typedef BOOL __stdcall PCeMoveFile(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName);
typedef BOOL __stdcall PCeCopyFile(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, BOOL bFailIfExists);
typedef BOOL __stdcall PCeGetVersionEx(LPCEOSVERSIONINFO lpVersionInformation);
typedef void __stdcall PCeGlobalMemoryStatus(LPMEMORYSTATUS lpmst);
typedef BOOL __stdcall PCeGetDiskFreeSpaceEx(LPCWSTR lpDirectoryName, PULARGE_INTEGER lpFreeBytesAvailable, PULARGE_INTEGER lpTotalNumberOfBytes, PULARGE_INTEGER lpTotalNumberOfFreeBytes);
typedef LONG __stdcall PCeRegOpenKeyEx(HKEY hKey, LPCWSTR lpszSubKey, DWORD ulOptions, REGSAM samDesired, PHKEY phkResult);
typedef LONG __stdcall PCeRegQueryValueEx(HKEY hKey, LPCWSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData);
typedef LONG __stdcall PCeRegCloseKey(HKEY hKey);
typedef BOOL __stdcall PCeCreateProcess(LPCWSTR lpApplicationName, LPCWSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPWSTR lpCurrentDirectory, LPSTARTUPINFO lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation);
typedef BOOL __stdcall PCeGetSystemPowerStatusEx(PSYSTEM_POWER_STATUS_EX pstatus, BOOL fUpdate);
typedef DWORD __stdcall PCeGetLastError();
typedef HRESULT __stdcall PCeRapiGetError();

typedef HRESULT __stdcall PCeSvcOpenW(UINT uSvc, LPWSTR pszPath, BOOL fCreate, PHCESVC phSvc);
typedef HRESULT __stdcall PCeSvcClose(HCESVC hSvc);
typedef HRESULT __stdcall PCeSvcEnumProfiles(PHCESVC phSvc, DWORD lProfileIndex, PDWORD plProfile);
typedef HRESULT __stdcall PCeSvcOpenExW(HCESVC hSvcRoot, LPWSTR pszPath, BOOL fCreate, PHCESVC phSvc);

struct RapiLibrary {
  PCeRapiInitEx* CeRapiInitEx;
  PCeRapiUninit* CeRapiUninit;
  PCeFindAllFiles* CeFindAllFiles;
  PCeRapiFreeBuffer* CeRapiFreeBuffer;
  PCeFindFirstFile* CeFindFirstFile;
  PCeFindNextFile* CeFindNextFile;
  PCeFindClose* CeFindClose;
  PCeCreateFile* CeCreateFile;
  PCeCloseHandle* CeCloseHandle;
  PCeReadFile* CeReadFile;
  PCeWriteFile* CeWriteFile;
  PCeDeleteFile* CeDeleteFile;
  PCeGetFileAttributes* CeGetFileAttributes;
  PCeSetFileAttributes* CeSetFileAttributes;
  PCeSetFileTime* CeSetFileTime;
  PCeCreateDirectory* CeCreateDirectory;
  PCeRemoveDirectory* CeRemoveDirectory;
  PCeMoveFile* CeMoveFile;
  PCeCopyFile* CeCopyFile;
  PCeGetVersionEx* CeGetVersionEx;
  PCeGlobalMemoryStatus* CeGlobalMemoryStatus;
  PCeGetDiskFreeSpaceEx* CeGetDiskFreeSpaceEx;
  PCeRegOpenKeyEx* CeRegOpenKeyEx;
  PCeRegQueryValueEx* CeRegQueryValueEx;
  PCeRegCloseKey* CeRegCloseKey;
  PCeCreateProcess* CeCreateProcess;
  PCeGetSystemPowerStatusEx* CeGetSystemPowerStatusEx;
  PCeGetLastError* CeGetLastError;
  PCeRapiGetError* CeRapiGetError;
};

struct CeUtilLibrary {
  PCeSvcOpenW* CeSvcOpenW;
  PCeSvcClose* CeSvcClose;
  PCeSvcEnumProfiles* CeSvcEnumProfiles;
  PCeSvcOpenExW* CeSvcOpenExW;
};

extern RapiLibrary rapi;
extern CeUtilLibrary ceutil;

#endif // _AS_API_H
