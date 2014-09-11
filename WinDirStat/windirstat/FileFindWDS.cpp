// FileFinWDS.cpp	- Implementation of CFileFindWDS
//
// WinDirStat - Directory Statistics
// Copyright (C) 2004 Assarbad
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// Author: assarbad@users.sourceforge.net
//
// Last modified: $Date$

#include "StdAfx.h"
//#include "FileFindWDS.h"
//#include "windirstat.h"

CFileFindWDS::CFileFindWDS(void)
{
}

CFileFindWDS::~CFileFindWDS(void)
{
}

// Function to access the file attributes from outside
DWORD CFileFindWDS::GetAttributes( ) const {
	ASSERT( m_hContext != NULL );
	ASSERT_VALID( this );

	if ( m_pFoundInfo != NULL ) {
		return ( ( LPWIN32_FIND_DATA ) m_pFoundInfo )->dwFileAttributes;
		}
	else {
		return INVALID_FILE_ATTRIBUTES;
		}
	}

_Success_( return != NULL ) ULONGLONG CFileFindWDS::GetCompressedLength( ) const {
	/*
	  Wrapper for file size retrieval
	  This function tries to return compressed file size whenever possible.
	  If the file is not compressed the uncompressed size is being returned.
	*/
	
	ULARGE_INTEGER ret;
	ret.QuadPart = 0;//it's a union, but I'm being careful.
	ret.LowPart = GetCompressedFileSize( altGetFilePath( ), &ret.HighPart );
	if ( ( ret.LowPart == INVALID_FILE_SIZE ) ) {
		if ( ret.HighPart != NULL ) {
			if ( ( GetLastError( ) != NO_ERROR ) ) {
				return ret.QuadPart;// IN case of an error return size from CFileFind object
				}
			else {
				return GetLength( );
				}
			}
		else if ( GetLastError( ) != NO_ERROR ) {
#ifdef _DEBUG
			TRACE( _T( "Error while getting size of compressed file! Filepath: %s, %s, GetLastError: %lu, Filepath length: %i\r\n" ), altGetFilePath( ), altGetFileName( ), GetLastError(), altGetFilePath( ).GetLength( ) );
#endif
			return GetLength( );
			}
		}
		
	else {
		return ret.QuadPart;
		}
	ASSERT( false );
	return NULL;
	}

PWSTR CFileFindWDS::altGetFileName( ) const {
	ASSERT( m_hContext != NULL );
	ASSERT_VALID( this );

	if ( m_pFoundInfo != NULL ) {
		return ( ( LPWIN32_FIND_DATA ) m_pFoundInfo )->cFileName;
		}
	}


CString CFileFindWDS::altGetFilePath( ) const {
	ASSERT( m_hContext != NULL );
	ASSERT_VALID( this );

	CString strResult = m_strRoot;
	LPCTSTR pszResult = m_strRoot;
	LPCTSTR pchLast = _tcsdec( pszResult, pszResult + m_strRoot.GetLength( ) );
	ENSURE( pchLast != NULL );
	if ( ( *pchLast != _T( '\\' ) ) && ( *pchLast != _T( '/' ) ) ) {
		strResult += '\\';
		}
	strResult += altGetFileName( );
	return strResult;
	}


void altFileFind::stdWorkAsync( _In_ std::wstring arg, _In_ const bool isLargeFetch, _In_ const bool isBasicInfo, CItemBranch* workingItem ) {
	TRACE( _T("Working on: `%s`\r\n"), arg.c_str( ) );
	std::int64_t numberFiles = 0;
	arg.reserve( MAX_PATH );
	if ( arg.length( ) > 3 ) {

		auto strCmp = ( arg.compare( 0, 4, arg, 0, 4 ) );
		if ( strCmp != 0 ) {
			arg = L"\\\\?\\" + arg;
			TRACE( _T( "prefixed `%s`\r\n" ), arg.c_str( ) );
			}
		}
	else {
		arg = L"\\\\?\\" + arg;
		TRACE( _T( "prefixed `%s`\r\n" ), arg.c_str( ) );
		
		}
	numberFiles = stdRecurseFindFutures( arg, isLargeFetch, isBasicInfo, workingItem );
	TRACE( _T("\r\nNumber of items: %I64d\r\n"), numberFiles );
	}

