//http://blog.airesoft.co.uk/code/ntdll.h

//http://gate.upm.ro/os/LABs/Windows_OS_Internals_Curriculum_Resource_Kit-ACADEMIC/WindowsResearchKernel-WRK/WRK-v1.2/base/ntos/io/iomgr/dir.c
//http://fy.chalmers.se/~appro/LD_*-gallery/statpatch.c

#pragma warning( push, 3 )

#pragma warning( disable : 4514 )
#pragma warning( disable : 4710 )
//#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shlwapi.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <assert.h>
#include <cstdint>
//#include <ntdef.h>
//#include <ntstatus.h>

#pragma warning( pop )

#ifndef NT_SUCCESS
#define NT_SUCCESS(x) ((x)>=0)
#define STATUS_SUCCESS ((NTSTATUS)0)
#endif

#ifndef STATUS_NO_MORE_FILES
#define STATUS_NO_MORE_FILES 0x80000006L
#endif

#ifndef STATUS_BUFFER_OVERFLOW
#define STATUS_BUFFER_OVERFLOW 0x80000005L
#endif

enum CmdParseResult {
	DISPLAY_USAGE,
	LIST_DIR,
	DEL_FILE,
	ENUM_DIR
	};

// native bits required
#define FILE_DIRECTORY_FILE                     0x00000001
#define FILE_OPEN                               0x00000001
#define FILE_NON_DIRECTORY_FILE                 0x00000040
#define FILE_DELETE_ON_CLOSE                    0x00001000
#define FILE_OPEN_BY_FILE_ID                    0x00002000

#ifndef NT_SUCCESS
#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)
#endif

struct IO_STATUS_BLOCK {
	union {
		NTSTATUS stat;
		PVOID pointer;
		};
	ULONG_PTR info;
	};

typedef struct _UNICODE_STRING {
	USHORT  Length;
	USHORT  MaximumLength;
	PWSTR  Buffer;
	} UNICODE_STRING, *PUNICODE_STRING;

typedef struct _OBJECT_ATTRIBUTES {
	ULONG  Length;
	HANDLE  RootDirectory;
	PUNICODE_STRING  ObjectName;
	ULONG  Attributes;
	PVOID  SecurityDescriptor;
	PVOID  SecurityQualityOfService;
	} OBJECT_ATTRIBUTES, *POBJECT_ATTRIBUTES;

typedef enum _FILE_INFORMATION_CLASS {
	FileDispositionInformation = 13,
	FileIdBothDirectoryInformation = 37
	} FILE_INFORMATION_CLASS, *PFILE_INFORMATION_CLASS;

typedef NTSTATUS( NTAPI* pfnQueryDirFile )(
	HANDLE hFile,
	HANDLE hEvent,
	PVOID pApcRoutine,
	PVOID pApcContext,
	IO_STATUS_BLOCK* ioStatus,
	PVOID pBuffer,
	ULONG bufferSize,
	FILE_INFORMATION_CLASS infoClass,
	BOOLEAN singleEntry,
	PUNICODE_STRING pFileName,
	BOOLEAN restart
);

typedef NTSTATUS( NTAPI* pfnOpenFile )(
	 PHANDLE phFile,
	 ACCESS_MASK amPerms,
	 OBJECT_ATTRIBUTES* pAttrs,
	 IO_STATUS_BLOCK* ioStatus,
	 PLARGE_INTEGER allocationSize,
	 ULONG fileAttributes,
	 ULONG  ShareAccess,
	 ULONG  CreateDisposition,
	 ULONG  CreateOptions,
	 PVOID  EaBuffer,
	 ULONG  EaLength
);

typedef NTSTATUS( NTAPI* pfnSetInfoFile )( HANDLE hFile, IO_STATUS_BLOCK* iosb, PVOID pData, ULONG dataLen, FILE_INFORMATION_CLASS infoClass );

