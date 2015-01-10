//http://blog.airesoft.co.uk/code/ntdll.h
//http://gate.upm.ro/os/LABs/Windows_OS_Internals_Curriculum_Resource_Kit-ACADEMIC/WindowsResearchKernel-WRK/WRK-v1.2/base/ntos/io/iomgr/dir.c
//http://fy.chalmers.se/~appro/LD_*-gallery/statpatch.c

//On second thought: you don't need the DDK!
//NOTE: the WDK is not the DDK!
//Some important files are in the DDK!
//It's a bit hard to find the DDK!
//I found it here: https://dev.windowsphone.com/en-us/OEM/docs/Getting_Started/Preparing_for_Windows_Phone_development?logged_in=1






#pragma warning( push, 3 )

#pragma warning( disable : 4514 )
#pragma warning( disable : 4710 )
#include "NT_Q_DIR_TEST_2.h"


#pragma warning( pop )

std::wstring handyDandyErrMsgFormatter( const DWORD last_err ) {
	const size_t msgBufSize = 2 * 1024;
	wchar_t msgBuf[ msgBufSize ] = { 0 };
	//auto err = GetLastError( );
	auto ret = FormatMessageW( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, last_err, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), msgBuf, msgBufSize, NULL );
	if ( ret > 0 ) {
		return std::wstring( msgBuf );
		}
	return std::wstring( L"FormatMessage failed to format an error!" );
	}

_Success_( return != UINT64_MAX )
std::uint64_t GetCompressedFileSize_filename( const std::wstring path ) {
	ULARGE_INTEGER ret;
	const auto last_err_old = GetLastError( );
	ret.QuadPart = 0;//zero initializing this is critical!
	ret.LowPart = GetCompressedFileSizeW( path.c_str( ), &ret.HighPart );
	const auto last_err = GetLastError( );
	if ( ret.LowPart == INVALID_FILE_SIZE ) {
		if ( ret.HighPart != NULL ) {
			if ( ( last_err != NO_ERROR ) && ( last_err != last_err_old ) ) {
				fwprintf( stderr, L"Error! Filepath: %s, Filepath length: %i, GetLastError: %s\r\n", path.c_str( ), int( path.length( ) ), handyDandyErrMsgFormatter( last_err ).c_str( ) );
				return UINT64_MAX;// IN case of an error return size from CFileFind object
				}
			fwprintf( stderr, L"WTF ERROR! Filepath: %s, Filepath length: %i, GetLastError: %s\r\n", path.c_str( ), int( path.length( ) ), handyDandyErrMsgFormatter( last_err ).c_str( ) );
			return UINT64_MAX;
			}
		else {
			if ( ( last_err != NO_ERROR ) && ( last_err != last_err_old ) ) {
				fwprintf( stderr, L"Error! Filepath: %s, Filepath length: %i, GetLastError: %s\r\n", path.c_str( ), int( path.length( ) ), handyDandyErrMsgFormatter( last_err ).c_str( ) );
				return UINT64_MAX;
				}
			return ret.QuadPart;
			}
		}
	return ret.QuadPart;
	}




enum CmdParseResult {
	DISPLAY_USAGE,
	LIST_DIR,
	DEL_FILE,
	ENUM_DIR
	};

std::wstring handyDandyErrMsgFormatter( ) {
	const size_t msgBufSize = 2 * 1024;
	wchar_t msgBuf[ msgBufSize ] = { 0 };
	auto err = GetLastError( );
	auto ret = FormatMessageW( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, err, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), msgBuf, msgBufSize, NULL );
	if ( ret > 0 ) {
		return std::wstring( msgBuf );
		}
	return std::wstring( L"FormatMessage failed to format an error!" );
	}

const DOUBLE getAdjustedTimingFrequency( ) {
	LARGE_INTEGER timingFrequency;
	BOOL res1 = QueryPerformanceFrequency( &timingFrequency );
	if ( !res1 ) {
		fwprintf( stderr, L"QueryPerformanceFrequency failed!!!!!! Disregard any timing data!!\r\n" );
		}
	const DOUBLE adjustedTimingFrequency = ( DOUBLE( 1.00 ) / DOUBLE( timingFrequency.QuadPart ) );
	return adjustedTimingFrequency;
	}