std::int64_t altFileFind::descendDirectory( _Inout_ WIN32_FIND_DATA& fData, _In_ const std::wstring& normSzDir, _In_ const bool isLargeFetch, _In_ const bool isBasicInfo, CItemBranch* workingItem ) {
	std::wstring newSzDir = normSzDir;//MUST operate on copy!
	newSzDir.reserve( MAX_PATH );
	newSzDir += L"\\";
	newSzDir += fData.cFileName;
	std::int64_t num = 0;
	num += stdRecurseFindFutures( newSzDir, isLargeFetch, isBasicInfo, workingItem );
	return num;
	}

_Success_( return != ULONGLONG_MAX ) ULONGLONG altFileFind::GetCompressedLength( PCWSTR fName ) {
	ULARGE_INTEGER ret;
	ret.QuadPart = 0;//it's a union, but I'm being careful.
	ret.LowPart = GetCompressedFileSize( fName, &ret.HighPart );
	if ( ( ret.LowPart == INVALID_FILE_SIZE ) ) {
		if ( ret.HighPart != NULL ) {
			if ( ( GetLastError( ) != NO_ERROR ) ) {
				return ret.QuadPart;// IN case of an error return size from CFileFind object
				}
			else {
				return ULONGLONG_MAX;
				}
			}
		else if ( GetLastError( ) != NO_ERROR ) {
#ifdef _DEBUG
			TRACE( _T( "Error while getting size of compressed file! %s, GetLastError: %lu\r\n" ), fName, GetLastError( ) );
#endif
			return ULONGLONG_MAX;
			}
		}
		
	else {
		return ret.QuadPart;
		}
	ASSERT( false );
	return NULL;

	}