typedef DECLSPEC_ALIGN( 8 ) struct _FILE_ID_BOTH_DIR_INFORMATION {
	ULONG  NextEntryOffset;
	ULONG  FileIndex;
	LARGE_INTEGER  CreationTime;
	LARGE_INTEGER  LastAccessTime;
	LARGE_INTEGER  LastWriteTime;
	LARGE_INTEGER  ChangeTime;
	LARGE_INTEGER  EndOfFile;
	LARGE_INTEGER  AllocationSize;
	ULONG  FileAttributes;
	ULONG  FileNameLength;
	ULONG  EaSize;
	CCHAR  ShortNameLength;
	WCHAR  ShortName[ 12 ];
	LARGE_INTEGER  FileId;
	WCHAR  FileName[ 1 ];
	} FILE_ID_BOTH_DIR_INFORMATION, *PFILE_ID_BOTH_DIR_INFORMATION;

const DOUBLE getAdjustedTimingFrequency( ) {
	LARGE_INTEGER timingFrequency;
	BOOL res1 = QueryPerformanceFrequency( &timingFrequency );
	if ( !res1 ) {
		std::wcout << L"QueryPerformanceFrequency failed!!!!!! Disregard any timing data!!" << std::endl;
		}
	const DOUBLE adjustedTimingFrequency = ( DOUBLE( 1.00 ) / DOUBLE( timingFrequency.QuadPart ) );
	return adjustedTimingFrequency;
	}

class ntdllClass {
	HMODULE ntdll = nullptr;
	FARPROC WINAPI ntQueryDirectoryFuncPtr = nullptr;
	ntdllClass( ) {
		ntdll = GetModuleHandle( L"C:\\Windows\\System32\\ntdll.dll" );
		if ( ntdll ) {
			ntQueryDirectoryFuncPtr = GetProcAddress( ntdll, "NtQueryDirectory" );
			if ( !ntQueryDirectoryFuncPtr ) {
				std::wcout << L"Couldn't find NtQueryDirectoryFile in ntdll.dll!" << std::endl;
				}
			}
		else {
			std::wcout << L"Couldn't load ntdll.dll!" << std::endl;
			}
		}
	};

class ntQueryDirectoryFile_f {
	public:
	typedef NTSTATUS( NTAPI* pfnQueryDirFile )( _In_ HANDLE FileHandle, _In_opt_ HANDLE Event, _In_opt_ PVOID ApcRoutine, _In_opt_ PVOID ApcContext, _Out_  IO_STATUS_BLOCK* IoStatusBlock, _Out_  PVOID FileInformation, _In_ ULONG Length, _In_ FILE_INFORMATION_CLASS FileInformationClass, _In_ BOOLEAN ReturnSingleEntry, _In_opt_ PUNICODE_STRING FileName, _In_ BOOLEAN RestartScan );

	pfnQueryDirFile ntQueryDirectoryFuncPtr = nullptr;
	ntQueryDirectoryFile_f( FARPROC WINAPI ntQueryDirectoryFuncPtr_IN ) : ntQueryDirectoryFuncPtr( reinterpret_cast<pfnQueryDirFile>( ntQueryDirectoryFuncPtr_IN ) ) {
		if ( !ntQueryDirectoryFuncPtr ) {
			throw -1;
			}
		}

	NTSTATUS NTAPI operator()( _In_ HANDLE FileHandle, _In_opt_ HANDLE Event, _In_opt_ PVOID ApcRoutine, _In_opt_ PVOID ApcContext, _Out_  IO_STATUS_BLOCK* IoStatusBlock, _Out_  PVOID FileInformation, _In_ ULONG Length, _In_ FILE_INFORMATION_CLASS FileInformationClass, _In_ BOOLEAN ReturnSingleEntry, _In_opt_ PUNICODE_STRING FileName, _In_ BOOLEAN RestartScan ) {
		return ntQueryDirectoryFuncPtr( FileHandle, Event, ApcRoutine, ApcContext, IoStatusBlock, FileInformation, Length, FileInformationClass, ReturnSingleEntry, FileName, RestartScan );
		}
	};


