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
#include "globalhelpers.h"

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
				const auto pf_res = _snwprintf_s( tempBuf, 9, L",%03d", rest );
				ASSERT( pf_res != -1 );
				UNREFERENCED_PARAMETER( pf_res );
				}
			else {
#ifdef DEBUG
				s.Format( _T( "%d" ), rest );
#endif
				const auto pf_res = _snwprintf_s( tempBuf, 9, L"%d", rest );
				ASSERT( pf_res != -1 );
				UNREFERENCED_PARAMETER( pf_res );
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
				const auto pf_res = _snwprintf_s( tempBuf, 9, L",%03d", rest );
				ASSERT( pf_res != -1 );
				UNREFERENCED_PARAMETER( pf_res );
#ifdef DEBUG
				s.Format( _T( ",%03d" ) , rest );
#endif
				}
			else {
#ifdef DEBUG
				s.Format( _T( "%d" ), rest );
				
#endif	
				const auto pf_res = _snwprintf_s( tempBuf, 9, L"%d", rest );
				ASSERT( pf_res != -1 );
				UNREFERENCED_PARAMETER( pf_res );
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

//, _Out_ rsize_t& chars_written

_Success_( SUCCEEDED( return ) ) HRESULT FormatBytes( _In_ const std::uint64_t n, _Out_writes_z_( strSize ) _Pre_writable_size_( strSize ) PWSTR psz_formatted_bytes, _In_range_( 38, 64 ) const rsize_t strSize ) {
	rsize_t chars_written = 0;
	auto res = CStyle_FormatLongLongHuman( n, psz_formatted_bytes, strSize, chars_written );
	if ( !SUCCEEDED( res ) ) {
		write_BAD_FMT( psz_formatted_bytes, chars_written );
		return res;
		}
	return res;
	}

_Success_( SUCCEEDED( return ) ) HRESULT FormatBytes( _In_ const std::uint64_t n, _Out_writes_z_( strSize ) _Pre_writable_size_( strSize ) _Post_readable_size_( chars_written ) PWSTR psz_formatted_bytes, _In_range_( 38, 64 ) const rsize_t strSize, _Out_ rsize_t& chars_written ) {
	auto res = CStyle_FormatLongLongHuman( n, psz_formatted_bytes, strSize, chars_written );
	if ( !SUCCEEDED( res ) ) {
		write_BAD_FMT( psz_formatted_bytes, chars_written );
		return res;
		}
	return res;
	}


std::wstring FormatBytes( _In_ const std::uint64_t n, bool humanFormat ) {
	if ( humanFormat ) {
		//MAX value of a std::uint64_t is 20 digits
		const rsize_t strSize = 21;
		wchar_t psz_formatted_longlong[ strSize ] = { 0 };
		rsize_t chars_written = 0;
		auto res = CStyle_FormatLongLongHuman( n, psz_formatted_longlong, strSize, chars_written );
		if ( !SUCCEEDED( res ) ) {
			write_BAD_FMT( psz_formatted_longlong, chars_written );
			}
		return psz_formatted_longlong;
		}
	auto string = Format_uint64_t_Normal( n );
	return string;
	}


//, _Out_ rsize_t& chars_written


_Success_( SUCCEEDED( return ) ) HRESULT CStyle_FormatLongLongHuman_0( _Out_writes_z_( strSize ) _Pre_writable_size_( strSize ) _Post_readable_size_( chars_written ) PWSTR psz_formatted_LONGLONG_HUMAN, _In_range_( 8, 64 ) const rsize_t strSize, _Out_ rsize_t& chars_written ) {
	//psz_formatted_LONGLONG_HUMAN, strSize, chars_written
	size_t remaining_chars = 0;
	const HRESULT res = StringCchPrintfExW( psz_formatted_LONGLONG_HUMAN, strSize, NULL, &remaining_chars, 0, L"0" );
	if ( SUCCEEDED( res ) ) {
		chars_written = ( strSize - remaining_chars );
		return res;
		}
	write_BAD_FMT( psz_formatted_LONGLONG_HUMAN, chars_written );
	return res;
	}

_Success_( SUCCEEDED( return ) ) HRESULT CStyle_FormatLongLongHuman_B( _Out_writes_z_( strSize ) _Pre_writable_size_( strSize ) _Post_readable_size_( chars_written ) PWSTR psz_formatted_LONGLONG_HUMAN, _In_range_( 8, 64 ) const rsize_t strSize, _Out_ rsize_t& chars_written, _In_ const DOUBLE B ) {
	size_t remaining_chars = 0;
	const HRESULT res = StringCchPrintfExW( psz_formatted_LONGLONG_HUMAN, strSize, NULL, &remaining_chars, 0, L"%i Bytes", static_cast<INT>( B ) );
	if ( SUCCEEDED( res ) ) {
		ASSERT( strSize >= remaining_chars );
		chars_written = ( strSize - remaining_chars );
		return res;
		}
	//chars_written = strSize;
	write_BAD_FMT( psz_formatted_LONGLONG_HUMAN, chars_written );
	return res;
	}


_Success_( SUCCEEDED( return ) ) HRESULT CStyle_FormatLongLongHuman_KB( _Out_writes_z_( strSize ) _Pre_writable_size_( strSize ) _Post_readable_size_( chars_written ) PWSTR psz_formatted_LONGLONG_HUMAN, _In_range_( 8, 64 ) const rsize_t strSize, _Out_ rsize_t& chars_written, _In_ const DOUBLE B, _In_ const DOUBLE KB ) {
	const rsize_t bufSize = 19;
	wchar_t buffer[ bufSize ] = { 0 };
	const HRESULT res = CStyle_FormatDouble( KB + B / BASE, buffer, bufSize );
	if ( SUCCEEDED( res ) ) {
		const auto resSWPRINTF = swprintf_s( psz_formatted_LONGLONG_HUMAN, strSize, L"%s KB", buffer );
		if ( resSWPRINTF != -1 ) {
			ASSERT( resSWPRINTF >= 0 );
			chars_written = static_cast<rsize_t>( resSWPRINTF );
			return S_OK;
			}
		write_BAD_FMT( psz_formatted_LONGLONG_HUMAN, chars_written );
		return STRSAFE_E_INVALID_PARAMETER;
		}
	write_BAD_FMT( psz_formatted_LONGLONG_HUMAN, chars_written );
	return res;
	}

_Success_( SUCCEEDED( return ) ) HRESULT CStyle_FormatLongLongHuman_MB( _Out_writes_z_( strSize ) _Pre_writable_size_( strSize ) _Post_readable_size_( chars_written ) PWSTR psz_formatted_LONGLONG_HUMAN, _In_range_( 8, 64 ) const rsize_t strSize, _Out_ rsize_t& chars_written, _In_ const DOUBLE KB, _In_ const DOUBLE MB ) {
	const rsize_t bufSize = 19;
	wchar_t buffer[ bufSize ] = { 0 };
	const HRESULT res = CStyle_FormatDouble( MB + KB / BASE, buffer, bufSize );
	if ( SUCCEEDED( res ) ) {
		const auto resSWPRINTF = swprintf_s( psz_formatted_LONGLONG_HUMAN, strSize, L"%s MB", buffer );
		if ( resSWPRINTF != -1 ) {
			ASSERT( resSWPRINTF >= 0 );
			chars_written = static_cast<rsize_t>( resSWPRINTF );
			return S_OK;
			}
		write_BAD_FMT( psz_formatted_LONGLONG_HUMAN, chars_written );
		return STRSAFE_E_INVALID_PARAMETER;
		}
	write_BAD_FMT( psz_formatted_LONGLONG_HUMAN, chars_written );
	return res;
	}

_Success_( SUCCEEDED( return ) ) HRESULT CStyle_FormatLongLongHuman_GB( _Out_writes_z_( strSize ) _Pre_writable_size_( strSize ) _Post_readable_size_( chars_written ) PWSTR psz_formatted_LONGLONG_HUMAN, _In_range_( 8, 64 ) const rsize_t strSize, _Out_ rsize_t& chars_written, _In_ const DOUBLE MB, _In_ const DOUBLE GB ) {
	const rsize_t bufSize = 19;
	wchar_t buffer[ bufSize ] = { 0 };
	const HRESULT res = CStyle_FormatDouble( GB + MB / BASE, buffer, bufSize );
	if ( SUCCEEDED( res ) ) {
		const auto resSWPRINTF = swprintf_s( psz_formatted_LONGLONG_HUMAN, strSize, L"%s GB", buffer );
		if ( resSWPRINTF != -1 ) {
			ASSERT( resSWPRINTF >= 0 );
			chars_written = static_cast<rsize_t>( resSWPRINTF );
			return S_OK;
			}
		write_BAD_FMT( psz_formatted_LONGLONG_HUMAN, chars_written );
		return STRSAFE_E_INVALID_PARAMETER;
		}
	write_BAD_FMT( psz_formatted_LONGLONG_HUMAN, chars_written );
	return res;
	}

_Success_( SUCCEEDED( return ) ) HRESULT CStyle_FormatLongLongHuman_TB( _Out_writes_z_( strSize ) _Pre_writable_size_( strSize ) _Post_readable_size_( chars_written ) PWSTR psz_formatted_LONGLONG_HUMAN, _In_range_( 8, 64 ) const rsize_t strSize, _Out_ rsize_t& chars_written, _In_ const DOUBLE MB, _In_ const DOUBLE GB, _In_ const DOUBLE TB ) {
	const rsize_t bufSize = 19;
	wchar_t buffer[ bufSize ] = { 0 };
	const HRESULT res = CStyle_FormatDouble( TB + GB / BASE, buffer, bufSize );
	if ( SUCCEEDED( res ) ) {
		const auto resSWPRINTF = swprintf_s( psz_formatted_LONGLONG_HUMAN, strSize, L"%s TB", buffer );
		if ( resSWPRINTF != -1 ) {
			ASSERT( resSWPRINTF >= 0 );
			chars_written = static_cast<rsize_t>( resSWPRINTF );
			return S_OK;
			}
		write_BAD_FMT( psz_formatted_LONGLONG_HUMAN, chars_written );
		return STRSAFE_E_INVALID_PARAMETER;
		}
	write_BAD_FMT( psz_formatted_LONGLONG_HUMAN, chars_written );
	return res;
	}

_Success_( SUCCEEDED( return ) ) HRESULT CStyle_FormatLongLongHuman( _In_ std::uint64_t n, _Out_writes_z_( strSize ) _Pre_writable_size_( strSize ) _Post_readable_size_( chars_written ) PWSTR psz_formatted_LONGLONG_HUMAN, _In_range_( 8, 64 ) const rsize_t strSize, _Out_ rsize_t& chars_written ) {
	//MAX value of a LONGLONG is 19 digits
	const DOUBLE B  = static_cast<INT>( n % BASE );
	n /= BASE;
	const DOUBLE KB = static_cast<INT>( n % BASE );
	n /= BASE;
	const DOUBLE MB = static_cast<INT>( n % BASE );
	n /= BASE;
	const DOUBLE GB = static_cast<INT>( n % BASE );
	n /= BASE;
	const DOUBLE TB = static_cast<INT>( n );

	if ( TB != 0 || GB == BASE - 1 && MB >= HALF_BASE ) {
		return CStyle_FormatLongLongHuman_TB( psz_formatted_LONGLONG_HUMAN, strSize, chars_written, MB, GB, TB );
		}
	else if ( GB != 0 || MB == BASE - 1 && KB >= HALF_BASE ) {
		return CStyle_FormatLongLongHuman_GB( psz_formatted_LONGLONG_HUMAN, strSize, chars_written, MB, GB );
		}
	else if ( MB != 0 || KB == BASE - 1 && B >= HALF_BASE ) {
		return CStyle_FormatLongLongHuman_MB( psz_formatted_LONGLONG_HUMAN, strSize, chars_written, KB, MB );
		}
	else if ( KB != 0 ) {
		return CStyle_FormatLongLongHuman_KB( psz_formatted_LONGLONG_HUMAN, strSize, chars_written, B, KB );
		}
	else if ( B != 0 ) {
		return CStyle_FormatLongLongHuman_B( psz_formatted_LONGLONG_HUMAN, strSize, chars_written, B );
		}
	return CStyle_FormatLongLongHuman_0( psz_formatted_LONGLONG_HUMAN, strSize, chars_written );
	}

std::wstring FormatCount( _In_ const std::uint32_t n ) {
	return FormatLongLongNormal( static_cast<LONGLONG>( n ) );
	}

//CString FormatCount( _In_ const std::uint64_t n ) {
//	return Format_uint64_t_Normal( n ).c_str( );
//	}

//CString FormatDouble( _In_ DOUBLE d ) {// "98,4" or "98.4"
//	CString s;
//	s.Format( _T( "%.1f" ), d );
//	return s;
//	}

std::wstring FormatDouble_w( _In_ DOUBLE d ) {// "98,4" or "98.4"
	wchar_t fmt[ 64 ] = { 0 };
	auto resSWPRINTF = swprintf_s( fmt, 64, L"%.1f", d );
	if ( resSWPRINTF != -1 ) {
		return fmt;
		}
	return L"BAD swprintf_s!!!!";
	}


_Success_( SUCCEEDED( return ) ) HRESULT CStyle_FormatDouble( _In_ const DOUBLE d, _Out_writes_z_( strSize ) _Pre_writable_size_( strSize ) PWSTR psz_formatted_double, _In_range_( 3, 64 ) const rsize_t strSize ) {
	auto resSWPRINTF = swprintf_s( psz_formatted_double, strSize, L"%.1f", d );
	if ( resSWPRINTF != -1 ) {
		return S_OK;
		}
	return STRSAFE_E_INVALID_PARAMETER;

	//Range 3-64 is semi-arbitrary. I don't think I'll need to format a double that's more than 63 chars.
	//return StringCchPrintfW( psz_formatted_double, strSize, L"%.1f%", d );
	}

_Success_( SUCCEEDED( return ) ) HRESULT CStyle_FormatDouble( _In_ const DOUBLE d, _Out_writes_z_( strSize ) _Pre_writable_size_( strSize ) _Post_readable_size_( chars_written ) PWSTR psz_formatted_double, _In_range_( 3, 64 ) const rsize_t strSize, _Out_ rsize_t& chars_written ) {
	auto resSWPRINTF = swprintf_s( psz_formatted_double, strSize, L"%.1f", d );
	if ( resSWPRINTF != -1 ) {
		ASSERT( resSWPRINTF >= 0 );
		chars_written = static_cast<rsize_t>( resSWPRINTF );
		return S_OK;
		}

	return STRSAFE_E_INVALID_PARAMETER;

	//Range 3-64 is semi-arbitrary. I don't think I'll need to format a double that's more than 63 chars.
	//return StringCchPrintfW( psz_formatted_double, strSize, L"%.1f%", d );
	}


//CString FormatFileTime( _In_ const FILETIME& t ) {
//	ASSERT( &t != NULL );
//	SYSTEMTIME st;
//	if ( !FileTimeToSystemTime( &t, &st ) ) {
//		return GetLastErrorAsFormattedMessage( );
//		}
//#ifdef DEBUG
//	LCID lcid = MAKELCID( GetUserDefaultLangID( ), SORT_DEFAULT );
//	CString date;
//	VERIFY( 0 < GetDateFormatW( lcid, DATE_SHORTDATE, &st, NULL, date.GetBuffer( 256 ), 256 ) );//d M yyyy
//	date.ReleaseBuffer( );
//#endif
//
//	wchar_t psz_formatted_datetime[ 73 ] = { 0 };
//	auto res = CStyle_FormatFileTime( t, psz_formatted_datetime, 73 );
//	if ( ! ( res == 0 ) ) {
//		rsize_t chars_written = 0;
//		write_BAD_FMT( psz_formatted_datetime, chars_written );
//		return psz_formatted_datetime;
//		}
//
//	ASSERT( SUCCEEDED( res ) );
//
//#ifdef _DEBUG
//	CString time;
//	VERIFY( 0 < GetTimeFormatW( lcid, 0, &st, NULL, time.GetBuffer( 256 ), 256 ) );//h mm ss tt
//	time.ReleaseBuffer( );
//	CString result = date + _T( "  " ) + time;
//#endif
//
//
//#ifdef _DEBUG
//	auto didMatch = result.Compare( psz_formatted_datetime );
//	ASSERT( didMatch == 0 );
//	TRACE( _T( "Formatted file time (%i characters): %s\r\n" ), result.GetLength( ), result );
//	TRACE( _T( "Formatted file time  C-STYLE       : %s\r\n" ), psz_formatted_datetime );
//#endif
//	if ( res == 0 ) {
//		return psz_formatted_datetime;
//		}
//	else {
//		rsize_t chars_written = 0;
//		write_BAD_FMT( psz_formatted_datetime, chars_written );
//		}
//	return psz_formatted_datetime;
//	}
//
_Success_( SUCCEEDED( return ) ) HRESULT CStyle_FormatFileTime( _In_ const FILETIME t, _Out_writes_z_( strSize ) _Pre_writable_size_( strSize ) PWSTR psz_formatted_datetime, _In_range_( 128, 2048 ) const rsize_t strSize, _Out_ rsize_t& chars_written ) {
	ASSERT( &t != NULL );
	SYSTEMTIME st;
	if ( !FileTimeToSystemTime( &t, &st ) ) {
		const HRESULT err_res = CStyle_GetLastErrorAsFormattedMessage( psz_formatted_datetime, strSize, chars_written );
		if ( !SUCCEEDED( err_res ) ) {
			TRACE( _T( "Error in CStyle_GetLastErrorAsFormattedMessage!!\r\n" ) );
			displayWindowsMsgBoxWithMessage( std::wstring( L"Error in CStyle_GetLastErrorAsFormattedMessage!!\r\n" ) );
			return err_res;
			}
		return E_FAIL;
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
			TRACE( _T( "%s\r\n" ), global_strings::get_date_format_buffer_err );
			displayWindowsMsgBoxWithMessage( global_strings::get_date_format_buffer_err );
			std::terminate( );
			}
		if ( err == ERROR_INVALID_FLAGS ) {
			TRACE( _T( "%s\r\n" ), global_strings::get_date_format_flags_err );
			displayWindowsMsgBoxWithMessage( global_strings::get_date_format_flags_err );
			std::terminate( );
			}
		if ( err == ERROR_INVALID_PARAMETER ) {
			TRACE( _T( "%s\r\n" ), global_strings::get_date_format_param_err );
			displayWindowsMsgBoxWithMessage( global_strings::get_date_format_param_err );
			std::terminate( );
			}
		}
	if ( gtfres == 0 ) {
		const auto err = GetLastError( );
		if ( err == ERROR_INSUFFICIENT_BUFFER ) {
			TRACE( _T( "%s\r\n" ), global_strings::get_time_format_buffer_err );
			displayWindowsMsgBoxWithMessage( global_strings::get_time_format_buffer_err );
			std::terminate( );
			}
		if ( err == ERROR_INVALID_FLAGS ) {
			TRACE( _T( "%s\r\n" ), global_strings::get_time_format_flags_err );
			displayWindowsMsgBoxWithMessage( global_strings::get_time_format_flags_err );
			std::terminate( );
			}
		if ( err == ERROR_INVALID_PARAMETER ) {
			TRACE( _T( "%s\r\n" ), global_strings::get_time_format_param_err );
			displayWindowsMsgBoxWithMessage( global_strings::get_time_format_param_err );
			std::terminate( );
			}
		}

	rsize_t remaining_chars = 0;
	const HRESULT fmt_res = StringCchPrintfEx( psz_formatted_datetime, strSize, NULL, &remaining_chars, 0, L"%s  %s", psz_date_wchar, psz_time_wchar );
	if ( SUCCEEDED( fmt_res ) ) {
		chars_written = ( strSize - remaining_chars );
		}
	else {
		chars_written = 0;
		}

	//auto cpyres  = wcscpy_s( psz_formatted_datetime, static_cast<rsize_t>( gdfres ), psz_date_wchar );
	//auto cpyres  = wcscpy_s( psz_formatted_datetime, strSize, psz_date_wchar );
	//auto wcsres  = wcscat_s( psz_formatted_datetime, strSize, L"  " );
	//auto wcsres2 = wcscat_s( psz_formatted_datetime, strSize, psz_time_wchar );

	//auto lError = GetLastError( );

	return fmt_res;
	}