std::int64_t altFileFind::stdRecurseFindFutures( _In_ std::wstring dir, _In_ const bool isLargeFetch, _In_ const bool isBasicInfo, CItemBranch* workingItem ) {
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
	std::vector<std::future<std::int64_t>> futureDirs;
	futureDirs.reserve( 100 );//pseudo-arbitrary number
	WIN32_FIND_DATA fData;
	HANDLE fDataHand = NULL;

	if ( isLargeFetch ) {
		if ( isBasicInfo ) {
			fDataHand = FindFirstFileExW( dir.c_str( ), FindExInfoBasic,    &fData, FindExSearchNameMatch, NULL, FIND_FIRST_EX_LARGE_FETCH );
			}
		else {
			fDataHand = FindFirstFileExW( dir.c_str( ), FindExInfoStandard, &fData, FindExSearchNameMatch, NULL, FIND_FIRST_EX_LARGE_FETCH );
			}
		}
	else {
		if ( isBasicInfo ) {
			fDataHand = FindFirstFileExW( dir.c_str( ), FindExInfoBasic,    &fData, FindExSearchNameMatch, NULL, 0 );
			}
		else {
			fDataHand = FindFirstFileExW( dir.c_str( ), FindExInfoStandard, &fData, FindExSearchNameMatch, NULL, 0 );
			}
		}
	if ( fDataHand != INVALID_HANDLE_VALUE ) {
		if ( !( wcscmp( fData.cFileName, L".." ) == 0 ) ) {
			//++num;
			}
		auto res = FindNextFileW( fDataHand, &fData );
		while ( ( fDataHand != INVALID_HANDLE_VALUE ) && ( res != 0 ) ) {
			auto scmpVal = wcscmp( fData.cFileName, L".." );
			if ( ( !( scmpVal == 0 ) ) ) {
				++num;
				}
			if ( ( fData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) && ( scmpVal != 0 ) ) {
				//num += descendDirectory( fData, normSzDir, isLargeFetch, isBasicInfo );
				auto newDir = new CItemBranch { IT_DIRECTORY, fData.cFileName, 0, fData.ftLastWriteTime, fData.dwFileAttributes, false };
				workingItem->AddChild( newDir );

				futureDirs.emplace_back( std::async( std::launch::async | std::launch::deferred, &altFileFind::descendDirectory, fData, normSzDir, isLargeFetch, isBasicInfo, newDir ) );
				}
			else if ( ( scmpVal != 0 ) && ( ( ( fData.dwFileAttributes & FILE_ATTRIBUTE_DEVICE ) || ( fData.dwFileAttributes & FILE_ATTRIBUTE_INTEGRITY_STREAM ) || ( fData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN ) || ( fData.dwFileAttributes & FILE_ATTRIBUTE_NO_SCRUB_DATA ) || ( fData.dwFileAttributes & FILE_ATTRIBUTE_OFFLINE ) || ( fData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT ) || ( fData.dwFileAttributes & FILE_ATTRIBUTE_SPARSE_FILE ) || ( fData.dwFileAttributes & FILE_ATTRIBUTE_VIRTUAL ) ) ) ) {
				--num;
				}
			else if ( ( scmpVal != 0 ) && (! ( fData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) ) ) {
				//In `AddFile`: AddChild( new CItemBranch { IT_FILE, fi.name, fi.length, fi.lastWriteTime, fi.attributes, true } );
				//MSDN: `The size of the file is equal to (nFileSizeHigh * (MAXDWORD+1)) + nFileSizeLow.`
				if ( !( fData.dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED ) ) {
					workingItem->AddChild( new CItemBranch { IT_FILE, fData.cFileName, ( ( fData.nFileSizeHigh*( MAXDWORD + 1 ) ) + fData.nFileSizeLow ), fData.ftLastWriteTime, fData.dwFileAttributes, true } );
					}
				else {
					auto length = altFileFind::GetCompressedLength( fData.cFileName );
					if ( length != ULONGLONG_MAX ) {
						workingItem->AddChild( new CItemBranch { IT_FILE, fData.cFileName, length, fData.ftLastWriteTime, fData.dwFileAttributes, true } );
						}
					else {
						workingItem->AddChild( new CItemBranch { IT_FILE, fData.cFileName, ( ( fData.nFileSizeHigh*( MAXDWORD + 1 ) ) + fData.nFileSizeLow ), fData.ftLastWriteTime, fData.dwFileAttributes, true } );
						}
					}
				}
			//if ( std::wstring( normSzDir + fData.cFileName ).length( ) > 255 ) {
			//	std::wcout << L"" << std::endl;
			//	}

			res = FindNextFileW( fDataHand, &fData );

			}
		}
	for ( auto& a : futureDirs ) {
		num += a.get( );
		}
	workingItem->SetDone( );
	workingItem->m_readJobDone = true;

	FindClose( fDataHand );
	return num;

	}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//THIS IS NOT A WORKING IMPLEMENTATION!
