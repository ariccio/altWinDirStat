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

#define TRACE_OUT(x) std::endl << L"\t\t" << #x << L" = `" << x << L"` "//awesomely useful macro, included now, just in case I need it later.

#define TRACE_STR(x) << L" " << #x << L" = `" << x << L"`;"

//http://stackoverflow.com/questions/440133/how-do-i-create-a-random-alpha-numeric-string-in-c


//std::vector<char> charset( ) {
//	return std::vector<char>(
//		{ '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z' } );
//	}



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


template<typename T>
size_t findInRAI( const std::vector<T>& a, const T& b ) {
	auto it = std::find( a.begin( ), a.end( ), b );
	return it - a.begin( ) ;
	}



void fixWCout( ) {
	if ( !std::wcout.good()) {//Slower than it should be.
		auto badBits    = std::wcout.exceptions( );
		auto wasBadBit  = badBits & std::ios_base::badbit;
		auto wasFailBit = badBits & std::ios_base::failbit;
		auto wasEofBit  = badBits & std::ios_base::eofbit;
		std::wcout.clear( );
		std::wcout << L"wcout was in a bad state!" << std::endl;
		std::wcout << TRACE_OUT( wasBadBit );
		std::wcout << TRACE_OUT( wasFailBit );
		std::wcout << TRACE_OUT( wasEofBit );
		std::wcout << std::endl;
		}
	}

std::int64_t stdRecurseFind( _In_ std::wstring dir, _In_ bool isLargeFetch, _In_ bool isBasicInfo, _Inout_ std::vector<std::wstring>& files  ) {

	std::int64_t num = 0;
	dir.reserve( MAX_PATH );
	std::wstring normSzDir(dir);
	normSzDir.reserve( MAX_PATH );
	if ( ( dir.back( ) != L'*' ) && ( dir.at( dir.length( ) - 2 ) != L'\\' ) ) {
		dir += L"\\*";
		}
	else if ( dir.back( ) == L'\\' ) {
		dir += L"*";
		}

	WIN32_FIND_DATA fData;
	HANDLE fDataHand = NULL;

	if ( isLargeFetch ) {
		if ( isBasicInfo ) {
			fDataHand = FindFirstFileExW( dir.c_str( ), FindExInfoBasic, &fData, FindExSearchNameMatch, NULL, FIND_FIRST_EX_LARGE_FETCH );
			}
		else {
			fDataHand = FindFirstFileExW( dir.c_str( ), FindExInfoStandard, &fData, FindExSearchNameMatch, NULL, FIND_FIRST_EX_LARGE_FETCH );
			}
		}
	else {
		if ( isBasicInfo ) {
			fDataHand = FindFirstFileExW( dir.c_str( ), FindExInfoBasic, &fData, FindExSearchNameMatch, NULL, 0 );
			}
		else {
			fDataHand = FindFirstFileExW( dir.c_str( ), FindExInfoStandard, &fData, FindExSearchNameMatch, NULL, 0 );
			}
		}
	if ( fDataHand != INVALID_HANDLE_VALUE ) {
		if ( !( wcscmp( fData.cFileName, L".." ) == 0 ) ) {
			
			}
		auto res = FindNextFileW( fDataHand, &fData );
		while ( ( fDataHand != INVALID_HANDLE_VALUE ) && ( res != 0 ) ) {

			auto scmpVal = wcscmp( fData.cFileName, L".." );
			if ( ( !( scmpVal == 0 ) ) && ( !( fData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) ) ) {
				files.emplace_back( fData.cFileName );
				}
			if ( ( fData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) && ( scmpVal != 0 ) ) {
				num += descendDirectory( fData, normSzDir, isLargeFetch, isBasicInfo, files );
				}
			else if ( ( scmpVal != 0 ) && ( ( ( fData.dwFileAttributes & FILE_ATTRIBUTE_DEVICE ) || ( fData.dwFileAttributes & FILE_ATTRIBUTE_INTEGRITY_STREAM ) || ( fData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN ) || ( fData.dwFileAttributes & FILE_ATTRIBUTE_NO_SCRUB_DATA ) || ( fData.dwFileAttributes & FILE_ATTRIBUTE_OFFLINE ) || ( fData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT ) || ( fData.dwFileAttributes & FILE_ATTRIBUTE_SPARSE_FILE ) || ( fData.dwFileAttributes & FILE_ATTRIBUTE_VIRTUAL ) ) ) ) {
				
				--num;
				}
			res = FindNextFileW( fDataHand, &fData );
			
			}
		}
	
	FindClose( fDataHand );
	return num;
	}


