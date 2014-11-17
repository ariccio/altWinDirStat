// globalhelpers.cpp - Implementation of global helper functions
//
// WinDirStat - Directory Statistics
// Copyright (C) 2003-2004 Bernhard Seifert
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
// Author: bseifert@users.sourceforge.net, bseifert@daccord.net
//
// Last modified: $Date$

#include "stdafx.h"
//#include "globalhelpers.h"

//#include "windirstat.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define BASE 1024
#define HALF_BASE BASE/2
namespace
{
	std::wstring FormatLongLongNormal( _In_ LONGLONG n ) {
		// Returns formatted number like "123.456.789".
		//const rsize_t bufSize = 28;
		//wchar_t buffer[ bufSize ] = { 0 };
		ASSERT( n >= 0 );
#ifdef DEBUG
		CString all;
#endif
		std::wstring all_ws;
		all_ws.reserve( 27 );
		do
		{
			auto rest = INT( n % 1000 );
			n /= 1000;
			wchar_t tempBuf[ 10 ] = { 0 };
#ifdef DEBUG
			CString s;
#endif
			if ( n > 0 ) {
#ifdef DEBUG
				s.Format( _T( ",%03d" ) , rest );
#endif
				_snwprintf_s( tempBuf, 9, L",%03d", rest );
				}
			else {
#ifdef DEBUG
				s.Format( _T( "%d" ), rest );
#endif
				_snwprintf_s( tempBuf, 9, L"%d", rest );
				}
#ifdef DEBUG
			all = s + all;
#endif
			all_ws = tempBuf + all_ws;
			//wcscat_s( buffer, tempBuf );
			}
		while ( n > 0 );
			//ASSERT( all.Compare( buffer ) == 0 );
		ASSERT( all.CompareNoCase( all_ws.c_str( ) ) == 0 );
		return all_ws;
		}

	std::wstring Format_uint64_t_Normal( _In_ std::uint64_t n ) {
		// Returns formatted number like "123.456.789".
		// 18446744073709551615 is max
		//                     ^ 20 characters
		// 18,446,744,073,709,551,615
		//                           ^26 characters
		//                            26 + null terminator = 27
		//const rsize_t bufSize = 28;
		//wchar_t buffer[ bufSize ] = { 0 };

#ifdef DEBUG
		CString all;
#endif
		std::wstring all_ws;
		all_ws.reserve( 27 );

		do
		{
			auto rest = INT( n % 1000 );
			n /= 1000;
#ifdef DEBUG
			CString s;
#endif
			wchar_t tempBuf[ 10 ] = { 0 };
			if ( n > 0 ) {
				//wprintf(  );
				_snwprintf_s( tempBuf, 9, L",%03d", rest );
#ifdef DEBUG
				s.Format( _T( ",%03d" ) , rest );
#endif
				}
			else {
#ifdef DEBUG
				s.Format( _T( "%d" ), rest );
#endif	
				_snwprintf_s( tempBuf, 9, L"%d", rest );
				}
#ifdef DEBUG
			all = s + all;
#endif
			all_ws += tempBuf;
			//wcscat_s( buffer, tempBuf );
			}
		while ( n > 0 );
			//ASSERT( all.Compare( buffer ) == 0 );
		ASSERT( all.Compare( all_ws.c_str( ) ) == 0 );
		//return all;
		return all_ws;
		}


}

_Success_( SUCCEEDED( return ) ) HRESULT FormatBytes( _In_ const std::uint64_t n, _Out_writes_z_( strSize ) _Pre_writable_size_( strSize ) PWSTR psz_formatted_bytes, _In_range_( 38, 64 ) rsize_t strSize ) {
	auto res = CStyle_FormatLongLongHuman( n, psz_formatted_bytes, strSize );
	if ( !SUCCEEDED( res ) ) {
		write_BAD_FMT( psz_formatted_bytes );
		return res;
		}
	return res;
	}

std::wstring FormatBytes( _In_ const std::uint64_t n ) {
	if ( GetOptions( )->m_humanFormat ) {
		//MAX value of a std::uint64_t is 20 digits
		const rsize_t strSize = 21;
		wchar_t psz_formatted_longlong[ strSize ] = { 0 };
		auto res = CStyle_FormatLongLongHuman( n, psz_formatted_longlong, strSize );
		if ( !SUCCEEDED( res ) ) {
			write_BAD_FMT( psz_formatted_longlong );
			}
		return psz_formatted_longlong;
		}
	auto string = Format_uint64_t_Normal( n );
	return string;
	}

_Success_( SUCCEEDED( return ) ) HRESULT CStyle_FormatLongLongHuman( _In_ std::uint64_t n, _Out_writes_z_( strSize ) _Pre_writable_size_( strSize ) PWSTR psz_formatted_LONGLONG_HUMAN, _In_range_( 3, 64 ) rsize_t strSize ) {
	//MAX value of a LONGLONG is 19 digits
	DOUBLE B  = INT( n % BASE );
	n /= BASE;
	DOUBLE KB = INT( n % BASE );
	n /= BASE;
	DOUBLE MB = INT( n % BASE );
	n /= BASE;
	DOUBLE GB = INT( n % BASE );
	n /= BASE;
	DOUBLE TB = INT( n );
	const size_t bufSize = 19;
	const size_t bufSize2 = bufSize * 2;
	wchar_t buffer[ bufSize ] = { 0 };
	wchar_t buffer2[ bufSize2 ] = { 0 };

	HRESULT res = STRSAFE_E_INVALID_PARAMETER;
	HRESULT res2 = STRSAFE_E_INVALID_PARAMETER;
	if ( TB != 0 || GB == BASE - 1 && MB >= HALF_BASE ) {
		res = CStyle_FormatDouble( TB + GB / BASE, buffer, bufSize );
		if ( SUCCEEDED( res ) ) {
			//res2 = StringCchPrintfW( buffer2, bufSize2, L"%s TB", buffer );
			//auto resSWPRINTF = swprintf_s( buffer2, L"%s TB", buffer );
			auto resSWPRINTF = swprintf_s( psz_formatted_LONGLONG_HUMAN, strSize, L"%s TB", buffer );
			if ( resSWPRINTF != -1 ) {
				res2 = S_OK;
				}
			else {
				res2 = STRSAFE_E_INVALID_PARAMETER;
				}
			}
		}
	else if ( GB != 0 || MB == BASE - 1 && KB >= HALF_BASE ) {
		res = CStyle_FormatDouble( GB + MB / BASE, buffer, bufSize );
		if ( SUCCEEDED( res ) ) {
			//res2 = StringCchPrintfW( buffer2, bufSize2, L"%s GB", buffer );
			//auto resSWPRINTF = swprintf_s( buffer2, L"%s GB", buffer );
			auto resSWPRINTF = swprintf_s( psz_formatted_LONGLONG_HUMAN, strSize, L"%s GB", buffer );
			if ( resSWPRINTF != -1 ) {
				res2 = S_OK;
				}
			else {
				res2 = STRSAFE_E_INVALID_PARAMETER;
				}
			}
		}
	else if ( MB != 0 || KB == BASE - 1 && B >= HALF_BASE ) {
		res = CStyle_FormatDouble( MB + KB / BASE, buffer, bufSize );
		if ( SUCCEEDED( res ) ) {
			//res2 = StringCchPrintfW( buffer2, bufSize2, L"%s MB", buffer );
			//auto resSWPRINTF = swprintf_s( buffer2, L"%s MB", buffer );
			auto resSWPRINTF = swprintf_s( psz_formatted_LONGLONG_HUMAN, strSize, L"%s MB", buffer );
			if ( resSWPRINTF != -1 ) {
				res2 = S_OK;
				}
			else {
				res2 = STRSAFE_E_INVALID_PARAMETER;
				}
			}
		}
	else if ( KB != 0 ) {
		res = CStyle_FormatDouble( KB + B / BASE, buffer, bufSize );
		if ( SUCCEEDED( res ) ) {
			//res2 = StringCchPrintfW( buffer2, bufSize2, L"%s KB", buffer );
			//auto resSWPRINTF = swprintf_s( buffer2, L"%s KB", buffer );
			auto resSWPRINTF = swprintf_s( psz_formatted_LONGLONG_HUMAN, strSize, L"%s KB", buffer );
			if ( resSWPRINTF != -1 ) {
				res2 = S_OK;
				}
			else {
				res2 = STRSAFE_E_INVALID_PARAMETER;
				}
			}
		}
	else if ( B != 0 ) {
		//res = StringCchPrintfW( buffer2, bufSize2, L"%i Bytes", INT( B ) );
		res = StringCchPrintfW( psz_formatted_LONGLONG_HUMAN, strSize, L"%i Bytes", INT( B ) );
		res2 = res;
		}
	else {
		//res = StringCchPrintfW( buffer2, bufSize2, L"0%s", L"\0" );
		res = StringCchPrintfW( psz_formatted_LONGLONG_HUMAN, strSize, L"0%s", L"\0" );
		res2 = res;
		}
	if ( !SUCCEEDED( res2 ) ) {
		write_BAD_FMT( buffer2 );
		}
	return res2;
	//return StringCchCopyW( psz_formatted_LONGLONG_HUMAN, strSize, buffer2 );
	}