NtdllWrap::NtdllWrap( ) {
	ntdll = GetModuleHandleW( L"C:\\Windows\\System32\\ntdll.dll" );
	if ( ntdll ) {
		auto success = NtQueryDirectoryFile.init( GetProcAddress( ntdll, "NtQueryDirectoryFile" ) );
		if ( !success ) {
			fwprintf( stderr, L"Couldn't find NtQueryDirectoryFile in ntdll.dll!\r\n" );
			}
		}
	else {
		fwprintf( stderr, L"Couldn't load ntdll.dll!\r\n" );
		std::terminate( );
		}
	}

_Success_( NT_SUCCESS( return ) ) NTSTATUS NTAPI NtQueryDirectoryFile_f::operator()( _In_ HANDLE FileHandle, _In_opt_ HANDLE Event, _In_opt_ PVOID ApcRoutine, _In_opt_ PVOID ApcContext, _Out_  IO_STATUS_BLOCK* IoStatusBlock, _Out_  PVOID FileInformation, _In_ ULONG Length, _In_ FILE_INFORMATION_CLASS FileInformationClass, _In_ BOOLEAN ReturnSingleEntry, _In_opt_ PUNICODE_STRING FileName, _In_ BOOLEAN RestartScan ) {
	if ( ntQueryDirectoryFuncPtr ) {
		return ntQueryDirectoryFuncPtr( FileHandle, Event, ApcRoutine, ApcContext, IoStatusBlock, FileInformation, Length, FileInformationClass, ReturnSingleEntry, FileName, RestartScan );
		}
	fwprintf( stderr, L"ntQueryDirectoryFuncPtr not initialized!\r\n" );
	std::terminate( );
	//abort( );
	//return -LONG_MAX;
	}


//void DeleteFileByHandle( _In_ HANDLE hFile ) {
//	assert( false );
//	return;
//	IO_STATUS_BLOCK iosb = { 0 };
//
//	struct FILE_DISPOSITION_INFORMATION {
//		BOOLEAN  DeleteFile;
//		} fdi = { TRUE };
//
//	HMODULE hNtdll = GetModuleHandle( L"C:\\Windows\\System32\\ntdll.dll" );
//	if ( hNtdll ) {
//		pfnSetInfoFile ntSetInformationFile = ( pfnSetInfoFile ) GetProcAddress( hNtdll, "NtSetInformationFile" );
//		if ( ntSetInformationFile ) {
//			ntSetInformationFile( hFile, &iosb, &fdi, sizeof( fdi ), FileDispositionInformation );
//			}
//		}
//	}

LONGLONG WcsToLLDec( _In_z_ const wchar_t* pNumber, _In_ wchar_t* endChar ) {
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

CmdParseResult ParseCmdLine( _In_ int argc, _In_ _In_reads_( argc ) wchar_t** argv, _In_opt_ LONGLONG* fileId ) {
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
		if ( fileId != NULL ) {
			if ( ( argc < 4 ) || ( ( *fileId = WcsToLLDec( argv[ 3 ], &endChar ), endChar != 0 ) ) ) {
				return DISPLAY_USAGE;
				}
			else {
				return DEL_FILE;
				}
			}
		}
	else if ( lowerFirstArgChar == L'e' ) {
		return ENUM_DIR;
		}
	return DISPLAY_USAGE;
	}



