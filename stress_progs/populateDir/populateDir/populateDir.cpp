#include <iostream>
#include <assert.h>
#include <fstream>
#include <set>
#include <sstream>
#include <vector>
#include <string>
#include <Windows.h>
#include <cstdint>
#include <sstream>
#include <exception>
#include <iostream>
#include <ostream>
#include <string>
#include <strsafe.h>
#include <memory>
#include <ios>
#include <algorithm>
#include <functional>
#include <random>
#include "Header.h"
#include <stdio.h>

#define TRACE_OUT(x) std::endl << L"\t\t" << #x << L" = `" << x << L"` "//awesomely useful macro, included now, just in case I need it later.
#define TRACE_STR(x) << L" " << #x << L" = `" << x << L"`;"

#define TRACE_OUT_C_STYLE( x, fmt_spec ) wprintf( L"\r\n\t\t" L#x L" = `" L#fmt_spec L"` ", ##x )
#define TRACE_OUT_C_STYLE_ENDL( ) wprintf( L"\r\n" )


//http://stackoverflow.com/questions/440133/how-do-i-create-a-random-alpha-numeric-string-in-c


//std::vector<char> charset( ) {
//	return std::vector<char>(
//		{ '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z' } );
//	}

const LARGE_INTEGER help_QueryPerformanceCounter( ) {
	LARGE_INTEGER doneTime;
	const BOOL behavedWell = QueryPerformanceCounter( &doneTime );
	assert( behavedWell );
	if ( !behavedWell ) {
		MessageBoxW( NULL, L"QueryPerformanceCounter failed!!", L"ERROR!", MB_OK );
		doneTime.QuadPart = -1;
		}
	return doneTime;
	}

const LARGE_INTEGER help_QueryPerformanceFrequency( ) {
	LARGE_INTEGER doneTime;
	const BOOL behavedWell = QueryPerformanceFrequency( &doneTime );
	assert( behavedWell );
	if ( !behavedWell ) {
		MessageBoxW( NULL, L"QueryPerformanceFrequency failed!!", L"ERROR!", MB_OK );
		doneTime.QuadPart = -1;
		}
	return doneTime;
	}



// given a function that generates a random character,
// return a string of the requested length
std::wstring random_string( size_t length, std::function<char( void )> rand_char ) {
	std::wstring str( length, 0 );
	std::generate_n( str.begin( ), length, rand_char );
    return str;
	}

//char randchar( std::uniform_int_distribution<>& dist, std::default_random_engine& rng ) {
//	return alnumChars[ dist( rng ) ]; 
//	};


//template<typename T>
//size_t findInRAI( const std::vector<T>& a, const T& b ) {
//	auto it = std::find( a.begin( ), a.end( ), b );
//	return it - a.begin( ) ;
//	}



//void fixWCout( ) {
//	if ( !std::wcout.good()) {//Slower than it should be.
//		auto badBits    = std::wcout.exceptions( );
//		auto wasBadBit  = badBits & std::ios_base::badbit;
//		auto wasFailBit = badBits & std::ios_base::failbit;
//		auto wasEofBit  = badBits & std::ios_base::eofbit;
//		std::wcout.clear( );
//		std::wcout << L"wcout was in a bad state!" << std::endl;
//		std::wcout << TRACE_OUT( wasBadBit );
//		std::wcout << TRACE_OUT( wasFailBit );
//		std::wcout << TRACE_OUT( wasEofBit );
//		std::wcout << std::endl;
//		}
//	}