std::wstring FormatCount( _In_ const std::uint32_t n ) {
	return FormatLongLongNormal( LONGLONG( n ) );
	}

CString FormatCount( _In_ const std::uint64_t n ) {
	return Format_uint64_t_Normal( n ).c_str( );
	}

CString FormatDouble( _In_ DOUBLE d ) {// "98,4" or "98.4"
	CString s;
	s.Format( _T( "%.1f" ), d );
	return s;
	}

std::wstring FormatDouble_w( _In_ DOUBLE d ) {// "98,4" or "98.4"
	wchar_t fmt[ 64 ] = { 0 };
	auto resSWPRINTF = swprintf_s( fmt, 64, L"%.1f", d );
	if ( resSWPRINTF != -1 ) {
		return fmt;
		}
	return L"BAD swprintf_s!!!!";
	}


_Success_( SUCCEEDED( return ) ) HRESULT CStyle_FormatDouble( _In_ DOUBLE d, _Out_writes_z_( strSize ) _Pre_writable_size_( strSize ) PWSTR psz_formatted_double, _In_range_( 3, 64 ) rsize_t strSize ) {
	/*
	auto resSWPRINTF = swprintf_s( buffer2, L"%s KB", buffer );
	if ( resSWPRINTF != -1 ) {
		res2 = S_OK;
		}
	else {
		res2 = STRSAFE_E_INVALID_PARAMETER;
		}
	
	*/
	auto resSWPRINTF = swprintf_s( psz_formatted_double, strSize, L"%.1f", d );
	if ( resSWPRINTF != -1 ) {
		return S_OK;
		}
	return STRSAFE_E_INVALID_PARAMETER;

	//Range 3-64 is semi-arbitrary. I don't think I'll need to format a double that's more than 63 chars.
	//return StringCchPrintfW( psz_formatted_double, strSize, L"%.1f%", d );
	}

CString FormatFileTime( _In_ const FILETIME& t ) {
	ASSERT( &t != NULL );
	SYSTEMTIME st;
	if ( !FileTimeToSystemTime( &t, &st ) ) {
		return GetLastErrorAsFormattedMessage( );
		}
#ifdef DEBUG
	LCID lcid = MAKELCID( GetUserDefaultLangID( ), SORT_DEFAULT );
	CString date;
	VERIFY( 0 < GetDateFormatW( lcid, DATE_SHORTDATE, &st, NULL, date.GetBuffer( 256 ), 256 ) );//d M yyyy
	date.ReleaseBuffer( );
#endif

	wchar_t psz_formatted_datetime[ 73 ] = { 0 };
	auto res = CStyle_FormatFileTime( t, psz_formatted_datetime, 73 );
	if ( ! ( res == 0 ) ) {
		write_BAD_FMT( psz_formatted_datetime );
		return psz_formatted_datetime;
		}

	ASSERT( SUCCEEDED( res ) );

#ifdef _DEBUG
	CString time;
	VERIFY( 0 < GetTimeFormatW( lcid, 0, &st, NULL, time.GetBuffer( 256 ), 256 ) );//h mm ss tt
	time.ReleaseBuffer( );
	CString result = date + _T( "  " ) + time;
#endif


#ifdef _DEBUG
	auto didMatch = result.Compare( psz_formatted_datetime );
	ASSERT( didMatch == 0 );
	TRACE( _T( "Formatted file time (%i characters): %s\r\n" ), result.GetLength( ), result );
	TRACE( _T( "Formatted file time  C-STYLE       : %s\r\n" ), psz_formatted_datetime );
#endif
	if ( res == 0 ) {
		return psz_formatted_datetime;
		}
	else {
		write_BAD_FMT( psz_formatted_datetime );
		}
	return psz_formatted_datetime;
	}

