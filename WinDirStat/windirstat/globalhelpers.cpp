// globalhelpers.cpp - Implementation of global helper functions
//
// see `file_header_text.txt` for licensing & contact info.

#pragma once

#include "stdafx.h"

#ifndef WDS_GLOBALHELPERS_CPP
#define WDS_GLOBALHELPERS_CPP

#include "globalhelpers.h"

#pragma warning(3:4514) //'function': unreferenced inline function has been removed


#define BASE 1024
#define HALF_BASE BASE/2
namespace {

	const PCWSTR date_time_format_locale_name_str = LOCALE_NAME_INVARIANT;
	const DWORD GetDateFormatEx_flags             = DATE_SHORTDATE;
	const DWORD GetTimeFormatEx_flags             = 0;
	const double COLOR_MAX_VALUE = 255.0;

	std::wstring Format_uint64_t_Normal( _In_ std::uint64_t n ) {
		// Returns formatted number like "123.456.789".
		// 18446744073709551615 is max
		//                     ^ 20 characters
		// 18,446,744,073,709,551,615
		//                           ^26 characters
		//                            26 + null terminator = 27
		//const rsize_t number_formatted_buffer_size = 28;
		//wchar_t buffer[ number_formatted_buffer_size ] = { 0 };

		std::wstring all_ws;
		all_ws.reserve( 27 );

		do
		{
			const auto rest = static_cast<INT>( n % 1000 );
			n /= 1000;
			const rsize_t tempBuf_size = 10u;
			_Null_terminated_ wchar_t tempBuf[ tempBuf_size ] = { 0 };
			if ( n > 0 ) {
				const HRESULT fmt_res = StringCchPrintfW( tempBuf, tempBuf_size, L",%03d", rest );
				ASSERT( SUCCEEDED( fmt_res ) );
				if ( !SUCCEEDED( fmt_res ) ) {
					return L"FORMATTING FAILED!";
					}
				all_ws += tempBuf;
				}
			else {
				all_ws += std::to_wstring( rest );
				//const HRESULT fmt_res = StringCchPrintfW( tempBuf, tempBuf_size, L"%d", rest );
				//ASSERT( SUCCEEDED( fmt_res ) );
				//if ( !SUCCEEDED( fmt_res ) ) {
				//	return L"FORMATTING FAILED!";
				//	}
				}
			//all_ws += tempBuf;
			} while ( n > 0 );
		return all_ws;
		}

	//void get_date_format_err_double_fault( _In_ const SYSTEMTIME system_time, _In_ const HRESULT failed_fmt_res ) {
	//	displayWindowsMsgBoxWithMessage( L"GetDateFormatEx/get_date_format_err double faulted!!!" );
	//	std::terminate( );
	//	}

	void display_SYSTEMTIME_debugging_info( _In_ const SYSTEMTIME system_time ) {
		std::wstring fmt_str( L"WDS: GetDateFormatEx failed, given this SYSTEMTIME:\r\n\t" );
		fmt_str += L"year: ";
		fmt_str += std::to_wstring( static_cast< unsigned int >( system_time.wYear ) );
		fmt_str += L"\r\n\t";
		fmt_str += L"month: ";
		fmt_str += std::to_wstring( static_cast< unsigned int >( system_time.wMonth ) );
		fmt_str += L"\r\n\t";
		fmt_str += L"day of week: ";
		fmt_str += std::to_wstring( static_cast< unsigned int >( system_time.wDayOfWeek ) );
		fmt_str += L"\r\n\t";
		fmt_str += L"day: ";
		fmt_str += std::to_wstring( static_cast< unsigned int >( system_time.wDay ) );
		fmt_str += L"\r\n\t";
		fmt_str += L"hour: ";
		fmt_str += std::to_wstring( static_cast< unsigned int >( system_time.wHour ) );
		fmt_str += L"\r\n\t";
		fmt_str += L"minute: ";
		fmt_str += std::to_wstring( static_cast< unsigned int >( system_time.wMinute ) );
		fmt_str += L"\r\n\t";
		fmt_str += L"second: ";
		fmt_str += std::to_wstring( static_cast< unsigned int >( system_time.wSecond ) );
		fmt_str += L"\r\n\t";
		fmt_str += L"millisecond: ";
		fmt_str += std::to_wstring( static_cast< unsigned int >( system_time.wMilliseconds ) );
		fmt_str += L"\r\n";
		const std::wstring& fmt_str_finished = fmt_str;
		OutputDebugStringW( fmt_str_finished.c_str( ) );
		displayWindowsMsgBoxWithMessage( fmt_str_finished );
		}

	_Must_inspect_result_ 
	const bool display_GetDateFormatEx_flags( _In_ const DWORD flags ) {
		if ( ( flags bitand DATE_LONGDATE ) ) {
			displayWindowsMsgBoxWithMessage( L"GetDateFormatEx passed DATE_LONGDATE" );
			return true;
			}
		if ( ( flags bitand DATE_LTRREADING ) ) {
			displayWindowsMsgBoxWithMessage( L"GetDateFormatEx passed DATE_LTRREADING" );
			return true;
			}
		if ( ( flags bitand DATE_RTLREADING ) ) {
			displayWindowsMsgBoxWithMessage( L"GetDateFormatEx passed DATE_RTLREADING" );
			return true;
			}
		if ( ( flags bitand DATE_SHORTDATE ) ) {
			displayWindowsMsgBoxWithMessage( L"GetDateFormatEx passed DATE_SHORTDATE" );
			return true;
			}
		if ( ( flags bitand DATE_USE_ALT_CALENDAR ) ) {
			displayWindowsMsgBoxWithMessage( L"GetDateFormatEx passed DATE_USE_ALT_CALENDAR" );
			return true;
			}
		if ( ( flags bitand DATE_YEARMONTH ) ) {
			displayWindowsMsgBoxWithMessage( L"GetDateFormatEx passed DATE_YEARMONTH" );
			return true;
			}
		return false;
		}