void qDirFile( _In_ const std::wstring dir, std::uint64_t& numItems, const bool writeToScreen, NtdllWrap* ntdll, _In_ const std::wstring curDir, HANDLE hDir, std::uint64_t& total_size ) {
	//I do this to ensure there are NO issues with incorrectly sized buffers or mismatching parameters (or any other bad changes)
	const FILE_INFORMATION_CLASS InfoClass = FileDirectoryInformation;
	typedef FILE_DIRECTORY_INFORMATION THIS_FILE_INFORMATION_CLASS;
	typedef THIS_FILE_INFORMATION_CLASS* PTHIS_FILE_INFORMATION_CLASS;

	//sizeof(FILE_ALL_INFORMATION)/sizeof(std::filesystem::path::value_type)+32769
	
	//auto bufSize = sizeof(THIS_FILE_INFORMATION_CLASS)/sizeof(wchar_t)+32769;
	
	
	const auto init_bufSize = ( ( sizeof( FILE_ID_BOTH_DIR_INFORMATION ) + ( MAX_PATH * sizeof( wchar_t ) ) ) * 1000 );

	//wchar_t* idInfo = NULL;
	//idInfo = new __declspec( align( 8 ) ) wchar_t[ bufSize ];//this is a MAJOR bottleneck for async enumeration.
	//if ( idInfo != NULL ) {
	//	memset( idInfo, 0, bufSize );
	//	}

	__declspec( align( 8 ) ) wchar_t buffer[ init_bufSize ];

	std::vector<std::wstring> breadthDirs;
	std::vector<WCHAR> fNameVect;

	std::vector<std::future<std::pair<std::uint64_t, std::uint64_t>>> futureDirs;

	IO_STATUS_BLOCK iosb = { static_cast< ULONG_PTR >( 0 ) };

	UNICODE_STRING _glob;
	
	NTSTATUS stat = STATUS_PENDING;
	if ( writeToScreen ) {
		wprintf( L"Files in directory %s\r\n", dir.c_str( ) );
		wprintf( L"      File ID       |       File Name\r\n" );
		}
	assert( init_bufSize > 1 );
	//auto buffer = &( idInfo[ 0 ] );
	//++numItems;
	auto sBefore = stat;
	stat = ntdll->NtQueryDirectoryFile( hDir, NULL, NULL, NULL, &iosb, buffer, static_cast<ULONG>( init_bufSize ), InfoClass, FALSE, NULL, TRUE );
	if ( stat == STATUS_TIMEOUT ) {
		std::terminate( );
		}
	if ( stat == STATUS_PENDING ) {
		std::terminate( );
		}
	assert( NT_SUCCESS( stat ) );
	assert( stat != sBefore );
	assert( GetLastError( ) != ERROR_MORE_DATA );
	auto bufSize = init_bufSize;
	wchar_t* idInfo = NULL;
	while ( stat == STATUS_BUFFER_OVERFLOW ) {
		//idInfo.erase( idInfo.begin( ), idInfo.end( ) );
		//idInfo.resize( idInfo.size( ) * 2 );
		delete[ ] idInfo;
		bufSize = ( bufSize * 2 );
		idInfo = new __declspec( align( 8 ) ) wchar_t[ bufSize ];//this is a MAJOR bottleneck for async enumeration.
		//memset( idInfo, 0, bufSize );
		
		stat = ntdll->NtQueryDirectoryFile( hDir, NULL, NULL, NULL, &iosb, idInfo, static_cast<ULONG>( bufSize ), InfoClass, FALSE, NULL, TRUE );
		}
	assert( NT_SUCCESS( stat ) );
	bool id_info_heap = false;
	if ( !NT_SUCCESS( stat ) ) {
		wprintf( L"Critical error!\r\n" );
		std::terminate( );
		}
	auto bufSizeWritten = iosb.Information;
	if ( idInfo == NULL ) {
		//fwprintf( stderr, L"idInfo == NULL!\r\n" );
		idInfo = buffer;
		assert( bufSize == init_bufSize );
		//return;
		}
	else {
		id_info_heap = true;
		}

	const auto idInfoSize = bufSize;
	//This zeros just enough of the idInfo buffer ( after the end of valid data ) to halt the forthcoming while loop at the last valid data. This saves the effort of zeroing larger parts of the buffer.
	for ( size_t i = bufSizeWritten; i < bufSizeWritten + ( sizeof( THIS_FILE_INFORMATION_CLASS ) + ( MAX_PATH * sizeof( wchar_t ) ) * 2 ); ++i ) {
		if ( i == idInfoSize ) {
			break;
			}
		idInfo[ i ] = 0;
		}
	

	auto pFileInf = reinterpret_cast<PTHIS_FILE_INFORMATION_CLASS>( idInfo );

	assert( pFileInf != NULL );
	while ( NT_SUCCESS( stat ) && ( pFileInf != NULL ) ) {
		//PFILE_ID_BOTH_DIR_INFORMATION pFileInf = ( FILE_ID_BOTH_DIR_INFORMATION* ) buffer;

		assert( pFileInf->FileNameLength > 1 );
		if ( pFileInf->FileName[ 0 ] == L'.' && ( pFileInf->FileName[ 1 ] == 0 || ( pFileInf->FileName[ 1 ] == '.' ) ) ) {
			//continue;
			goto nextItem;
			}

		total_size += pFileInf->AllocationSize.QuadPart;
		//const auto lores = GetCompressedFileSizeW( , ) 
#ifdef DEBUG
		{
		const std::wstring this_file_name( pFileInf->FileName, ( pFileInf->FileNameLength / sizeof( WCHAR ) ) );
		const std::wstring some_name( dir + L'\\' + this_file_name );
		const auto comp_file_size = GetCompressedFileSize_filename( some_name );
		if ( !( pFileInf->FileAttributes & FILE_ATTRIBUTE_REPARSE_POINT ) ) {
			if ( !( pFileInf->AllocationSize.QuadPart == comp_file_size ) ) {
				/*
#define FILE_ATTRIBUTE_READONLY             0x00000001  
#define FILE_ATTRIBUTE_HIDDEN               0x00000002  
#define FILE_ATTRIBUTE_SYSTEM               0x00000004  
#define FILE_ATTRIBUTE_DIRECTORY            0x00000010  
#define FILE_ATTRIBUTE_ARCHIVE              0x00000020  
#define FILE_ATTRIBUTE_DEVICE               0x00000040  
#define FILE_ATTRIBUTE_NORMAL               0x00000080  
#define FILE_ATTRIBUTE_TEMPORARY            0x00000100  
#define FILE_ATTRIBUTE_SPARSE_FILE          0x00000200  
#define FILE_ATTRIBUTE_REPARSE_POINT        0x00000400  
#define FILE_ATTRIBUTE_COMPRESSED           0x00000800  
#define FILE_ATTRIBUTE_OFFLINE              0x00001000  
#define FILE_ATTRIBUTE_NOT_CONTENT_INDEXED  0x00002000  
#define FILE_ATTRIBUTE_ENCRYPTED            0x00004000  
#define FILE_ATTRIBUTE_INTEGRITY_STREAM     0x00008000  
#define FILE_ATTRIBUTE_VIRTUAL              0x00010000  
#define FILE_ATTRIBUTE_NO_SCRUB_DATA        0x00020000  

				*/
				wprintf( L"Attributes for file: %s\r\n", some_name.c_str( ) );
				wprintf( L"%s: %s\r\n", L"FILE_ATTRIBUTE_READONLY", ( ( pFileInf->FileAttributes & FILE_ATTRIBUTE_READONLY ) ? L"YES" : L"NO" ) );
				wprintf( L"%s: %s\r\n", L"FILE_ATTRIBUTE_HIDDEN", ( ( pFileInf->FileAttributes & FILE_ATTRIBUTE_HIDDEN ) ? L"YES" : L"NO" ) );
				wprintf( L"%s: %s\r\n", L"FILE_ATTRIBUTE_SYSTEM", ( ( pFileInf->FileAttributes & FILE_ATTRIBUTE_SYSTEM ) ? L"YES" : L"NO" ) );
				wprintf( L"%s: %s\r\n", L"FILE_ATTRIBUTE_DIRECTORY", ( ( pFileInf->FileAttributes & FILE_ATTRIBUTE_DIRECTORY ) ? L"YES" : L"NO" ) );
				wprintf( L"%s: %s\r\n", L"FILE_ATTRIBUTE_ARCHIVE", ( ( pFileInf->FileAttributes & FILE_ATTRIBUTE_ARCHIVE ) ? L"YES" : L"NO" ) );
				wprintf( L"%s: %s\r\n", L"FILE_ATTRIBUTE_DEVICE", ( ( pFileInf->FileAttributes & FILE_ATTRIBUTE_DEVICE ) ? L"YES" : L"NO" ) );
				wprintf( L"%s: %s\r\n", L"FILE_ATTRIBUTE_NORMAL", ( ( pFileInf->FileAttributes & FILE_ATTRIBUTE_NORMAL ) ? L"YES" : L"NO" ) );
				wprintf( L"%s: %s\r\n", L"FILE_ATTRIBUTE_TEMPORARY", ( ( pFileInf->FileAttributes & FILE_ATTRIBUTE_TEMPORARY ) ? L"YES" : L"NO" ) );
				wprintf( L"%s: %s\r\n", L"FILE_ATTRIBUTE_SPARSE_FILE", ( ( pFileInf->FileAttributes & FILE_ATTRIBUTE_SPARSE_FILE ) ? L"YES" : L"NO" ) );
				wprintf( L"%s: %s\r\n", L"FILE_ATTRIBUTE_REPARSE_POINT", ( ( pFileInf->FileAttributes & FILE_ATTRIBUTE_REPARSE_POINT ) ? L"YES" : L"NO" ) );
				wprintf( L"%s: %s\r\n", L"FILE_ATTRIBUTE_COMPRESSED", ( ( pFileInf->FileAttributes & FILE_ATTRIBUTE_COMPRESSED ) ? L"YES" : L"NO" ) );
				wprintf( L"%s: %s\r\n", L"FILE_ATTRIBUTE_OFFLINE", ( ( pFileInf->FileAttributes & FILE_ATTRIBUTE_OFFLINE ) ? L"YES" : L"NO" ) );
				wprintf( L"%s: %s\r\n", L"FILE_ATTRIBUTE_NOT_CONTENT_INDEXED", ( ( pFileInf->FileAttributes & FILE_ATTRIBUTE_NOT_CONTENT_INDEXED ) ? L"YES" : L"NO" ) );
				wprintf( L"%s: %s\r\n", L"FILE_ATTRIBUTE_ENCRYPTED", ( ( pFileInf->FileAttributes & FILE_ATTRIBUTE_ENCRYPTED ) ? L"YES" : L"NO" ) );
				wprintf( L"%s: %s\r\n", L"FILE_ATTRIBUTE_INTEGRITY_STREAM", ( ( pFileInf->FileAttributes & FILE_ATTRIBUTE_INTEGRITY_STREAM ) ? L"YES" : L"NO" ) );
				wprintf( L"%s: %s\r\n", L"FILE_ATTRIBUTE_VIRTUAL", ( ( pFileInf->FileAttributes & FILE_ATTRIBUTE_VIRTUAL ) ? L"YES" : L"NO" ) );
				wprintf( L"%s: %s\r\n", L"FILE_ATTRIBUTE_NO_SCRUB_DATA", ( ( pFileInf->FileAttributes & FILE_ATTRIBUTE_NO_SCRUB_DATA ) ? L"YES" : L"NO" ) );

				_CrtDbgBreak( );
				}
			}
		}
#endif
		++numItems;
		if ( writeToScreen || ( pFileInf->FileAttributes & FILE_ATTRIBUTE_DIRECTORY ) ) {//I'd like to avoid building a null terminated string unless it is necessary
			fNameVect.clear( );
			fNameVect.reserve( ( pFileInf->FileNameLength / sizeof( WCHAR ) ) + 1 );
			PWCHAR end = pFileInf->FileName + ( pFileInf->FileNameLength / sizeof( WCHAR ) );
			fNameVect.insert( fNameVect.end( ), pFileInf->FileName, end );
			fNameVect.emplace_back( L'\0' );
			PWSTR fNameChar = &( fNameVect[ 0 ] );
			
			if ( writeToScreen ) {

				//std::wcout << std::setw( std::numeric_limits<LONGLONG>::digits10 ) << pFileInf->FileId.QuadPart << L"    " << std::setw( 0 ) << curDir << L"\\" << fNameChar;
				//wprintf( L"%I64d    %s\\%s\r\n", std::int64_t( pFileInf->FileId.QuadPart ), curDir.c_str( ), fNameChar );
				if ( pFileInf->FileAttributes & FILE_ATTRIBUTE_COMPRESSED ) {
					wprintf( L"AllocationSize: %I64d    %s\\%s\r\n", std::int64_t( pFileInf->AllocationSize.QuadPart ), curDir.c_str( ), fNameChar );
					}

				//auto state = std::wcout.fail( );
				//if ( state != 0 ) {
				//	std::wcout.clear( );
				//	std::wcout << std::endl << L"std::wcout.fail( ): " << state << std::endl;
				//	}
				}
			if ( pFileInf->FileAttributes & FILE_ATTRIBUTE_DIRECTORY ) {

				if ( curDir[ curDir.length( ) - 1 ] != L'\\' ) {
					//breadthDirs.emplace_back( std::wstring( curDir ) + L'\\' + fNameChar + L'\\' );
					auto query = std::wstring( curDir + L'\\' + fNameChar + L'\\' );
					futureDirs.emplace_back( std::async( std::launch::async | std::launch::deferred, ListDirectory, query, writeToScreen, ntdll ) );
					}
				else {
					//breadthDirs.emplace_back( std::wstring( curDir ) + fNameChar + L'\\' );
					auto query = std::wstring( curDir + fNameChar + L'\\' );
					futureDirs.emplace_back( std::async( std::launch::async | std::launch::deferred, ListDirectory, query, writeToScreen, ntdll ) );
					}
				//std::wstring dirstr = curDir + L"\\" + fNameChar + L"\\";
				//breadthDirs.emplace_back( dirstr );
				//numItems += ListDirectory( dirstr.c_str( ), dirs, idInfo, writeToScreen );
				}
			}

	nextItem:
		//stat = NtQueryDirectoryFile( hDir, NULL, NULL, NULL, &iosb, &idInfo[ 0 ], idInfo.size( ), FileIdBothDirectoryInformation, TRUE, NULL, FALSE );
		if ( writeToScreen ) {
			wprintf( L"\t\tpFileInf: %p, pFileInf->NextEntryOffset: %lu, ( pFileInf + pFileInf->NextEntryOffset ): %p\r\n", pFileInf, pFileInf->NextEntryOffset, ( pFileInf + pFileInf->NextEntryOffset ) );
			//std::wcout << L"\t\tpFileInf: " << pFileInf << L", pFileInf->NextEntryOffset: " << pFileInf->NextEntryOffset << L", pFileInf + pFileInf->NextEntryOffset " << ( pFileInf + pFileInf->NextEntryOffset ) << std::endl;
			}
		pFileInf = ( pFileInf->NextEntryOffset != 0 ) ? reinterpret_cast<PTHIS_FILE_INFORMATION_CLASS>( reinterpret_cast<std::uint64_t>( pFileInf ) + ( static_cast<std::uint64_t>( pFileInf->NextEntryOffset ) ) ) : NULL;
		}

	//for ( auto& aDir : breadthDirs ) {
	//	numItems += ListDirectory( aDir.c_str( ), writeToScreen, ntdll );
	//	}
	for ( auto& a : futureDirs ) {
		const auto a_pair = a.get( );
		numItems += a_pair.first;
		total_size += a_pair.second;
		//numItems += a.get( );
		}

	assert( ( pFileInf == NULL ) || ( !NT_SUCCESS( stat ) ) );
	if ( id_info_heap ) {
		delete[ ] idInfo;
		}
	}