_Success_( return == 0 ) int CStyle_FormatFileTime( _In_ const FILETIME t, _Out_writes_z_( strSize ) _Pre_writable_size_( strSize ) PWSTR psz_formatted_datetime, rsize_t strSize  ) {
	ASSERT( &t != NULL );
	SYSTEMTIME st;
	if ( !FileTimeToSystemTime( &t, &st ) ) {
		//psz_formatted_datetime = GetLastErrorAsFormattedMessage( );
		auto res = StringCchCopyW( psz_formatted_datetime, strSize, GetLastErrorAsFormattedMessage( ).GetBuffer( ) );
		if ( !SUCCEEDED( res ) ) {
			if ( res == STRSAFE_E_INVALID_PARAMETER ) {
				TRACE( _T( "STRSAFE_E_INVALID_PARAMETER\r\n" ) );
				}
			if ( res == STRSAFE_E_INSUFFICIENT_BUFFER ) {
				TRACE( _T( "STRSAFE_E_INSUFFICIENT_BUFFER\r\n" ) );
				}
			return 1;
			}
		return 0;
		}
	LCID lcid = MAKELCID( GetUserDefaultLangID( ), SORT_DEFAULT );

	wchar_t psz_date_wchar[ 36 ] = { 0 };
	wchar_t psz_time_wchar[ 36 ] = { 0 };
	//wchar_t psz_formatted_datetime[ 73 ];
	auto gdfres = GetDateFormatW( lcid, DATE_SHORTDATE, &st, NULL, psz_date_wchar, 36 );
	auto gtfres = GetTimeFormatW( lcid, 0, &st, NULL, psz_time_wchar, 36 );
	/*
	This function returns 0 if it does not succeed. To get extended error information, the application can call GetLastError, which can return one of the following error codes:
		ERROR_INSUFFICIENT_BUFFER. A supplied buffer size was not large enough, or it was incorrectly set to NULL.
		ERROR_INVALID_FLAGS.       The values supplied for flags were not valid.
		ERROR_INVALID_PARAMETER.   Any of the parameter values was invalid.	
	*/
	ENSURE( ( gdfres + gtfres + 2 ) < strSize );
	
	
	//TODO: rewrite these to NOT throw exceptions.
	if ( gdfres == 0 ) {
		const auto err = GetLastError( );
		if ( err == ERROR_INSUFFICIENT_BUFFER ) {
			throw "A supplied buffer size was not large enough, or it was incorrectly set to NULL.";
			}
		if ( err == ERROR_INVALID_FLAGS ) {
			throw "The values supplied for flags were not valid.";
			}
		if ( err == ERROR_INVALID_PARAMETER ) {
			throw "Any of the parameter values was invalid.";
			}
		}
	if ( gtfres == 0 ) {
		const auto err = GetLastError( );
		if ( err == ERROR_INSUFFICIENT_BUFFER ) {
			throw "A supplied buffer size was not large enough, or it was incorrectly set to NULL.";
			}
		if ( err == ERROR_INVALID_FLAGS ) {
			throw "The values supplied for flags were not valid.";
			}
		if ( err == ERROR_INVALID_PARAMETER ) {
			throw "Any of the parameter values was invalid.";
			}
		}



	//auto cpyres  = wcscpy_s( psz_formatted_datetime, static_cast<rsize_t>( gdfres ), psz_date_wchar );
	auto cpyres  = wcscpy_s( psz_formatted_datetime, strSize, psz_date_wchar );
	auto wcsres  = wcscat_s( psz_formatted_datetime, strSize, L"  " );
	auto wcsres2 = wcscat_s( psz_formatted_datetime, strSize, psz_time_wchar );

	//auto lError = GetLastError( );

	return cpyres + wcsres + wcsres2;
	}

CString FormatAttributes( _In_ const DWORD attr ) {
	if ( attr == INVALID_FILE_ATTRIBUTES ) {
		return _T( "?????" );
		}

	CString attributes;
	attributes.Append( ( attr bitand FILE_ATTRIBUTE_READONLY )   ? _T( "R" ) : _T( "" ) );
	attributes.Append( ( attr bitand FILE_ATTRIBUTE_HIDDEN )     ? _T( "H" ) : _T( "" ) );
	attributes.Append( ( attr bitand FILE_ATTRIBUTE_SYSTEM )     ? _T( "S" ) : _T( "" ) );
	attributes.Append( ( attr bitand FILE_ATTRIBUTE_ARCHIVE )    ? _T( "A" ) : _T( "" ) );
	attributes.Append( ( attr bitand FILE_ATTRIBUTE_COMPRESSED ) ? _T( "C" ) : _T( "" ) );
	attributes.Append( ( attr bitand FILE_ATTRIBUTE_ENCRYPTED )  ? _T( "E" ) : _T( "" ) );

	return attributes;
	}

_Success_( return == 0 ) int CStyle_FormatAttributes( _In_ const DWORD attr, _Out_writes_z_( strSize ) _Pre_writable_size_( strSize ) PWSTR psz_formatted_attributes, _In_range_( 1, 6 ) rsize_t strSize ) {
	if ( attr == INVALID_FILE_ATTRIBUTES ) {
		psz_formatted_attributes = _T( "?????" );
		}
	int errCode[ 6 ] = { 0 };
	rsize_t charsWritten = 0;
	CString attributes;
	if ( ( attr bitand FILE_ATTRIBUTE_READONLY ) != 0 ) {
		errCode[ 0 ] = wcscpy_s( psz_formatted_attributes + charsWritten, strSize - 1 - charsWritten, L"R" );
		charsWritten += ( ( errCode[ 0 ] == 0 ) ? 1 : 0 );
		}
	if ( ( attr bitand FILE_ATTRIBUTE_HIDDEN ) != 0 ) {
		errCode[ 1 ] = wcscpy_s( psz_formatted_attributes + charsWritten, strSize - 1 - charsWritten, L"H" );
		charsWritten += ( ( errCode[ 1 ] == 0 ) ? 1 : 0 );
		}
	if ( ( attr bitand FILE_ATTRIBUTE_SYSTEM ) != 0 ) {
		errCode[ 2 ] = wcscpy_s( psz_formatted_attributes + charsWritten, strSize - 1 - charsWritten, L"S" );
		charsWritten += ( ( errCode[ 2 ] == 0 ) ? 1 : 0 );
		}
	if ( ( attr bitand FILE_ATTRIBUTE_ARCHIVE ) != 0 ) {
		errCode[ 3 ] = wcscpy_s( psz_formatted_attributes + charsWritten, strSize - 1 - charsWritten, L"A" );
		charsWritten += ( ( errCode[ 3 ] == 0 ) ? 1 : 0 );
		}
	if ( ( attr bitand FILE_ATTRIBUTE_COMPRESSED ) != 0 ) {
		errCode[ 4 ] = wcscpy_s( psz_formatted_attributes + charsWritten, strSize - 1 - charsWritten, L"C" );
		charsWritten += ( ( errCode[ 4 ] == 0 ) ? 1 : 0 );
		}
	if ( ( attr bitand FILE_ATTRIBUTE_ENCRYPTED ) != 0 ) {
		errCode[ 5 ] = wcscpy_s( psz_formatted_attributes + charsWritten, strSize - 1 - charsWritten, L"E" );
		charsWritten += ( ( errCode[ 5 ] == 0 ) ? 1 : 0 );
		}
	ASSERT( charsWritten < strSize );
	ASSERT( strSize > 0 );
	psz_formatted_attributes[ strSize - 1 ] = 0;
	return std::accumulate( errCode, errCode + 6, 0 );
	}

_Success_( return == 0 ) int CStyle_FormatAttributes( _In_ const attribs& attr, _Out_writes_z_( strSize ) _Pre_writable_size_( strSize ) PWSTR psz_formatted_attributes, _In_range_( 1, 6 ) rsize_t strSize ) {
	if ( attr.invalid ) {
		psz_formatted_attributes = _T( "?????" );
		}
	int errCode[ 6 ] = { 0 };
	rsize_t charsWritten = 0;
	CString attributes;
	if ( attr.readonly ) {
		errCode[ 0 ] = wcscpy_s( psz_formatted_attributes + charsWritten, strSize - 1 - charsWritten, L"R" );
		charsWritten += ( ( errCode[ 0 ] == 0 ) ? 1 : 0 );
		}
	if ( attr.hidden ) {
		errCode[ 1 ] = wcscpy_s( psz_formatted_attributes + charsWritten, strSize - 1 - charsWritten, L"H" );
		charsWritten += ( ( errCode[ 1 ] == 0 ) ? 1 : 0 );
		}
	if ( attr.system ) {
		errCode[ 2 ] = wcscpy_s( psz_formatted_attributes + charsWritten, strSize - 1 - charsWritten, L"S" );
		charsWritten += ( ( errCode[ 2 ] == 0 ) ? 1 : 0 );
		}
	if ( attr.archive ) {
		errCode[ 3 ] = wcscpy_s( psz_formatted_attributes + charsWritten, strSize - 1 - charsWritten, L"A" );
		charsWritten += ( ( errCode[ 3 ] == 0 ) ? 1 : 0 );
		}
	if ( attr.compressed ) {
		errCode[ 4 ] = wcscpy_s( psz_formatted_attributes + charsWritten, strSize - 1 - charsWritten, L"C" );
		charsWritten += ( ( errCode[ 4 ] == 0 ) ? 1 : 0 );
		}
	if ( attr.encrypted ) {
		errCode[ 5 ] = wcscpy_s( psz_formatted_attributes + charsWritten, strSize - 1 - charsWritten, L"E" );
		charsWritten += ( ( errCode[ 5 ] == 0 ) ? 1 : 0 );
		}
	ASSERT( charsWritten < strSize );
	ASSERT( strSize > 0 );
	psz_formatted_attributes[ strSize - 1 ] = 0;
	return std::accumulate( errCode, errCode + 6, 0 );
	}