	_Must_inspect_result_ 
	const bool display_GetTimeFormatEx_flags( _In_ const DWORD flags ) {
		if ( ( flags bitand TIME_NOMINUTESORSECONDS ) ) {
			displayWindowsMsgBoxWithMessage( L"GetTimeFormatEx passed TIME_NOMINUTESORSECONDS" );
			return true;
			}
		if ( ( flags bitand TIME_NOSECONDS ) ) {
			displayWindowsMsgBoxWithMessage( L"GetTimeFormatEx passed TIME_NOSECONDS" );
			return true;
			}
		if ( ( flags bitand TIME_NOTIMEMARKER ) ) {
			displayWindowsMsgBoxWithMessage( L"GetTimeFormatEx passed TIME_NOTIMEMARKER" );
			return true;
			}
		if ( ( flags bitand TIME_FORCE24HOURFORMAT ) ) {
			displayWindowsMsgBoxWithMessage( L"GetTimeFormatEx passed TIME_FORCE24HOURFORMAT" );
			return true;
			}
		return false;
		}

	//continue if returns true?
	_Must_inspect_result_ _Success_( return == true ) //_In_range_ upper bound is totally arbitrary! Lower bound is enough for WRITE_BAD_FMT
	const bool get_date_format_err( _In_ const SYSTEMTIME system_time, _In_ _In_range_( 24, 2048 ) const rsize_t str_size, _In_ const DWORD flags ) {
		const auto err = GetLastError( );

		//sorry, this is ugly, isn't it?
		display_SYSTEMTIME_debugging_info( system_time );

		if ( err == ERROR_INSUFFICIENT_BUFFER ) {
			TRACE( _T( "%s\r\n" ), global_strings::get_date_format_buffer_err );
			displayWindowsMsgBoxWithMessage( global_strings::get_date_format_buffer_err );
			const std::wstring buffer_size( L"size of (the insufficient) buffer: " + std::to_wstring( str_size ) );
			displayWindowsMsgBoxWithMessage( buffer_size );
			//std::terminate( );
			return true;
			}
		if ( err == ERROR_INVALID_FLAGS ) {
			TRACE( _T( "%s\r\n" ), global_strings::get_date_format_flags_err );
			displayWindowsMsgBoxWithMessage( global_strings::get_date_format_flags_err );
			return display_GetDateFormatEx_flags( flags );
			}
		if ( err == ERROR_INVALID_PARAMETER ) {
			TRACE( _T( "%s\r\n" ), global_strings::get_date_format_param_err );
			displayWindowsMsgBoxWithMessage( global_strings::get_date_format_param_err );
			return display_GetDateFormatEx_flags( flags );
			//std::terminate( );
			}
		ASSERT( ( err == ERROR_INSUFFICIENT_BUFFER ) || ( err == ERROR_INVALID_FLAGS ) || ( err == ERROR_INVALID_PARAMETER ) );
		displayWindowsMsgBoxWithMessage( L"GetDateFormat failed, and GetLastError returned an unexpected error code!" );
		std::terminate( );
		return false;
		}

	//continue if returns true?
	_Must_inspect_result_ _Success_( return == true )
	const bool get_time_format_err( _In_ const rsize_t str_size, _In_ const DWORD flags ) {
		const auto err = GetLastError( );
		if ( err == ERROR_INSUFFICIENT_BUFFER ) {
			TRACE( _T( "%s\r\n" ), global_strings::get_time_format_buffer_err );
			displayWindowsMsgBoxWithMessage( global_strings::get_time_format_buffer_err );
			const std::wstring buffer_size( L"size of (the insufficient) buffer: " + std::to_wstring( str_size ) );
			displayWindowsMsgBoxWithMessage( buffer_size );
			return true;
			}
		if ( err == ERROR_INVALID_FLAGS ) {
			TRACE( _T( "%s\r\n" ), global_strings::get_time_format_flags_err );
			displayWindowsMsgBoxWithMessage( global_strings::get_time_format_flags_err );
			return display_GetTimeFormatEx_flags( flags );

			//std::terminate( );
			}
		if ( err == ERROR_INVALID_PARAMETER ) {
			TRACE( _T( "%s\r\n" ), global_strings::get_time_format_param_err );
			displayWindowsMsgBoxWithMessage( global_strings::get_time_format_param_err );
			return true;
			}

		if ( err == ERROR_OUTOFMEMORY ) {
			TRACE( _T( "%s\r\n" ), global_strings::get_time_format_err_OUTOFMEMORY );
			displayWindowsMsgBoxWithMessage( global_strings::get_time_format_err_OUTOFMEMORY );
			std::terminate( );
			return false;
			}

		ASSERT( ( err == ERROR_INSUFFICIENT_BUFFER ) || ( err == ERROR_INVALID_FLAGS ) || ( err == ERROR_INVALID_PARAMETER ) || ( err == ERROR_OUTOFMEMORY ) );
		displayWindowsMsgBoxWithMessage( L"FileTimeToSystemTime failed, and GetLastError returned an unexpected error code!" );
		std::terminate( );
		return false;
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

	//_In_range_ upper bound is totally arbitrary! Lower bound is enough for WRITE_BAD_FMT
	void ensure_valid_return_date( _In_ const int gdfres, _In_ _In_ _In_range_( 24, 2048 ) const rsize_t strSize, _In_ const SYSTEMTIME system_time, _Inout_ PWSTR psz_formatted_datetime ) {
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
			const bool we_continue = get_date_format_err( system_time, strSize, GetDateFormatEx_flags );
			if ( we_continue ) {
				rsize_t dummy_var = 0;
				wds_fmt::write_BAD_FMT( psz_formatted_datetime, dummy_var );
				return;
				}
			std::terminate( );
			}
		std::terminate( );
		}