std::pair<std::uint64_t, std::uint64_t> ListDirectory( _In_ std::wstring dir, _In_ const bool writeToScreen, _In_ NtdllWrap* ntdll ) {
	std::wstring curDir;
	std::uint64_t numItems = 0;
	if ( dir.size( ) == 0 ) {
		curDir.resize( GetCurrentDirectoryW( 0, NULL ) );
		GetCurrentDirectoryW( static_cast<DWORD>( curDir.size( ) ), &curDir[ 0 ] );
		dir = curDir;
		}
	else {
		//curDir = dir;
		}
	HANDLE hDir = CreateFile( dir.c_str( ), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL );
	if ( hDir == INVALID_HANDLE_VALUE ) {
		DWORD err = GetLastError( );
		//std::wcout << L"Failed to open directory " << dir << L" because of error " << err << std::endl;
		//const size_t bufSize = 256;
		//wchar_t buffer[ bufSize ] = { 0 };
		//FormatError( buffer, bufSize );
		wprintf( L"Failed to open directory %s because of error %lu\r\n", dir.c_str( ), err );
		wprintf( L"err: `%lu` means: %s\r\n", err, handyDandyErrMsgFormatter( ).c_str( ) );
		return std::make_pair( numItems, 0 );
		}
	std::uint64_t total_size = 0;
	qDirFile( dir, numItems, writeToScreen, ntdll, dir, hDir, total_size );
	if ( writeToScreen ) {
		wprintf( L"%I64u items in directory : %s\r\n", numItems, dir.c_str( ) );
		wprintf( L"%I64u total size of items\r\n", total_size );
		//std::wcout << std::setw( std::numeric_limits<LONGLONG>::digits10 ) << numItems << std::setw( 0 ) << L" items in directory " << dir << std::endl;
		}
	CloseHandle( hDir );
	return std::make_pair( numItems, total_size );
	}