void DeleteFileByHandle( HANDLE hFile ) {
	assert( false );
	return;
	IO_STATUS_BLOCK iosb = { 0 };

	struct FILE_DISPOSITION_INFORMATION {
		BOOLEAN  DeleteFile;
		} fdi = { TRUE };

	HMODULE hNtdll = GetModuleHandle( L"C:\\Windows\\System32\\ntdll.dll" );
	if ( hNtdll ) {
		pfnSetInfoFile ntSetInformationFile = ( pfnSetInfoFile ) GetProcAddress( hNtdll, "NtSetInformationFile" );
		if ( ntSetInformationFile ) {
			ntSetInformationFile( hFile, &iosb, &fdi, sizeof( fdi ), FileDispositionInformation );
			}
		}
	}

LONGLONG WcsToLLDec( const wchar_t* pNumber, wchar_t* endChar ) {
	LONGLONG temp = 0;
	while ( iswdigit( *pNumber ) ) {
		temp *= 10;
		temp += ( *pNumber - L'0' );
		++pNumber;
		}
	if ( endChar ) {
		*endChar = *pNumber;
		}
	return temp;
	}

CmdParseResult ParseCmdLine( int argc, wchar_t** argv, LONGLONG* fileId ) {
	if ( ( argc < 2 ) || ( ( argv[ 1 ][ 0 ] != L'/' ) && ( argv[ 1 ][ 0 ] != L'-' ) ) ) {
		return DISPLAY_USAGE;
		}
	wchar_t lowerFirstArgChar = towlower( argv[ 1 ][ 1 ] );
	// argv[2] is optional
	if ( lowerFirstArgChar == L'l' ) {
		if ( ( argc == 2 ) || ( PathIsDirectory( argv[ 2 ] ) ) ) {
			return LIST_DIR;
			}
		else return DISPLAY_USAGE;
		}
	else if ( lowerFirstArgChar == L'd' ) {
		wchar_t endChar = 0;
		if ( ( argc < 4 ) || ( ( *fileId = WcsToLLDec( argv[ 3 ], &endChar ), endChar != 0 ) ) ) {
			return DISPLAY_USAGE;
			}
		else return DEL_FILE;
		}
	else if ( lowerFirstArgChar == L'e' ) {
		return ENUM_DIR;
		}
	return DISPLAY_USAGE;
	}

uint64_t ListDirectory( const wchar_t* dir, std::vector<std::wstring>& dirs, std::vector<UCHAR>& idInfo, const bool writeToScreen );