	void ensure_valid_return_time( const int gtfres, const rsize_t strSize, _Inout_ PWSTR psz_formatted_datetime ) {
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
			const bool we_continue = get_time_format_err( strSize, GetTimeFormatEx_flags );
			if ( we_continue ) {
				rsize_t dummy_var = 0;
				wds_fmt::write_BAD_FMT( psz_formatted_datetime, dummy_var );
				return;
				}
			std::terminate( );
			}
		std::terminate( );
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
		wds_fmt::write_BAD_FMT( psz_formatted_LONGLONG_HUMAN, chars_written );
		return res;
		}

	_Success_( SUCCEEDED( return ) ) _Pre_satisfies_( chars_written == 0 )
	HRESULT CStyle_FormatLongLongHuman_KB( WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_formatted_LONGLONG_HUMAN, _In_range_( 23, 64 ) const rsize_t strSize, _Out_ rsize_t& chars_written, _In_ const DOUBLE B, _In_ const DOUBLE KB ) {
		const rsize_t number_formatted_buffer_size = 19;
		_Null_terminated_ wchar_t buffer[ number_formatted_buffer_size ] = { 0 };
		rsize_t buffer_chars_written = 0;
		const HRESULT res = wds_fmt::CStyle_FormatDouble( KB + B / BASE, buffer, number_formatted_buffer_size, buffer_chars_written );
		if ( !SUCCEEDED( res ) ) {
			wds_fmt::write_BAD_FMT( psz_formatted_LONGLONG_HUMAN, chars_written );
			return res;
			}
		ASSERT( SUCCEEDED( res ) );
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
			wds_fmt::write_BAD_FMT( psz_formatted_LONGLONG_HUMAN, chars_written );
			return fmt_res;
			}
		return fmt_res;
		}

	_Success_( SUCCEEDED( return ) ) HRESULT CStyle_FormatLongLongHuman_MB( WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_formatted_LONGLONG_HUMAN, _In_range_( 23, 64 ) const rsize_t strSize, _Out_ rsize_t& chars_written, _In_ const DOUBLE KB, _In_ const DOUBLE MB ) {
		const rsize_t number_formatted_buffer_size = 19;
		_Null_terminated_ wchar_t buffer[ number_formatted_buffer_size ] = { 0 };
		rsize_t buffer_chars_written = 0;
		const HRESULT res = wds_fmt::CStyle_FormatDouble( MB + KB / BASE, buffer, number_formatted_buffer_size, buffer_chars_written );
		if ( !SUCCEEDED( res ) ) {
			wds_fmt::write_BAD_FMT( psz_formatted_LONGLONG_HUMAN, chars_written );
			return res;
			}
		ASSERT( SUCCEEDED( res ) );
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
			wds_fmt::write_BAD_FMT( psz_formatted_LONGLONG_HUMAN, chars_written );
			return fmt_res;
			}
		return fmt_res;
		}

	_Success_( SUCCEEDED( return ) ) HRESULT CStyle_FormatLongLongHuman_GB( WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_formatted_LONGLONG_HUMAN, _In_range_( 8, 64 ) const rsize_t strSize, _Out_ rsize_t& chars_written, _In_ const DOUBLE MB, _In_ const DOUBLE GB ) {
		const rsize_t number_formatted_buffer_size = 19;
		_Null_terminated_ wchar_t buffer[ number_formatted_buffer_size ] = { 0 };
		rsize_t buffer_chars_written = 0;
		const HRESULT res = wds_fmt::CStyle_FormatDouble( GB + MB / BASE, buffer, number_formatted_buffer_size, buffer_chars_written );
		if ( !SUCCEEDED( res ) ) {
			wds_fmt::write_BAD_FMT( psz_formatted_LONGLONG_HUMAN, chars_written );
			return res;
			}
		ASSERT( SUCCEEDED( res ) );
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
			wds_fmt::write_BAD_FMT( psz_formatted_LONGLONG_HUMAN, chars_written );
			return fmt_res;
			}
		return fmt_res;
		}

	_Success_( SUCCEEDED( return ) ) HRESULT CStyle_FormatLongLongHuman_TB( WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_formatted_LONGLONG_HUMAN, _In_range_( 8, 64 ) const rsize_t strSize, _Out_ rsize_t& chars_written, _In_ const DOUBLE GB, _In_ const DOUBLE TB ) {
		const rsize_t number_formatted_buffer_size = 19;
		_Null_terminated_ wchar_t buffer[ number_formatted_buffer_size ] = { 0 };
		rsize_t buffer_chars_written = 0;
		const HRESULT res = wds_fmt::CStyle_FormatDouble( TB + GB / BASE, buffer, number_formatted_buffer_size, buffer_chars_written );
		if ( !SUCCEEDED( res ) ) {
			wds_fmt::write_BAD_FMT( psz_formatted_LONGLONG_HUMAN, chars_written );
			return res;
			}
		ASSERT( SUCCEEDED( res ) );
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
			wds_fmt::write_BAD_FMT( psz_formatted_LONGLONG_HUMAN, chars_written );
			return fmt_res;
			}
		return fmt_res;
		}

	void convert_number_to_string_failed_display_debugging_info( _In_range_( 19, 128 ) const rsize_t bufSize, _In_ const std::int64_t number ) {
		const std::wstring err_str( L"DEBUGGING INFO: bufSize: " + std::to_wstring( bufSize ) + L", number: " + std::to_wstring( number ) );
		displayWindowsMsgBoxWithMessage( err_str.c_str( ) );
		std::terminate( );
		}

	void convert_number_to_string_failed( _In_range_( 19, 128 ) const rsize_t bufSize, _In_ const std::int64_t number, _In_ const HRESULT strsafe_printf_res ) {
		if ( strsafe_printf_res == STRSAFE_E_INSUFFICIENT_BUFFER ) {
			displayWindowsMsgBoxWithMessage( L"STRSAFE_E_INSUFFICIENT_BUFFER in CStyle_GetNumberFormatted!(aborting)" );
			return convert_number_to_string_failed_display_debugging_info( bufSize, number );
			}
		if ( strsafe_printf_res == STRSAFE_E_END_OF_FILE ) {
			displayWindowsMsgBoxWithMessage( L"STRSAFE_E_END_OF_FILE in CStyle_GetNumberFormatted!(aborting)" );
			return convert_number_to_string_failed_display_debugging_info( bufSize, number );
			}
		if ( strsafe_printf_res == STRSAFE_E_INVALID_PARAMETER ) {
			displayWindowsMsgBoxWithMessage( L"STRSAFE_E_INVALID_PARAMETER in CStyle_GetNumberFormatted!(aborting)" );
			return convert_number_to_string_failed_display_debugging_info( bufSize, number );
			}
		displayWindowsMsgBoxWithMessage( L"Unknown error in CStyle_GetNumberFormatted!(aborting)" );
		return convert_number_to_string_failed_display_debugging_info( bufSize, number );
		}

	void convert_number_to_string( _In_range_( 19, 128 ) const rsize_t bufSize, _Pre_writable_size_( bufSize ) _Post_z_ PWSTR number_str_buffer, _In_ const std::int64_t number ) {
		rsize_t chars_remaining = 0;

		const HRESULT strsafe_printf_res = StringCchPrintfExW( number_str_buffer, bufSize, NULL, &chars_remaining, 0, L"%I64d", number );
		if ( SUCCEEDED( strsafe_printf_res ) ) {
			return;
			}
		ASSERT( !SUCCEEDED( strsafe_printf_res ) );
		
		convert_number_to_string_failed( bufSize, number, strsafe_printf_res );
		}


}