_Success_( return != false ) bool GetVolumeName( _In_z_ const PCWSTR rootPath, _Out_ CString& volumeName ) {
	//CString ret;
	//DWORD dummy;

	auto old = SetErrorMode( SEM_FAILCRITICALERRORS );
	
	//GetVolumeInformation returns 0 on failure
	auto buffer = volumeName.GetBuffer( MAX_PATH );
	BOOL b = GetVolumeInformationW( rootPath, buffer, MAX_PATH, NULL, NULL, NULL, NULL, 0 );
	volumeName.ReleaseBuffer( );

	if ( b == 0 ) {
		TRACE( _T( "GetVolumeInformation(%s) failed: %u\n" ), rootPath, GetLastError( ) );
		}
	SetErrorMode( old );
	
	return ( b != 0 );
	}

bool GetVolumeName( _In_z_ const PCWSTR rootPath ) {
	//CString ret;
	//DWORD dummy;

	auto old = SetErrorMode( SEM_FAILCRITICALERRORS );
	
	//GetVolumeInformation returns 0 on failure
	BOOL b = GetVolumeInformationW( rootPath, NULL, 0, NULL, NULL, NULL, NULL, 0 );

	if ( b == 0 ) {
		TRACE( _T( "GetVolumeInformation(%s) failed: %u\n" ), rootPath, GetLastError( ) );
		}
	SetErrorMode( old );
	
	return ( b != 0 );
	}


CString FormatVolumeName( _In_ const CString& rootPath, _In_ const CString& volumeName ) {
	ASSERT( rootPath != _T( "" ) );
	CString ret;
	ret.Format( _T( "%s (%s)" ), volumeName.GetString( ), rootPath.Left( 2 ).GetString( ) );
	return ret;
	}

CString MyGetFullPathName( _In_ const CString& relativePath ) {
	CString buffer;

	ULONG len = _MAX_PATH;

	auto dw = GetFullPathNameW( relativePath, static_cast<DWORD>( len ), buffer.GetBuffer( static_cast<int>( len ) ), NULL );
	buffer.ReleaseBuffer( );

	while ( dw >= len ) {
		len *= 2;
		dw = GetFullPathNameW( relativePath, static_cast<DWORD>( len ), buffer.GetBuffer( static_cast<int>( len ) ), NULL );
		buffer.ReleaseBuffer( );
		}

	if ( dw == 0 ) {
		TRACE( "GetFullPathName(%s) failed: GetLastError returns %u\r\n", relativePath, GetLastError( ) );
		return relativePath;
		}

	return buffer;
	}

//CString GetAppFileName( ) {
//	CString s;
//	VERIFY( GetModuleFileNameW( NULL, s.GetBuffer( MAX_PATH ), MAX_PATH ) );
//	s.ReleaseBuffer( );
//	return s;
//	}


void MyShellExecute( _In_opt_ HWND hwnd, _In_opt_z_ PCWSTR pOperation, _In_z_ PCWSTR pFile, _In_opt_z_ PCWSTR pParameters, _In_opt_z_ PCWSTR pDirectory, _In_ const INT nShowCmd ) {
	CWaitCursor wc;
	auto h = reinterpret_cast<INT_PTR>( ShellExecuteW( hwnd, pOperation, pFile, pParameters, pDirectory, nShowCmd ) );
	if ( h <= 32 ) {
		CString a;
		a += ( _T( "ShellExecute failed: (error #: " ) + h );
		a += +_T( " ), message: " ) + GetLastErrorAsFormattedMessage( );

		AfxMessageBox( a );
		displayWindowsMsgBoxWithError( );
		return;

		}
	}


_Success_( return > 32 ) INT_PTR ShellExecuteWithAssocDialog( _In_ const HWND hwnd, _In_z_ const PCWSTR filename ) {
	CWaitCursor wc;
	auto u = reinterpret_cast<INT_PTR>( ShellExecuteW( hwnd, NULL, filename, NULL, NULL, SW_SHOWNORMAL ) );
	if ( u == SE_ERR_NOASSOC ) {
		// Q192352
		CString sysDir;
		//-- Get the system directory so that we know where Rundll32.exe resides.
		GetSystemDirectoryW( sysDir.GetBuffer( _MAX_PATH ), _MAX_PATH );
		sysDir.ReleaseBuffer( );
		
		CString parameters = _T( "shell32.dll,OpenAs_RunDLL " );
		u = reinterpret_cast<INT_PTR>( ShellExecuteW( hwnd, _T( "open" ), _T( "RUNDLL32.EXE" ), parameters + filename, sysDir, SW_SHOWNORMAL ) );
		}
	return u;
	}

void MyGetDiskFreeSpace( _In_z_ const PCWSTR pszRootPath, _Out_ _Out_range_( 0, 18446744073709551615 ) std::uint64_t& total, _Out_ _Out_range_( 0, 18446744073709551615 ) std::uint64_t& unused ) {
	//ASSERT( pszRootPath != _T( "" ) );
	ULARGE_INTEGER uavailable = { { 0 } };
	ULARGE_INTEGER utotal     = { { 0 } };
	ULARGE_INTEGER ufree      = { { 0 } };
	uavailable.QuadPart       = 0;
	utotal.QuadPart           = 0;
	ufree.QuadPart            = 0;

	// On NT 4.0, the 2nd Parameter to this function must NOT be NULL.
	BOOL b = GetDiskFreeSpaceExW( pszRootPath, &uavailable, &utotal, &ufree );
	if ( !b ) {
		TRACE( _T( "\tGetDiskFreeSpaceEx(%s) failed.\r\n" ), pszRootPath );
		}
	else {
		TRACE( _T( "stats:(%s) avail: %llu, total: %llu, free: %llu\r\n" ), pszRootPath, uavailable, utotal, ufree );
		ASSERT( uavailable.QuadPart <= utotal.QuadPart);
		ASSERT( ufree.QuadPart <= utotal.QuadPart );
		ASSERT( uavailable.QuadPart != utotal.QuadPart );
		ASSERT( ufree.QuadPart != utotal.QuadPart );
		}
	total  = utotal.QuadPart; // will fail, when more than 2^63 Bytes free ....
	unused = ufree.QuadPart;
	ASSERT( unused <= total );
	}