void qDirFile( const wchar_t* dir, std::vector<std::wstring>& dirs, std::uint64_t& numItems, const bool writeToScreen, pfnQueryDirFile ntQueryDirectoryFile, std::wstring& curDir, HANDLE hDir, std::vector<UCHAR>& idInfo ) {
	IO_STATUS_BLOCK iosb = { 0 };
	//for ( auto& a : idInfo ) {
	//	assert( a == 0 );
	//	}
	//idInfo.erase( idInfo.begin( ), idInfo.end( ) );
	NTSTATUS stat = STATUS_PENDING;
	if ( writeToScreen ) {
		std::wcout << L"Files in directory " << dir << L'\n';
		std::wcout << L"      File ID       |       File Name\n";
		}
	assert( idInfo.size( ) > 1 );
	auto buffer = &( idInfo[ 0 ] );
	//++numItems;
	auto sBefore = stat;
	stat = ntQueryDirectoryFile( hDir, NULL, NULL, NULL, &iosb, ( &idInfo[ 0 ] ), idInfo.size( ), FileIdBothDirectoryInformation, FALSE, NULL, TRUE );
	assert( stat != sBefore );
	while ( stat == STATUS_BUFFER_OVERFLOW ) {
		idInfo.erase( idInfo.begin( ), idInfo.end( ) );
		idInfo.resize( idInfo.size( ) * 2 );
		buffer = ( &idInfo[ 0 ] );
		stat = ntQueryDirectoryFile( hDir, NULL, NULL, NULL, &iosb, buffer, idInfo.size( ), FileIdBothDirectoryInformation, FALSE, NULL, TRUE );
		}
	
	auto bufSizeWritten = iosb.info;
	assert( NT_SUCCESS( stat ) );
	for ( size_t i = bufSizeWritten; i < bufSizeWritten + ( sizeof( FILE_ID_BOTH_DIR_INFORMATION ) + ( MAX_PATH * sizeof( UCHAR ) ) * 2 ); ++i ) {
		if ( i == idInfo.size( ) ) {
			break;
			}
		idInfo.at( i ) = 0;
		}
	

	PFILE_ID_BOTH_DIR_INFORMATION pFileInf = PFILE_ID_BOTH_DIR_INFORMATION( &idInfo[ 0 ] );

	assert( pFileInf != NULL );
	std::vector<std::wstring> breadthDirs;
	std::vector<WCHAR> fNameVect;
	while ( NT_SUCCESS( stat ) && ( pFileInf != NULL ) ) {
		//PFILE_ID_BOTH_DIR_INFORMATION pFileInf = ( FILE_ID_BOTH_DIR_INFORMATION* ) buffer;
		fNameVect.clear( );
		fNameVect.reserve( ( pFileInf->FileNameLength / sizeof( WCHAR ) ) + 1 );
		PWCHAR end = pFileInf->FileName + ( pFileInf->FileNameLength / sizeof( WCHAR ) );
		fNameVect.insert( fNameVect.end( ), pFileInf->FileName, end );
		fNameVect.emplace_back( L'\0' );
		//std::wstring thisFileName( fNameVect.begin( ), fNameVect.end( ) );
		PWSTR fNameChar = &( fNameVect[ 0 ] );
		if ( fNameChar[ 0 ] == L'.' && ( fNameChar[ 1 ] == 0 || ( fNameChar[ 1 ] == '.' && ( fNameChar[ 2 ] == 0 ) ) ) ) {
			//continue;
			goto nextItem;
			}
		
		++numItems;
		if ( writeToScreen ) {

			std::wcout << std::setw( std::numeric_limits<LONGLONG>::digits10 ) << pFileInf->FileId.QuadPart << L"    " << std::setw( 0 ) << curDir << L"\\" << fNameChar;

			auto state = std::wcout.fail( );
			if ( state != 0 ) {
				std::wcout.clear( );
				std::wcout << std::endl << L"std::wcout.fail( ): " << state << std::endl;
				}
			}
		if ( pFileInf->FileAttributes & FILE_ATTRIBUTE_DIRECTORY ) {
			if ( writeToScreen ) {
				//std::wcout << L" (Directory)" << std::endl;
				}
			if ( curDir.compare( curDir.length( ) - 2, 2, L"\\" ) == 0 ) {
				DebugBreak( );
				}
			std::wstring dirstr;
			if ( curDir.back( ) != L'\\') {
				dirstr = curDir + L"\\" + fNameChar + L"\\";
				}
			else {
				dirstr = curDir + fNameChar + L"\\";
				}
			//std::wstring dirstr = curDir + L"\\" + fNameChar + L"\\";
			breadthDirs.emplace_back( dirstr );
			//numItems += ListDirectory( dirstr.c_str( ), dirs, idInfo, writeToScreen );
			}
		if ( writeToScreen ) {
			//std::wcout << std::endl;
			}
		
	nextItem:
		//stat = ntQueryDirectoryFile( hDir, NULL, NULL, NULL, &iosb, &idInfo[ 0 ], idInfo.size( ), FileIdBothDirectoryInformation, TRUE, NULL, FALSE );
		if ( writeToScreen ) {
			std::wcout << L"\t\tpFileInf: " << pFileInf << L", pFileInf->NextEntryOffset: " << pFileInf->NextEntryOffset << L", pFileInf + pFileInf->NextEntryOffset " << ( pFileInf + pFileInf->NextEntryOffset ) << std::endl;
			}
		pFileInf = ( pFileInf->NextEntryOffset != 0 ) ? reinterpret_cast<PFILE_ID_BOTH_DIR_INFORMATION>( reinterpret_cast<std::uint64_t>( pFileInf ) + ( static_cast<std::uint64_t>( pFileInf->NextEntryOffset ) ) ) : NULL;
		}

	for ( auto& aDir : breadthDirs ) {
		numItems += ListDirectory( aDir.c_str( ), dirs, idInfo, writeToScreen );
		}
	assert( ( pFileInf == NULL ) || ( !NT_SUCCESS( stat ) ) );
	}