//std::future<std::unique_ptr<std::vector<FILEINFO>>> AsyncWalk::descendDirectory( _Inout_ WIN32_FIND_DATA& fData, _In_ const std::wstring& normSzDir, _In_ const bool isLargeFetch, _In_ const bool isBasicInfo ) {
//	std::wstring newSzDir = normSzDir;//MUST operate on copy!
//	newSzDir.reserve( MAX_PATH );
//	newSzDir += L"\\";
//	newSzDir += fData.cFileName;
//	std::int64_t num = 0;
//	return std::async( std::launch::async | std::launch::deferred, stdRecurseFindFutures, newSzDir, isLargeFetch, isBasicInfo );
//	}
//
//
//std::unique_ptr<std::vector<FILEINFO>> AsyncWalk::stdRecurseFindFutures( _In_ std::wstring dir, _In_ const bool isLargeFetch, _In_ const bool isBasicInfo ) {
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
//	std::vector<std::future<std::unique_ptr<std::vector<FILEINFO>>>> futureDirs;
//	std::vector < std::unique_ptr<std::vector<FILEINFO>> > dirs;
//	auto files = std::make_unique<std::vector < FILEINFO >>( );
//	futureDirs.reserve( 100 );//pseudo-arbitrary number
//	WIN32_FIND_DATA fData;
//	HANDLE fDataHand = NULL;
//
//	if ( isLargeFetch ) {
//		if ( isBasicInfo ) {
//			fDataHand = FindFirstFileExW( dir.c_str( ), FindExInfoBasic,    &fData, FindExSearchNameMatch, NULL, FIND_FIRST_EX_LARGE_FETCH );
//			}
//		else {
//			fDataHand = FindFirstFileExW( dir.c_str( ), FindExInfoStandard, &fData, FindExSearchNameMatch, NULL, FIND_FIRST_EX_LARGE_FETCH );
//			}
//		}
//	else {
//		if ( isBasicInfo ) {
//			fDataHand = FindFirstFileExW( dir.c_str( ), FindExInfoBasic,    &fData, FindExSearchNameMatch, NULL, 0 );
//			}
//		else {
//			fDataHand = FindFirstFileExW( dir.c_str( ), FindExInfoStandard, &fData, FindExSearchNameMatch, NULL, 0 );
//			}
//		}
//	if ( fDataHand != INVALID_HANDLE_VALUE ) {
//		auto res = FindNextFileW( fDataHand, &fData );
//		while ( ( fDataHand != INVALID_HANDLE_VALUE ) && ( res != 0 ) ) {
//			auto scmpVal = wcscmp( fData.cFileName, L".." );
//			if ( ( !( scmpVal == 0 ) ) ) {
//				++num;
//				}
//			if ( ( fData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) && ( scmpVal != 0 ) ) {
//				futureDirs.emplace_back( std::async( std::launch::async | std::launch::deferred, descendDirectory, fData, normSzDir, isLargeFetch, isBasicInfo ) );
//				}
//			else if ( ( scmpVal != 0 ) && ( fData.dwFileAttributes & FILE_ATTRIBUTE_NORMAL ) ) {
//				FILEINFO fi;
//				fi.attributes = fData.dwFileAttributes;
//				fi.lastWriteTime = fData.ftLastWriteTime;
//				fi.length = fData.nFileSizeLow;
//				fi.name = fData.cFileName;
//				files->emplace_back( std::move( fi ) );
//				}
//			else if ( ( scmpVal != 0 ) && ( ( ( fData.dwFileAttributes & FILE_ATTRIBUTE_DEVICE ) || ( fData.dwFileAttributes & FILE_ATTRIBUTE_INTEGRITY_STREAM ) || ( fData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN ) || ( fData.dwFileAttributes & FILE_ATTRIBUTE_NO_SCRUB_DATA ) || ( fData.dwFileAttributes & FILE_ATTRIBUTE_OFFLINE ) || ( fData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT ) || ( fData.dwFileAttributes & FILE_ATTRIBUTE_SPARSE_FILE ) || ( fData.dwFileAttributes & FILE_ATTRIBUTE_VIRTUAL ) ) ) ) {
//				--num;
//				}
//			res = FindNextFileW( fDataHand, &fData );
//			}
//		}
//	for ( auto& a : futureDirs ) {
//		dirs.emplace_back( std::move( a.get( ) ) );
//		}
//	FindClose( fDataHand );
//	return std::move( files );
//	}




// $Log$
// Revision 1.3  2004/11/29 07:07:47  bseifert
// Introduced SRECT. Saves 8 Bytes in sizeof(CItem). Formatting changes.
//
// Revision 1.2  2004/11/28 14:40:06  assarbad
// - Extended CFileFindWDS to replace a global function
// - Now packing/unpacking the file attributes. This even spares a call to find encrypted/compressed files.
//
// Revision 1.1  2004/11/25 23:07:24  assarbad
// - Derived CFileFindWDS from CFileFind to correct a problem of the ANSI version
//
