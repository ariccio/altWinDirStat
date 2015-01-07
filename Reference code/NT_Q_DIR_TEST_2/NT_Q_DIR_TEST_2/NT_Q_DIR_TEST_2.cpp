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
		//std::wcout << L"QueryPerformanceFrequency failed!!!!!! Disregard any timing data!!" << std::endl;
		wprintf( L"QueryPerformanceFrequency failed!!!!!! Disregard any timing data!!\r\n" );
		}
	const DOUBLE adjustedTimingFrequency = ( DOUBLE( 1.00 ) / DOUBLE( timingFrequency.QuadPart ) );
	return adjustedTimingFrequency;
	}


NtdllWrap::NtdllWrap( ) {
	ntdll = GetModuleHandle( L"C:\\Windows\\System32\\ntdll.dll" );
	if ( ntdll ) {
		auto success = NtQueryDirectoryFile.init( GetProcAddress( ntdll, "NtQueryDirectoryFile" ) );
		if ( !success ) {
			fwprintf( stderr, L"Couldn't find NtQueryDirectoryFile in ntdll.dll!\r\n" );
			//std::wcerr << L"Couldn't find NtQueryDirectoryFile in ntdll.dll!" << std::endl;
			}
		}
	else {
		fwprintf( stderr, L"Couldn't load ntdll.dll!\r\n" );
		//std::wcerr << L"Couldn't load ntdll.dll!" << std::endl;
		//throw 666;
		std::terminate( );
		}
	}

