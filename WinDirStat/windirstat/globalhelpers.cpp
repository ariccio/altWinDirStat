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
namespace {
	std::wstring Format_uint64_t_Normal( _In_ std::uint64_t n ) {
		// Returns formatted number like "123.456.789".
		// 18446744073709551615 is max
		//                     ^ 20 characters
		// 18,446,744,073,709,551,615
		//                           ^26 characters
		//                            26 + null terminator = 27
		//const rsize_t bufSize = 28;
		//wchar_t buffer[ bufSize ] = { 0 };

		std::wstring all_ws;
		all_ws.reserve( 27 );

		do
		{
			auto rest = INT( n % 1000 );
			n /= 1000;
			const rsize_t tempBuf_size = 10;
			wchar_t tempBuf[ tempBuf_size ] = { 0 };
			if ( n > 0 ) {
				//wprintf(  );
				const HRESULT fmt_res = StringCchPrintfW( tempBuf, tempBuf_size, L",%03d", rest );
				ASSERT( SUCCEEDED( fmt_res ) );
				if ( !SUCCEEDED( fmt_res ) ) {
					return L"FORMATTING FAILED!";
					}

				//const auto pf_res = _snwprintf_s( tempBuf, 9, _TRUNCATE, L",%03d", rest );
				//ASSERT( pf_res != -1 );
				//UNREFERENCED_PARAMETER( pf_res );
				}
			else {
				const HRESULT fmt_res = StringCchPrintfW( tempBuf, tempBuf_size, L"%d", rest );
				ASSERT( SUCCEEDED( fmt_res ) );
				if ( !SUCCEEDED( fmt_res ) ) {
					return L"FORMATTING FAILED!";
					}

				//const auto pf_res = _snwprintf_s( tempBuf, 9, _TRUNCATE, L"%d", rest );
				//ASSERT( pf_res != -1 );
				//UNREFERENCED_PARAMETER( pf_res );
				}
			all_ws += tempBuf;
			//wcscat_s( buffer, tempBuf );
			}
		while ( n > 0 );
		return all_ws;
		}