//std::int64_t stdRecurseFind( _In_ std::wstring dir, _In_ bool isLargeFetch, _In_ bool isBasicInfo, _Inout_ std::vector<std::wstring>& files  ) {
//
//	std::int64_t num = 0;
//	dir.reserve( MAX_PATH );
//	std::wstring normSzDir(dir);
//	normSzDir.reserve( MAX_PATH );
//	if ( ( dir.back( ) != L'*' ) && ( dir.at( dir.length( ) - 2 ) != L'\\' ) ) {
//		dir += L"\\*";
//		}
//	else if ( dir.back( ) == L'\\' ) {
//		dir += L"*";
//		}
//
//	WIN32_FIND_DATA fData;
//	HANDLE fDataHand = NULL;
//
//	if ( isLargeFetch ) {
//		if ( isBasicInfo ) {
//			fDataHand = FindFirstFileExW( dir.c_str( ), FindExInfoBasic, &fData, FindExSearchNameMatch, NULL, FIND_FIRST_EX_LARGE_FETCH );
//			}
//		else {
//			fDataHand = FindFirstFileExW( dir.c_str( ), FindExInfoStandard, &fData, FindExSearchNameMatch, NULL, FIND_FIRST_EX_LARGE_FETCH );
//			}
//		}
//	else {
//		if ( isBasicInfo ) {
//			fDataHand = FindFirstFileExW( dir.c_str( ), FindExInfoBasic, &fData, FindExSearchNameMatch, NULL, 0 );
//			}
//		else {
//			fDataHand = FindFirstFileExW( dir.c_str( ), FindExInfoStandard, &fData, FindExSearchNameMatch, NULL, 0 );
//			}
//		}
//	if ( fDataHand != INVALID_HANDLE_VALUE ) {
//		if ( !( wcscmp( fData.cFileName, L".." ) == 0 ) ) {
//			
//			}
//		auto res = FindNextFileW( fDataHand, &fData );
//		while ( ( fDataHand != INVALID_HANDLE_VALUE ) && ( res != 0 ) ) {
//
//			auto scmpVal = wcscmp( fData.cFileName, L".." );
//			if ( ( !( scmpVal == 0 ) ) && ( !( fData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) ) ) {
//				files.emplace_back( fData.cFileName );
//				}
//			if ( ( fData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) && ( scmpVal != 0 ) ) {
//				num += descendDirectory( fData, normSzDir, isLargeFetch, isBasicInfo, files );
//				}
//			else if ( ( scmpVal != 0 ) && ( ( ( fData.dwFileAttributes & FILE_ATTRIBUTE_DEVICE ) || ( fData.dwFileAttributes & FILE_ATTRIBUTE_INTEGRITY_STREAM ) || ( fData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN ) || ( fData.dwFileAttributes & FILE_ATTRIBUTE_NO_SCRUB_DATA ) || ( fData.dwFileAttributes & FILE_ATTRIBUTE_OFFLINE ) || ( fData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT ) || ( fData.dwFileAttributes & FILE_ATTRIBUTE_SPARSE_FILE ) || ( fData.dwFileAttributes & FILE_ATTRIBUTE_VIRTUAL ) ) ) ) {
//				
//				--num;
//				}
//			res = FindNextFileW( fDataHand, &fData );
//			
//			}
//		}
//	
//	FindClose( fDataHand );
//	return num;
//	}


//size_t stdWork( _In_ std::wstring arg, _In_ bool isLargeFetch, _In_ bool isBasicInfo, _Inout_ std::vector<std::wstring>& files ) {
//	fixWCout( );
//	std::wcout << L"Working on: `" << arg << L"`" << std::endl;
//	std::int64_t numberFiles = 0;
//	arg.reserve( MAX_PATH );
//	if ( arg.length( ) > 3 ) {
//
//		auto strCmp = ( arg.compare( 0, 4, arg, 0, 4 ) );
//		if ( strCmp != 0 ) {
//			arg = L"\\\\?\\" + arg;
//			std::wcout << L"prefixed `" << arg << L"`, value now: `" << arg << L"`" << std::endl << std::endl;
//			numberFiles = stdRecurseFind( arg, isLargeFetch, isBasicInfo, files );
//			}
//		else {
//			numberFiles = stdRecurseFind( arg, isLargeFetch, isBasicInfo, files );
//			}
//		}
//	else {
//		arg = L"\\\\?\\" + arg;
//		std::wcout << L"prefixed `" << arg << L"`, value now: `" << arg << L"`" << std::endl << std::endl;
//		
//		numberFiles = stdRecurseFind( arg, isLargeFetch, isBasicInfo, files );
//		}
//	fixWCout( );
//	std::wcout << std::endl << L"Number of items: " << files.size( ) << std::endl;
//	return files.size( );
//	}

