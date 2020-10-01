// globalhelpers.cpp - Implementation of global helper functions
//
// see `file_header_text.txt` for licensing & contact info. If you can't find that file, then assume you're NOT allowed to do whatever you wanted to do.
#include "stdafx.h"

#pragma once


#ifndef WDS_GLOBALHELPERS_CPP
#define WDS_GLOBALHELPERS_CPP

WDS_FILE_INCLUDE_MESSAGE

#include "globalhelpers.h"
#include "ScopeGuard.h"
#include "signum.h"
#include "stringformatting.h"

#pragma warning(3:4514) //'function': unreferenced inline function has been removed



namespace {

	const constexpr PCWSTR date_time_format_locale_name_str = LOCALE_NAME_INVARIANT;
	const constexpr DWORD GetDateFormatEx_flags             = DATE_SHORTDATE;
	const constexpr DWORD GetTimeFormatEx_flags             = 0;
	const constexpr double COLOR_MAX_VALUE = 255.0;



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
	const bool display_GetDateFormatEx_flags( _In_ const DWORD flags ) noexcept {
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
	const bool display_GetTimeFormatEx_flags( _In_ const DWORD flags ) noexcept {
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
	//This is an error handling function, and is intended to be called rarely!
	__declspec(noinline)
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
	//This is an error handling function, and is intended to be called rarely!
	__declspec(noinline)
	_Must_inspect_result_ _Success_( return == true )
	const bool get_time_format_err( _In_ const rsize_t str_size, _In_ const DWORD flags ) noexcept {
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

	//The compiler will automatically inline if /Ob2 is on, so we'll ask anyways.
	_Success_( SUCCEEDED( return ) ) inline HRESULT file_time_to_system_time_err( _Out_writes_z_( strSize ) _Pre_writable_size_( strSize ) PWSTR psz_formatted_datetime, _In_range_( 128, 2048 ) const rsize_t strSize, _Out_ rsize_t& chars_written ) noexcept {
		const HRESULT err_res = CStyle_GetLastErrorAsFormattedMessage( psz_formatted_datetime, strSize, chars_written );
		ASSERT( SUCCEEDED( err_res ) );
		if ( !SUCCEEDED( err_res ) ) {
			TRACE( _T( "Error in file_time_to_system_time_err->CStyle_GetLastErrorAsFormattedMessage!!\r\n" ) );
			displayWindowsMsgBoxWithMessage( L"Error in file_time_to_system_time_err->CStyle_GetLastErrorAsFormattedMessage!!\r\n" );
			return err_res;
			}
		//return a failure, with the buffer filled with the error message (not intended to be read in by a function)
		return E_FAIL;
		}

	//_In_range_ upper bound is totally arbitrary! Lower bound is enough for WRITE_BAD_FMT
	void ensure_valid_return_date( _In_ const int gdfres, _In_ _In_ _In_range_( 24, 2048 ) const rsize_t strSize, _In_ const SYSTEMTIME system_time, _Inout_ PWSTR psz_formatted_datetime ) {
		if ( ( static_cast<std::int64_t>(gdfres) + 1 ) >= static_cast< std::int64_t >( strSize ) ) {
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
		//std::terminate here is WRONG!
		//std::terminate( );
		}

	void ensure_valid_return_time( const int gtfres, const rsize_t strSize, _Inout_ PWSTR psz_formatted_datetime ) {
		if ( ( static_cast<std::int64_t>( gtfres ) + 1 ) >= static_cast< std::int64_t >( strSize ) ) {
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
		//std::terminate here is WRONG!
		//std::terminate( );
		}



	void convert_number_to_string_failed_display_debugging_info( _In_range_( 19, 128 ) const rsize_t bufSize, _In_ const std::int64_t number ) {
		const std::wstring err_str( L"DEBUGGING INFO: bufSize: " + std::to_wstring( bufSize ) + L", number: " + std::to_wstring( number ) );
		displayWindowsMsgBoxWithMessage( err_str.c_str( ) );
		std::terminate( );
		}

	void convert_number_to_string_failed( _In_range_( 19, 128 ) const rsize_t bufSize, _In_ const std::int64_t number, _In_ const HRESULT strsafe_printf_res ) noexcept {
		auto guard = WDS_SCOPEGUARD_INSTANCE( [ &] { convert_number_to_string_failed_display_debugging_info( bufSize, number ); } );
		
		if ( strsafe_printf_res == STRSAFE_E_INSUFFICIENT_BUFFER ) {
			displayWindowsMsgBoxWithMessage( L"STRSAFE_E_INSUFFICIENT_BUFFER in CStyle_GetNumberFormatted!(aborting)" );
			return;
			}
		if ( strsafe_printf_res == STRSAFE_E_END_OF_FILE ) {
			//this doesn't make any sense.
			displayWindowsMsgBoxWithMessage( L"STRSAFE_E_END_OF_FILE in CStyle_GetNumberFormatted!(aborting)" );
			return;
			}
		if ( strsafe_printf_res == STRSAFE_E_INVALID_PARAMETER ) {
			displayWindowsMsgBoxWithMessage( L"STRSAFE_E_INVALID_PARAMETER in CStyle_GetNumberFormatted!(aborting)" );
			return;
			}
		displayWindowsMsgBoxWithMessage( L"Unknown error in CStyle_GetNumberFormatted!(aborting)" );
		return;
		}

	void convert_number_to_string( _In_range_( 19, 128 ) const rsize_t bufSize, _Pre_writable_size_( bufSize ) _Post_z_ PWSTR number_str_buffer, _In_ const std::int64_t number ) noexcept {
		rsize_t chars_remaining = 0;

		const HRESULT strsafe_printf_res = ::StringCchPrintfExW( number_str_buffer, bufSize, NULL, &chars_remaining, 0, L"%I64d", number );
		if ( SUCCEEDED( strsafe_printf_res ) ) {
			return;
			}
		WDS_ASSERT_EXPECTED_STRING_FORMAT_FAILURE_HRESULT( strsafe_printf_res );
		WDS_STRSAFE_E_INVALID_PARAMETER_HANDLER( strsafe_printf_res, "StringCchPrintfExW" );
		
		convert_number_to_string_failed( bufSize, number, strsafe_printf_res );
		}

	inline void DistributeFirst( _Inout_ _Out_range_(0, 255) INT& first, _Inout_ _Out_range_(0, 255) INT& second, _Inout_ _Out_range_(0, 255) INT& third ) noexcept {
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




}

_Success_( SUCCEEDED( return ) )
const HRESULT WriteToStackBuffer_do_nothing( WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) noexcept {
	if ( strSize > 1 ) {
		psz_text[ 0 ] = 0;
		chars_written = 0;
		ASSERT( chars_written == wcslen( psz_text ) );
		return S_OK;
		}
	//do nothing
	sizeBuffNeed = 6u; //you've got to be kidding me if you've passed a buffer that too small.
	return STRSAFE_E_INSUFFICIENT_BUFFER;
	//return StringCchPrintfExW( psz_text, strSize, NULL, &chars_remaining, 0, L"" );
	}


QPC_timer::QPC_timer( ) : m_frequency( help_QueryPerformanceFrequency( ).QuadPart ), m_start( 0 ), m_end( 0 ) {
	ASSERT( m_frequency > 0 );
	}

void QPC_timer::begin( ) noexcept {
	m_start = help_QueryPerformanceCounter( ).QuadPart;
	}

void QPC_timer::end( ) noexcept {
	m_end = help_QueryPerformanceCounter( ).QuadPart;
	}

const double QPC_timer::total_time_elapsed( ) const noexcept {
	ASSERT( m_end > m_start );
	static_assert( std::is_same<std::int64_t, LONGLONG>::value, "difference is wrong!" );
	const auto difference = ( m_end - m_start );
	const DOUBLE adjustedTimingFrequency = ( static_cast< DOUBLE >( 1.00 ) ) / static_cast< DOUBLE >( m_frequency );
	const auto total_time = ( static_cast<DOUBLE>( difference ) * adjustedTimingFrequency );
	return total_time;
	}


void InitializeCriticalSection_wrapper( _Pre_invalid_ _Post_valid_ _Out_ CRITICAL_SECTION& cs ) noexcept {
	::InitializeCriticalSection( &cs );
	}

void DeleteCriticalSection_wrapper( _Pre_valid_ _Post_invalid_ CRITICAL_SECTION& cs ) noexcept {
	::DeleteCriticalSection( &cs );
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

void test_if_null_funcptr( const void* func_ptr, _In_z_ PCWSTR const function_name ) noexcept {
	if ( func_ptr == NULL ) {
		error_getting_pointer_to( function_name );
		}
	}

_Post_satisfies_( rect->left <= rect->right )
void normalize_RECT_left_right( _Inout_ RECT* const rect ) noexcept {
	ASSERT( rect->left > rect->right );
	const auto temp = rect->left;
	rect->left = rect->right;
	rect->right = temp;
	ASSERT( rect->left <= rect->right );
	}

_Post_satisfies_( rect->top <= rect->bottom )
void normalize_RECT_top_bottom( _Inout_ RECT* const rect ) noexcept {
	ASSERT( rect->top > rect->bottom );
	const auto temp = rect->top;
	rect->top = rect->bottom;
	rect->bottom = temp;
	ASSERT( rect->top <= rect->bottom );
	}

_Post_satisfies_( rect->left <= rect->right ) /*_Post_satisfies_( rect->top <= rect->bottom )*/
void normalize_RECT( _Inout_ RECT* const rect ) noexcept {
	if ( rect->left > rect->right ) {
		normalize_RECT_left_right( rect );
		}
	if ( rect->top > rect->bottom ) {
		normalize_RECT_top_bottom( rect );
		}
	}


void fill_solid_RECT( _In_ const HDC hDC, _In_ const RECT* const rect, COLORREF clr) noexcept {
		/*
	void CDC::FillSolidRect(LPCRECT lpRect, COLORREF clr)
	{
		ENSURE_VALID(this);
		ENSURE(m_hDC != NULL);
		ENSURE(lpRect);

		::SetBkColor(m_hDC, clr);
		::ExtTextOut(m_hDC, 0, 0, ETO_OPAQUE, lpRect, NULL, 0, NULL);
	}
	*/

		ASSERT( hDC != NULL );

		//SetBkColor function: https://msdn.microsoft.com/en-us/library/dd162964.aspx
		//If the [SetBkColor] succeeds, the return value specifies the previous background color as a COLORREF value.
		//If [SetBkColor] fails, the return value is CLR_INVALID.
		if ( ::SetBkColor( hDC, clr ) == CLR_INVALID ) {
			std::terminate( );
			}

		//ExtTextOut function: https://msdn.microsoft.com/en-us/library/dd162713.aspx
		//If the string is drawn, the return value [of ExtTextOutW] is nonzero.
		//However, if the ANSI version of ExtTextOut is called with ETO_GLYPH_INDEX, the function returns TRUE even though the function does nothing.
		//If the function fails, the return value is zero.
		VERIFY( ::ExtTextOutW( hDC, 0, 0, ETO_OPAQUE, rect, NULL, 0, NULL ) );

	}



//TODO: mark to only return STRSAFE_E_INSUFFICIENT_BUFFER, E_FAIL, or S_OK.
_Success_( SUCCEEDED( return ) ) HRESULT wds_fmt::CStyle_FormatFileTime( _In_ const FILETIME t, _Out_writes_z_( strSize ) _Pre_writable_size_( strSize ) PWSTR psz_formatted_datetime, _In_range_( 128, 2048 ) const rsize_t strSize, _Out_ rsize_t& chars_written ) noexcept {
	ASSERT( &t != NULL );
	SYSTEMTIME st;
	if ( !::FileTimeToSystemTime( &t, &st ) ) {
		return file_time_to_system_time_err( psz_formatted_datetime, strSize, chars_written );
		}
	
	//const LCID lcid = MAKELCID( GetUserDefaultLangID( ), SORT_DEFAULT );
	//const int gdfres = GetDateFormatW( lcid, DATE_SHORTDATE, &st, NULL, psz_formatted_datetime, static_cast<int>( strSize ) );
	//GRR DATE_AUTOLAYOUT doesn't work, because we're not targeting a Windows 7 minimum!!
	//const int gdfres = GetDateFormatEx( LOCALE_NAME_INVARIANT, DATE_SHORTDATE bitor DATE_AUTOLAYOUT, )

	const int gdfres = ::GetDateFormatEx( date_time_format_locale_name_str, GetDateFormatEx_flags, &st, NULL, psz_formatted_datetime, static_cast< int >( strSize ), NULL );

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

	// GetDateFormatEx function: https://msdn.microsoft.com/en-us/library/windows/desktop/dd318088.aspx
	// Returns the number of characters written to the lpDateStr buffer if successful.
	// If the cchDate parameter is set to 0, the function returns the number of characters required to hold the formatted date string, including the terminating null character.
	// This function returns 0 if it does not succeed. 


	// GetTimeFormatEx function: https://msdn.microsoft.com/en-us/library/windows/desktop/dd318131.aspx
	// Returns the number of characters retrieved in the buffer indicated by lpTimeStr.
	// If the cchTime parameter is set to 0, the function returns the size of the buffer required to hold the formatted time string, including a terminating null character.
	// This function returns 0 if it does not succeed. 

	const int gtfres = ::GetTimeFormatEx( date_time_format_locale_name_str, GetTimeFormatEx_flags, &st, NULL, ( psz_formatted_datetime + chars_written ), static_cast<int>( static_cast<int>( strSize ) - static_cast<int>( chars_written ) ) );

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



//
_Success_( SUCCEEDED( return ) ) HRESULT wds_fmt::CStyle_GetNumberFormatted( const std::int64_t number, _Pre_writable_size_( strSize ) PWSTR psz_formatted_number, _In_range_( 21, 64 ) const rsize_t strSize, _Out_ rsize_t& chars_written ) noexcept {
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

	constexpr const rsize_t bufSize = 66;
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
	static_assert( !SUCCEEDED( E_FAIL ), "bad error return type!" );
	}

_Success_( SUCCEEDED( return ) )
const HRESULT allocate_and_copy_name_str( _Pre_invalid_ _Post_z_ _Post_readable_size_( new_name_length ) wchar_t*& new_name_ptr, _In_ _In_range_( 0, UINT16_MAX ) const rsize_t& new_name_length, const std::wstring& name ) {
	ASSERT( new_name_length < UINT16_MAX );
	new_name_ptr = new wchar_t[ new_name_length + 2u ];
	PWSTR pszend = nullptr;
	rsize_t chars_remaining = new_name_length;
	const HRESULT res = ::StringCchCopyExW( new_name_ptr, ( new_name_length + 1u ), name.c_str( ), &pszend, &chars_remaining, 0 );
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


//This is an error handling function, and is intended to be called rarely!
__declspec(noinline)
void unexpected_strsafe_invalid_parameter_handler( _In_z_ PCSTR const strsafe_func_name, _In_z_ PCSTR const file_name_in, _In_z_ PCSTR const func_name_in, _In_ _In_range_( 0, INT_MAX ) const int line_number_in ) noexcept {
	std::string err_str( strsafe_func_name );
	err_str += " returned STRSAFE_E_INVALID_PARAMETER, in: file `";
	err_str += file_name_in;
	err_str += "`, function: `";
	err_str += func_name_in;
	err_str += "` line: `";
	err_str += std::to_string( line_number_in );
	err_str += "`! This (near universally) means an issue where incorrect compile-time constants were passed to a strsafe function. Thus it's probably not recoverable. We'll abort. Sorry!";
	displayWindowsMsgBoxWithMessage( err_str );
	std::terminate( );
	}

//this function is only called in the rare/error path, so NON-inline code is faster, and smaller.
__declspec(noinline)
void handle_stack_insufficient_buffer( _In_ const rsize_t str_size, _In_ const rsize_t generic_size_needed, _Out_ rsize_t& size_buff_need, _Out_ rsize_t& chars_written ) noexcept {
	chars_written = str_size;
	if ( str_size < generic_size_needed ) {
		size_buff_need = generic_size_needed;
		return;
		}
	size_buff_need = ( str_size * 2 );
	return;
	}


const LARGE_INTEGER help_QueryPerformanceCounter( ) noexcept {
	LARGE_INTEGER doneTime;
	const BOOL behavedWell = ::QueryPerformanceCounter( &doneTime );
	ASSERT( behavedWell );
	if ( !behavedWell ) {
		::MessageBoxW( nullptr, L"QueryPerformanceCounter failed!!", L"ERROR!", MB_OK );
		doneTime.QuadPart = -1;
		}
	return doneTime;
	}

const LARGE_INTEGER help_QueryPerformanceFrequency( ) noexcept {
	LARGE_INTEGER doneTime;
	//QueryPerformanceFrequency function: https://msdn.microsoft.com/en-us/library/windows/desktop/ms644905.aspx
	//On systems that run Windows XP or later, the function will always succeed and will thus never return zero.
	const BOOL behavedWell = QueryPerformanceFrequency( &doneTime );
	if ( !behavedWell ) {
		std::terminate( );
		}
	return doneTime;
	}


static_assert( !SUCCEEDED( E_FAIL ), "CStyle_GetLastErrorAsFormattedMessage doesn't return a valid error code!" );
static_assert( SUCCEEDED( S_OK ), "CStyle_GetLastErrorAsFormattedMessage doesn't return a valid success code!" );
//On returning E_FAIL, call GetLastError for details. That's not my idea! //TODO: mark as only returning S_OK, E_FAIL
_Success_( SUCCEEDED( return ) ) HRESULT CStyle_GetLastErrorAsFormattedMessage( WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_formatted_error, _In_range_( 128, 32767 ) const rsize_t strSize, _Out_ rsize_t& chars_written, const DWORD error ) noexcept {
	//const auto err = GetLastError( );
	const auto err = error;
	const auto ret = ::FormatMessageW( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, err, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), psz_formatted_error, static_cast<DWORD>( strSize ), nullptr );
	if ( ret != 0 ) {
		chars_written = ret;
		return S_OK;
		}
	const DWORD error_err = ::GetLastError( );
	TRACE( _T( "FormatMessageW failed with error code: `%lu`!!\r\n" ), error_err );
	
	constexpr const rsize_t err_msg_buff_size = 512;
	_Null_terminated_ char err_msg_buff[ err_msg_buff_size ] = { 0 };
	const HRESULT output_error_message_format_result = StringCchPrintfA( err_msg_buff, err_msg_buff_size, "WDS: FormatMessageW failed with error code: `%lu`!!\r\n", error_err );
	if ( SUCCEEDED( output_error_message_format_result ) ) {
		::OutputDebugStringA( err_msg_buff );
		}
	else {
		WDS_ASSERT_EXPECTED_STRING_FORMAT_FAILURE_HRESULT( output_error_message_format_result );
		WDS_STRSAFE_E_INVALID_PARAMETER_HANDLER( output_error_message_format_result, "StringCchPrintfA" );
		::OutputDebugStringA( "WDS: FormatMessageW failed, and THEN formatting the error message for FormatMessageW failed!\r\n" );
		}
	if ( strSize > 41 ) {
		wds_fmt::write_bad_fmt_msg( psz_formatted_error, chars_written );
		return E_FAIL;
		}
	if ( strSize > 8 ) {
		wds_fmt::write_BAD_FMT( psz_formatted_error, chars_written );
		return E_FAIL;
		}
	chars_written = 0;
	return E_FAIL;
	}

//This is an error handling function, and is intended to be called rarely!
__declspec(noinline)
void wds_fmt::write_bad_fmt_msg( _Out_writes_z_( 41 ) _Pre_writable_size_( 42 ) _Post_readable_size_( chars_written ) PWSTR psz_fmt_msg, _Out_ rsize_t& chars_written ) noexcept {
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

//This is an error handling function, and is intended to be called rarely!
__declspec(noinline)
void displayWindowsMsgBoxWithError( const DWORD error ) noexcept {
	constexpr const rsize_t err_msg_size = 1024u;
	_Null_terminated_ wchar_t err_msg[ err_msg_size ] = { 0 };
	rsize_t chars_written = 0;

	const HRESULT err_res = CStyle_GetLastErrorAsFormattedMessage( err_msg, err_msg_size, chars_written, error );
	ASSERT( SUCCEEDED( err_res ) );
	if ( SUCCEEDED( err_res ) ) {
		::MessageBoxW( nullptr, err_msg, L"Error", MB_OK );
		TRACE( _T( "Error: %s\r\n" ), err_msg );
		return;
		}
	TRACE( _T( "First attempt to get last error as a formatted message FAILED!\r\n" ) );

	constexpr const rsize_t err_msg_size_2 = 4096u;
	_Null_terminated_ wchar_t err_msg_2[ err_msg_size_2 ] = { 0 };
	rsize_t chars_written_2 = 0;
	const HRESULT err_res_2 = CStyle_GetLastErrorAsFormattedMessage( err_msg_2, err_msg_size_2, chars_written_2, error );
	ASSERT( SUCCEEDED( err_res_2 ) );
	if ( SUCCEEDED( err_res_2 ) ) {
		::MessageBoxW( nullptr, err_msg_2, L"Error", MB_OK );
		TRACE( _T( "Error: %s\r\n" ), err_msg_2 );
		return;
		}
	TRACE( _T( "Error while getting error message!\r\n" ), err_msg_2 );
	::MessageBoxW( nullptr, L"Error while getting error message!\r\n", TEXT( "Error" ), MB_OK );
	}

//This is an error handling function, and is intended to be called rarely!
__declspec(noinline)
void displayWindowsMsgBoxWithMessage( const std::wstring message ) {
	displayWindowsMsgBoxWithMessage( message.c_str( ) );
	}

//This is an error handling function, and is intended to be called rarely!
__declspec(noinline)
void displayWindowsMsgBoxWithMessage( const std::string message ) {
	//MessageBoxW( NULL, message.c_str( ), TEXT( "Error" ), MB_OK );
	
	//see: https://code.google.com/p/hadesmem/source/browse/trunk/Include/Common/HadesCommon/I18n.hpp?r=1163
	//auto convert_obj = stdext::cvt::wstring_convert<std::codecvt<wchar_t, char, mbstate_t>, wchar_t>( );

	//const auto new_wide_str = convert_obj.from_bytes( message );
	::MessageBoxA( nullptr, message.c_str( ), "Error", MB_OK | MB_ICONINFORMATION );
	TRACE( _T( "Error: %S\r\n" ), message.c_str( ) );
	}

//This is an error handling function, and is intended to be called rarely!
__declspec(noinline)
void displayWindowsMsgBoxWithMessage( PCWSTR const message ) noexcept {
	::MessageBoxW( nullptr, message, L"Error", MB_OK );
	TRACE( _T( "Error: %s\r\n" ), message );
	}

RECT BuildRECT( const SRECT& in ) noexcept {
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

	normalize_RECT( &out );

	//out.NormalizeRect( );
	ASSERT( out.right >= out.left );
	ASSERT( out.bottom >= out.top );
	return out;
	}




//This is an error handling function, and is intended to be called rarely!
__declspec(noinline)
void wds_fmt::write_MEM_INFO_ERR( _Out_writes_z_( 13 ) _Pre_writable_size_( 13 ) PWSTR psz_formatted_usage ) noexcept {
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

//This is an error handling function, and is intended to be called rarely!
__declspec(noinline)
void wds_fmt::write_RAM_USAGE( _Out_writes_z_( 12 ) _Pre_writable_size_( 13 ) PWSTR psz_ram_usage ) noexcept {
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
void CheckMinMax( _Inout_ LONG& val, _In_ const LONG min_val, _In_ const LONG max_val ) noexcept {
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
void CheckMinMax( _Inout_ LONG& val, _In_ const INT min_val, _In_ const INT max_val ) noexcept {
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
void CheckMinMax( _Inout_ INT& val, _In_ const INT min_val, _In_ const INT max_val ) noexcept {
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

bool Compare_FILETIME_eq( const FILETIME& t1, const FILETIME& t2 ) noexcept {
	//CompareFileTime returns 0 when first FILETIME is equal to the second FILETIME
	//Therefore: we can 'emulate' the `==` operator, by checking if ( CompareFileTime( &t1, &t2 ) == ( 0 ) );
	return ( ::CompareFileTime( &t1, &t2 ) == ( 0 ) );
	}







void NormalizeColor( _Inout_ _Out_range_(0, 255) INT& red, _Inout_ _Out_range_(0, 255) INT& green, _Inout_ _Out_range_(0, 255) INT& blue ) noexcept {
	ASSERT( static_cast<std::int64_t>(red) + static_cast<std::int64_t>(green) + static_cast<std::int64_t>(blue) <= static_cast<std::int64_t>(3) * COLOR_MAX_VALUE );
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



COLORREF CColorSpace::MakeBrightColor( _In_ const COLORREF color, _In_ _In_range_( 0, 1 ) const DOUBLE brightness ) noexcept {
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


	ASSERT( ( std::lrint( dred * static_cast<int>( COLOR_MAX_VALUE ) ) ) == ( std::lrint( dred * COLOR_MAX_VALUE ) ) );

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
void close_handle( const HANDLE handle ) noexcept {
	//If [CloseHandle] succeeds, the return value is nonzero.
	const BOOL res = ::CloseHandle( handle );
	ASSERT( res != 0 );
	if ( !res ) {
		TRACE( _T( "Closing handle failed!\r\n" ) );
		}
	
	}

#ifdef DEBUG

#ifdef COLOR_DEBUGGING
//this function exists for the singular purpose of tracing to console, as doing so from a .cpp is cleaner.
void trace_m_stripe_color_make_bright_color( _In_ const COLORREF m_windowColor, _In_ const DOUBLE b ) {
	TRACE( _T( "m_stripeColor = MakeBrightColor( %ld, %f )\r\n" ), m_windowColor, b );
	}

//this function exists for the singular purpose of tracing to console, as doing so from a .cpp is cleaner.
void trace_m_stripeColor( _In_ const COLORREF m_stripeColor ) {
	TRACE( _T( "m_stripeColor: %ld\r\n" ), m_stripeColor );
	}
#endif

//this function exists for the singular purpose of tracing to console, as doing so from a .cpp is cleaner.
void trace_on_destroy( _In_z_ PCWSTR const m_persistent_name ) {
	TRACE( _T( "%s received OnDestroy!\r\n" ), m_persistent_name );
	}

//this function exists for the singular purpose of tracing to console, as doing so from a .cpp is cleaner.
void trace_full_path( _In_z_ PCWSTR const path ) {
	TRACE( _T( "MyGetFullPathName( m_folder_name_heap ): %s\r\n" ), path );
	}
#endif


int GetItemCount_HDM_GETITEMCOUNT( _In_ const HWND hWnd ) noexcept {
	ASSERT( ::IsWindow( hWnd ) );
	//SendMessage function: https://msdn.microsoft.com/en-us/library/windows/desktop/ms644950.aspx
	//The return value [of SendMessage] specifies the result of the message processing; it depends on the message sent.


	//HDM_GETITEMCOUNT message: https://msdn.microsoft.com/en-us/library/windows/desktop/bb775337.aspx
	//Gets a count of the items in a header control.
	//[HDM_GETITEMCOUNT message] Returns the number of items if successful, or -1 otherwise.
	const LRESULT msg_result = ::SendMessageW( hWnd, HDM_GETITEMCOUNT, 0, 0L );
	if ( msg_result == -1 ) {
		std::terminate( );
		}
	return static_cast< int >( msg_result );
	}


int GetColumnWidth_LVM_GETCOLUMNWIDTH( _In_ const HWND hWnd, _In_ _In_range_( >=, 0 ) const int nCol ) noexcept {
	ASSERT( ::IsWindow( hWnd ) );
	//SendMessage function: https://msdn.microsoft.com/en-us/library/windows/desktop/ms644950.aspx
	//The return value [of SendMessage] specifies the result of the message processing; it depends on the message sent.

	//LVM_GETCOLUMNWIDTH message: https://msdn.microsoft.com/en-us/library/windows/desktop/bb774915.aspx
	//Gets the width of a column in report or list view.
	//wParam
		//The index of the column. This parameter is ignored in list view.
	//lParam
		//Must be zero.
	//Returns the column width if successful, or zero otherwise.
	//If this message is sent to a list-view control with the LVS_REPORT style and the specified column does not exist, the return value is undefined.

	return static_cast< int >( ::SendMessageW( hWnd, LVM_GETCOLUMNWIDTH, static_cast<WPARAM>( nCol ), 0 ) );
	}

BOOL SetColumnWidth_LVM_SETCOLUMNWIDTH( _In_ const HWND hWnd, _In_ _In_range_( >=, 0 ) const int nCol, _In_ _In_range_( >=, 0 ) const int cx ) noexcept {
	ASSERT( ::IsWindow( hWnd ) );
	//_AFXCMN_INLINE BOOL CListCtrl::SetColumnWidth(_In_ int nCol, _In_ int cx)
	//{ ASSERT(::IsWindow(m_hWnd)); return (BOOL) ::SendMessage(m_hWnd, LVM_SETCOLUMNWIDTH, nCol, MAKELPARAM(cx, 0)); }

	//SendMessage function: https://msdn.microsoft.com/en-us/library/windows/desktop/ms644950.aspx
	//The return value [of SendMessage] specifies the result of the message processing; it depends on the message sent.

	//LVM_SETCOLUMNWIDTH message: https://msdn.microsoft.com/en-us/library/windows/desktop/bb761163.aspx
	//Changes the width of a column in report-view mode or the width of all columns in list-view mode.
	//wParam
		//Zero-based index of a valid column. For list-view mode, this parameter must be set to zero.
	//lParam
		//New width of the column, in pixels. For report-view mode, the following special values are supported:
	//Returns TRUE if successful, or FALSE otherwise.
	
	//Assume that you have a 2-column list-view control with a width of 500 pixels.
		//If the width of column zero is set to 200 pixels, and you send this message with wParam = 1 and lParam = LVSCW_AUTOSIZE_USEHEADER, the second (and last) column will be 300 pixels wide.

	return static_cast<BOOL>( ::SendMessageW( hWnd, LVM_SETCOLUMNWIDTH, static_cast<WPARAM>( nCol ), MAKELPARAM( cx, 0 ) ) );
	}

BOOL EnsureVisible_LVM_ENSUREVISIBLE( _In_ const HWND hWnd, _In_ _In_range_( >=, 0 ) const int nItem, _In_  _In_range_( FALSE, TRUE ) const BOOL bPartialOK ) noexcept {
	ASSERT( ::IsWindow( hWnd ) );

	//_AFXCMN_INLINE BOOL CListCtrl::EnsureVisible(_In_ int nItem, _In_ BOOL bPartialOK)
	//{ ASSERT(::IsWindow(m_hWnd)); return (BOOL) ::SendMessage(m_hWnd, LVM_ENSUREVISIBLE, nItem, MAKELPARAM(bPartialOK, 0)); }

	//SendMessage function: https://msdn.microsoft.com/en-us/library/windows/desktop/ms644950.aspx
	//The return value [of SendMessage] specifies the result of the message processing; it depends on the message sent.

	//LVM_ENSUREVISIBLE message: https://msdn.microsoft.com/en-us/library/windows/desktop/bb774902.aspx
	//Ensures that a list-view item is either entirely or partially visible, scrolling the list-view control if necessary.
	//wParam
		//The index of the list-view item.
	//lParam
		//A value specifying whether the item must be entirely visible. If this parameter is TRUE, no scrolling occurs if the item is at least partially visible.
	//Returns TRUE if successful, or FALSE otherwise.
	//The message fails if the window style includes LVS_NOSCROLL.

	return static_cast<BOOL>( ::SendMessageW( hWnd, LVM_ENSUREVISIBLE, static_cast<WPARAM>( nItem ), bPartialOK ) );
	}

_Success_( return )
BOOL GetItem_HDM_GETITEM( _In_ const HWND hWnd, _In_ _In_range_( >=, 0 ) const int nPos, _Out_ HDITEM* const pHeaderItem ) noexcept {
	ASSERT( ::IsWindow( hWnd ) );

	pHeaderItem->mask = HDI_WIDTH;
	//VERIFY( thisHeader->GetItem( 0, &hditem ) );
	/*
	_AFXCMN_INLINE BOOL CHeaderCtrl::GetItem(_In_ int nPos, _Out_ HDITEM* pHeaderItem) const
		{ ASSERT(::IsWindow(m_hWnd)); return (BOOL)::SendMessage(m_hWnd, HDM_GETITEM, nPos, (LPARAM)pHeaderItem); }
	*/
	//SendMessage function: https://msdn.microsoft.com/en-us/library/windows/desktop/ms644950.aspx
	//The return value [of SendMessage] specifies the result of the message processing; it depends on the message sent.

	//HDM_GETITEM message: https://msdn.microsoft.com/en-us/library/windows/desktop/bb775335.aspx
	//Gets information about an item in a header control. 
	//Returns TRUE if successful, or FALSE otherwise.
	//wParam: The index of the item for which information is to be retrieved.
	//lParam: A pointer to an HDITEM structure.
		//When the message is sent, the mask member indicates the type of information being requested.
		//When the message returns, the other members receive the requested information.
		//If the mask member specifies zero, the message returns TRUE but copies no information to the structure.
	//If the HDI_TEXT flag is set in the mask member of the HDITEM structure:
		//the control may change the pszText member of the structure to point to the new text instead of filling the buffer with the requested text.
		//Applications should not assume that the text will always be placed in the requested buffer.
			

	ASSERT( ::IsWindow( hWnd ) );

	const LRESULT get_item_result = ::SendMessageW( hWnd, HDM_GETITEM, static_cast<WPARAM>( nPos ), reinterpret_cast<LPARAM>( pHeaderItem ) );
	//if ( get_item_result != TRUE ) {
	//	std::terminate( );
	//	}
	//return TRUE;
	return static_cast<BOOL>( get_item_result );
	}

BOOL SetItem_HDM_SETITEM( _In_ const HWND hWnd, _In_ _In_range_( >=, 0 ) const int nPos, _In_ const HDITEM * const pHeaderItem ) noexcept {
	ASSERT( ::IsWindow( hWnd ) );
	//_AFXCMN_INLINE BOOL CHeaderCtrl::SetItem(_In_ int nPos, _In_ HDITEM* pHeaderItem)
	//{ ASSERT(::IsWindow(m_hWnd)); return (BOOL)::SendMessage(m_hWnd, HDM_SETITEM, nPos, (LPARAM)pHeaderItem); }

	//SendMessage function: https://msdn.microsoft.com/en-us/library/windows/desktop/ms644950.aspx
	//The return value [of SendMessage] specifies the result of the message processing; it depends on the message sent.

	//HDM_SETITEM message: https://msdn.microsoft.com/en-us/library/windows/desktop/bb775367.aspx
	//Sets the attributes of the specified item in a header control.
	//wParam
		//The current index of the item whose attributes are to be changed.
	//lParam
		//A pointer to an HDITEM structure that contains item information. When this message is sent, the mask member of the structure must be set to indicate which attributes are being set.
	//Returns nonzero upon success, or zero otherwise.
	//The HDITEM structure that supports this message supports item order and image list information. By using these members, you can control the order in which items are displayed and specify images to appear with items.

	return static_cast<BOOL>( ::SendMessageW( hWnd, HDM_SETITEM, static_cast<WPARAM>( nPos ), reinterpret_cast<LPARAM>( pHeaderItem ) ) );

	}

_Success_( return )
BOOL GetItemRect_LVM_GETITEMRECT( _In_ const HWND hWnd, _In_ _In_range_( >=, 0 ) const int nItem, _Out_ RECT* const rect, _In_ _In_range_( LVIR_BOUNDS, LVIR_SELECTBOUNDS ) const LONG nCode ) noexcept {
	/*
	BOOL GetItemRect(int nItem, LPRECT lpRect, UINT nCode) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		lpRect->left = nCode;
		return (BOOL)::SendMessage(m_hWnd, LVM_GETITEMRECT, (WPARAM)nItem, (LPARAM)lpRect);
	}
	*/
	//LVM_GETITEMRECT message: https://msdn.microsoft.com/en-us/library/windows/desktop/bb761049.aspx
	//Retrieves the bounding rectangle for all or part of an item in the current view.
	//Returns TRUE if successful, or FALSE otherwise.
	//wParam: Index of the list-view item.
	//lParam: Pointer to a RECT structure that receives the bounding rectangle.
		//When the message is sent, the left member of this structure is used to specify the portion of the list-view item from which to retrieve the bounding rectangle.
		//It must be set to one of the following values:
			//LVIR_BOUNDS
			//LVIR_ICON
			//LVIR_LABEL
			//LVIR_SELECTBOUNDS
	ASSERT( ::IsWindow( hWnd ) );
	rect->left = nCode;
	return static_cast<BOOL>( ::SendMessageW( hWnd, LVM_GETITEMRECT, static_cast<WPARAM>( nItem ), reinterpret_cast<LPARAM>( rect ) ) );
	}

_Ret_range_( 0, 100 ) INT Treemap_Options::GetBrightnessPercent( ) const noexcept {
	return RoundDouble( brightness   * 100 );
	}


_Ret_range_( 0, 100 ) INT Treemap_Options::GetHeightPercent( ) const noexcept {
	return RoundDouble( height       * 100 );
	}

_Ret_range_( 0, 100 ) INT Treemap_Options::GetScaleFactorPercent( ) const noexcept {
	return RoundDouble( scaleFactor  * 100 );
	}


_Ret_range_( 0, 100 ) INT Treemap_Options::GetAmbientLightPercent( ) const noexcept {
	return RoundDouble( ambientLight * 100 );
	}

_Ret_range_( 0, 100 ) INT Treemap_Options::GetLightSourceXPercent( ) const noexcept {
	return RoundDouble( lightSourceX * 100 );
	}

_Ret_range_( 0, 100 ) INT Treemap_Options::GetLightSourceYPercent( ) const noexcept {
	return RoundDouble( lightSourceY * 100 );
	}

POINT Treemap_Options::GetLightSourcePoint( ) const noexcept {
	return POINT { GetLightSourceXPercent( ), GetLightSourceYPercent( ) };
	}

_Ret_range_( 0, 100 ) INT Treemap_Options::RoundDouble ( const DOUBLE d ) const noexcept {
	return ::signum( d ) * static_cast<INT>( ::abs( d ) + 0.5 );
	}

void Treemap_Options::SetBrightnessPercent( const INT    n   ) noexcept {
	brightness   = n / 100.0;
	}

void Treemap_Options::SetHeightPercent( const INT    n   ) noexcept {
	height       = n / 100.0;
	}

void Treemap_Options::SetScaleFactorPercent( const INT    n   ) noexcept {
	scaleFactor  = n / 100.0;
	}

void Treemap_Options::SetAmbientLightPercent( const INT    n   ) noexcept {
	ambientLight = n / 100.0;
	}

void Treemap_Options::SetLightSourceXPercent( const INT    n   ) noexcept {
	lightSourceX = n / 100.0; 
	}

void Treemap_Options::SetLightSourceYPercent( const INT    n   ) noexcept {
	lightSourceY = n / 100.0;
	}


void Treemap_Options::SetLightSourcePoint   ( const POINT  pt  ) noexcept {
	SetLightSourceXPercent( pt.x );
	SetLightSourceYPercent( pt.y );
	}

SRECT::SRECT( ) : left( 0 ), top( 0 ), right( 0 ), bottom( 0 ) { }
SRECT::SRECT( std::int16_t iLeft, std::int16_t iTop, std::int16_t iRight, std::int16_t iBottom ) : left { iLeft }, top { iTop }, right { iRight }, bottom { iBottom } { }

SRECT::SRECT( const RECT& in ) {
	left   = static_cast<std::int16_t>( in.right );
	top    = static_cast<std::int16_t>( in.top );
	right  = static_cast<std::int16_t>( in.right );
	bottom = static_cast<std::int16_t>( in.bottom );
	}


const int SRECT::Width( ) const noexcept {
	return right - left;
	}

const int SRECT::Height( ) const noexcept {
	return bottom - top;
	}

#else

#endif

HDC gdi::CreateCompatibleDeviceContext(_In_ const HDC hDC) {
	//CreateCompatibleDC function: https://msdn.microsoft.com/en-us/library/dd183489.aspx
	//If [CreateCompatibleDC] succeeds, the return value is the handle to a memory DC.
	//If [CreateCompatibleDC] fails, the return value is NULL.

	//When you no longer need the memory DC, call the DeleteDC function.
	//We recommend that you call DeleteDC to delete the DC.
	//However, you can also call DeleteObject with the HDC to delete the DC.

	const HDC newHDC = ::CreateCompatibleDC(hDC);
	if (newHDC == NULL) {
		TRACE(L"CreateCompatibleDeviceContext failed!\r\n");
		std::terminate();
	}

	return newHDC;
	}

void gdi::DeleteDeviceContext(_In_ _Post_ptr_invalid_ HDC hDC) {
	//DeleteDC function: https://msdn.microsoft.com/en-us/library/dd183533.aspx
	//If [DeleteDC] succeeds, the return value is nonzero.
	//If [DeleteDC] fails, the return value is zero.

	const BOOL deleted = ::DeleteDC(hDC);
	if (deleted == 0) {
		TRACE(L"DeleteDeviceContext failed!\r\n");
		std::terminate();
		}
	}

HBITMAP gdi::CreateCompatibleBitmap(_In_ HDC hDC, int cx, int cy) {
	//CreateCompatibleBitmap function: https://docs.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-createcompatiblebitmap
	//If the function succeeds, the return value is a handle to the compatible bitmap (DDB).
	//If the function fails, the return value is NULL.

	const HBITMAP bm = ::CreateCompatibleBitmap(hDC, cx, cy);
	if (bm == nullptr) {
		TRACE(L"CreateCompatibleBitmap failed!\r\n");
		std::terminate();
	}
	return bm;

	}