//void DelFile( _In_ WCHAR fileVolume, _In_ LONGLONG fileId ) {
//	assert( false );
//	return;
//	HMODULE hNtdll = GetModuleHandleW( L"C:\\Windows\\System32\\ntdll.dll" );
//	if ( hNtdll == NULL ) {
//		return;
//		}
//	pfnOpenFile ntCreateFile = ( pfnOpenFile ) GetProcAddress( hNtdll, "NtCreateFile" );
//	WCHAR volumePath[ ] = { L'\\', L'\\', L'.', L'\\', fileVolume, L':', 0 };
//	HANDLE hVolume = CreateFileW( volumePath, 0, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0x80, NULL );
//	if ( hVolume == INVALID_HANDLE_VALUE ) {
//		DWORD err = GetLastError( );
//		std::wcout << L"Failed to open volume " << fileVolume << L": because of error " << err << L'\n';
//		return;
//		}
//	HANDLE hFile = NULL;
//	IO_STATUS_BLOCK iosb = { 0 };
//	OBJECT_ATTRIBUTES oa = { sizeof( oa ), 0 };
//	UNICODE_STRING name;
//	name.Buffer = ( PWSTR ) &fileId;
//	name.Length = name.MaximumLength = sizeof( fileId );
//	oa.ObjectName = &name;
//	oa.RootDirectory = hVolume;
//	NTSTATUS stat = ntCreateFile(
//		&hFile, GENERIC_READ | GENERIC_WRITE | DELETE, &oa, &iosb, NULL, FILE_ATTRIBUTE_NORMAL, 0, FILE_OPEN, FILE_NON_DIRECTORY_FILE | FILE_OPEN_BY_FILE_ID, NULL, 0 );
//	CloseHandle( hVolume );
//	if ( !NT_SUCCESS( stat ) ) {
//		std::cout << "Failed to delete file because of error " << std::hex << stat << '\n';
//		}
//	else {
//		BYTE buffer[ ( ( sizeof( WCHAR ) * MAX_PATH ) + sizeof( ULONG ) ) ] = { 0 };
//		GetFileInformationByHandleEx( hFile, FileNameInfo, buffer, sizeof( buffer ) );
//		FILE_NAME_INFO* pFni = ( FILE_NAME_INFO* ) buffer;
//		std::wcout << L"Deleting " << pFni->FileName << L'\n';
//		DeleteFileByHandle( hFile );
//		CloseHandle( hFile );
//		std::cout << "File deleted\n";
//		}
//	}