//std::int64_t descendDirectory( _Inout_ WIN32_FIND_DATA& fData, _In_ std::wstring& normSzDir, _In_ bool isLargeFetch, _In_ bool isBasicInfo, _Inout_ std::vector<std::wstring>& files ) {
//	std::wstring newSzDir = normSzDir;//MUST operate on copy!
//	newSzDir.reserve( MAX_PATH );
//	newSzDir += L"\\";
//	newSzDir += fData.cFileName;
//	std::int64_t num = 0;
//	num += stdRecurseFind( newSzDir, isLargeFetch, isBasicInfo, files );
//	return num;
//	}

void single_file( _In_ const std::wstring newStr ) {
	const auto fileHandle = CreateFileW( newStr.c_str( ), GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL );
	if ( fileHandle == INVALID_HANDLE_VALUE ) {
		const auto last_err = GetLastError( );
		wprintf( L"Error creating file: %u\r\n", last_err );
		}
	else {
		DWORD bytesWritten = 0;
		const auto val = WriteFile( fileHandle, newStr.c_str( ), DWORD( newStr.size( ) ), &bytesWritten, NULL );
		if ( val != TRUE ) {
			const auto last_err = GetLastError( );
			wprintf( L"Error writing file: %u, Bytes written: %u\r\n", last_err, bytesWritten );
			}
		else {
			wprintf( L"Successfully wrote to file %s, Bytes written: %u\r\n", newStr.c_str( ), bytesWritten );
			}
		CloseHandle( fileHandle );
		}

	}

void fillDir( _In_ std::wstring theDir, _In_ const std::int64_t iterations ) {
	///http://stackoverflow.com/a/12468109
	const char alnumChars_arr[ ] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z' };

	const std::vector<char> alnumChars( alnumChars_arr, ( alnumChars_arr + sizeof( alnumChars_arr ) -1 ) );
	std::default_random_engine rng( std::random_device { }( ) );
	std::uniform_int_distribution<> dist( 6, int( sizeof( alnumChars ) - 1 ) );

	const auto randchar = [ alnumChars, &dist, &rng ] ( ) { return alnumChars[ dist( rng ) ]; };

	int iterations_so_far = 0;
	const auto retval = SetCurrentDirectoryW( theDir.c_str( ) );
	if ( !retval ) {
		wprintf( L"SetCurrentDirectoryW failed!\r\n" );
		return;
		}
	for ( std::int64_t i = 0; i < iterations; ++i ) {
		++iterations_so_far;
		const auto newStr = random_string( dist( rng ), randchar );

		single_file( newStr );

		//const auto fileHandle = CreateFileW( newStr.c_str( ), GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL );
		//if ( fileHandle == INVALID_HANDLE_VALUE ) {
		//	const auto last_err = GetLastError( );
		//	wprintf( L"Error creating file: %u\r\n", last_err );
		//	}
		//else {
		//	DWORD bytesWritten = 0;
		//	const auto val = WriteFile( fileHandle, newStr.c_str( ), DWORD( newStr.size( ) ), &bytesWritten, NULL );
		//	if ( val != TRUE ) {
		//		const auto last_err = GetLastError( );
		//		wprintf( L"Error writing file: %u, Bytes written: %u\r\n", last_err, bytesWritten );
		//		}
		//	else {
		//		wprintf( L"Successfully wrote to file %s, Bytes written: %u\r\n", newStr.c_str( ), bytesWritten );
		//		}
		//	CloseHandle( fileHandle );
		//	}

		}
	//fixWCout( );
	TRACE_OUT_C_STYLE( iterations_so_far, %i );
	TRACE_OUT_C_STYLE_ENDL( );

	}

int main( ) {
	const auto qpc_1 = help_QueryPerformanceCounter( );

	fillDir( L"C:\\Users\\Alexander Riccio\\Documents\\test_junk_dir\\cpp_junk", 1500 );
	const auto qpc_2 = help_QueryPerformanceCounter( );
	const auto qpf = help_QueryPerformanceFrequency( );
	const auto timing = ( static_cast<double>( qpc_2.QuadPart - qpc_1.QuadPart ) * ( static_cast<double>( 1.0 ) / static_cast<double>( qpf.QuadPart ) ) );
	wprintf( L"total time: %f\r\n", timing );

	return 0;
	}