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
#include "FileFindWDS.h"


// Function to access the file attributes from outside
_Pre_satisfies_( this->m_hContext != NULL ) DWORD CFileFindWDS::GetAttributes( ) const {
	ASSERT_VALID( this );

	if ( m_pFoundInfo != NULL ) {
		return static_cast<PWIN32_FIND_DATA>( m_pFoundInfo )->dwFileAttributes;
		}
	else {
		return INVALID_FILE_ATTRIBUTES;
		}
	}

_Success_( return != ULONGLONG_MAX ) ULONGLONG CFileFindWDS::GetCompressedLength( ) const {
	/*
	  Wrapper for file size retrieval
	  This function tries to return compressed file size whenever possible.
	  If the file is not compressed the uncompressed size is being returned.
	*/
	
	ULARGE_INTEGER ret;
	ret.QuadPart = 0;//it's a union, but I'm being careful.
	ret.LowPart = GetCompressedFileSizeW( GetFilePath( ), &ret.HighPart );
	if ( ret.LowPart == INVALID_FILE_SIZE ) {
		if ( ret.HighPart != NULL ) {
			if ( GetLastError( ) != NO_ERROR ) {
				return ret.QuadPart;// IN case of an error return size from CFileFind object
				}
			return GetLength( );
			}
		else if ( GetLastError( ) != NO_ERROR ) {
#ifdef _DEBUG
			TRACE( _T( "Error! Filepath: %s, Filepath length: %I64u, GetLastError: %s\r\n" ), altGetFilePath_wstring( ).c_str( ), static_cast<std::uint64_t>( altGetFilePath_wstring( ).length( ) ), GetLastErrorAsFormattedMessage( ) );
#endif
			return GetLength( );
			}
		}	
	return ret.QuadPart;
	}

_Success_( return != ULONGLONG_MAX ) ULONGLONG CFileFindWDS::GetCompressedLength( PCWSTR name ) const {
	/*
	  Wrapper for file size retrieval
	  This function tries to return compressed file size whenever possible.
	  If the file is not compressed the uncompressed size is being returned.
	*/
	
	ULARGE_INTEGER ret;
	ret.QuadPart = 0;//it's a union, but I'm being careful.
	ret.LowPart = GetCompressedFileSizeW( name, &ret.HighPart );
	if ( ret.LowPart == INVALID_FILE_SIZE ) {
		if ( ret.HighPart != NULL ) {
			if ( GetLastError( ) != NO_ERROR ) {
				return ret.QuadPart;// IN case of an error return size from CFileFind object
				}
			return GetLength( );
			}
		else if ( GetLastError( ) != NO_ERROR ) {
#ifdef _DEBUG
			TRACE( _T( "Error! Filepath: %s, Filepath length: %I64u, GetLastError: %s\r\n" ), altGetFilePath_wstring( ).c_str( ), static_cast<std::uint64_t>( altGetFilePath_wstring( ).length( ) ), GetLastErrorAsFormattedMessage( ) );
#endif
			return GetLength( );
			}
		}	
	return ret.QuadPart;
	}



_Success_(return != NULL ) PWSTR CFileFindWDS::altGetFileName( ) const {
	ASSERT( m_hContext != NULL );
	ASSERT_VALID( this );

	if ( m_pFoundInfo != NULL ) {
		//return a pointer to the filename ( already store in m_pFoundInfo ), instead of the CString that GetFileName returns. There's much less work involved by returning only a pointer, and a CString can append text from a _Field_z_ WCHAR[].
		return static_cast<PWIN32_FIND_DATA>( m_pFoundInfo )->cFileName;
		}
	return NULL;
	}


//CString CFileFindWDS::altGetFilePath( ) const {
//	ASSERT( m_hContext != NULL );
//	ASSERT_VALID( this );
//
//	CString strResult = m_strRoot;
//	PCWSTR pszResult = m_strRoot;
//	//_tcsdec: "Moves a string pointer back one character"
//	PCWSTR pchLast = _tcsdec( pszResult, pszResult + m_strRoot.GetLength( ) );
//	ENSURE( pchLast != NULL );
//	if ( ( *pchLast != _T( '\\' ) ) && ( *pchLast != _T( '/' ) ) ) {
//		strResult += '\\';
//		}
//	auto fName = altGetFileName( );
//	if ( fName != NULL ) {
//		strResult += fName;
//		}
//	else {
//		strResult += GetFileName( );
//		}
//	return strResult;
//	}

std::wstring CFileFindWDS::altGetFilePath_wstring( ) const {
	ASSERT( m_hContext != NULL );
	ASSERT_VALID( this );

	std::wstring strResult = m_strRoot.GetString( );
	PCWSTR pszResult = m_strRoot;
	//_tcsdec: "Moves a string pointer back one character"
	PCWSTR pchLast = _tcsdec( pszResult, pszResult + m_strRoot.GetLength( ) );
	ENSURE( pchLast != NULL );
	if ( ( *pchLast != _T( '\\' ) ) && ( *pchLast != _T( '/' ) ) ) {
		strResult += '\\';
		}
	auto fName = altGetFileName( );
	if ( fName != NULL ) {
		strResult += fName;
		}
	return strResult;
	}



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