Children_String_Heap_Manager::Children_String_Heap_Manager( const rsize_t number_of_characters_needed ) : m_buffer_size( number_of_characters_needed ), m_buffer_filled( 0 ), m_string_buffer( new wchar_t[ number_of_characters_needed ] ) { }

_Success_( SUCCEEDED( return ) )
const HRESULT Children_String_Heap_Manager::copy_name_str_into_buffer( _Pre_invalid_ _Post_z_ _Post_readable_size_( new_name_length ) wchar_t*& new_name_ptr, _In_ _In_range_( 0, UINT16_MAX ) const rsize_t& new_name_length, const std::wstring& name ) {
	ASSERT( new_name_length < UINT16_MAX );
	new_name_ptr = ( m_string_buffer.get( ) + m_buffer_filled );
	ASSERT( ( m_buffer_filled + new_name_length ) < m_buffer_size );
	m_buffer_filled += new_name_length;

	PWSTR pszend = NULL;

	//god this is ugly.
	const rsize_t buffer_space_remaining = ( m_buffer_size - m_buffer_filled + new_name_length );

	rsize_t chars_remaining = buffer_space_remaining;
	const HRESULT res = StringCchCopyExW( new_name_ptr, ( buffer_space_remaining ), name.c_str( ), &pszend, &chars_remaining, 0 );
	ASSERT( SUCCEEDED( res ) );
	if ( SUCCEEDED( res ) ) {
#ifdef DEBUG
		ASSERT( wcslen( new_name_ptr ) == new_name_length );
		ASSERT( wcscmp( new_name_ptr, name.c_str( ) ) == 0 );
		const auto da_ptrdiff = ( std::ptrdiff_t( pszend ) - std::ptrdiff_t( new_name_ptr ) );
		ASSERT( ( da_ptrdiff / sizeof( wchar_t ) ) == new_name_length );
#endif
		return res;
		}
	displayWindowsMsgBoxWithMessage( L"Copy of name_str into Children_String_Heap_Manager failed!!!" );
	std::terminate( );
	return res;
	}



QPC_timer::QPC_timer( ) : m_frequency( help_QueryPerformanceFrequency( ).QuadPart ), m_start( 0 ), m_end( 0 ) {
	ASSERT( m_frequency > 0 );
	}

void QPC_timer::begin( ) {
	m_start = help_QueryPerformanceCounter( ).QuadPart;
	}

void QPC_timer::end( ) {
	m_end = help_QueryPerformanceCounter( ).QuadPart;
	}

const double QPC_timer::total_time_elapsed( ) const {
	ASSERT( m_end > m_start );
	static_assert( std::is_same<std::int64_t, LONGLONG>::value, "difference is wrong!" );
	const auto difference = ( m_end - m_start );
	const DOUBLE adjustedTimingFrequency = ( static_cast< DOUBLE >( 1.00 ) ) / static_cast< DOUBLE >( m_frequency );
	const auto total_time = ( difference * adjustedTimingFrequency );
	return total_time;
	}


void InitializeCriticalSection_wrapper( _Pre_invalid_ _Post_valid_ _Out_ CRITICAL_SECTION& cs ) {
	InitializeCriticalSection( &cs );
	}

void DeleteCriticalSection_wrapper( _Pre_valid_ _Post_invalid_ CRITICAL_SECTION& cs ) {
	DeleteCriticalSection( &cs );
	}


void error_getting_pointer_to( _In_z_ PCWSTR const function_name ) {
	std::wstring message;
	message.reserve( 75 );
	message += ( L"Failed to get pointer to " );
	message += function_name;
	message += L'!';
	TRACE( L"%s\r\n", message.c_str( ) );
	displayWindowsMsgBoxWithMessage( std::move( message ) );
	}