size_t stdWork( _In_ std::wstring arg, _In_ bool isLargeFetch, _In_ bool isBasicInfo, _Inout_ std::vector<std::wstring>& files ) {
	fixWCout( );
	std::wcout << L"Working on: `" << arg << L"`" << std::endl;
	std::int64_t numberFiles = 0;
	arg.reserve( MAX_PATH );
	if ( arg.length( ) > 3 ) {

		auto strCmp = ( arg.compare( 0, 4, arg, 0, 4 ) );
		if ( strCmp != 0 ) {
			arg = L"\\\\?\\" + arg;
			std::wcout << L"prefixed `" << arg << L"`, value now: `" << arg << L"`" << std::endl << std::endl;
			numberFiles = stdRecurseFind( arg, isLargeFetch, isBasicInfo, files );
			}
		else {
			numberFiles = stdRecurseFind( arg, isLargeFetch, isBasicInfo, files );
			}
		}
	else {
		arg = L"\\\\?\\" + arg;
		std::wcout << L"prefixed `" << arg << L"`, value now: `" << arg << L"`" << std::endl << std::endl;
		
		numberFiles = stdRecurseFind( arg, isLargeFetch, isBasicInfo, files );
		}
	fixWCout( );
	std::wcout << std::endl << L"Number of items: " << files.size( ) << std::endl;
	return files.size( );
	}

std::int64_t descendDirectory( _Inout_ WIN32_FIND_DATA& fData, _In_ std::wstring& normSzDir, _In_ bool isLargeFetch, _In_ bool isBasicInfo, _Inout_ std::vector<std::wstring>& files ) {
	std::wstring newSzDir = normSzDir;//MUST operate on copy!
	newSzDir.reserve( MAX_PATH );
	newSzDir += L"\\";
	newSzDir += fData.cFileName;
	std::int64_t num = 0;
	num += stdRecurseFind( newSzDir, isLargeFetch, isBasicInfo, files );
	return num;
	}

void fillDir( _In_ std::wstring theDir, _In_ const std::int64_t iterations ) {
	///http://stackoverflow.com/a/12468109
	auto alnumChars = std::vector<char>(
		{ '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z' } );
	std::default_random_engine rng( std::random_device { }( ) );
	std::uniform_int_distribution<> dist( 6, int( alnumChars.size( ) - 1 ) );

	auto randchar = [ alnumChars, &dist, &rng ] ( ) { return alnumChars[ dist( rng ) ]; };

	std::set<std::wstring> setFiles;
		{//limit lifetime of a by placing in own scope
		std::vector<std::wstring> a;

		stdWork( theDir, true, true, a );//default to large fetch and basic info
		std::sort( a.begin( ), a.end( ) );
		std::unique( a.begin( ), a.end( ) );

		for ( auto& fileName : a ) {
			setFiles.emplace( std::move( fileName ) );
			}
		std::wcout << L"size of files: " << a.size( ) << std::endl;
		}
	int b = 0;
	auto retval = SetCurrentDirectory( theDir.c_str( ) );
	for ( std::int64_t i = 0; i < iterations; ++i ) {
		++b;
		auto newStr = random_string( dist( rng ), randchar );
		if ( !setFiles.empty( ) ) {
			assert( !setFiles.empty( ) );
			while ( setFiles.find( newStr ) != setFiles.end( ) ) {
				newStr = random_string( dist( rng ), randchar );
				}
			if ( setFiles.find( newStr ) == setFiles.end( ) ) {
				setFiles.insert( newStr );
				auto fileHandle = CreateFile( newStr.c_str( ), GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL );
				if ( fileHandle == INVALID_HANDLE_VALUE ) {
					std::wcout << L"Error creating file: " << GetLastError( ) << std::endl;
					}
				else {
					DWORD bytesWritten = 0;
					auto val = WriteFile( fileHandle, newStr.c_str( ), DWORD( newStr.size( ) ), &bytesWritten, NULL );
					if ( !( val == TRUE ) ) {
						std::wcout << L"Error writing file: " << GetLastError( ) << L"Bytes written: " << bytesWritten << std::endl;
						}
					}
				}
			}
		else {
			assert( setFiles.empty( ) );
			setFiles.insert( newStr );
			auto fileHandle = CreateFile( newStr.c_str( ), GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL );
			if ( fileHandle == INVALID_HANDLE_VALUE ) {
				std::wcout << L"Error creating file: " << GetLastError( ) << std::endl;
				}
			else {
				DWORD bytesWritten = 0;
				auto val = WriteFile( fileHandle, newStr.c_str( ), DWORD( newStr.size( ) ), &bytesWritten, NULL );
				if ( !( val == TRUE ) ) {
					std::wcout << L"Error writing file: " << GetLastError( ) << L"Bytes written: " << bytesWritten << std::endl;
					}
				}
			}
		assert( !setFiles.empty( ) );
		}
	fixWCout( );
	std::wcout << L"size of set: " << setFiles.size( ) << std::endl;
	std::wcout << L"iterations: " << b << std::endl;
	size_t num = 0;
		{//limit lifetime of a by placing in own scope
		std::vector<std::wstring> a;
		num = stdWork( theDir, true, true, a );
		}
	std::wcout << L"number of files currently in directory: " << num << std::endl;
	}

int main( ) {
	fillDir( L"C:\\Users\\Alexander Riccio\\Documents\\test_junk_dir\\cpp_junk", 1000000 );
	return 0;
	}