void MyGetDiskFreeSpace( _In_z_ const PCWSTR pszRootPath, _Inout_ LONGLONG& total, _Inout_ LONGLONG& unused, _Inout_ LONGLONG& available ) {
	//ASSERT( pszRootPath != _T( "" ) );
	ULARGE_INTEGER uavailable = { { 0 } };
	ULARGE_INTEGER utotal     = { { 0 } };
	ULARGE_INTEGER ufree      = { { 0 } };
	uavailable.QuadPart       = 0;
	utotal.QuadPart           = 0;
	ufree.QuadPart            = 0;

	// On NT 4.0, the 2nd Parameter to this function must NOT be NULL.
	BOOL b = GetDiskFreeSpaceExW( pszRootPath, &uavailable, &utotal, &ufree );
	if ( !b ) {
		TRACE( _T( "\tGetDiskFreeSpaceEx(%s) failed.\r\n" ), pszRootPath );
		}
	else {
		TRACE( _T("\tGetDiskFreeSpaceEx(%s) successfully returned uavailable: %llu, utotal: %llu, ufree: %llu\r\n"), pszRootPath, uavailable, utotal, ufree);
		ASSERT( uavailable.QuadPart <= utotal.QuadPart);
		ASSERT( ufree.QuadPart <= utotal.QuadPart );
		ASSERT( uavailable.QuadPart != utotal.QuadPart );
		ASSERT( ufree.QuadPart != utotal.QuadPart );
		}
	total     = LONGLONG( utotal.QuadPart ); // will fail, when more than 2^63 Bytes free ....
	unused    = LONGLONG( ufree.QuadPart);
	available = LONGLONG( uavailable.QuadPart );
	ASSERT( unused <= total );
	}


CString GetCOMSPEC( ) {
	CString cmd;

	auto dw = GetEnvironmentVariableW( _T( "COMSPEC" ), cmd.GetBuffer( _MAX_PATH ), _MAX_PATH );
	cmd.ReleaseBuffer( );

	if ( dw == 0 ) {
		TRACE(_T("COMSPEC not set.\n"));
		cmd = _T( "cmd.exe" );
		}
	return cmd;
	}


bool DriveExists( _In_ const CString& path ) {
	//ASSERT( path != _T( "" ) );
	if ( path.GetLength( ) != 3 || path[ 1 ] != _T( ':' ) || path[ 2 ] != _T( '\\' ) ) {
		return false;
		}
	//auto letter = path.Left( 1 ).MakeLower( ).GetString( );
	wchar_t ltr[ 2 ] = { 0 };
	ltr[ 0 ] = path.Left( 1 ).MakeLower( )[ 0 ];
	ltr[ 1 ] = 0;
	//INT d = letter[ 0 ] - _T( 'a' );//????BUGBUG TODO: ?
	
	//is 'a' == 97?
	INT d = ltr[ 0 ] - _T( 'a' );//????BUGBUG TODO: ?

	DWORD mask = 0x1 << d;

	if ( ( mask bitand GetLogicalDrives( ) ) == 0 ) {
		return false;
		}

	
	if ( !GetVolumeName( path ) ) {
		return false;
		}

	return true;
	}


CString MyQueryDosDevice( _In_z_ const PCWSTR drive ) {
	/*
	  drive is a drive spec like C: or C:\ or C:\path (path is ignored).
	  This function returns "", if QueryDosDevice is unsupported or drive doesn't begin with a drive letter, 'Information about MS-DOS device names' otherwise: Something like

	  \Device\Harddisk\Volume1                               for a local drive
	  \Device\LanmanRedirector\;T:0000000011e98\spock\temp   for a network drive 
	  \??\C:\programme                                       for a SUBSTed local path
	  \??\T:\Neuer Ordner                                    for a SUBSTed SUBSTed path
	  \??\UNC\spock\temp                                     for a SUBSTed UNC path

	  As always, I had to experimentally determine these strings, Microsoft didn't think it necessary to document them. (Sometimes I think, they even don't document such things internally...)

	  I hope that a drive is SUBSTed iff this string starts with \??\.

	  assarbad:
		It cannot be safely determined weather a path is or is not SUBSTed on NT via this API. You would have to lookup the volume mount points because SUBST only works per session by definition whereas volume mount points work across sessions (after restarts).
	*/
	CString d = drive;

	if ( d.GetLength( ) < 2 || d[ 1 ] != _T( ':' ) ) {//parenthesis, maybe?
		return _T( "" );
		}

	d = d.Left( 2 );

	//CQueryDosDeviceApi api;

	CString info;
	auto dw = QueryDosDeviceW( d, info.GetBuffer( 512 ), 512 );//eek
	info.ReleaseBuffer( );

	if ( dw == 0 ) {
		TRACE( _T( "QueryDosDevice(%s) failed: %s\r\n" ), d, GetLastErrorAsFormattedMessage( ) );
		return _T( "" );
		}

	return info;
	}

bool IsSUBSTedDrive( _In_z_ const PCWSTR drive ) {
	/*
	  drive is a drive spec like C: or C:\ or C:\path (path is ignored).
	  This function returns true, if QueryDosDevice() is supported and drive is a SUBSTed drive.
	*/
	auto info = MyQueryDosDevice( drive );
	return ( info.GetLength( ) >= 4 && info.Left( 4 ) == "\\??\\" );
	}

const LARGE_INTEGER help_QueryPerformanceCounter( ) {
	LARGE_INTEGER doneTime;
	BOOL behavedWell = QueryPerformanceCounter( &doneTime );
	ASSERT( behavedWell );
	if ( !behavedWell ) {
		std::string a;
		//a += ( __FUNCTION__, __LINE__ );
		a += __FUNCTION__;
		a += std::to_string( __LINE__ );
		std::wstring b( a.begin( ), a.end( ) );
		MessageBoxW( NULL, TEXT( "QueryPerformanceCounter failed!!" ), b.c_str( ), MB_OK );
		doneTime.QuadPart = -1;
		}
	return doneTime;
	}

const LARGE_INTEGER help_QueryPerformanceFrequency( ) {
	LARGE_INTEGER doneTime;
	BOOL behavedWell = QueryPerformanceFrequency( &doneTime );
	ASSERT( behavedWell );
	if ( !behavedWell ) {
		std::string a;
		//a += ( __FUNCTION__, __LINE__ );
		a += __FUNCTION__;
		a += std::to_string( __LINE__ );
		std::wstring b( a.begin( ), a.end( ) );
		MessageBoxW( NULL, TEXT( "QueryPerformanceFrequency failed!!" ), b.c_str( ), MB_OK );
		doneTime.QuadPart = -1;
		}
	return doneTime;
	}


//All the zeroInits assume this
static_assert( NULL == 0, "Check the zeroInit functions! Make sure that they're actually initializing to zero!" );
SHELLEXECUTEINFO partInitSEI( ) {
	SHELLEXECUTEINFO sei;
	sei.cbSize       = sizeof( sei );
	sei.dwHotKey     = NULL;
	sei.fMask        = NULL;
	sei.hIcon        = NULL;
	sei.hInstApp     = NULL;
	sei.hkeyClass    = NULL;
	sei.hMonitor     = NULL;
	sei.hProcess     = NULL;
	sei.hwnd         = NULL;
	sei.lpClass      = NULL;
	sei.lpDirectory  = NULL;
	sei.lpFile       = NULL;
	sei.lpIDList     = NULL;
	sei.lpParameters = NULL;
	sei.lpVerb       = NULL;
	sei.nShow        = NULL;
	return std::move( sei );
	}