uint64_t ListDirectory( const wchar_t* dir, std::vector<std::wstring>& dirs, std::vector<UCHAR>& idInfo, const bool writeToScreen ) {
	std::wstring curDir;
	std::uint64_t numItems = 0;
	if ( !dir ) {
		curDir.resize( GetCurrentDirectoryW( 0, NULL ) );
		GetCurrentDirectoryW( curDir.size( ), &curDir[ 0 ] );
		dir = curDir.c_str( );
		}
	else {
		curDir = dir;
		}
	HANDLE hDir = CreateFile( dir, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL );
	if ( hDir == INVALID_HANDLE_VALUE ) {
		DWORD err = GetLastError( );
		std::wcout << L"Failed to open directory " << dir << L" because of error " << err << std::endl;
		return numItems;
		}
	HMODULE hNtDll = GetModuleHandle( L"C:\\Windows\\System32\\ntdll.dll" );
	if ( hNtDll == NULL ) {
		return numItems;
		}
	pfnQueryDirFile ntQueryDirectoryFile = ( pfnQueryDirFile ) GetProcAddress( hNtDll, "NtQueryDirectoryFile" );
	if ( !ntQueryDirectoryFile ) {
		std::wcout << L"Couldn't find NtQueryDirectoryFile in ntdll.dll!" << std::endl;
		return numItems;
		}
	
	qDirFile( dir, dirs, numItems, writeToScreen, ntQueryDirectoryFile, curDir, hDir, idInfo );
	if ( writeToScreen ) {
		std::wcout << std::setw( std::numeric_limits<LONGLONG>::digits10 ) << numItems << std::setw( 0 ) << L" items in directory " << dir << std::endl;
		}
	CloseHandle( hDir );
	return numItems + dirs.size( );
	}

void DelFile( WCHAR fileVolume, LONGLONG fileId ) {
	assert( false );
	return;
	HMODULE hNtdll = GetModuleHandleW( L"C:\\Windows\\System32\\ntdll.dll" );
	if ( hNtdll == NULL ) {
		return;
		}
	pfnOpenFile ntCreateFile = ( pfnOpenFile ) GetProcAddress( hNtdll, "NtCreateFile" );
	WCHAR volumePath[ ] = { L'\\', L'\\', L'.', L'\\', fileVolume, L':', 0 };
	HANDLE hVolume = CreateFileW( volumePath, 0, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0x80, NULL );
	if ( hVolume == INVALID_HANDLE_VALUE ) {
		DWORD err = GetLastError( );
		std::wcout << L"Failed to open volume " << fileVolume << L": because of error " << err << L'\n';
		return;
		}
	HANDLE hFile = NULL;
	IO_STATUS_BLOCK iosb = { 0 };
	OBJECT_ATTRIBUTES oa = { sizeof( oa ), 0 };
	UNICODE_STRING name;
	name.Buffer = ( PWSTR ) &fileId;
	name.Length = name.MaximumLength = sizeof( fileId );
	oa.ObjectName = &name;
	oa.RootDirectory = hVolume;
	NTSTATUS stat = ntCreateFile(
		&hFile,
		GENERIC_READ | GENERIC_WRITE | DELETE,
		&oa,
		&iosb,
		NULL,
		FILE_ATTRIBUTE_NORMAL,
		0,
		FILE_OPEN,
		FILE_NON_DIRECTORY_FILE | FILE_OPEN_BY_FILE_ID,
		NULL,
		0
	);
	CloseHandle( hVolume );
	if ( !NT_SUCCESS( stat ) ) {
		std::cout << "Failed to delete file because of error " << std::hex << stat << '\n';
		}
	else {
		BYTE buffer[ ( ( sizeof( WCHAR ) * MAX_PATH ) + sizeof( ULONG ) ) ] = { 0 };
		GetFileInformationByHandleEx( hFile, FileNameInfo, buffer, sizeof( buffer ) );
		FILE_NAME_INFO* pFni = ( FILE_NAME_INFO* ) buffer;
		std::wcout << L"Deleting " << pFni->FileName << L'\n';
		DeleteFileByHandle( hFile );
		CloseHandle( hFile );
		std::cout << "File deleted\n";
		}
	}