void test_if_null_funcptr( void* func_ptr, _In_z_ PCWSTR const function_name ) {
	if ( func_ptr == NULL ) {
		error_getting_pointer_to( function_name );
		}
	}

_Pre_satisfies_( rect.left > rect.right ) _Post_satisfies_( rect.left <= rect.right )
void normalize_RECT_left_right( _Inout_ RECT& rect ) {
	ASSERT( rect.left > rect.right );
	const auto temp = rect.left;
	rect.left = rect.right;
	rect.right = temp;
	ASSERT( rect.left <= rect.right );
	}

_Pre_satisfies_( rect.top > rect.bottom ) _Post_satisfies_( rect.top <= rect.bottom )
void normalize_RECT_top_bottom( _Inout_ RECT& rect ) {
	ASSERT( rect.top > rect.bottom );
	const auto temp = rect.top;
	rect.top = rect.bottom;
	rect.bottom = temp;
	ASSERT( rect.top <= rect.bottom );
	}

_Post_satisfies_( rect.left <= rect.right ) _Post_satisfies_( rect.top <= rect.bottom )
void normalize_RECT( _Inout_ RECT& rect ) {
	if ( rect.left > rect.right ) {
		normalize_RECT_left_right( rect );
		}
	if ( rect.top > rect.bottom ) {
		normalize_RECT_top_bottom( rect );
		}
	}

_Success_( SUCCEEDED( return ) ) HRESULT wds_fmt::FormatBytes( _In_ const std::uint64_t n, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_formatted_bytes, _In_range_( 38, 64 ) const rsize_t strSize, _Out_ rsize_t& chars_written, _On_failure_( _Post_valid_ ) rsize_t& size_needed ) {
	const auto res = wds_fmt::CStyle_FormatLongLongHuman( n, psz_formatted_bytes, strSize, chars_written );
	if ( !SUCCEEDED( res ) ) {
		wds_fmt::write_BAD_FMT( psz_formatted_bytes, chars_written );
		size_needed = ( strSize * 2 );
		return res;
		}
	return res;
	}