SHELLEXECUTEINFO zeroInitSEI( ) {
	SHELLEXECUTEINFO sei;
	sei.cbSize       = NULL;
	sei.dwHotKey     = NULL;
	sei.fMask        = NULL;
	sei.hIcon        = NULL;
	sei.hInstApp     = NULL;
	sei.hkeyClass    = NULL;
	sei.hMonitor     = NULL;
	sei.hProcess     = NULL;
	sei.hwnd         = NULL;
	sei.lpClass      = NULL;
	sei.lpDirectory  = NULL;
	sei.lpFile       = NULL;
	sei.lpIDList     = NULL;
	sei.lpParameters = NULL;
	sei.lpVerb       = NULL;
	sei.nShow        = NULL;
	return std::move( sei );
	}

WINDOWPLACEMENT zeroInitWINDOWPLACEMENT( ) {
	WINDOWPLACEMENT wp;
	wp.flags                   = NULL;
	wp.ptMaxPosition.x         = NULL;
	wp.ptMaxPosition.y         = NULL;
	wp.ptMinPosition.x         = NULL;
	wp.ptMinPosition.y         = NULL;
	wp.rcNormalPosition.bottom = NULL;
	wp.rcNormalPosition.left   = NULL;
	wp.rcNormalPosition.right  = NULL;
	wp.rcNormalPosition.top    = NULL;
	wp.showCmd                 = NULL;
	wp.length                  = sizeof( wp );

	return std::move( wp );
	}

LVHITTESTINFO zeroInitLVHITTESTINFO( ) {
	LVHITTESTINFO hti;
	hti.flags    = NULL;
	hti.iGroup   = NULL;
	hti.iItem    = NULL;
	hti.iSubItem = NULL;
	hti.pt.x     = NULL;
	hti.pt.y     = NULL;
	return std::move( hti );
	}

HDITEM zeroInitHDITEM( ) {
	HDITEM hditem;

	hditem.cchTextMax = NULL;
	hditem.cxy        = NULL;
	hditem.fmt        = NULL;
	hditem.hbm        = NULL;
	hditem.iImage     = NULL;
	hditem.iOrder     = NULL;
	hditem.lParam     = NULL;
	hditem.mask       = NULL;
	hditem.pszText    = NULL;
	hditem.pvFilter   = NULL;
	hditem.state      = NULL;
	hditem.type       = NULL;
	return std::move( hditem );
	}

LVFINDINFO zeroInitLVFINDINFO( ) {
	LVFINDINFO fi;
	fi.flags       = NULL;
	fi.lParam      = NULL;
	fi.psz         = NULL;
	fi.pt.x        = NULL;
	fi.pt.y        = NULL;
	fi.vkDirection = NULL;
	return std::move( fi );
	}

LVITEM partInitLVITEM( ) {
	LVITEM lvitem;
	lvitem.cchTextMax = NULL;
	lvitem.cColumns   = NULL;
	lvitem.iGroup     = NULL;
	lvitem.iGroupId   = NULL;
	lvitem.iIndent    = NULL;
	lvitem.iSubItem   = NULL;
	lvitem.piColFmt   = NULL;
	lvitem.puColumns  = NULL;
	lvitem.state      = NULL;
	lvitem.stateMask  = NULL;
	return std::move( lvitem );
	}

PROCESS_MEMORY_COUNTERS zeroInitPROCESS_MEMORY_COUNTERS( ) {
	PROCESS_MEMORY_COUNTERS pmc;
	pmc.cb                         = NULL;
	pmc.PageFaultCount             = NULL;
	pmc.PagefileUsage              = NULL;
	pmc.PeakPagefileUsage          = NULL;
	pmc.PeakWorkingSetSize         = NULL;
	pmc.QuotaNonPagedPoolUsage     = NULL;
	pmc.QuotaPagedPoolUsage        = NULL;
	pmc.QuotaPeakNonPagedPoolUsage = NULL;
	pmc.QuotaPeakPagedPoolUsage    = NULL;
	pmc.WorkingSetSize             = NULL;
	return std::move( pmc );
	}
STARTUPINFO zeroInitSTARTUPINFO( ) {
	STARTUPINFO si;
	si.cb              = NULL;
	si.cbReserved2     = NULL;
	si.dwFillAttribute = NULL;
	si.dwFlags         = NULL;
	si.dwX             = NULL;
	si.dwXCountChars   = NULL;
	si.dwXSize         = NULL;
	si.dwY             = NULL;
	si.dwYCountChars   = NULL;
	si.dwYSize         = NULL;
	si.hStdError       = NULL;
	si.hStdInput       = NULL;
	si.hStdOutput      = NULL;
	si.lpDesktop       = NULL;
	si.lpReserved      = NULL;
	si.lpReserved2     = NULL;
	si.lpTitle         = NULL;
	si.wShowWindow     = NULL;
	return std::move( si );
	}

PROCESS_INFORMATION zeroInitPROCESS_INFORMATION( ) {
	PROCESS_INFORMATION pi;
	pi.dwProcessId = NULL;
	pi.dwThreadId  = NULL;
	pi.hProcess    = NULL;
	pi.hThread     = NULL;
	return std::move( pi );
	}

NMLISTVIEW zeroInitNMLISTVIEW( ) {
	NMLISTVIEW listView;
	listView.hdr.code     = NULL;
	listView.hdr.hwndFrom = NULL;
	listView.hdr.idFrom   = NULL;
	listView.iItem        = NULL;
	listView.iSubItem     = NULL;
	listView.lParam       = NULL;
	listView.ptAction.x   = NULL;
	listView.ptAction.y   = NULL;
	listView.uChanged     = NULL;
	listView.uNewState    = NULL;
	listView.uOldState    = NULL;
	return std::move( listView );
	}

BROWSEINFO zeroInitBROWSEINFO( ) {
	BROWSEINFO bi;
	bi.hwndOwner      = NULL;
	bi.iImage         = NULL;
	bi.lParam         = NULL;
	bi.lpfn           = NULL;
	bi.lpszTitle      = NULL;
	bi.pidlRoot       = NULL;
	bi.pszDisplayName = NULL;
	bi.ulFlags        = NULL;
	return std::move( bi );
	}

SHFILEOPSTRUCT zeroInitSHFILEOPSTRUCT( ) {
	SHFILEOPSTRUCT sfos;
	sfos.fAnyOperationsAborted = NULL;
	sfos.fFlags                = NULL;
	sfos.hNameMappings         = NULL;
	sfos.hwnd                  = NULL;
	sfos.lpszProgressTitle     = NULL;
	sfos.pFrom                 = NULL;
	sfos.pTo                   = NULL;
	sfos.wFunc                 = NULL;
	return std::move( sfos );
	}

CString GetLastErrorAsFormattedMessage( ) {
	const size_t msgBufSize = 2 * 1024;
	wchar_t msgBuf[ msgBufSize ] = { 0 };
	auto err = GetLastError( );
	auto ret = FormatMessageW( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, err, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), msgBuf, msgBufSize, NULL );
	if ( ret > 0 ) {
		return CString( msgBuf );
		}
	return CString( "FormatMessage failed to format an error!" );
	}

void displayWindowsMsgBoxWithError( ) {
	auto errMsg = GetLastErrorAsFormattedMessage( );
	MessageBoxW( NULL, PCWSTR( errMsg ), TEXT( "Error" ), MB_OK );
	TRACE( _T( "Error: %s\r\n" ), errMsg );
	}