	void get_date_format_err( ) {
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

	void get_time_format_err( ) {
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

	_Success_( SUCCEEDED( return ) ) HRESULT file_time_to_system_time_err( _Out_writes_z_( strSize ) _Pre_writable_size_( strSize ) PWSTR psz_formatted_datetime, _In_range_( 128, 2048 ) const rsize_t strSize, _Out_ rsize_t& chars_written ) {
		const HRESULT err_res = CStyle_GetLastErrorAsFormattedMessage( psz_formatted_datetime, strSize, chars_written );
		if ( !SUCCEEDED( err_res ) ) {
			TRACE( _T( "Error in file_time_to_system_time_err->CStyle_GetLastErrorAsFormattedMessage!!\r\n" ) );
			displayWindowsMsgBoxWithMessage( L"Error in file_time_to_system_time_err->CStyle_GetLastErrorAsFormattedMessage!!\r\n" );
			return err_res;
			}
		return E_FAIL;
		}

	void ensure_valid_return_date( const int gdfres, const rsize_t strSize ) {
		if ( !( ( gdfres + 1 ) < static_cast< std::int64_t >( strSize ) ) ) {
			displayWindowsMsgBoxWithMessage( L"Error in ensure_valid_return_date!(aborting)" );
			std::wstring err_str( L"DEBUGGING INFO: strSize: " );
			err_str += std::to_wstring( strSize );
			err_str += L", gdfres: ";
			err_str += std::to_wstring( gdfres );
			err_str += L".";
			displayWindowsMsgBoxWithMessage( err_str.c_str( ) );
			std::terminate( );
			}
		if ( gdfres == 0 ) {
			get_date_format_err( );
			}
		}

	void ensure_valid_return_time( const int gtfres, const rsize_t strSize ) {
		if ( !( ( gtfres + 1 ) < static_cast< std::int64_t >( strSize ) ) ) {
			displayWindowsMsgBoxWithMessage( L"Error in ensure_valid_return_time!(aborting)" );
			std::wstring err_str( L"DEBUGGING INFO: strSize: " );
			err_str += std::to_wstring( strSize );
			err_str += L", gtfres: ";
			err_str += std::to_wstring( gtfres );
			err_str += L".";
			displayWindowsMsgBoxWithMessage( err_str.c_str( ) );
			std::terminate( );
			}
		if ( gtfres == 0 ) {
			get_time_format_err( );
			}
		}

	_Success_( SUCCEEDED( return ) ) inline HRESULT CStyle_FormatLongLongHuman_0( WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_formatted_LONGLONG_HUMAN, _In_range_( 8, 64 ) const rsize_t strSize, _Out_ rsize_t& chars_written ) {
		if ( strSize > 2 ) {
			psz_formatted_LONGLONG_HUMAN[ 0 ] = L'0';
			psz_formatted_LONGLONG_HUMAN[ 1 ] = 0;
			chars_written = 1;
			ASSERT( wcslen( psz_formatted_LONGLONG_HUMAN ) == chars_written );
			return S_OK;
			}
		return STRSAFE_E_INSUFFICIENT_BUFFER;
		}

	_Success_( SUCCEEDED( return ) ) HRESULT CStyle_FormatLongLongHuman_B( WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_formatted_LONGLONG_HUMAN, _In_range_( 8, 64 ) const rsize_t strSize, _Out_ rsize_t& chars_written, _In_ const DOUBLE B ) {
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

	_Success_( SUCCEEDED( return ) ) _Pre_satisfies_( chars_written == 0 )
	HRESULT CStyle_FormatLongLongHuman_KB( WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_formatted_LONGLONG_HUMAN, _In_range_( 23, 64 ) const rsize_t strSize, _Out_ rsize_t& chars_written, _In_ const DOUBLE B, _In_ const DOUBLE KB ) {
		const rsize_t bufSize = 19;
		wchar_t buffer[ bufSize ] = { 0 };
		rsize_t buffer_chars_written = 0;
		const HRESULT res = CStyle_FormatDouble( KB + B / BASE, buffer, bufSize, buffer_chars_written );
		if ( SUCCEEDED( res ) ) {
			ASSERT( wcslen( buffer ) == buffer_chars_written );
			rsize_t chars_remaining = 0;
			const HRESULT fmt_res = StringCchPrintfExW( psz_formatted_LONGLONG_HUMAN, strSize, NULL, &chars_remaining, 0, L"%s KB", buffer );
			if ( SUCCEEDED( fmt_res ) ) {
				chars_written = ( strSize - chars_remaining );
				return fmt_res;
				}
			else if ( fmt_res == STRSAFE_E_INSUFFICIENT_BUFFER ) {
				chars_written = strSize;
				ASSERT( chars_written == strSize );
				return STRSAFE_E_INSUFFICIENT_BUFFER;
				}
			else if ( ( fmt_res != STRSAFE_E_INSUFFICIENT_BUFFER ) && ( FAILED( fmt_res ) ) ) {
				chars_written = 0;
				write_BAD_FMT( psz_formatted_LONGLONG_HUMAN, chars_written );
				}
			return fmt_res;
			}
		write_BAD_FMT( psz_formatted_LONGLONG_HUMAN, chars_written );
		return res;
		}

	_Success_( SUCCEEDED( return ) ) HRESULT CStyle_FormatLongLongHuman_MB( WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_formatted_LONGLONG_HUMAN, _In_range_( 23, 64 ) const rsize_t strSize, _Out_ rsize_t& chars_written, _In_ const DOUBLE KB, _In_ const DOUBLE MB ) {
		const rsize_t bufSize = 19;
		wchar_t buffer[ bufSize ] = { 0 };
		rsize_t buffer_chars_written = 0;
		const HRESULT res = CStyle_FormatDouble( MB + KB / BASE, buffer, bufSize, buffer_chars_written );
		if ( SUCCEEDED( res ) ) {
			ASSERT( wcslen( buffer ) == buffer_chars_written );
			rsize_t chars_remaining = 0;
			const HRESULT fmt_res = StringCchPrintfExW( psz_formatted_LONGLONG_HUMAN, strSize, NULL, &chars_remaining, 0, L"%s MB", buffer );
			if ( SUCCEEDED( fmt_res ) ) {
				chars_written = ( strSize - chars_remaining );
				return fmt_res;
				}
			else if ( fmt_res == STRSAFE_E_INSUFFICIENT_BUFFER ) {
				chars_written = strSize;
				ASSERT( chars_written == strSize );
				return STRSAFE_E_INSUFFICIENT_BUFFER;
				}
			else if ( ( fmt_res != STRSAFE_E_INSUFFICIENT_BUFFER ) && ( FAILED( fmt_res ) ) ) {
				chars_written = 0;
				write_BAD_FMT( psz_formatted_LONGLONG_HUMAN, chars_written );
				}
			return fmt_res;
			}
		write_BAD_FMT( psz_formatted_LONGLONG_HUMAN, chars_written );
		return res;
		}

	_Success_( SUCCEEDED( return ) ) HRESULT CStyle_FormatLongLongHuman_GB( WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_formatted_LONGLONG_HUMAN, _In_range_( 8, 64 ) const rsize_t strSize, _Out_ rsize_t& chars_written, _In_ const DOUBLE MB, _In_ const DOUBLE GB ) {
		const rsize_t bufSize = 19;
		wchar_t buffer[ bufSize ] = { 0 };
		rsize_t buffer_chars_written = 0;
		const HRESULT res = CStyle_FormatDouble( GB + MB / BASE, buffer, bufSize, buffer_chars_written );
		if ( SUCCEEDED( res ) ) {
			ASSERT( wcslen( buffer ) == buffer_chars_written );
			rsize_t chars_remaining = 0;
			const HRESULT fmt_res = StringCchPrintfExW( psz_formatted_LONGLONG_HUMAN, strSize, NULL, &chars_remaining, 0, L"%s GB", buffer );
			if ( SUCCEEDED( fmt_res ) ) {
				chars_written = ( strSize - chars_remaining );
				return fmt_res;
				}
			else if ( fmt_res == STRSAFE_E_INSUFFICIENT_BUFFER ) {
				chars_written = strSize;
				ASSERT( chars_written == strSize );
				return STRSAFE_E_INSUFFICIENT_BUFFER;
				}
			else if ( ( fmt_res != STRSAFE_E_INSUFFICIENT_BUFFER ) && ( FAILED( fmt_res ) ) ) {
				chars_written = 0;
				write_BAD_FMT( psz_formatted_LONGLONG_HUMAN, chars_written );
				}
			return fmt_res;
			}
		write_BAD_FMT( psz_formatted_LONGLONG_HUMAN, chars_written );
		return res;
		}

	_Success_( SUCCEEDED( return ) ) HRESULT CStyle_FormatLongLongHuman_TB( WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_formatted_LONGLONG_HUMAN, _In_range_( 8, 64 ) const rsize_t strSize, _Out_ rsize_t& chars_written, _In_ const DOUBLE GB, _In_ const DOUBLE TB ) {
		const rsize_t bufSize = 19;
		wchar_t buffer[ bufSize ] = { 0 };
		rsize_t buffer_chars_written = 0;
		const HRESULT res = CStyle_FormatDouble( TB + GB / BASE, buffer, bufSize, buffer_chars_written );
		if ( SUCCEEDED( res ) ) {
			ASSERT( wcslen( buffer ) == buffer_chars_written );
			rsize_t chars_remaining = 0;
			const HRESULT fmt_res = StringCchPrintfExW( psz_formatted_LONGLONG_HUMAN, strSize, NULL, &chars_remaining, 0, L"%s TB", buffer );
			if ( SUCCEEDED( fmt_res ) ) {
				chars_written = ( strSize - chars_remaining );
				return fmt_res;
				}
			else if ( fmt_res == STRSAFE_E_INSUFFICIENT_BUFFER ) {
				chars_written = strSize;
				ASSERT( chars_written == strSize );
				return STRSAFE_E_INSUFFICIENT_BUFFER;
				}
			else if ( ( fmt_res != STRSAFE_E_INSUFFICIENT_BUFFER ) && ( FAILED( fmt_res ) ) ) {
				chars_written = 0;
				write_BAD_FMT( psz_formatted_LONGLONG_HUMAN, chars_written );
				}
			return fmt_res;
			}
		write_BAD_FMT( psz_formatted_LONGLONG_HUMAN, chars_written );
		return res;
		}


}

_Success_( SUCCEEDED( return ) ) HRESULT FormatBytes( _In_ const std::uint64_t n, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_formatted_bytes, _In_range_( 38, 64 ) const rsize_t strSize, _Out_ rsize_t& chars_written ) {
	auto res = CStyle_FormatLongLongHuman( n, psz_formatted_bytes, strSize, chars_written );
	if ( !SUCCEEDED( res ) ) {
		write_BAD_FMT( psz_formatted_bytes, chars_written );
		return res;
		}
	return res;
	}


_Success_( SUCCEEDED( return ) ) HRESULT CStyle_FormatLongLongHuman( _In_ std::uint64_t n, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_formatted_LONGLONG_HUMAN, _In_range_( 8, 64 ) const rsize_t strSize, _Out_ rsize_t& chars_written ) {
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

	if (      ( TB != 0 ) || ( GB == BASE - 1 ) && ( MB >= HALF_BASE ) ) {
		return CStyle_FormatLongLongHuman_TB( psz_formatted_LONGLONG_HUMAN, strSize, chars_written, GB, TB );
		}
	else if ( ( GB != 0 ) || ( MB == BASE - 1 ) && ( KB >= HALF_BASE ) ) {
		return CStyle_FormatLongLongHuman_GB( psz_formatted_LONGLONG_HUMAN, strSize, chars_written, MB, GB );
		}
	else if ( ( MB != 0 ) || ( KB == BASE - 1 ) && (  B >= HALF_BASE ) ) {
		return CStyle_FormatLongLongHuman_MB( psz_formatted_LONGLONG_HUMAN, strSize, chars_written, KB, MB );
		}
	else if (   KB != 0 ) {
		return CStyle_FormatLongLongHuman_KB( psz_formatted_LONGLONG_HUMAN, strSize, chars_written, B, KB );
		}
	else if (    B != 0 ) {
		return CStyle_FormatLongLongHuman_B( psz_formatted_LONGLONG_HUMAN, strSize, chars_written, B );
		}
	return CStyle_FormatLongLongHuman_0( psz_formatted_LONGLONG_HUMAN, strSize, chars_written );
	}

//maximum representable integral component of a double SEEMS to be 15 characters long, so we need at least 17
_Success_( SUCCEEDED( return ) ) HRESULT CStyle_FormatDouble( _In_ const DOUBLE d, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_formatted_double, _In_range_( 17, 64 ) const rsize_t strSize, _Out_ rsize_t& chars_written ) {
	rsize_t chars_remaining = 0;
	const HRESULT fmt_res = StringCchPrintfExW( psz_formatted_double, strSize, NULL, &chars_remaining, 0, L"%.1f", d );
	if ( SUCCEEDED( fmt_res ) ) {
		chars_written = ( strSize - chars_remaining );
		return fmt_res;
		}
	else if ( fmt_res == STRSAFE_E_INSUFFICIENT_BUFFER ) {
		chars_written = strSize;
		return fmt_res;
		}
	chars_written = 0;
	return fmt_res;
	}


_Success_( SUCCEEDED( return ) ) HRESULT CStyle_FormatFileTime( _In_ const FILETIME t, _Out_writes_z_( strSize ) _Pre_writable_size_( strSize ) PWSTR psz_formatted_datetime, _In_range_( 128, 2048 ) const rsize_t strSize, _Out_ rsize_t& chars_written ) {
	ASSERT( &t != NULL );
	SYSTEMTIME st;
	if ( !FileTimeToSystemTime( &t, &st ) ) {
		return file_time_to_system_time_err( psz_formatted_datetime, strSize, chars_written );
		}
	LCID lcid = MAKELCID( GetUserDefaultLangID( ), SORT_DEFAULT );

	const int gdfres = GetDateFormatW( lcid, DATE_SHORTDATE, &st, NULL, psz_formatted_datetime, static_cast<int>( strSize ) );
	ensure_valid_return_date( gdfres, strSize );
	chars_written = static_cast<rsize_t>( gdfres );

	//if we have room for two spaces and a null:
	if ( ( gdfres + 3 ) < static_cast<int>( strSize ) ) {
		psz_formatted_datetime[ gdfres - 1 ] = L' ';
		psz_formatted_datetime[ gdfres     ] = L' ';
		psz_formatted_datetime[ gdfres + 1 ] = 0;
		chars_written = static_cast<rsize_t>( gdfres + 1 );
		}
	else {
		return STRSAFE_E_INSUFFICIENT_BUFFER;
		}

	const int gtfres = GetTimeFormatW( lcid, 0, &st, NULL, ( psz_formatted_datetime + chars_written ), static_cast<int>( strSize - chars_written ) );
	ensure_valid_return_time( gtfres, strSize );

	chars_written += gtfres;
	chars_written -= 1;

	/*
	This function returns 0 if it does not succeed. To get extended error information, the application can call GetLastError, which can return one of the following error codes:
		ERROR_INSUFFICIENT_BUFFER. A supplied buffer size was not large enough, or it was incorrectly set to NULL.
		ERROR_INVALID_FLAGS.       The values supplied for flags were not valid.
		ERROR_INVALID_PARAMETER.   Any of the parameter values was invalid.	
	*/

#ifdef DEBUG

	const rsize_t psz_size = 36;
	wchar_t psz_date_wchar[ psz_size ] = { 0 };
	const auto gdfres_dbg = GetDateFormatW( lcid, DATE_SHORTDATE, &st, NULL, psz_date_wchar, psz_size );
	ensure_valid_return_date( gdfres_dbg, psz_size );

	wchar_t psz_time_wchar[ psz_size ] = { 0 };
	const auto gtfres_dbg = GetTimeFormatW( lcid, 0, &st, NULL, psz_time_wchar, psz_size );
	ensure_valid_return_time( gtfres_dbg, psz_size );


	wchar_t psz_datetime_wchar[ psz_size ] = { 0 };
	rsize_t remaining_chars = 0;
	const HRESULT fmt_res = StringCchPrintfExW( psz_datetime_wchar, psz_size, NULL, &remaining_chars, 0, L"%s  %s", psz_date_wchar, psz_time_wchar );
	ASSERT( SUCCEEDED( fmt_res ) );
	ASSERT( wcscmp( psz_datetime_wchar, psz_formatted_datetime ) == 0 );
	return S_OK;
#else
	return S_OK;
#endif
	//return fmt_res;
	}
_Success_( SUCCEEDED( return ) ) HRESULT CStyle_FormatAttributes( _In_ const attribs& attr, _Out_writes_z_( strSize ) _Pre_writable_size_( strSize ) PWSTR psz_formatted_attributes, _In_range_( 6, 18 ) const rsize_t strSize, _Out_ rsize_t& chars_written  ) {
	if ( attr.invalid ) {
		psz_formatted_attributes[ 0 ] = L'?';
		psz_formatted_attributes[ 1 ] = L'?';
		psz_formatted_attributes[ 2 ] = L'?';
		psz_formatted_attributes[ 3 ] = L'?';
		psz_formatted_attributes[ 4 ] = L'?';
		psz_formatted_attributes[ 5 ] =   0;
		psz_formatted_attributes[ 6 ] =   0;
		chars_written = 5;
		return S_OK;
		}
	const auto alt_errCode = swprintf_s( psz_formatted_attributes, strSize, L"%s%s%s%s%s", ( ( attr.readonly ) ? L"R" : L"" ),  ( ( attr.hidden ) ? L"H" : L"" ),  ( ( attr.system ) ? L"S" : L"" ),  ( ( attr.compressed ) ? L"C" : L"" ), ( ( attr.encrypted ) ? L"E" : L"" ) );
	if ( alt_errCode == -1 ) {
		return STRSAFE_E_INVALID_PARAMETER;
		}
	ASSERT( alt_errCode >= 0 );
	chars_written = static_cast<rsize_t>( alt_errCode );
	return S_OK;
	}

//
_Success_( SUCCEEDED( return ) ) HRESULT CStyle_GetNumberFormatted( const std::int64_t number, _Pre_writable_size_( strSize ) PWSTR psz_formatted_number, _In_range_( 21, 64 ) const rsize_t strSize, _Out_ rsize_t& chars_written ) {
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

	const rsize_t bufSize = 66;

	wchar_t number_str_buffer[ bufSize ] = { 0 };
	//const auto sw_printf_s_res = swprintf_s( number_str_buffer, L"%I64d", number );
	rsize_t chars_remaining = 0;

	const HRESULT strsafe_printf_res = StringCchPrintfExW( number_str_buffer, bufSize, NULL, &chars_remaining, 0, L"%I64d", number );
	if ( !SUCCEEDED( strsafe_printf_res ) ) {
		
		if ( strsafe_printf_res == STRSAFE_E_INSUFFICIENT_BUFFER ) {
			displayWindowsMsgBoxWithMessage( L"STRSAFE_E_INSUFFICIENT_BUFFER in CStyle_GetNumberFormatted!(aborting)" );
			}
		if ( strsafe_printf_res == STRSAFE_E_END_OF_FILE ) {
			displayWindowsMsgBoxWithMessage( L"STRSAFE_E_END_OF_FILE in CStyle_GetNumberFormatted!(aborting)" );
			}
		if ( strsafe_printf_res == STRSAFE_E_INVALID_PARAMETER ) {
			displayWindowsMsgBoxWithMessage( L"STRSAFE_E_INVALID_PARAMETER in CStyle_GetNumberFormatted!(aborting)" );
			}
		else {
			displayWindowsMsgBoxWithMessage( L"Unknown error in CStyle_GetNumberFormatted!(aborting)" );
			}

		
		std::wstring err_str( L"DEBUGGING INFO: bufSize: " );
		err_str += std::to_wstring( bufSize );
		err_str += L", number: ";
		err_str += std::to_wstring( number );
		displayWindowsMsgBoxWithMessage( err_str.c_str( ) );


		std::terminate( );
		}


	NUMBERFMT format_struct;
	format_struct.NumDigits = 0;
	format_struct.LeadingZero = 0;
	format_struct.Grouping = 3;
	format_struct.lpDecimalSep = L".";
	format_struct.lpThousandSep = L",";
	format_struct.NegativeOrder = 1;


	//0 indicates failure! http://msdn.microsoft.com/en-us/library/windows/desktop/dd318113.aspx
	const auto get_number_fmt_ex_res = GetNumberFormatEx( NULL, 0, number_str_buffer, &format_struct, psz_formatted_number, static_cast<int>( strSize ) );
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
				displayWindowsMsgBoxWithMessage( L"Unexpected error in CStyle_GetNumberFormatted, after GetNumberFormatEx!(aborting)" );
				displayWindowsMsgBoxWithError( last_err );
				std::terminate( );
			}
		ASSERT( false );
		displayWindowsMsgBoxWithMessage( L"Unintended execution in CStyle_GetNumberFormatted, after GetNumberFormatEx!(aborting!)" );
		std::terminate( );
		}
	ASSERT( get_number_fmt_ex_res > 0 );
	chars_written = static_cast<rsize_t>( get_number_fmt_ex_res - 1u );
	ASSERT( chars_written == wcslen( psz_formatted_number ) );
	return S_OK;
	}

//_Success_( SUCCEEDED( return ) )
//const HRESULT allocate_and_copy_name_str( _Deref_pre_invalid_ _Outref_ _Deref_post_z_ _Deref_post_cap_( new_name_length ) wchar_t*& new_name_ptr, _In_range_( 0, UINT16_MAX ) const rsize_t& new_name_length, const std::wstring& name ) {
//	ASSERT( new_name_length < UINT16_MAX );
//	new_name_ptr = new wchar_t[ new_name_length + 1u ];
//	PWSTR pszend = NULL;
//	rsize_t chars_remaining = new_name_length;
//	const HRESULT res = StringCchCopyExW( new_name_ptr, new_name_length, name.c_str( ), &pszend, &chars_remaining, 0 );
//	if ( SUCCEEDED( res ) ) {
//		ASSERT( wcslen( new_name_ptr ) == new_name_length );
//		ASSERT( wcscmp( new_name_ptr, name.c_str( ) ) == 0 );
//		ASSERT( ( std::ptrdiff_t( pszend ) - std::ptrdiff_t( new_name_ptr ) ) == new_name_length );
//		}
//	return res;
//	//const auto cpy_res = wcscpy_s( new_name_ptr, ( new_name_length + 1u ), name.c_str( ) );
//	//if ( cpy_res != 0 ) {
//		//std::terminate( );
//		//}
//	//ASSERT( wcslen( new_name_ptr ) == new_name_length );
//	//ASSERT( wcscmp( new_name_ptr, name.c_str( ) ) == 0 );
//
//	}


_Success_( SUCCEEDED( return ) )
const HRESULT allocate_and_copy_name_str( _Pre_invalid_ _Post_z_ _Post_readable_size_( new_name_length ) wchar_t*& new_name_ptr, _In_ _In_range_( 0, UINT16_MAX ) const rsize_t& new_name_length, const std::wstring& name ) {
	ASSERT( new_name_length < UINT16_MAX );
	new_name_ptr = new wchar_t[ new_name_length + 2u ];
	PWSTR pszend = NULL;
	rsize_t chars_remaining = new_name_length;
	const HRESULT res = StringCchCopyExW( new_name_ptr, ( new_name_length + 1u ), name.c_str( ), &pszend, &chars_remaining, 0 );
	ASSERT( SUCCEEDED( res ) );
	if ( SUCCEEDED( res ) ) {
#ifdef DEBUG
		ASSERT( wcslen( new_name_ptr ) == new_name_length );
		ASSERT( wcscmp( new_name_ptr, name.c_str( ) ) == 0 );
		const auto da_ptrdiff = ( std::ptrdiff_t( pszend ) - std::ptrdiff_t( new_name_ptr ) );
		ASSERT( ( da_ptrdiff / sizeof( wchar_t ) ) == new_name_length );
#endif
		}
	else {
		displayWindowsMsgBoxWithMessage( L"Copy of name_str failed!!!" );
		std::terminate( );
		}
	ASSERT( SUCCEEDED( res ) );
	return res;
	//const auto cpy_res = wcscpy_s( new_name_ptr, ( new_name_length + 1u ), name.c_str( ) );
	//if ( cpy_res != 0 ) {
		//std::terminate( );
		//}
	//ASSERT( wcslen( new_name_ptr ) == new_name_length );
	//ASSERT( wcscmp( new_name_ptr, name.c_str( ) ) == 0 );

	}



_Success_( return != false ) bool GetVolumeName( _In_z_ const PCWSTR rootPath, _Out_ _Post_z_ wchar_t ( &volumeName )[ MAX_PATH + 1u ] ) {
	const auto old = SetErrorMode( SEM_FAILCRITICALERRORS );
	
	//GetVolumeInformation returns 0 on failure
	const BOOL b = GetVolumeInformationW( rootPath, volumeName, MAX_PATH, NULL, NULL, NULL, NULL, 0 );

	if ( b == 0 ) {
		TRACE( _T( "GetVolumeInformation(%s) failed: %u\n" ), rootPath, GetLastError( ) );
		}
	SetErrorMode( old );
	
	return ( b != 0 );
	}

_Success_( return != false ) 
bool GetVolumeName( _In_z_ const PCWSTR rootPath ) {
	const auto old = SetErrorMode( SEM_FAILCRITICALERRORS );
	
	//GetVolumeInformation returns 0 on failure
	const BOOL b = GetVolumeInformationW( rootPath, NULL, 0, NULL, NULL, NULL, NULL, 0 );

	if ( b == 0 ) {
		TRACE( _T( "GetVolumeInformation(%s) failed: %u\n" ), rootPath, GetLastError( ) );
		}
	SetErrorMode( old );
	
	return ( b != 0 );
	}


void FormatVolumeName( _In_ const std::wstring& rootPath, _In_z_ PCWSTR volumeName, _Out_ _Post_z_ _Pre_writable_size_( MAX_PATH + 1u ) PWSTR formatted_volume_name ) {
	const HRESULT fmt_res = StringCchPrintfW( formatted_volume_name, ( MAX_PATH + 1u ), L"%s (%s)", volumeName, rootPath.substr( 0, 2 ).c_str( ) );
	if ( SUCCEEDED( fmt_res ) ) {
		return;
		}
	displayWindowsMsgBoxWithMessage( L"FormatVolumeName failed!" );
	std::terminate( );
	}

#pragma strict_gs_check(push, on)
_Success_( SUCCEEDED( return ) ) HRESULT GetFullPathName_WriteToStackBuffer( _In_z_ PCWSTR relativePath, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_full_path, _In_range_( 128, 512 ) const DWORD strSize, _Out_ rsize_t& chars_written ) {
	const DWORD dw = GetFullPathNameW( relativePath, strSize, psz_full_path, NULL );
	if ( dw == 0 ) {
		static_assert( !SUCCEEDED( E_FAIL ), "" );
		return E_FAIL;
		}
	if ( dw >= strSize ) {
		return STRSAFE_E_INSUFFICIENT_BUFFER;
		}
	if ( dw < strSize ) {
		ASSERT( dw == wcslen( psz_full_path ) );
		chars_written = dw;
		return S_OK;
		}
	ASSERT( false );
	return E_FAIL;
	}
#pragma strict_gs_check(pop)

std::wstring dynamic_GetFullPathName( _In_z_ PCWSTR relativePath ) {
	rsize_t path_len = MAX_PATH;
	auto pszPath = std::make_unique<wchar_t[ ]>( path_len );
	auto dw = GetFullPathNameW( relativePath, static_cast<DWORD>( path_len ), pszPath.get( ), NULL );
	while ( dw >= path_len ) {
		path_len *= 2;
		pszPath.reset( new wchar_t[ path_len ] );
		dw = GetFullPathNameW( relativePath, static_cast<DWORD>( path_len ), pszPath.get( ), NULL );
		}
	
	return std::wstring( pszPath.get( ) );
	}




void MyGetDiskFreeSpace( _In_z_ const PCWSTR pszRootPath, _Out_ _Out_range_( 0, 18446744073709551615 ) std::uint64_t& total, _Out_ _Out_range_( 0, 18446744073709551615 ) std::uint64_t& unused ) {
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

bool DriveExists( _In_z_ _In_reads_( path_len ) const PCWSTR path, _In_ _In_range_( 0, 4 ) const rsize_t path_len ) {
	//const auto path_ws = std::wstring( path );
	//ASSERT( path_ws.length( ) == wcslen( path ) );
	//ASSERT( path_ws.length( ) == path_len );
	//ASSERT( path_ws.compare( path ) == 0 );
	if ( path_len != 3 || path[ 1 ] != _T( ':' ) || path[ 2 ] != _T( '\\' ) ) {
		return false;
		}
	ASSERT( path_len >= 1 );
	const rsize_t size_ltr_str = 2;

	//auto left_1_char = path.Left( 1 );
	//const auto left_1_char_lower = left_1_char.MakeLower( );
	//wchar_t ltr[ size_ltr_str ] = { 0 };
	//ltr[ 0 ] = left_1_char_lower[ 0 ];
	//ltr[ 1 ] = 0;
	
	//const auto left_1_char_ws = path_ws.substr( 0, 1 );
	const auto left_1_char_ws = path[ 0 ];
	wchar_t ltr_ws[ size_ltr_str ] = { 0 };
	ltr_ws[ 0 ] = left_1_char_ws;
	ltr_ws[ 1 ] = 0;
	const auto result = _wcslwr_s( ltr_ws, size_ltr_str );
	ASSERT( result == 0 );
	if ( result != 0 ) {
		displayWindowsMsgBoxWithMessage( L"Failed to convert first letter of drive path to lowercase! (DriveExists)(aborting!)" );
		displayWindowsMsgBoxWithMessage( ltr_ws );
		return false;
		}

	//ASSERT( wcscmp( ltr_ws, ltr ) == 0 );

	static_assert( L'a' == 97, "wtf!" );
	const DWORD d = ltr_ws[ 0 ] - 97u;

#ifdef DEBUG
	const INT e = ltr_ws[ 0 ] - _T( 'a' );
	ASSERT( ( 0x1 << d ) == ( 0x1 << e ) );
#pragma warning(suppress:4389)
	ASSERT( ( 0x1 << d ) == ( 0x1u << d ) );
	ASSERT( INT( 0x1 << d ) == INT( 0x1u << d ) );
#pragma warning(suppress:4365)
	const DWORD mask_test_1 = 0x1 << d;
	const DWORD mask_test_2 = 0x1u << d;
	ASSERT( mask_test_1 == mask_test_2 );
#endif

	//const DWORD mask = 0x1 << d;
	const DWORD mask = 0x1u << d;

	if ( ( mask bitand GetLogicalDrives( ) ) == 0 ) {
		return false;
		}

	
	if ( !GetVolumeName( path ) ) {
		return false;
		}

	return true;
	}


_Success_( return ) bool MyQueryDosDevice( _In_z_ const PCWSTR drive, _Out_ _Post_z_ wchar_t ( &drive_info )[ 512u ] ) {
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


	if ( wcslen( drive ) < 2 || drive[ 1 ] != L':' ) {//parenthesis, maybe?
		return false;
		}

	const rsize_t left_two_chars_buffer_size = 3;
	wchar_t left_two_chars_buffer[ left_two_chars_buffer_size ] = { 0 };
	left_two_chars_buffer[ 0 ] = drive[ 0 ];
	left_two_chars_buffer[ 1 ] = drive[ 1 ];
	//left_two_chars_buffer[ 2 ] = drive[ 2 ];
	//d = d.Left( 2 );
	//ASSERT( d.Compare( left_two_chars_buffer ) == 0 );
	
	
	static_assert( ( sizeof( drive_info ) / sizeof( wchar_t ) ) == 512u, "" );
	const auto dw = QueryDosDeviceW( left_two_chars_buffer, drive_info, 512u );//eek
	//info.ReleaseBuffer( );

	if ( dw == 0 ) {
		const rsize_t error_buffer_size = 128;
		wchar_t error_buffer[ error_buffer_size ] = { 0 };
		rsize_t error_chars_written = 0;
		const DWORD error_code = GetLastError( );
		const HRESULT fmt_res = CStyle_GetLastErrorAsFormattedMessage( error_buffer, error_buffer_size, error_chars_written, error_code );
		if ( SUCCEEDED( fmt_res ) ) {
			TRACE( _T( "QueryDosDevice(%s) failed: %s\r\n" ), left_two_chars_buffer, error_buffer );
			}
		else {
			TRACE( _T( "QueryDosDevice(%s) failed. Couldn't get error message for code: %u\r\n" ), left_two_chars_buffer, error_code );
			}
		
		return false;
		}

	return true;
	}

bool IsSUBSTedDrive( _In_z_ const PCWSTR drive ) {
	/*
	  drive is a drive spec like C: or C:\ or C:\path (path is ignored).
	  This function returns true, if QueryDosDevice() is supported and drive is a SUBSTed drive.
	*/
	const rsize_t info_buffer_size = 512u;
	wchar_t drive_info[ info_buffer_size ] = { 0 };

	const bool query_res = MyQueryDosDevice( drive, drive_info );
	//ASSERT( info.Compare( drive_info ) == 0 );
	if ( query_res ) {
		wchar_t drive_info_left_4[ 5 ] = { 0 };
		drive_info_left_4[ 0 ] = drive_info[ 0 ];
		drive_info_left_4[ 1 ] = drive_info[ 1 ];
		drive_info_left_4[ 2 ] = drive_info[ 2 ];
		drive_info_left_4[ 3 ] = drive_info[ 3 ];

		return ( ( wcslen( drive_info ) >= 4 ) && ( wcscmp( drive_info_left_4, L"\\??\\" ) == 0 ) );
		}

	return false;
	}

const LARGE_INTEGER help_QueryPerformanceCounter( ) {
	LARGE_INTEGER doneTime;
	const BOOL behavedWell = QueryPerformanceCounter( &doneTime );
	ASSERT( behavedWell );
	if ( !behavedWell ) {
		WTL::AtlMessageBox( NULL, L"QueryPerformanceCounter failed!!", L"ERROR!", MB_OK );
		doneTime.QuadPart = -1;
		}
	return doneTime;
	}

const LARGE_INTEGER help_QueryPerformanceFrequency( ) {
	LARGE_INTEGER doneTime;
	const BOOL behavedWell = QueryPerformanceFrequency( &doneTime );
	ASSERT( behavedWell );
	if ( !behavedWell ) {
		WTL::AtlMessageBox( NULL, L"QueryPerformanceFrequency failed!!", L"ERROR!", MB_OK );
		doneTime.QuadPart = -1;
		}
	return doneTime;
	}




//On returning E_FAIL, call GetLastError for details. That's not my idea!
_Success_( SUCCEEDED( return ) ) HRESULT CStyle_GetLastErrorAsFormattedMessage( WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_formatted_error, _In_range_( 128, 32767 ) const rsize_t strSize, _Out_ rsize_t& chars_written, const DWORD error ) {
	//const auto err = GetLastError( );
	const auto err = error;
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
	ASSERT( wcslen( psz_fmt_msg ) == chars_written );
	}

void displayWindowsMsgBoxWithError( const DWORD error ) {
	const rsize_t err_msg_size = 1024;
	wchar_t err_msg[ err_msg_size ] = { 0 };
	rsize_t chars_written = 0;

	const HRESULT err_res = CStyle_GetLastErrorAsFormattedMessage( err_msg, err_msg_size, chars_written, error );
	if ( SUCCEEDED( err_res ) ) {
		WTL::AtlMessageBox( NULL, err_msg, TEXT( "Error" ), MB_OK );
		TRACE( _T( "Error: %s\r\n" ), err_msg );
		return;
		}

	const rsize_t err_msg_size_2 = 4096;
	wchar_t err_msg_2[ err_msg_size_2 ] = { 0 };
	rsize_t chars_written_2 = 0;
	const HRESULT err_res_2 = CStyle_GetLastErrorAsFormattedMessage( err_msg_2, err_msg_size_2, chars_written_2, error );
	if ( SUCCEEDED( err_res_2 ) ) {
		WTL::AtlMessageBox( NULL, err_msg_2, TEXT( "Error" ), MB_OK );
		TRACE( _T( "Error: %s\r\n" ), err_msg_2 );
		return;
		}
	TRACE( _T( "Error while getting error message!\r\n" ), err_msg_2 );
	WTL::AtlMessageBox( NULL, _T( "Error while getting error message!\r\n" ), TEXT( "Error" ), MB_OK );
	}

void displayWindowsMsgBoxWithMessage( const std::wstring message ) {
	//MessageBoxW( NULL, message.c_str( ), TEXT( "Error" ), MB_OK );
	WTL::AtlMessageBox( NULL, message.c_str( ), L"Error", MB_OK | MB_ICONINFORMATION );
	TRACE( _T( "Error: %s\r\n" ), message.c_str( ) );
	}

void displayWindowsMsgBoxWithMessage( PCWSTR message ) {
	WTL::AtlMessageBox( NULL, message, TEXT( "Error" ), MB_OK );
	TRACE( _T( "Error: %s\r\n" ), message );
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
	out.left   = static_cast<LONG>( in.left );
	out.top    = static_cast<LONG>( in.top );
	out.right  = static_cast<LONG>( in.right );
	out.bottom = static_cast<LONG>( in.bottom );
	ASSERT( out.left == in.left );
	ASSERT( out.top == in.top );
	ASSERT( out.right == in.right );
	ASSERT( out.bottom == in.bottom );
	out.NormalizeRect( );
	ASSERT( out.right >= out.left );
	ASSERT( out.bottom >= out.top );
	return out;
	}


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


//BUGBUG: TODO: "Do not cast a pointer to a FILETIME structure to either a ULARGE_INTEGER* or __int64* value because it can cause alignment faults on 64-bit Windows." - http://msdn.microsoft.com/en-us/library/ms724284%28VS.85%29.aspx
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