_Success_( SUCCEEDED( return ) ) HRESULT wds_fmt::CStyle_FormatLongLongHuman( _In_ std::uint64_t n, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_formatted_LONGLONG_HUMAN, _In_range_( 19, 64 ) const rsize_t strSize, _Out_ rsize_t& chars_written ) {
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
_Success_( SUCCEEDED( return ) ) HRESULT wds_fmt::CStyle_FormatDouble( _In_ const DOUBLE d, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_formatted_double, _In_range_( 17, 64 ) const rsize_t strSize, _Out_ rsize_t& chars_written ) {
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


_Success_( SUCCEEDED( return ) ) HRESULT wds_fmt::CStyle_FormatFileTime( _In_ const FILETIME t, _Out_writes_z_( strSize ) _Pre_writable_size_( strSize ) PWSTR psz_formatted_datetime, _In_range_( 128, 2048 ) const rsize_t strSize, _Out_ rsize_t& chars_written ) {
	ASSERT( &t != NULL );
	SYSTEMTIME st;
	if ( !FileTimeToSystemTime( &t, &st ) ) {
		return file_time_to_system_time_err( psz_formatted_datetime, strSize, chars_written );
		}
	
	//const LCID lcid = MAKELCID( GetUserDefaultLangID( ), SORT_DEFAULT );

	//const int gdfres = GetDateFormatW( lcid, DATE_SHORTDATE, &st, NULL, psz_formatted_datetime, static_cast<int>( strSize ) );
		
	//GRR DATE_AUTOLAYOUT doesn't work, because we're not targeting a Windows 7 minimum!!
	//const int gdfres = GetDateFormatEx( LOCALE_NAME_INVARIANT, DATE_SHORTDATE bitor DATE_AUTOLAYOUT, )

	const int gdfres = GetDateFormatEx( date_time_format_locale_name_str, GetDateFormatEx_flags, &st, NULL, psz_formatted_datetime, static_cast< int >( strSize ), NULL );

	ensure_valid_return_date( gdfres, strSize, st, psz_formatted_datetime );
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

	//const int gtfres = GetTimeFormatW( lcid, 0, &st, NULL, ( psz_formatted_datetime + chars_written ), static_cast<int>( static_cast<int>( strSize ) - static_cast<int>( chars_written ) ) );
	const int gtfres = GetTimeFormatEx( date_time_format_locale_name_str, GetTimeFormatEx_flags, &st, NULL, ( psz_formatted_datetime + chars_written ), static_cast<int>( static_cast<int>( strSize ) - static_cast<int>( chars_written ) ) );

	ensure_valid_return_time( gtfres, strSize, psz_formatted_datetime );

	chars_written += gtfres;
	chars_written -= 1;

	/*
	This function returns 0 if it does not succeed. To get extended error information, the application can call GetLastError, which can return one of the following error codes:
		ERROR_INSUFFICIENT_BUFFER. A supplied buffer size was not large enough, or it was incorrectly set to NULL.
		ERROR_INVALID_FLAGS.       The values supplied for flags were not valid.
		ERROR_INVALID_PARAMETER.   Any of the parameter values was invalid.	
	*/

	return S_OK;
	}

_Success_( SUCCEEDED( return ) ) HRESULT wds_fmt::CStyle_FormatAttributes( _In_ const attribs& attr, _Out_writes_z_( strSize ) _Pre_writable_size_( strSize ) PWSTR psz_formatted_attributes, _In_range_( 6, 18 ) const rsize_t strSize, _Out_ rsize_t& chars_written  ) {
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
_Success_( SUCCEEDED( return ) ) HRESULT wds_fmt::CStyle_GetNumberFormatted( const std::int64_t number, _Pre_writable_size_( strSize ) PWSTR psz_formatted_number, _In_range_( 21, 64 ) const rsize_t strSize, _Out_ rsize_t& chars_written ) {
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
	_Null_terminated_ wchar_t number_str_buffer[ bufSize ] = { 0 };
	
	convert_number_to_string( bufSize, number_str_buffer, number );
	NUMBERFMT format_struct = { 0, 0, 3, L".", L",", 1 };

	//0 indicates failure! http://msdn.microsoft.com/en-us/library/windows/desktop/dd318113.aspx
	const auto get_number_fmt_ex_res = GetNumberFormatEx( NULL, 0, number_str_buffer, &format_struct, psz_formatted_number, static_cast<int>( strSize ) );
	if ( get_number_fmt_ex_res != 0 ) {
		ASSERT( get_number_fmt_ex_res > 0 );
		chars_written = static_cast<rsize_t>( get_number_fmt_ex_res - 1u );
		ASSERT( chars_written == wcslen( psz_formatted_number ) );
		return S_OK;
		}
	ASSERT( get_number_fmt_ex_res == 0 );
	const DWORD last_err = GetLastError( );
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
	static_assert( !SUCCEEDED( E_FAIL ), "bad error return type!" );
	return E_FAIL;
	}

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
		return res;
		}
	displayWindowsMsgBoxWithMessage( L"Copy of name_str failed!!!" );
	std::terminate( );
	return res;
	}



_Success_( return != false ) bool GetVolumeName( _In_z_ PCWSTR const rootPath, _Out_ _Post_z_ wchar_t ( &volumeName )[ MAX_PATH + 1u ] ) {
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
bool GetVolumeName( _In_z_ PCWSTR const rootPath ) {
	const auto old = SetErrorMode( SEM_FAILCRITICALERRORS );
	
	//GetVolumeInformation returns 0 on failure
	const BOOL b = GetVolumeInformationW( rootPath, NULL, 0, NULL, NULL, NULL, NULL, 0 );

	if ( b == 0 ) {
		TRACE( _T( "GetVolumeInformation(%s) failed: %u\n" ), rootPath, GetLastError( ) );
		}
	SetErrorMode( old );
	
	return ( b != 0 );
	}


void wds_fmt::FormatVolumeName( _In_ const std::wstring& rootPath, _In_z_ PCWSTR const volumeName, _Out_ _Post_z_ _Pre_writable_size_( MAX_PATH + 1u ) PWSTR formatted_volume_name ) {
	const HRESULT fmt_res = StringCchPrintfW( formatted_volume_name, ( MAX_PATH + 1u ), L"%s (%s)", volumeName, rootPath.substr( 0, 2 ).c_str( ) );
	if ( SUCCEEDED( fmt_res ) ) {
		return;
		}
	displayWindowsMsgBoxWithMessage( L"FormatVolumeName failed!" );
	std::terminate( );
	}

#pragma strict_gs_check(push, on)
_Success_( SUCCEEDED( return ) ) HRESULT GetFullPathName_WriteToStackBuffer( _In_z_ PCWSTR const relativePath, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_full_path, _In_range_( 128, 512 ) const DWORD strSize, _Out_ rsize_t& chars_written ) {
	psz_full_path[ 0 ] = 0;
	const DWORD dw = GetFullPathNameW( relativePath, strSize, psz_full_path, NULL );
	//ASSERT( dw >= 0 );
	if ( dw == 0 ) {
		static_assert( !SUCCEEDED( E_FAIL ), "" );
		return E_FAIL;
		}
	ASSERT( dw != 0 );
	if ( dw >= strSize ) {
		return STRSAFE_E_INSUFFICIENT_BUFFER;
		}
	ASSERT( dw < strSize );
	ASSERT( dw != 0 );
	if ( dw < strSize ) {
		ASSERT( dw == wcslen( psz_full_path ) );
		chars_written = dw;
		return S_OK;
		}
	ASSERT( false );
	return E_FAIL;
	}
#pragma strict_gs_check(pop)

std::wstring dynamic_GetFullPathName( _In_z_ PCWSTR const relativePath ) {
	rsize_t path_len = MAX_PATH;
	auto pszPath = std::make_unique<_Null_terminated_ wchar_t[ ]>( path_len );
	auto dw = GetFullPathNameW( relativePath, static_cast<DWORD>( path_len ), pszPath.get( ), NULL );
	while ( dw >= path_len ) {
		path_len *= 2;
		pszPath.reset( new wchar_t[ path_len ] );
		dw = GetFullPathNameW( relativePath, static_cast<DWORD>( path_len ), pszPath.get( ), NULL );
		}
	
	return std::wstring( pszPath.get( ) );
	}




void MyGetDiskFreeSpace( _In_z_ PCWSTR const pszRootPath, _Out_ _Out_range_( 0, 18446744073709551615 ) std::uint64_t& total, _Out_ _Out_range_( 0, 18446744073709551615 ) std::uint64_t& unused ) {
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
		total  = utotal.QuadPart; // will fail, when more than 2^63 Bytes free ....
		unused = ufree.QuadPart;
		ASSERT( unused <= total );
		return;
		}
	TRACE( _T( "stats:(%s) avail: %llu, total: %llu, free: %llu\r\n" ), pszRootPath, uavailable, utotal, ufree );
	ASSERT( uavailable.QuadPart <= utotal.QuadPart);
	ASSERT( ufree.QuadPart <= utotal.QuadPart );
	ASSERT( uavailable.QuadPart != utotal.QuadPart );
	ASSERT( ufree.QuadPart != utotal.QuadPart );
	total  = utotal.QuadPart; // will fail, when more than 2^63 Bytes free ....
	unused = ufree.QuadPart;
	ASSERT( unused <= total );
	return;
	}

bool DriveExists( _In_z_ _In_reads_( path_len ) PCWSTR const path, _In_ _In_range_( 0, 4 ) const rsize_t path_len ) {
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
	_Null_terminated_ wchar_t ltr_ws[ size_ltr_str ] = { 0 };
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


_Success_( return ) bool MyQueryDosDevice( _In_z_ PCWSTR const drive, _Out_ _Post_z_ wchar_t ( &drive_info )[ 512u ] ) {
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
	_Null_terminated_ wchar_t left_two_chars_buffer[ left_two_chars_buffer_size ] = { 0 };
	left_two_chars_buffer[ 0 ] = drive[ 0 ];
	left_two_chars_buffer[ 1 ] = drive[ 1 ];
	//left_two_chars_buffer[ 2 ] = drive[ 2 ];
	//d = d.Left( 2 );
	//ASSERT( d.Compare( left_two_chars_buffer ) == 0 );
	
	
	static_assert( ( sizeof( drive_info ) / sizeof( wchar_t ) ) == 512u, "" );
	const auto dw = QueryDosDeviceW( left_two_chars_buffer, drive_info, 512u );//eek
	//info.ReleaseBuffer( );

	if ( dw != 0 ) {
		return true;
		}

	const rsize_t error_buffer_size = 128;
	_Null_terminated_ wchar_t error_buffer[ error_buffer_size ] = { 0 };
	rsize_t error_chars_written = 0;
	const DWORD error_code = GetLastError( );
	const HRESULT fmt_res = CStyle_GetLastErrorAsFormattedMessage( error_buffer, error_buffer_size, error_chars_written, error_code );
	if ( SUCCEEDED( fmt_res ) ) {
		TRACE( _T( "QueryDosDevice(%s) failed: %s\r\n" ), left_two_chars_buffer, error_buffer );
		return false;
		}
	TRACE( _T( "QueryDosDevice(%s) failed. Couldn't get error message for code: %u\r\n" ), left_two_chars_buffer, error_code );
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


static_assert( !SUCCEEDED( E_FAIL ), "CStyle_GetLastErrorAsFormattedMessage doesn't return a valid error code!" );
static_assert( SUCCEEDED( S_OK ), "CStyle_GetLastErrorAsFormattedMessage doesn't return a valid success code!" );
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
		wds_fmt::write_bad_fmt_msg( psz_formatted_error, chars_written );
		return E_FAIL;
		}
	else if ( strSize > 8 ) {
		wds_fmt::write_BAD_FMT( psz_formatted_error, chars_written );
		return E_FAIL;
		}
	chars_written = 0;
	return E_FAIL;
	}

void wds_fmt::write_bad_fmt_msg( _Out_writes_z_( 41 ) _Pre_writable_size_( 42 ) _Post_readable_size_( chars_written ) PWSTR psz_fmt_msg, _Out_ rsize_t& chars_written ) {
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
	_Null_terminated_ wchar_t err_msg[ err_msg_size ] = { 0 };
	rsize_t chars_written = 0;

	const HRESULT err_res = CStyle_GetLastErrorAsFormattedMessage( err_msg, err_msg_size, chars_written, error );
	if ( SUCCEEDED( err_res ) ) {
		WTL::AtlMessageBox( NULL, err_msg, TEXT( "Error" ), MB_OK );
		TRACE( _T( "Error: %s\r\n" ), err_msg );
		return;
		}

	const rsize_t err_msg_size_2 = 4096;
	_Null_terminated_ wchar_t err_msg_2[ err_msg_size_2 ] = { 0 };
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

void displayWindowsMsgBoxWithMessage( PCWSTR const message ) {
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

RECT BuildRECT( const SRECT& in ) {
	//ASSERT( ( in.left != -1 ) && ( in.top != -1 ) && ( in.right != -1 ) && ( in.bottom != -1 ) );
	ASSERT( ( in.right + 1 ) >= in.left );
	ASSERT( in.bottom >= in.top );
	RECT out;
	out.left   = static_cast<LONG>( in.left );
	out.top    = static_cast<LONG>( in.top );
	out.right  = static_cast<LONG>( in.right );
	out.bottom = static_cast<LONG>( in.bottom );
	ASSERT( out.left == in.left );
	ASSERT( out.top == in.top );
	ASSERT( out.right == in.right );
	ASSERT( out.bottom == in.bottom );
	
	/*
inline void CRect::NormalizeRect() throw()
{
	int nTemp;
	if (left > right)
	{
		nTemp = left;
		left = right;
		right = nTemp;
	}
	if (top > bottom)
	{
		nTemp = top;
		top = bottom;
		bottom = nTemp;
	}
}
*/

	//if ( out.left > out.right ) {
	//	normalize_RECT_left_right( out );
	//	}

	//if ( out.top > out.bottom ) {
	//	normalize_RECT_top_bottom( out );
	//	}

	normalize_RECT( out );

	//out.NormalizeRect( );
	ASSERT( out.right >= out.left );
	ASSERT( out.bottom >= out.top );
	return out;
	}


void wds_fmt::write_BAD_FMT( _Out_writes_z_( 8 ) _Pre_writable_size_( 8 ) _Post_readable_size_( 8 ) PWSTR pszFMT, _Out_ rsize_t& chars_written ) {
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


void wds_fmt::write_MEM_INFO_ERR( _Out_writes_z_( 13 ) _Pre_writable_size_( 13 ) PWSTR psz_formatted_usage ) {
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

void wds_fmt::write_RAM_USAGE( _Out_writes_z_( 12 ) _Pre_writable_size_( 13 ) PWSTR psz_ram_usage ) {
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

//bool Compare_FILETIME_lessthan( const FILETIME& t1, const FILETIME& t2 ) {
//	//CompareFileTime returns -1 when first FILETIME is less than second FILETIME
//	//Therefore: we can 'emulate' the `<` operator, by checking if ( CompareFileTime( &t1, &t2 ) == ( -1 ) );
//	return ( CompareFileTime( &t1, &t2 ) == ( -1 ) );
//	
//	//const auto u1 = reinterpret_cast<const ULARGE_INTEGER&>( t1 );
//	//const auto u2 = reinterpret_cast<const ULARGE_INTEGER&>( t2 );
//	//return ( u1.QuadPart < u2.QuadPart );
//	}

//INT Compare_FILETIME( const FILETIME& lhs, const FILETIME& rhs ) {
//	//duhh, there's a win32 function for this!
//	return CompareFileTime( &lhs, &rhs );
//
//
//	//if ( Compare_FILETIME_cast( lhs, rhs ) ) {
//	//	return -1;
//	//	}
//	//else if ( ( lhs.dwLowDateTime == rhs.dwLowDateTime ) && ( lhs.dwHighDateTime == rhs.dwHighDateTime ) ) {
//	//	return 0;
//	//	}
//	//return 1;
//	}

bool Compare_FILETIME_eq( const FILETIME& t1, const FILETIME& t2 ) {
	//CompareFileTime returns 0 when first FILETIME is equal to the second FILETIME
	//Therefore: we can 'emulate' the `==` operator, by checking if ( CompareFileTime( &t1, &t2 ) == ( 0 ) );
	return ( CompareFileTime( &t1, &t2 ) == ( 0 ) );

	//const auto u1 = reinterpret_cast< const ULARGE_INTEGER& >( t1 );
	//const auto u2 = reinterpret_cast< const ULARGE_INTEGER& >( t2 );
	//return ( u1.QuadPart == u2.QuadPart );
	}

std::wstring wds_fmt::FormatBytes( _In_ const std::uint64_t n, const bool humanFormat ) {
	if ( !humanFormat ) {
		return Format_uint64_t_Normal( n );
		}
	ASSERT( humanFormat );
	//MAX value of a std::uint64_t is 20 digits
	const rsize_t strSize = 21;
	_Null_terminated_ wchar_t psz_formatted_longlong[ strSize ] = { 0 };
	rsize_t chars_written = 0;
	auto res = wds_fmt::CStyle_FormatLongLongHuman( n, psz_formatted_longlong, strSize, chars_written );
	if ( SUCCEEDED( res ) ) {
		return psz_formatted_longlong;
		}
	wds_fmt::write_BAD_FMT( psz_formatted_longlong, chars_written );
	return psz_formatted_longlong;
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
	ASSERT( red + green + blue <= 3 * COLOR_MAX_VALUE );
	if ( red > 255 ) {
#ifdef COLOR_DEBUGGING
		TRACE( _T( "Distributing red...\r\n" ) );
#endif
		DistributeFirst( red, green, blue );
		}
	else if ( green > 255 ) {
#ifdef COLOR_DEBUGGING
		TRACE( _T( "Distributing green...\r\n" ) );
#endif
		DistributeFirst( green, red, blue );
		}
	else if ( blue > 255 ) {
#ifdef COLOR_DEBUGGING
		TRACE( _T( "Distributing blue...\r\n" ) );
#endif
		DistributeFirst( blue, red, green );
		}
	}



COLORREF CColorSpace::MakeBrightColor( _In_ const COLORREF color, _In_ _In_range_( 0, 1 ) const DOUBLE brightness ) {
	ASSERT( brightness >= 0.0 );
	ASSERT( brightness <= 1.0 );

	DOUBLE dred   = GetRValue( color ) / COLOR_MAX_VALUE;
	DOUBLE dgreen = GetGValue( color ) / COLOR_MAX_VALUE;
	DOUBLE dblue  = GetBValue( color ) / COLOR_MAX_VALUE;
#ifdef COLOR_DEBUGGING
	TRACE( _T( "passed brightness: %.3f, red: %.3f, green: %.3f, blue: %.3f\r\n" ), brightness, dred, dgreen, dblue );
#endif

	const DOUBLE f = 3.0 * brightness / ( dred + dgreen + dblue );
	dred   *= f;
	dgreen *= f;
	dblue  *= f;

#ifdef COLOR_DEBUGGING
	TRACE( _T( "Intermediate colors,     red: %.3f, green: %.3f, blue: %.3f\r\n" ), dred, dgreen, dblue );
#endif


	ASSERT( ( std::lrint( dred * int( COLOR_MAX_VALUE ) ) ) == ( std::lrint( dred * COLOR_MAX_VALUE ) ) );

	INT red   = std::lrint( dred   * COLOR_MAX_VALUE );
	INT green = std::lrint( dgreen * COLOR_MAX_VALUE );
	INT blue  = std::lrint( dblue  * COLOR_MAX_VALUE );
	
	NormalizeColor( red, green, blue );
	ASSERT( RGB( red, green, blue ) != 0 );
#ifdef COLOR_DEBUGGING
	TRACE( _T( "MakeBrightColor returning red: %i, green: %i, blue: %i\r\n" ), red, green, blue );
#endif

	return RGB( red, green, blue );
	}

_Pre_satisfies_( handle != INVALID_HANDLE_VALUE )
_At_( handle, _Post_invalid_ )
_At_( handle, _Pre_valid_ )
void close_handle( const HANDLE handle ) {
	//If [CloseHandle] succeeds, the return value is nonzero.
	const BOOL res = CloseHandle( handle );
	ASSERT( res != 0 );
	if ( !res ) {
		TRACE( _T( "Closing handle failed!\r\n" ) );
		}
	//TODO: trace error message
	
	}

#else

#endif