void displayWindowsMsgBoxWithMessage( CString message ) {
	MessageBoxW( NULL, message, TEXT( "Error" ), MB_OK );
	TRACE( _T( "Error: %s\r\n" ), message );
	}

void check8Dot3NameCreationAndNotifyUser( ) {
	HKEY keyHandle = NULL;

	auto res = RegOpenKeyExW( HKEY_LOCAL_MACHINE, _T( "SYSTEM\\CurrentControlSet\\Control\\FileSystem" ), NULL, KEY_READ, &keyHandle );

	if ( res != ERROR_SUCCESS ) {
		TRACE( _T( "key not found!\r\n" ) );
		return;
		}

	else {
		}

	DWORD valueType = 0;
	//std::unique_ptr<char[ ]> databuffer = std::make_unique<char[]>(4);//I wish...
	static_assert( sizeof( BYTE ) == 1, "bad BYTE size!" );
	BYTE data[ 4 ];
	static_assert( sizeof( data ) == sizeof( REG_DWORD ), "bad size!" );
			
	DWORD bufferSize = sizeof( data );
			
	res = RegQueryValueExW( keyHandle, _T( "NtfsDisable8dot3NameCreation" ), NULL, &valueType, &data[0], &bufferSize );

	if ( res != ERROR_SUCCESS ) {
		if ( res == ERROR_MORE_DATA ) {
			return;
			}
		else if ( res == ERROR_FILE_NOT_FOUND) {
			return;
			}
		else {
			return;
			}
		}
	DWORD value = data[ 0 ];
	/*
		0 = NTFS creates short file names. This setting enables applications that cannot process long file names and computers that use differentcode pages to find the files.
		1 = NTFS does not create short file names. Although this setting increases file performance, applications that cannot process long file names, and computers that use different code pages, might not be able to find the files.
		2 = NTFS sets the 8.3 naming convention creation on a per volume basis.
		3 = NTFS disables 8dot3 name creation on all volumes except the system volume.
	*/
	if ( value == 0 ) {
		MessageBoxW( NULL, _T( "Your computer is set to create short (8.3 style) names for files on all NTFS volumes. This can TREMENDOUSLY slow directory operations - As a result, the amount of time required to perform a directory listing increases with the square of the number of files in the directory! For more, see Microsoft KnowledgeBase article ID: 130694" ), _T( "Performance warning!"), MB_ICONWARNING );
		}

	if ( value == 2 ) {
		MessageBoxW( NULL, _T( "Your computer is set to create short (8.3 style) names for files on NTFS volumes, on a per-volume-setting basis. Shore file name creation can TREMENDOUSLY slow directory operations - As a result, the amount of time required to perform a directory listing increases with the square of the number of files in the directory! For more, see Microsoft KnowledgeBase article ID: 130694" ), _T( "Performance warning!"), MB_ICONWARNING );
		}

	if ( value == 3 ) {
		MessageBoxW( NULL, _T( "Your computer is set to create short (8.3 style) names for files on the system volume. If you're running WinDirStat against any other volume you can safely ignore this warning. Short file name creation can TREMENDOUSLY slow directory operations - As a result, the amount of time required to perform a directory listing increases with the square of the number of files in the directory! For more, see Microsoft KnowledgeBase article ID: 130694" ), _T( "Performance warning!"), MB_ICONWARNING );
		}
	}
void zeroDate( _Out_ FILETIME& in ) {
	in.dwHighDateTime = 0;
	in.dwLowDateTime  = 0;
	}

FILETIME zeroInitFILETIME( ) {
	FILETIME ft;
	ft.dwHighDateTime = NULL;
	ft.dwLowDateTime = NULL;
	return std::move( ft );
	}

// Encodes a selection from the CSelectDrivesDlg into a string which can be routed as a pseudo document "path" through MFC and finally arrives in OnOpenDocument().
CString EncodeSelection( _In_ const RADIO radio, _In_ const CString folder, _In_ const CStringArray& drives ) {
	CString ret;
	TRACE( _T( "Encoding selection %s\r\n" ), folder );
	switch ( radio ) {
			case RADIO_ALLLOCALDRIVES:
			case RADIO_SOMEDRIVES:
				{
				for ( INT i = 0; i < drives.GetSize( ); i++ ) {
					if ( i > 0 ) {
						ret += CString( _T( '|' ) );// `|` is the encoding separator, which is not allowed in file names.;
						}
					ret += drives[ i ];
					}
				}
				break;

			case RADIO_AFOLDER:
				ret.Format( _T( "%s" ), folder.GetString( ) );
				break;
		}
	TRACE( _T( "Selection encoded as '%s'\r\n" ), ret );
	return ret;
	}

CRect BuildCRect( const SRECT& in ) {
	//ASSERT( ( in.left != -1 ) && ( in.top != -1 ) && ( in.right != -1 ) && ( in.bottom != -1 ) );
	ASSERT( ( in.right + 1 ) >= in.left );
	ASSERT( in.bottom >= in.top );
	CRect out;
	out.left   = LONG( in.left );
	out.top    = LONG( in.top );
	out.right  = LONG( in.right );
	out.bottom = LONG( in.bottom );
	ASSERT( out.left == in.left );
	ASSERT( out.top == in.top );
	ASSERT( out.right == in.right );
	ASSERT( out.bottom == in.bottom );
	out.NormalizeRect( );
	ASSERT( out.right >= out.left );
	ASSERT( out.bottom >= out.top );
	return std::move( out );
	}


std::vector<COLORREF> GetDefaultPaletteAsVector( ) {
	std::vector<COLORREF> colorVector;
	std::vector<COLORREF> defaultColorVec = { RGB( 0, 0, 255 ), RGB( 255, 0, 0 ), RGB( 0, 255, 0 ), RGB( 0, 255, 255 ), RGB( 255, 0, 255 ), RGB( 255, 255, 0 ), RGB( 150, 150, 255 ), RGB( 255, 150, 150 ), RGB( 150, 255, 150 ), RGB( 150, 255, 255 ), RGB( 255, 150, 255 ), RGB( 255, 255, 150 ), RGB( 255, 255, 255 ) };
	colorVector.reserve( defaultColorVec.size( ) + 1 );
	for ( auto& aColor : defaultColorVec ) {
		colorVector.emplace_back( CColorSpace::MakeBrightColor( aColor, PALETTE_BRIGHTNESS ) );
		}
	return std::move( colorVector );
	}


void write_BAD_FMT( _Out_writes_z_( 8 ) _Pre_writable_size_( 8 ) PWSTR pszFMT ) {
	pszFMT[ 0 ] = 'B';
	pszFMT[ 1 ] = 'A';
	pszFMT[ 2 ] = 'D';
	pszFMT[ 3 ] = '_';
	pszFMT[ 4 ] = 'F';
	pszFMT[ 5 ] = 'M';
	pszFMT[ 6 ] = 'T';
	pszFMT[ 7 ] = 0;
	}