std::pair<std::uint64_t, std::uint64_t> RecurseListDirectory( _In_z_ const wchar_t* dir, _In_ const bool writeToScreen ) {
	//__declspec( align( 8 ) ) std::vector<wchar_t> idInfo( ( sizeof( FILE_ID_BOTH_DIR_INFORMATION ) + ( MAX_PATH * sizeof( wchar_t ) ) ) * 500000 );
	std::uint64_t items = 1;
	static NtdllWrap ntdll;
	std::uint64_t total_size = 0;
	auto a_pair = ListDirectory( dir, writeToScreen, &ntdll );
	items += a_pair.first;
	total_size += a_pair.second;
	wprintf( L"Total items in directory tree of %s: %I64u\r\n", dir, items );
	return std::make_pair( items, total_size );
	}

int __cdecl wmain( _In_ int argc, _In_ _Deref_prepost_count_( argc ) wchar_t** argv ) {
	assert( !NT_SUCCESS( STATUS_INTERNAL_ERROR ) );
		{
		LONGLONG fileId = 0;
		CmdParseResult res = ParseCmdLine( argc, argv, &fileId );
		std::vector<std::wstring> dirs;
		dirs.reserve( 10 );
		for ( size_t i = 0; i < argc; ++i ) {
			//std::wcout << L"arg # " << i << L": " << argv[ i ] << std::endl;
			wprintf( L"arg # %I64u: %s\r\n", std::uint64_t( i ), argv[ i ] );
			}
		if ( res == DISPLAY_USAGE ) {
			wprintf( L"Usage:\nFileId /list <C:\\Path\\To\\Directory>\nFileId /delete <volume> FileId\n\twhere <volume> is the drive letter of the drive the file id is on\r\n");
			return -1;
			}
		if ( res == LIST_DIR ) {
			dirs.emplace_back( argv[ 2 ] );
			//ListDirectory( argc < 3 ? NULL : argv[ 2 ], dirs, true );
			}
		if ( res == ENUM_DIR ) {
			std::wstring _path( argv[ 2 ] );
			wprintf( L"Arg: %s\r\n", _path.c_str( ) );
			std::wstring nativePath = L"\\\\?\\" + _path;
			wprintf( L"'native' path: %s\r\n", nativePath.c_str( ) );
			LARGE_INTEGER startTime = { 0 };
			LARGE_INTEGER endTime = { 0 };
	
			//std::int64_t fileSizeTotal = 0;
			const auto adjustedTimingFrequency = getAdjustedTimingFrequency( );
			const BOOL res2 = QueryPerformanceCounter( &startTime );
			//std::uint64_t total_size = 0;

			const auto a_pair = RecurseListDirectory( nativePath.c_str( ), false );
			//const auto a_pair = RecurseListDirectory( L"C:\\", false );
			const auto items = a_pair.first;
			const auto total_size = a_pair.second;
			const BOOL res3 = QueryPerformanceCounter( &endTime );
	
			if ( ( !res2 ) || ( !res3 ) ) {
				wprintf( L"QueryPerformanceCounter Failed!!!!!! Disregard any timing data!!\r\n" );
				}
			const auto totalTime = ( endTime.QuadPart - startTime.QuadPart ) * adjustedTimingFrequency;

			//items = RecurseListDirectory( L"\\\\?\\C:", false );
			wprintf( L"Time in seconds: %f\r\n", totalTime );
			wprintf( L"Items: %I64u\r\n", items );
			wprintf( L"total size of items: %I64u\r\n", total_size );
			wprintf( L"Items/second: %f\r\n", ( static_cast< DOUBLE >( items ) / totalTime ) );
			//ListDirectory( argc < 3 ? NULL : argv[ 2 ], dirs, false );
			}
		else // DEL_FILE
			{
			//return 0;
			//DelFile( argv[ 2 ][ 0 ], fileId );
			
			}
		//auto state = std::wcout.fail( );
		//std::wcout.clear( );
		//std::wcout << L"std::wcout.fail( ): " << state << std::endl;
		return 0;
		}
	}