void RecurseListDirectory( const wchar_t* dir, std::vector<std::wstring>& dirs, const bool writeToScreen ) {
	std::vector<UCHAR> idInfo( ( sizeof( FILE_ID_BOTH_DIR_INFORMATION ) + ( MAX_PATH * sizeof( UCHAR ) ) ) * 500000 );
	std::uint64_t items = 0;
	std::vector<std::wstring> downDirs;
	for ( auto& dir : dirs ) {
		//auto aDir = dirs.at( dirs.size( ) - 1 );
		//dirs.pop_back( );
		items += ListDirectory( dir.c_str( ), downDirs, idInfo, writeToScreen );
		++items;
		}
	std::wcout << L"Total items in directory tree of " << dir << L": " << items << std::endl;
	}

int __cdecl wmain( int argc, wchar_t** argv ) {
		{
		LONGLONG fileId = 0;
		CmdParseResult res = ParseCmdLine( argc, argv, &fileId );
		std::vector<std::wstring> dirs;
		dirs.reserve( 1000 );
		for ( size_t i = 0; i < argc; ++i ) {
			std::wcout << L"arg # " << i << L": " << argv[ i ] << std::endl;
			}
		if ( res == DISPLAY_USAGE ) {
			std::cout <<
				"Usage:\n"
				"FileId /list <C:\\Path\\To\\Directory>\n"
				"FileId /delete <volume> FileId\n"
				"\twhere <volume> is the drive letter of the drive the file id is on";
			return -1;
			}
		if ( res == LIST_DIR ) {
			dirs.emplace_back( argv[ 2 ] );
			//ListDirectory( argc < 3 ? NULL : argv[ 2 ], dirs, true );
			}
		if ( res == ENUM_DIR ) {
			std::wstring _path( argv[ 2 ] );
			std::wcout << L"Arg: " << _path << std::endl;
			std::wstring nativePath = L"\\\\?\\" + _path;
			std::wcout << L"'native' path: " << nativePath << std::endl;
			dirs.emplace_back( nativePath );
			LARGE_INTEGER startTime = { 0 };
			LARGE_INTEGER endTime = { 0 };
	
			//std::int64_t fileSizeTotal = 0;
			auto adjustedTimingFrequency = getAdjustedTimingFrequency( );
			BOOL res2 = QueryPerformanceCounter( &startTime );

			RecurseListDirectory( nativePath.c_str( ), dirs, false );

			BOOL res3 = QueryPerformanceCounter( &endTime );
	
			if ( ( !res2 ) || ( !res3 ) ) {
				std::wcout << L"QueryPerformanceCounter Failed!!!!!! Disregard any timing data!!" << std::endl;
				}
			auto totalTime = ( endTime.QuadPart - startTime.QuadPart ) * adjustedTimingFrequency;

			std::wcout << L"Time in seconds: " << totalTime << std::endl;
			
			//ListDirectory( argc < 3 ? NULL : argv[ 2 ], dirs, false );
			}
		else // DEL_FILE
			{
			return 0;
			DelFile( argv[ 2 ][ 0 ], fileId );
			
			}
		auto state = std::wcout.fail( );
		std::wcout.clear( );
		std::wcout << L"std::wcout.fail( ): " << state << std::endl;
		return 0;
		}
	}