void write_MEM_INFO_ERR( _Out_writes_z_( 13 ) _Pre_writable_size_( 13 ) PWSTR psz_formatted_usage ) {
	psz_formatted_usage[ 0  ] = 'M';
	psz_formatted_usage[ 1  ] = 'E';
	psz_formatted_usage[ 2  ] = 'M';
	psz_formatted_usage[ 3  ] = '_';
	psz_formatted_usage[ 4  ] = 'I';
	psz_formatted_usage[ 5  ] = 'N';
	psz_formatted_usage[ 6  ] = 'F';
	psz_formatted_usage[ 7  ] = 'O';
	psz_formatted_usage[ 8  ] = '_';
	psz_formatted_usage[ 9  ] = 'E';
	psz_formatted_usage[ 10 ] = 'R';
	psz_formatted_usage[ 11 ] = 'R';
	psz_formatted_usage[ 12 ] =  0;
	}


void zeroFILEINFO( _Pre_invalid_ _Post_valid_ FILEINFO& fi ) {
	fi.attributes = 0;
	fi.lastWriteTime.dwHighDateTime = 0;
	fi.lastWriteTime.dwLowDateTime  = 0;
	fi.length = 0;
	//fi.name = _T( "" );
	//fi.name.Truncate( 0 );
	}

void zeroDIRINFO( _Pre_invalid_ _Post_valid_ DIRINFO& di ) {
	di.attributes = 0;
	di.lastWriteTime.dwHighDateTime = 0;
	di.lastWriteTime.dwLowDateTime  = 0;
	di.length = 0;
	di.name = _T( "" );
	}


_Ret_maybenull_ CItemBranch* const FindCommonAncestor( _In_ _Pre_satisfies_( item1->m_type != IT_FILE ) const CItemBranch* const item1, _In_ const CItemBranch& item2 ) {
	auto parent = item1;
	while ( ( parent != NULL ) && ( !parent->IsAncestorOf( item2 ) ) ) {
		if ( parent != NULL ) {
			parent = parent->GetParent( );
			}
		else {
			break;
			}
		}
	ASSERT( parent != NULL );
	return const_cast<CItemBranch*>( parent );
	}

INT __cdecl CItem_compareBySize( _In_ _Points_to_data_ const void* const p1, _In_ _Points_to_data_ const void* const p2 ) {
	const auto size1 = ( *( reinterpret_cast< const CItemBranch * const* const >( p1 ) ) )->size_recurse( );
	const auto size2 = ( *( reinterpret_cast< const CItemBranch * const* const >( p2 ) ) )->size_recurse( );
	return signum( std::int64_t( size2 ) - std::int64_t( size1 ) ); // biggest first// TODO: Use 2nd sort column (as set in our TreeListView?)
	}

void CheckMinMax( _Inout_ LONG& val, _In_ const INT min_val, _In_ const INT max_val ) {
	ASSERT( min_val <= max_val );

	if ( val < LONG( min_val ) ) {
		val = LONG( min_val );
		}
	if ( val > LONG( max_val ) ) {
		val = LONG( max_val );
		}
	ASSERT( val <= LONG( max_val ) );
	ASSERT( LONG( min_val ) <= val );
	}

void CheckMinMax( _Inout_ INT& val, _In_ const INT min_val, _In_ const INT max_val ) {
	ASSERT( min_val <= max_val );

	if ( val < min_val ) {
		val = min_val;
		}
	if ( val > max_val ) {
		val = max_val;
		}
	ASSERT( val <= max_val );
	ASSERT( min_val <= val );
	}

bool Compare_FILETIME_cast( const FILETIME& t1, const FILETIME& t2 ) {
    const auto u1 = reinterpret_cast<const ULARGE_INTEGER&>( t1 );
    const auto u2 = reinterpret_cast<const ULARGE_INTEGER&>( t2 );
    return ( u1.QuadPart < u2.QuadPart );
	}

INT Compare_FILETIME( const FILETIME& lhs, const FILETIME& rhs ) {
	if ( Compare_FILETIME_cast( lhs, rhs ) ) {
		return -1;
		}
	else if ( ( lhs.dwLowDateTime == rhs.dwLowDateTime ) && ( lhs.dwHighDateTime == rhs.dwHighDateTime ) ) {
		return 0;
		}
	return 1;
	}

bool Compare_FILETIME_eq( const FILETIME& t1, const FILETIME& t2 ) {
	const auto u1 = reinterpret_cast< const ULARGE_INTEGER& >( t1 );
	const auto u2 = reinterpret_cast< const ULARGE_INTEGER& >( t2 );
	return ( u1.QuadPart == u2.QuadPart );
	}


_Success_( return != UINT64_MAX ) std::uint64_t GetCompressedFileSize_filename( const std::wstring path ) {
	ULARGE_INTEGER ret;
	ret.QuadPart = 0;//it's a union, but I'm being careful.
	ret.LowPart = GetCompressedFileSizeW( path.c_str( ), &ret.HighPart );
	if ( ret.LowPart == INVALID_FILE_SIZE ) {
		if ( ret.HighPart != NULL ) {
			if ( GetLastError( ) != NO_ERROR ) {
				return ret.QuadPart;// IN case of an error return size from CFileFind object
				}
			return UINT64_MAX;
			}
		else if ( GetLastError( ) != NO_ERROR ) {
			if ( GetLastError( ) == ERROR_FILE_NOT_FOUND ) {
#ifdef _DEBUG
				TRACE( _T( "Error! Filepath: %s, Filepath length: %i, GetLastError: %s\r\n" ), path.c_str( ), path.length( ), GetLastErrorAsFormattedMessage( ) );
#pragma message("Investigate this!")
				//ASSERT( path.length( ) >= MAX_PATH );
#endif
				return UINT64_MAX;
				}
			}
		}
	return ret.QuadPart;
	}

// $Log$
// Revision 1.20  2004/11/28 14:40:06  assarbad
// - Extended CFileFindWDS to replace a global function
// - Now packing/unpacking the file attributes. This even spares a call to find encrypted/compressed files.
//
// Revision 1.19  2004/11/25 21:13:38  assarbad
// - Implemented "attributes" column in the treelist
// - Adopted width in German dialog
// - Provided German, Russian and English version of IDS_TREECOL_ATTRIBUTES
//
// Revision 1.18  2004/11/25 11:58:52  assarbad
// - Minor fixes (odd behavior of coloring in ANSI version, caching of the GetCompressedFileSize API)
//   for details see the changelog.txt
//
// Revision 1.17  2004/11/14 21:50:44  assarbad
// - Pre-select the last used folder
//
// Revision 1.16  2004/11/14 08:49:06  bseifert
// Date/Time/Number formatting now uses User-Locale. New option to force old behavior.
//
// Revision 1.15  2004/11/12 22:14:16  bseifert
// Eliminated CLR_NONE. Minor corrections.
//
// Revision 1.14  2004/11/12 13:19:44  assarbad
// - Minor changes and additions (in preparation for the solution of the "Browse for Folder" problem)
//
// Revision 1.13  2004/11/12 00:47:42  assarbad
// - Fixed the code for coloring of compressed/encrypted items. Now the coloring spans the full row!
//
// Revision 1.12  2004/11/07 21:10:25  assarbad
// - Corrected IF statement
// - Changed solution file to build consistent "Unicode Debug" configuration for "Unicode Debug" of main project
//
// Revision 1.11  2004/11/07 20:14:30  assarbad
// - Added wrapper for GetCompressedFileSize() so that by default the compressed file size will be shown.
//
// Revision 1.10  2004/11/05 16:53:07  assarbad
// Added Date and History tag where appropriate.
//