//CString FormatAttributes( _In_ const DWORD attr ) {
//	if ( attr == INVALID_FILE_ATTRIBUTES ) {
//		return _T( "?????" );
//		}
//
//	CString attributes;
//	attributes.Append( ( attr bitand FILE_ATTRIBUTE_READONLY )   ? _T( "R" ) : _T( "" ) );
//	attributes.Append( ( attr bitand FILE_ATTRIBUTE_HIDDEN )     ? _T( "H" ) : _T( "" ) );
//	attributes.Append( ( attr bitand FILE_ATTRIBUTE_SYSTEM )     ? _T( "S" ) : _T( "" ) );
//	attributes.Append( ( attr bitand FILE_ATTRIBUTE_ARCHIVE )    ? _T( "A" ) : _T( "" ) );
//	attributes.Append( ( attr bitand FILE_ATTRIBUTE_COMPRESSED ) ? _T( "C" ) : _T( "" ) );
//	attributes.Append( ( attr bitand FILE_ATTRIBUTE_ENCRYPTED )  ? _T( "E" ) : _T( "" ) );
//
//	return attributes;
//	}

_Success_( return == 0 ) int CStyle_FormatAttributes( _In_ const DWORD attr, _Out_writes_z_( strSize ) _Pre_writable_size_( strSize ) PWSTR psz_formatted_attributes, _In_range_( 1, 6 ) rsize_t strSize ) {
	if ( attr == INVALID_FILE_ATTRIBUTES ) {
		psz_formatted_attributes = _T( "?????" );
		}
	int errCode[ 6 ] = { 0 };
	rsize_t charsWritten = 0;
	//CString attributes;
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

_Success_( SUCCEEDED( return ) ) HRESULT CStyle_FormatAttributes( _In_ const attribs& attr, _Out_writes_z_( strSize ) _Pre_writable_size_( strSize ) PWSTR psz_formatted_attributes, _In_range_( 6, 18 ) const rsize_t strSize, _Out_ rsize_t& chars_written  ) {
	if ( attr.invalid ) {
		psz_formatted_attributes[ 0 ] = L'?';
		psz_formatted_attributes[ 1 ] = L'?';
		psz_formatted_attributes[ 2 ] = L'?';
		psz_formatted_attributes[ 3 ] = L'?';
		psz_formatted_attributes[ 4 ] = L'?';
		psz_formatted_attributes[ 5 ] =   0;
		chars_written = 5;
		return S_OK;
		}
	//int errCode[ 6 ] = { 0 };
	//rsize_t charsWritten = 0;
	//charsWritten += ( ( errCode[ 0 ] == 0 ) ? 1 : 0 );
	const auto alt_errCode = swprintf_s( psz_formatted_attributes, strSize, L"%s%s%s%s%s", ( ( attr.readonly ) ? L"R" : L"" ),  ( ( attr.hidden ) ? L"H" : L"" ),  ( ( attr.system ) ? L"S" : L"" ),  ( ( attr.compressed ) ? L"C" : L"" ), ( ( attr.encrypted ) ? L"E" : L"" ) );
	if ( alt_errCode == -1 ) {
		return STRSAFE_E_INVALID_PARAMETER;
		}
	ASSERT( alt_errCode >= 0 );
	chars_written = alt_errCode;
	return S_OK;
	////CString attributes;
	//if ( attr.readonly ) {
	//	errCode[ 0 ] = wcscpy_s( psz_formatted_attributes + charsWritten, strSize - 1 - charsWritten, L"R" );
	//	charsWritten += ( ( errCode[ 0 ] == 0 ) ? 1 : 0 );
	//	}
	//if ( attr.hidden ) {
	//	errCode[ 1 ] = wcscpy_s( psz_formatted_attributes + charsWritten, strSize - 1 - charsWritten, L"H" );
	//	charsWritten += ( ( errCode[ 1 ] == 0 ) ? 1 : 0 );
	//	}
	//if ( attr.system ) {
	//	errCode[ 2 ] = wcscpy_s( psz_formatted_attributes + charsWritten, strSize - 1 - charsWritten, L"S" );
	//	charsWritten += ( ( errCode[ 2 ] == 0 ) ? 1 : 0 );
	//	}
	//if ( attr.compressed ) {
	//	errCode[ 4 ] = wcscpy_s( psz_formatted_attributes + charsWritten, strSize - 1 - charsWritten, L"C" );
	//	charsWritten += ( ( errCode[ 4 ] == 0 ) ? 1 : 0 );
	//	}
	//if ( attr.encrypted ) {
	//	errCode[ 5 ] = wcscpy_s( psz_formatted_attributes + charsWritten, strSize - 1 - charsWritten, L"E" );
	//	charsWritten += ( ( errCode[ 5 ] == 0 ) ? 1 : 0 );
	//	}
	//chars_written = charsWritten;
	//ASSERT( charsWritten < strSize );
	//ASSERT( strSize > 0 );
	//psz_formatted_attributes[ strSize - 1 ] = 0;
	//return std::accumulate( errCode, errCode + 6, 0 );
	}

//
_Success_( SUCCEEDED( return ) ) HRESULT CStyle_GetNumberFormatted( const int number, _Pre_writable_size_( strSize ) PWSTR psz_formatted_number, _In_range_( 21, 64 ) const rsize_t strSize, _Out_ rsize_t& chars_written ) {
	// Returns formatted number like "123.456.789".
	// 18446744073709551615 is max ( for std::uint64_t )
	//                     ^ 20 characters
	// 18,446,744,073,709,551,615
	//                           ^26 characters
	//                            26 + null terminator = 27
	// 9223372036854775807 is max ( for 64 bit int, INT64_MAX )
	//                    ^ 19 characters
	//-9223372036854775807 is min ( for 64 bit int, INT64_MIN ) //<-- Ok, (-9223372036854775807i64 - 1) is. But I'll ignore for now.
	//                    ^ 20 characters
	//-9,223,372,036,854,775,807
	//                          ^26 characters
	//                           26 + null terminator = 27
	
	//Our plan:
	//	Convert number to string, unformatted
	//	Pass THAT string to GetNumberFormatEx
	
	const int bufSize = 64;
	static_assert( bufSize > 0, "dude, ya need to use a positive buffer size!" );

	wchar_t number_str_buffer[ bufSize ] = { 0 };
	const auto sw_printf_s_res = swprintf_s( number_str_buffer, L"%i", number );

	NUMBERFMT format_struct;
	format_struct.NumDigits = 0;
	format_struct.LeadingZero = 0;
	format_struct.Grouping = 3;
	format_struct.lpDecimalSep = L".";
	format_struct.lpThousandSep = L",";
	format_struct.NegativeOrder = 1;


	//0 indicates failure! http://msdn.microsoft.com/en-us/library/windows/desktop/dd318113.aspx
	const auto get_number_fmt_ex_res = GetNumberFormatEx( NULL, 0, number_str_buffer, &format_struct, psz_formatted_number, strSize );
	if ( get_number_fmt_ex_res == 0 ) {
		const auto last_err = GetLastError( );
		ASSERT( ( last_err == ERROR_INSUFFICIENT_BUFFER ) || ( last_err == ERROR_INVALID_FLAGS ) || ( last_err == ERROR_INVALID_PARAMETER ) || ( last_err == ERROR_OUTOFMEMORY ) );
		switch ( last_err ) 
			{
			case ERROR_INSUFFICIENT_BUFFER:
				return STRSAFE_E_INSUFFICIENT_BUFFER;
			case ERROR_INVALID_FLAGS:
			case ERROR_INVALID_PARAMETER:
				return STRSAFE_E_INVALID_PARAMETER;
			case ERROR_OUTOFMEMORY:
				return STRSAFE_E_END_OF_FILE;
			default:
				ASSERT( false );
				std::terminate( );
			}
		ASSERT( false );
		std::terminate( );
		}
	chars_written = get_number_fmt_ex_res;
	return S_OK;
	}


_Success_( return != false ) bool GetVolumeName( _In_z_ const PCWSTR rootPath, _Out_ CString& volumeName ) {
	//CString ret;
	//DWORD dummy;

	const auto old = SetErrorMode( SEM_FAILCRITICALERRORS );
	
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

_Success_( return != false ) bool GetVolumeName( _In_z_ const PCWSTR rootPath, _Out_ std::wstring& volumeName ) {
	//CString ret;
	//DWORD dummy;

	const auto old = SetErrorMode( SEM_FAILCRITICALERRORS );
	
	//GetVolumeInformation returns 0 on failure
	const DWORD bufferSize = ( MAX_PATH + MAX_PATH );

	wchar_t buffer[ bufferSize ] = { 0 };
	
	const BOOL b = GetVolumeInformationW( rootPath, buffer, bufferSize, NULL, NULL, NULL, NULL, 0 );

	if ( b == 0 ) {
		TRACE( _T( "GetVolumeInformation(%s) failed: %u\n" ), rootPath, GetLastError( ) );
		}
	SetErrorMode( old );
	
	volumeName = buffer;

	return ( b != 0 );
	}


_Success_( return != false ) 
bool GetVolumeName( _In_z_ const PCWSTR rootPath ) {
	//CString ret;
	//DWORD dummy;

	const auto old = SetErrorMode( SEM_FAILCRITICALERRORS );
	
	//GetVolumeInformation returns 0 on failure
	const BOOL b = GetVolumeInformationW( rootPath, NULL, 0, NULL, NULL, NULL, NULL, 0 );

	if ( b == 0 ) {
		TRACE( _T( "GetVolumeInformation(%s) failed: %u\n" ), rootPath, GetLastError( ) );
		}
	SetErrorMode( old );
	
	return ( b != 0 );
	}


//CString FormatVolumeName( _In_ const CString& rootPath, _In_ const CString& volumeName ) {
//	ASSERT( rootPath != _T( "" ) );
//	CString ret;
//	ret.Format( _T( "%s (%s)" ), volumeName.GetString( ), rootPath.Left( 2 ).GetString( ) );
//	return ret;
//	}

std::wstring FormatVolumeName( _In_ const std::wstring& rootPath, _In_ const std::wstring& volumeName ) {
	std::wstring ret;
	ret.reserve( volumeName.length( ) + 2 + 2 + 1 );
	ret += volumeName;
	ret += L" (";
	ret += rootPath.substr( 0, 2 );
	ret += L")";
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


//void MyShellExecute( _In_opt_ HWND hwnd, _In_opt_z_ PCWSTR pOperation, _In_z_ PCWSTR pFile, _In_opt_z_ PCWSTR pParameters, _In_opt_z_ PCWSTR pDirectory, _In_ const INT nShowCmd ) {
//	CWaitCursor wc;
//	auto h = reinterpret_cast<INT_PTR>( ShellExecuteW( hwnd, pOperation, pFile, pParameters, pDirectory, nShowCmd ) );
//	if ( h <= 32 ) {
//		CString a;
//		a += ( _T( "ShellExecute failed: (error #: " ) + h );
//		a += +_T( " ), message: " ) + GetLastErrorAsFormattedMessage( );
//
//		AfxMessageBox( a );
//		displayWindowsMsgBoxWithError( );
//		return;
//
//		}
//	}


_Success_( return > 32 ) INT_PTR ShellExecuteWithAssocDialog( _In_ const HWND hwnd, _In_ std::wstring filename ) {
	CWaitCursor wc;
	auto u = reinterpret_cast< INT_PTR >( ShellExecuteW( hwnd, NULL, filename.c_str( ), NULL, NULL, SW_SHOWNORMAL ) );
	if ( u == SE_ERR_NOASSOC ) {
		// Q192352
		const rsize_t dir_buf_size = MAX_PATH;
		wchar_t dir_buf[ MAX_PATH ] = { 0 };
		std::wstring parameters_filename( L"shell32.dll,OpenAs_RunDLL " + std::move( filename ) );

		//-- Get the system directory so that we know where Rundll32.exe resides.
		const auto sys_dir_res = GetSystemDirectoryW( dir_buf, dir_buf_size );
		if ( ( sys_dir_res != 0 ) && ( sys_dir_res < dir_buf_size ) ) {
			u = reinterpret_cast< INT_PTR >( ShellExecuteW( hwnd, _T( "open" ), _T( "RUNDLL32.EXE" ), parameters_filename.c_str( ), dir_buf, SW_SHOWNORMAL ) );
			}
		if ( sys_dir_res == 0 ) {
			displayWindowsMsgBoxWithError( );
			std::terminate( );
			}
		ASSERT( sys_dir_res > dir_buf_size );
		if ( sys_dir_res > 4096 ) {
			const auto str_ptr = std::make_unique<wchar_t[ ]>( sys_dir_res );
			const auto sys_dir_res_2 = GetSystemDirectoryW( str_ptr.get( ), sys_dir_res );
			if ( ( sys_dir_res_2 != 0 ) && ( sys_dir_res_2 < sys_dir_res ) ) {
				u = reinterpret_cast< INT_PTR >( ShellExecuteW( hwnd, _T( "open" ), _T( "RUNDLL32.EXE" ), parameters_filename.c_str( ), str_ptr.get( ), SW_SHOWNORMAL ) );
				}
			else {
				displayWindowsMsgBoxWithMessage( L"Something is extremely wrong (GetSystemDirectoryW)!!" );
				std::terminate( );
				}
			}
		const rsize_t dir_buf_size_2 = 4096;
		wchar_t dir_buf_2[ dir_buf_size_2 ] = { 0 };
		const auto sys_dir_res_3 = GetSystemDirectoryW( dir_buf_2, dir_buf_size_2 );
		if ( ( sys_dir_res_3 != 0 ) && ( sys_dir_res_3 < dir_buf_size_2 ) ) {
			u = reinterpret_cast< INT_PTR >( ShellExecuteW( hwnd, _T( "open" ), _T( "RUNDLL32.EXE" ), parameters_filename.c_str( ), dir_buf_2, SW_SHOWNORMAL ) );
			}
		else {
			displayWindowsMsgBoxWithMessage( L"Something is extremely, seriously, wrong (GetSystemDirectoryW)!!" );
			std::terminate( );
			}
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
	const BOOL b = GetDiskFreeSpaceExW( pszRootPath, &uavailable, &utotal, &ufree );
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
	const BOOL b = GetDiskFreeSpaceExW( pszRootPath, &uavailable, &utotal, &ufree );
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


//CString GetCOMSPEC( ) {
//	CString cmd;
//
//	auto dw = GetEnvironmentVariableW( _T( "COMSPEC" ), cmd.GetBuffer( _MAX_PATH ), _MAX_PATH );
//	cmd.ReleaseBuffer( );
//
//	if ( dw == 0 ) {
//		TRACE(_T("COMSPEC not set.\n"));
//		cmd = _T( "cmd.exe" );
//		}
//	return cmd;
//	}


bool DriveExists( _In_ const CString& path ) {
	//ASSERT( path != _T( "" ) );
	if ( path.GetLength( ) != 3 || path[ 1 ] != _T( ':' ) || path[ 2 ] != _T( '\\' ) ) {
		return false;
		}
	//auto letter = path.Left( 1 ).MakeLower( ).GetString( );
	wchar_t ltr[ 2 ] = { 0 };
	ltr[ 0 ] = path.Left( 1 ).MakeLower( )[ 0 ];
	ltr[ 1 ] = 0;

	//is 'a' == 97?
	const INT d = ltr[ 0 ] - _T( 'a' );//????BUGBUG TODO: ?
	static_assert( L'a' == 97, "wtf!" );

	const DWORD mask = 0x1 << d;

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
	const auto dw = QueryDosDeviceW( d, info.GetBuffer( 512 ), 512 );//eek
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
	const auto info = MyQueryDosDevice( drive );
	return ( info.GetLength( ) >= 4 && info.Left( 4 ) == "\\??\\" );
	}

const LARGE_INTEGER help_QueryPerformanceCounter( ) {
	LARGE_INTEGER doneTime;
	const BOOL behavedWell = QueryPerformanceCounter( &doneTime );
	ASSERT( behavedWell );
	if ( !behavedWell ) {
		std::string a;
		//a += ( __FUNCTION__, __LINE__ );
		a += __FUNCTION__;
		a += std::to_string( __LINE__ );
		MessageBoxA( NULL, "QueryPerformanceCounter failed!!", a.c_str( ), MB_OK );
		doneTime.QuadPart = -1;
		}
	return doneTime;
	}

const LARGE_INTEGER help_QueryPerformanceFrequency( ) {
	LARGE_INTEGER doneTime;
	const BOOL behavedWell = QueryPerformanceFrequency( &doneTime );
	ASSERT( behavedWell );
	if ( !behavedWell ) {
		std::string a;
		//a += ( __FUNCTION__, __LINE__ );
		a += __FUNCTION__;
		a += std::to_string( __LINE__ );
		MessageBoxA( NULL, "QueryPerformanceFrequency failed!!", a.c_str( ), MB_OK );
		doneTime.QuadPart = -1;
		}
	return doneTime;
	}


//All the zeroInits assume this
static_assert( NULL == 0, "Check the zeroInit functions! Make sure that they're actually initializing to zero!" );
SHELLEXECUTEINFO partInitSEI( ) {
	SHELLEXECUTEINFO sei;
	sei.cbSize       = sizeof( sei );
	sei.dwHotKey     = { NULL };
	sei.fMask        = { NULL };
	sei.hIcon        = { NULL };
	sei.hInstApp     = { NULL };
	sei.hkeyClass    = { NULL };
	sei.hMonitor     = { NULL };
	sei.hProcess     = { NULL };
	sei.hwnd         = { NULL };
	sei.lpClass      = { NULL };
	sei.lpDirectory  = { NULL };
	sei.lpFile       = { NULL };
	sei.lpIDList     = { NULL };
	sei.lpParameters = { NULL };
	sei.lpVerb       = { NULL };
	sei.nShow        = { NULL };
	return sei ;
	}


//SHELLEXECUTEINFO zeroInitSEI( ) {
//	SHELLEXECUTEINFO sei;
//	sei.cbSize       = { NULL };
//	sei.dwHotKey     = { NULL };
//	sei.fMask        = { NULL };
//	sei.hIcon        = { NULL };
//	sei.hInstApp     = { NULL };
//	sei.hkeyClass    = { NULL };
//	sei.hMonitor     = { NULL };
//	sei.hProcess     = { NULL };
//	sei.hwnd         = { NULL };
//	sei.lpClass      = { NULL };
//	sei.lpDirectory  = { NULL };
//	sei.lpFile       = { NULL };
//	sei.lpIDList     = { NULL };
//	sei.lpParameters = { NULL };
//	sei.lpVerb       = { NULL };
//	sei.nShow        = { NULL };
//	return std::move( sei );
//	}

WINDOWPLACEMENT zeroInitWINDOWPLACEMENT( ) {
	WINDOWPLACEMENT wp;
	wp.flags                   = { NULL };
	wp.ptMaxPosition.x         = { NULL };
	wp.ptMaxPosition.y         = { NULL };
	wp.ptMinPosition.x         = { NULL };
	wp.ptMinPosition.y         = { NULL };
	wp.rcNormalPosition.bottom = { NULL };
	wp.rcNormalPosition.left   = { NULL };
	wp.rcNormalPosition.right  = { NULL };
	wp.rcNormalPosition.top    = { NULL };
	wp.showCmd                 = { NULL };
	wp.length                  = { sizeof( wp ) };

	return wp;
	}

LVHITTESTINFO zeroInitLVHITTESTINFO( ) {
	LVHITTESTINFO hti;
	hti.flags    = { NULL };
	hti.iGroup   = { NULL };
	hti.iItem    = { NULL };
	hti.iSubItem = { NULL };
	hti.pt.x     = { NULL };
	hti.pt.y     = { NULL };
	return hti;
	}

HDITEM zeroInitHDITEM( ) {
	HDITEM hditem;

	hditem.cchTextMax = { NULL };
	hditem.cxy        = { NULL };
	hditem.fmt        = { NULL };
	hditem.hbm        = { NULL };
	hditem.iImage     = { NULL };
	hditem.iOrder     = { NULL };
	hditem.lParam     = { NULL };
	hditem.mask       = { NULL };
	hditem.pszText    = { NULL };
	hditem.pvFilter   = { NULL };
	hditem.state      = { NULL };
	hditem.type       = { NULL };
	return hditem;
	}

LVFINDINFO zeroInitLVFINDINFO( ) {
	LVFINDINFO fi;
	fi.flags       = { NULL };
	fi.lParam      = { NULL };
	fi.psz         = { NULL };
	fi.pt.x        = { NULL };
	fi.pt.y        = { NULL };
	fi.vkDirection = { NULL };
	return fi;
	}

LVITEM partInitLVITEM( ) {
	LVITEM lvitem;
	lvitem.cchTextMax = { NULL };
	lvitem.cColumns   = { NULL };
	lvitem.iGroup     = { NULL };
	lvitem.iGroupId   = { NULL };
	lvitem.iIndent    = { NULL };
	lvitem.iSubItem   = { NULL };
	lvitem.piColFmt   = { NULL };
	lvitem.puColumns  = { NULL };
	lvitem.state      = { NULL };
	lvitem.stateMask  = { NULL };
	return lvitem;
	}

PROCESS_MEMORY_COUNTERS zeroInitPROCESS_MEMORY_COUNTERS( ) {
	PROCESS_MEMORY_COUNTERS pmc;
	pmc.cb                         = { NULL };
	pmc.PageFaultCount             = { NULL };
	pmc.PagefileUsage              = { NULL };
	pmc.PeakPagefileUsage          = { NULL };
	pmc.PeakWorkingSetSize         = { NULL };
	pmc.QuotaNonPagedPoolUsage     = { NULL };
	pmc.QuotaPagedPoolUsage        = { NULL };
	pmc.QuotaPeakNonPagedPoolUsage = { NULL };
	pmc.QuotaPeakPagedPoolUsage    = { NULL };
	pmc.WorkingSetSize             = { NULL };
	return pmc;
	}
STARTUPINFO zeroInitSTARTUPINFO( ) {
	STARTUPINFO si;
	si.cb              = { NULL };
	si.cbReserved2     = { NULL };
	si.dwFillAttribute = { NULL };
	si.dwFlags         = { NULL };
	si.dwX             = { NULL };
	si.dwXCountChars   = { NULL };
	si.dwXSize         = { NULL };
	si.dwY             = { NULL };
	si.dwYCountChars   = { NULL };
	si.dwYSize         = { NULL };
	si.hStdError       = { NULL };
	si.hStdInput       = { NULL };
	si.hStdOutput      = { NULL };
	si.lpDesktop       = { NULL };
	si.lpReserved      = { NULL };
	si.lpReserved2     = { NULL };
	si.lpTitle         = { NULL };
	si.wShowWindow     = { NULL };
	return si;
	}

PROCESS_INFORMATION zeroInitPROCESS_INFORMATION( ) {
	PROCESS_INFORMATION pi;
	pi.dwProcessId = { NULL };
	pi.dwThreadId  = { NULL };
	pi.hProcess    = { NULL };
	pi.hThread     = { NULL };
	return pi;
	}

NMLISTVIEW zeroInitNMLISTVIEW( ) {
	NMLISTVIEW listView;
	listView.hdr.code     = { NULL };
	listView.hdr.hwndFrom = { NULL };
	listView.hdr.idFrom   = { NULL };
	listView.iItem        = { NULL };
	listView.iSubItem     = { NULL };
	listView.lParam       = { NULL };
	listView.ptAction.x   = { NULL };
	listView.ptAction.y   = { NULL };
	listView.uChanged     = { NULL };
	listView.uNewState    = { NULL };
	listView.uOldState    = { NULL };
	return listView;
	}

//NMLISTVIEW* zeroInitNMLISTVIEW_heap( ) {
//	auto listView = new NMLISTVIEW;
//	listView->hdr.code     = { NULL };
//	listView->hdr.hwndFrom = { NULL };
//	listView->hdr.idFrom   = { NULL };
//	listView->iItem        = { NULL };
//	listView->iSubItem     = { NULL };
//	listView->lParam       = { NULL };
//	listView->ptAction.x   = { NULL };
//	listView->ptAction.y   = { NULL };
//	listView->uChanged     = { NULL };
//	listView->uNewState    = { NULL };
//	listView->uOldState    = { NULL };
//	return listView;
//	}


BROWSEINFO zeroInitBROWSEINFO( ) {
	BROWSEINFO bi;
	bi.hwndOwner      = { NULL };
	bi.iImage         = { NULL };
	bi.lParam         = { NULL };
	bi.lpfn           = { NULL };
	bi.lpszTitle      = { NULL };
	bi.pidlRoot       = { NULL };
	bi.pszDisplayName = { NULL };
	bi.ulFlags        = { NULL };
	return bi;
	}

SHFILEOPSTRUCT zeroInitSHFILEOPSTRUCT( ) {
	SHFILEOPSTRUCT sfos;
	sfos.fAnyOperationsAborted = { NULL };
	sfos.fFlags                = { NULL };
	sfos.hNameMappings         = { NULL };
	sfos.hwnd                  = { NULL };
	sfos.lpszProgressTitle     = { NULL };
	sfos.pFrom                 = { NULL };
	sfos.pTo                   = { NULL };
	sfos.wFunc                 = { NULL };
	return sfos;
	}

CString GetLastErrorAsFormattedMessage( const DWORD last_err ) {
	const rsize_t msgBufSize = 2 * 1024;
	wchar_t msgBuf[ msgBufSize ] = { 0 };
	const auto ret = FormatMessageW( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, last_err, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), msgBuf, msgBufSize, NULL );
	if ( ret > 0 ) {
		return CString( msgBuf );
		}
	return CString( "FormatMessage failed to format an error!" );
	}

//On returning E_FAIL, call GetLastError for details. That's not my idea!
_Success_( SUCCEEDED( return ) ) HRESULT CStyle_GetLastErrorAsFormattedMessage( _Out_writes_z_( strSize ) _Pre_writable_size_( strSize ) _Post_readable_size_( chars_written ) PWSTR psz_formatted_error, _In_range_( 128, 32767 ) const rsize_t strSize, _Out_ rsize_t& chars_written ) {
	const auto err = GetLastError( );
	const auto ret = FormatMessageW( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, err, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), psz_formatted_error, static_cast<DWORD>( strSize ), NULL );
	if ( ret != 0 ) {
		chars_written = ret;
		return S_OK;
		}
	if ( strSize > 41 ) {
		write_bad_fmt_msg( psz_formatted_error, chars_written );
		return E_FAIL;
		}
	else if ( strSize > 8 ) {
		write_BAD_FMT( psz_formatted_error, chars_written );
		return E_FAIL;
		}
	chars_written = 0;
	return E_FAIL;
	}

void write_bad_fmt_msg( _Out_writes_z_( 41 ) _Pre_writable_size_( 42 ) _Post_readable_size_( chars_written ) PWSTR psz_fmt_msg, _Out_ rsize_t& chars_written ) {
	psz_fmt_msg[  0 ] = L'F';
	psz_fmt_msg[  1 ] = L'o';
	psz_fmt_msg[  2 ] = L'r';
	psz_fmt_msg[  3 ] = L'm';
	psz_fmt_msg[  4 ] = L'a';
	psz_fmt_msg[  5 ] = L't';
	psz_fmt_msg[  6 ] = L'M';
	psz_fmt_msg[  7 ] = L'e';
	psz_fmt_msg[  8 ] = L's';
	psz_fmt_msg[  9 ] = L's';
	psz_fmt_msg[ 10 ] = L'a';
	psz_fmt_msg[ 11 ] = L'g';
	psz_fmt_msg[ 12 ] = L'e';
	psz_fmt_msg[ 13 ] = L' ';
	psz_fmt_msg[ 14 ] = L'f';
	psz_fmt_msg[ 15 ] = L'a';
	psz_fmt_msg[ 16 ] = L'i';
	psz_fmt_msg[ 17 ] = L'l';
	psz_fmt_msg[ 18 ] = L'e';
	psz_fmt_msg[ 19 ] = L'd';
	psz_fmt_msg[ 20 ] = L' ';
	psz_fmt_msg[ 21 ] = L't';
	psz_fmt_msg[ 22 ] = L'o';
	psz_fmt_msg[ 23 ] = L' ';
	psz_fmt_msg[ 24 ] = L'f';
	psz_fmt_msg[ 25 ] = L'o';
	psz_fmt_msg[ 26 ] = L'r';
	psz_fmt_msg[ 27 ] = L'm';
	psz_fmt_msg[ 28 ] = L'a';
	psz_fmt_msg[ 29 ] = L't';
	psz_fmt_msg[ 30 ] = L' ';
	psz_fmt_msg[ 31 ] = L'a';
	psz_fmt_msg[ 32 ] = L'n';
	psz_fmt_msg[ 33 ] = L' ';
	psz_fmt_msg[ 34 ] = L'e';
	psz_fmt_msg[ 35 ] = L'r';
	psz_fmt_msg[ 36 ] = L'r';
	psz_fmt_msg[ 37 ] = L'o';
	psz_fmt_msg[ 38 ] = L'r';
	psz_fmt_msg[ 39 ] = L'!';
	psz_fmt_msg[ 40 ] = 0;
	chars_written = 41;
	}

void displayWindowsMsgBoxWithError( ) {
	const rsize_t err_msg_size = 1024;
	wchar_t err_msg[ err_msg_size ] = { 0 };
	//const auto errMsg = GetLastErrorAsFormattedMessage( );
	rsize_t chars_written = 0;

	const HRESULT err_res = CStyle_GetLastErrorAsFormattedMessage( err_msg, err_msg_size, chars_written );
	if ( SUCCEEDED( err_res ) ) {
		MessageBoxW( NULL, err_msg, TEXT( "Error" ), MB_OK );
		TRACE( _T( "Error: %s\r\n" ), err_msg );
		return;
		}

	const rsize_t err_msg_size_2 = 4096;
	wchar_t err_msg_2[ err_msg_size_2 ] = { 0 };
	//const auto errMsg = GetLastErrorAsFormattedMessage( );
	rsize_t chars_written_2 = 0;
	const HRESULT err_res_2 = CStyle_GetLastErrorAsFormattedMessage( err_msg_2, err_msg_size_2, chars_written_2 );
	if ( SUCCEEDED( err_res_2 ) ) {
		MessageBoxW( NULL, err_msg_2, TEXT( "Error" ), MB_OK );
		TRACE( _T( "Error: %s\r\n" ), err_msg_2 );
		return;
		}
	TRACE( _T( "Error while getting error message!\r\n" ), err_msg_2 );
	MessageBoxW( NULL, _T( "Error while getting error message!\r\n" ), TEXT( "Error" ), MB_OK );
	}

//void displayWindowsMsgBoxWithMessage( const CString message ) {
//	MessageBoxW( NULL, message, TEXT( "Error" ), MB_OK );
//	TRACE( _T( "Error: %s\r\n" ), message );
//	}

void displayWindowsMsgBoxWithMessage( const std::wstring message ) {
	MessageBoxW( NULL, message.c_str( ), TEXT( "Error" ), MB_OK );
	TRACE( _T( "Error: %s\r\n" ), message.c_str( ) );
	}

void displayWindowsMsgBoxWithMessage( PCWSTR message ) {
	MessageBoxW( NULL, message, TEXT( "Error" ), MB_OK );
	TRACE( _T( "Error: %s\r\n" ), message );
	}


void check8Dot3NameCreationAndNotifyUser( ) {
	HKEY keyHandle = { NULL };

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
	const DWORD value = data[ 0 ];
	/*
		0 = NTFS creates short file names. This setting enables applications that cannot process long file names and computers that use differentcode pages to find the files.
		1 = NTFS does not create short file names. Although this setting increases file performance, applications that cannot process long file names, and computers that use different code pages, might not be able to find the files.
		2 = NTFS sets the 8.3 naming convention creation on a per volume basis.
		3 = NTFS disables 8dot3 name creation on all volumes except the system volume.
	*/
	if ( value == 0 ) {
		std::wstring message = std::wstring( global_strings::eight_dot_three_gen_notif1 ) + std::wstring( global_strings::eight_dot_three_all_volume ) + std::wstring( global_strings::eight_dot_three_gen_notif2 );
		MessageBoxW( NULL, message.c_str( ), global_strings::gen_performance_warning, MB_ICONWARNING );
		}

	if ( value == 2 ) {
		std::wstring message = std::wstring( global_strings::eight_dot_three_gen_notif1 ) + std::wstring( global_strings::eight_dot_three_per_volume ) + std::wstring( global_strings::eight_dot_three_gen_notif2 );
		MessageBoxW( NULL, message.c_str( ), global_strings::gen_performance_warning, MB_ICONWARNING );
		}

	if ( value == 3 ) {
		std::wstring message = std::wstring( global_strings::eight_dot_three_gen_notif1 ) + std::wstring( global_strings::eight_dot_three_sys_volume ) + std::wstring( global_strings::eight_dot_three_gen_notif2 );
		MessageBoxW( NULL, message.c_str( ), global_strings::gen_performance_warning, MB_ICONWARNING );
		}
	}
void zeroDate( _Out_ FILETIME& in ) {
	in.dwHighDateTime = 0;
	in.dwLowDateTime  = 0;
	}

FILETIME zeroInitFILETIME( ) {
	FILETIME ft;
	ft.dwHighDateTime = { NULL };
	ft.dwLowDateTime = { NULL };
	return ft;
	}

// Encodes a selection from the CSelectDrivesDlg into a string which can be routed as a pseudo document "path" through MFC and finally arrives in OnOpenDocument().
std::wstring EncodeSelection( _In_ const RADIO radio, _In_ const std::wstring folder, _In_ const std::vector<std::wstring>& drives ) {
	std::wstring ret;
	TRACE( _T( "Encoding selection %s\r\n" ), folder.c_str( ) );
	switch ( radio ) {
			case RADIO_ALLLOCALDRIVES:
			case RADIO_SOMEDRIVES:
				{
				for ( size_t i = 0; i < drives.size( ); i++ ) {
					if ( i > 0 ) {
						ret += L'|';// `|` is the encoding separator, which is not allowed in file names.;
						}
					ret += drives.at( i );
					}
				}
				break;

			case RADIO_AFOLDER:
				return folder;
				break;
		}
	TRACE( _T( "Selection encoded as '%s'\r\n" ), ret.c_str( ) );
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
	return out;
	}


//std::vector<COLORREF> GetDefaultPaletteAsVector( ) {
//	std::vector<COLORREF> colorVector;
//	std::vector<COLORREF> defaultColorVec = { RGB( 0, 0, 255 ), RGB( 255, 0, 0 ), RGB( 0, 255, 0 ), RGB( 0, 255, 255 ), RGB( 255, 0, 255 ), RGB( 255, 255, 0 ), RGB( 150, 150, 255 ), RGB( 255, 150, 150 ), RGB( 150, 255, 150 ), RGB( 150, 255, 255 ), RGB( 255, 150, 255 ), RGB( 255, 255, 150 ), RGB( 255, 255, 255 ) };
//	colorVector.reserve( defaultColorVec.size( ) + 1 );
//	for ( auto& aColor : defaultColorVec ) {
//		colorVector.emplace_back( CColorSpace::MakeBrightColor( aColor, PALETTE_BRIGHTNESS ) );
//		}
//	return std::move( colorVector );
//	}


void write_BAD_FMT( _Out_writes_z_( 8 ) _Pre_writable_size_( 8 ) _Post_readable_size_( 8 ) PWSTR pszFMT, _Out_ rsize_t& chars_written ) {
	pszFMT[ 0 ] = 'B';
	pszFMT[ 1 ] = 'A';
	pszFMT[ 2 ] = 'D';
	pszFMT[ 3 ] = '_';
	pszFMT[ 4 ] = 'F';
	pszFMT[ 5 ] = 'M';
	pszFMT[ 6 ] = 'T';
	pszFMT[ 7 ] = 0;
	chars_written = 8;
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

void write_RAM_USAGE( _Out_writes_z_( 12 ) _Pre_writable_size_( 13 ) PWSTR psz_ram_usage ) {
	//psz_ram_usage = L"RAM Usage: ";
	psz_ram_usage[ 0  ] = 'R';
	psz_ram_usage[ 1  ] = 'A';
	psz_ram_usage[ 2  ] = 'M';
	psz_ram_usage[ 3  ] = ' ';
	psz_ram_usage[ 4  ] = 'U';
	psz_ram_usage[ 5  ] = 's';
	psz_ram_usage[ 6  ] = 'a';
	psz_ram_usage[ 7  ] = 'g';
	psz_ram_usage[ 8  ] = 'e';
	psz_ram_usage[ 9  ] = ':';
	psz_ram_usage[ 10 ] = ' ';
	psz_ram_usage[ 11 ] = 0;

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

_Pre_satisfies_( min_val < max_val )
_Post_satisfies_( min_val <= val )
_Post_satisfies_( val <= max_val )
void CheckMinMax( _Inout_ LONG& val, _In_ const LONG min_val, _In_ const LONG max_val ) {
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

_Pre_satisfies_( min_val < max_val )
_Post_satisfies_( min_val <= val )
_Post_satisfies_( val <= max_val )
void CheckMinMax( _Inout_ LONG& val, _In_ const INT min_val, _In_ const INT max_val ) {
	ASSERT( min_val <= max_val );

	if ( val < static_cast<LONG>( min_val ) ) {
		val = static_cast<LONG>( min_val );
		}
	if ( val > static_cast<LONG>( max_val ) ) {
		val = static_cast<LONG>( max_val );
		}
	ASSERT( val <= static_cast<LONG>( max_val ) );
	ASSERT( static_cast<LONG>( min_val ) <= val );
	}

_Pre_satisfies_( min_val < max_val )
_Post_satisfies_( min_val <= val )
_Post_satisfies_( val <= max_val )
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



void DistributeFirst( _Inout_ _Out_range_(0, 255) INT& first, _Inout_ _Out_range_(0, 255) INT& second, _Inout_ _Out_range_(0, 255) INT& third ) {
	const INT h = ( first - 255 ) / 2;
	first = 255;
	second += h;
	third += h;

	if ( second > 255 ) {
		const auto h2 = second - 255;
		second = 255;
		third += h2;
		}
	else if ( third > 255 ) {
		const auto h3 = third - 255;
		third = 255;
		second += h3;
		}
	ASSERT( second <= 255 );
	ASSERT( third <= 255 );
	}

void NormalizeColor( _Inout_ _Out_range_(0, 255) INT& red, _Inout_ _Out_range_(0, 255) INT& green, _Inout_ _Out_range_(0, 255) INT& blue ) {
	ASSERT( red + green + blue <= 3 * 255 );
	if ( red > 255 ) {
		DistributeFirst( red, green, blue );
		}
	else if ( green > 255 ) {
		DistributeFirst( green, red, blue );
		}
	else if ( blue > 255 ) {
		DistributeFirst( blue, red, green );
		}
	}


COLORREF CColorSpace::MakeBrightColor( _In_ const COLORREF color, _In_ _In_range_( 0, 1 ) const DOUBLE brightness ) {
	ASSERT( brightness >= 0.0 );
	ASSERT( brightness <= 1.0 );

	DOUBLE dred   = GetRValue( color ) / 255.0;
	DOUBLE dgreen = GetGValue( color ) / 255.0;
	DOUBLE dblue  = GetBValue( color ) / 255.0;
#ifdef COLOR_DEBUGGING
	TRACE( _T( "CColorSpace::MakeBrightColor passed color: %ld, brightness: %f\r\nred: %f, green: %f, blue: %f\r\n" ), color, brightness, dred, dgreen, dblue );
#endif

	const DOUBLE f = 3.0 * brightness / ( dred + dgreen + dblue );
	dred   *= f;
	dgreen *= f;
	dblue  *= f;

	INT red   = std::lrint( dred   * 255 );
	INT green = std::lrint( dgreen * 255 );
	INT blue  = std::lrint( dblue  * 255 );
	
	NormalizeColor(red, green, blue);
	ASSERT( RGB( red, green, blue ) != 0 );
#ifdef COLOR_DEBUGGING
	TRACE( _T( "CColorSpace::MakeBrightColor returning red: %i, green: %i, blue: %i\r\n" ), red, green, blue );
#endif

	return RGB( red, green, blue );
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