_Success_( return != -LONG_MAX ) NTSTATUS NTAPI NtQueryDirectoryFile_f::operator()( _In_ HANDLE FileHandle, _In_opt_ HANDLE Event, _In_opt_ PVOID ApcRoutine, _In_opt_ PVOID ApcContext, _Out_  IO_STATUS_BLOCK* IoStatusBlock, _Out_  PVOID FileInformation, _In_ ULONG Length, _In_ FILE_INFORMATION_CLASS FileInformationClass, _In_ BOOLEAN ReturnSingleEntry, _In_opt_ PUNICODE_STRING FileName, _In_ BOOLEAN RestartScan ) {
	if ( ntQueryDirectoryFuncPtr ) {
		return ntQueryDirectoryFuncPtr( FileHandle, Event, ApcRoutine, ApcContext, IoStatusBlock, FileInformation, Length, FileInformationClass, ReturnSingleEntry, FileName, RestartScan );
		}
	return -LONG_MAX;
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



void qDirFile( _In_ const std::wstring dir, std::uint64_t& numItems, const bool writeToScreen, NtdllWrap* ntdll, _In_ const std::wstring curDir, HANDLE hDir ) {
	//I do this to ensure there are NO issues with incorrectly sized buffers or mismatching parameters (or any other bad changes)
	const FILE_INFORMATION_CLASS InfoClass = FileIdFullDirectoryInformation;
	//typedef FILE_ID_FULL_DIR_INFORMATION THIS_FILE_INFORMATION_CLASS;
	using THIS_FILE_INFORMATION_CLASS = FILE_ID_FULL_DIR_INFORMATION;
	typedef THIS_FILE_INFORMATION_CLASS* PTHIS_FILE_INFORMATION_CLASS;

	

	//auto bufSize = ( ( sizeof( FILE_ID_BOTH_DIR_INFORMATION ) + ( MAX_PATH * sizeof( wchar_t ) ) ) * 100 );
	
	//std::unique_ptr<wchar_t> idInfo( new __declspec( align( 8 ) ) wchar_t[ bufSize ] );//this is a MAJOR bottleneck for async enumeration.
	

	const auto init_bufSize = ( ( sizeof( FILE_ID_BOTH_DIR_INFORMATION ) + ( MAX_PATH * sizeof( wchar_t ) ) ) * 100 );

	__declspec( align( 8 ) ) wchar_t stack_buffer[ init_bufSize ] = { 0 };

	//memset( idInfo.get( ), 0, bufSize );

	std::vector<std::wstring> breadthDirs;
	std::vector<WCHAR> fNameVect;

	std::vector<std::future<std::uint64_t>> futureDirs;

	IO_STATUS_BLOCK iosb = { static_cast< ULONG_PTR >( 0 ) };

	UNICODE_STRING _glob;
	
	NTSTATUS stat = STATUS_PENDING;
	if ( writeToScreen ) {
		wprintf( L"Files in directory %s\r\n", dir.c_str( ) );
		wprintf( L"      File ID       |       File Name\r\n" );
		//std::wcout << L"Files in directory " << dir << L'\n';
		//std::wcout << L"      File ID       |       File Name\n";
		}
	//auto buffer = &( idInfo[ 0 ] );
	//++numItems;
	const  auto sBefore = stat;
	stat = ntdll->NtQueryDirectoryFile( hDir, NULL, NULL, NULL, &iosb, stack_buffer, init_bufSize, InfoClass, FALSE, NULL, TRUE );
	if ( stat == -LONG_MAX ) {
		fwprintf( stderr, L"NtQueryDirectoryFile address invalid!\r\n" );
		//std::cerr << L"NtQueryDirectoryFile address invalid!" << std::endl;
		return;
		}
	auto bufSize = init_bufSize;
	assert( bufSize > 1 );
	assert( bufSize < ULONG_MAX );
	std::unique_ptr<wchar_t> idInfo;
	assert( NT_SUCCESS( stat ) );
	assert( stat != sBefore );
	while ( stat == STATUS_BUFFER_OVERFLOW ) {
		//idInfo.erase( idInfo.begin( ), idInfo.end( ) );
		//idInfo.resize( idInfo.size( ) * 2 );
		//delete[ ] idInfo;
		bufSize = ( bufSize * 2 );
		idInfo.reset( new __declspec( align( 8 ) ) wchar_t[ bufSize ] );//this is a MAJOR bottleneck for async enumeration.
		memset( idInfo.get( ), 0, bufSize );
		stat = ntdll->NtQueryDirectoryFile( hDir, NULL, NULL, NULL, &iosb, idInfo.get( ), bufSize, InfoClass, FALSE, NULL, TRUE );
		}
	
	auto bufSizeWritten = iosb.Information;
	assert( NT_SUCCESS( stat ) );

	const auto idInfoSize = bufSize;
	//This zeros just enough of the idInfo buffer ( after the end of valid data ) to halt the forthcoming while loop at the last valid data. This saves the effort of zeroing larger parts of the buffer.
	if ( idInfo ) {
		for ( size_t i = bufSizeWritten; i < bufSizeWritten + ( sizeof( THIS_FILE_INFORMATION_CLASS ) + ( MAX_PATH * sizeof( wchar_t ) ) * 2 ); ++i ) {
			if ( i == idInfoSize ) {
				break;
				}
			idInfo.get( )[ i ] = 0;
			}
		}

	PTHIS_FILE_INFORMATION_CLASS pFileInf = nullptr;
	//PTHIS_FILE_INFORMATION_CLASS pFileInf = reinterpret_cast<PTHIS_FILE_INFORMATION_CLASS>( idInfo.get( ) );
	if ( idInfo ) {
		pFileInf = reinterpret_cast<PTHIS_FILE_INFORMATION_CLASS>( idInfo.get( ) );
		}
	else {
		pFileInf = reinterpret_cast<PTHIS_FILE_INFORMATION_CLASS>( stack_buffer );
		}



	assert( pFileInf != NULL );
	while ( NT_SUCCESS( stat ) && ( pFileInf != NULL ) ) {
		//PFILE_ID_BOTH_DIR_INFORMATION pFileInf = ( FILE_ID_BOTH_DIR_INFORMATION* ) buffer;

		assert( pFileInf->FileNameLength > 1 );
		if ( pFileInf->FileName[ 0 ] == L'.' && ( pFileInf->FileName[ 1 ] == 0 || ( pFileInf->FileName[ 1 ] == '.' ) ) ) {
			//continue;
			goto nextItem;
			}
		
		++numItems;
		if ( writeToScreen || ( pFileInf->FileAttributes & FILE_ATTRIBUTE_DIRECTORY ) ) {//I'd like to avoid building a null terminated string unless it is necessary
			fNameVect.clear( );
			fNameVect.reserve( ( pFileInf->FileNameLength / sizeof( wchar_t ) ) + 1 );
			PWCHAR end = pFileInf->FileName + ( pFileInf->FileNameLength / sizeof( wchar_t ) );
			fNameVect.insert( fNameVect.end( ), pFileInf->FileName, end );
			fNameVect.emplace_back( L'\0' );
			PWSTR fNameChar = &( fNameVect[ 0 ] );

			if ( writeToScreen ) {

				//std::wcout << std::setw( std::numeric_limits<LONGLONG>::digits10 ) << pFileInf->FileId.QuadPart << L"    " << std::setw( 0 ) << curDir << L"\\" << fNameChar;
				wprintf( L"%lld    %s\\%s\r\n", pFileInf->FileId.QuadPart, curDir.c_str( ), fNameChar );
				//const auto state = std::wcout.fail( );
				//if ( state != 0 ) {
				//	//std::wcout.clear( );
				//	//std::wcout << std::endl << L"std::wcout.fail( ): " << state << std::endl;
				//	wprintf( L"std::wcout.fail( ): %s\r\n", ( ( state ) ? L"TRUE" : L"FALSE" ) );
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
			//std::wcout << L"\t\tpFileInf: " << pFileInf << L", pFileInf->NextEntryOffset: " << pFileInf->NextEntryOffset << L", pFileInf + pFileInf->NextEntryOffset " << ( pFileInf + pFileInf->NextEntryOffset ) << std::endl;
			wprintf( L"\t\tpFileInf: %p, pFileInf->NextEntryOffset: %lu, pFileInf + pFileInf->NextEntryOffset %p\r\n", pFileInf, pFileInf->NextEntryOffset, ( pFileInf + pFileInf->NextEntryOffset ) );
			}
		pFileInf = ( pFileInf->NextEntryOffset != 0 ) ? reinterpret_cast<PTHIS_FILE_INFORMATION_CLASS>( reinterpret_cast<std::uint64_t>( pFileInf ) + ( static_cast<std::uint64_t>( pFileInf->NextEntryOffset ) ) ) : NULL;
		}

	//for ( auto& aDir : breadthDirs ) {
	//	numItems += ListDirectory( aDir.c_str( ), writeToScreen, ntdll );
	//	}
	for ( auto& a : futureDirs ) {
		numItems += a.get( );
		}

	assert( ( pFileInf == NULL ) || ( !NT_SUCCESS( stat ) ) );

	//delete[ ] idInfo;
	}

uint64_t ListDirectory( _In_ std::wstring dir, _In_ const bool writeToScreen, _In_ NtdllWrap* ntdll ) {
	std::wstring curDir;
	std::uint64_t numItems = 0;
	if ( dir.size( ) == 0 ) {
		curDir.resize( GetCurrentDirectoryW( 0, NULL ) );
		GetCurrentDirectoryW( static_cast<DWORD>( curDir.size( ) ), &curDir[ 0 ] );
		dir = curDir;
		}
	const HANDLE hDir = CreateFileW( dir.c_str( ), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL );
	if ( hDir == INVALID_HANDLE_VALUE ) {
		DWORD err = GetLastError( );
		//std::wcout << L"Failed to open directory " << dir << L" because of error " << err << std::endl;
		//const size_t bufSize = 256;
		//wchar_t buffer[ bufSize ] = { 0 };
		//FormatError( buffer, bufSize );
		fwprintf( stderr, L"Failed to open directory %s because of error %lu\r\n", dir.c_str( ), err );
		fwprintf( stderr, L"err: `%lu` means: %s\r\n", err, handyDandyErrMsgFormatter( ).c_str( ) );
		return numItems;
		}
	
	qDirFile( dir, numItems, writeToScreen, ntdll, dir, hDir );
	if ( writeToScreen ) {
		//std::wcout << std::setw( std::numeric_limits<LONGLONG>::digits10 ) << numItems << std::setw( 0 ) << L" items in directory " << dir << std::endl;
		wprintf( L"%I64u items in directory %s\r\n", numItems, dir.c_str( ) );
		}
	CloseHandle( hDir );
	return numItems;
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

std::uint64_t RecurseListDirectory( _In_z_ std::wstring dir, _In_ const bool writeToScreen ) {
	//__declspec( align( 8 ) ) std::vector<bufferChar> idInfo( ( sizeof( FILE_ID_BOTH_DIR_INFORMATION ) + ( MAX_PATH * sizeof( bufferChar ) ) ) * 500000 );
	std::uint64_t items = 1;
	static NtdllWrap ntdll;
	items += ListDirectory( dir, writeToScreen, &ntdll );
	wprintf( L"Total items in directory tree of %s: %I64u\r\n", dir.c_str( ), items );
	return items;
	}

int __cdecl wmain( _In_ int argc, _In_ _Deref_prepost_count_( argc ) wchar_t** argv ) {
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
			std::wstring nativePath( L"\\\\?\\" + _path );
			wprintf( L"'native' path: %s\r\n", nativePath.c_str( ) );
			LARGE_INTEGER startTime = { 0 };
			LARGE_INTEGER endTime = { 0 };
	
			//std::int64_t fileSizeTotal = 0;
			const auto adjustedTimingFrequency = getAdjustedTimingFrequency( );
			const BOOL res2 = QueryPerformanceCounter( &startTime );

			const auto items = RecurseListDirectory( nativePath.c_str( ), false );

			const BOOL res3 = QueryPerformanceCounter( &endTime );
	
			if ( ( !res2 ) || ( !res3 ) ) {
				wprintf( L"QueryPerformanceCounter Failed!!!!!! Disregard any timing data!!\r\n" );
				}
			const auto totalTime = ( endTime.QuadPart - startTime.QuadPart ) * adjustedTimingFrequency;

			//items = RecurseListDirectory( L"\\\\?\\C:", false );
			wprintf( L"Time in seconds: %f\r\n", totalTime );
			wprintf( L"Items: %I64u\r\n", items );
			wprintf( L"Items/second: %f\r\n", ( static_cast< DOUBLE >( items ) / totalTime ) );
			//ListDirectory( argc < 3 ? NULL : argv[ 2 ], dirs, false );
			}
		else // DEL_FILE
			{
			return 0;
			//DelFile( argv[ 2 ][ 0 ], fileId );
			
			}
		//auto state = std::wcout.fail( );
		//std::wcout.clear( );
		//std::wcout << L"std::wcout.fail( ): " << state << std::endl;
		//wprintf( L"std::wcout.fail( ): %s\r\n", ( ( state ) ? L"TRUE" : L"FALSE" ) );
		return 0;
		}
	}