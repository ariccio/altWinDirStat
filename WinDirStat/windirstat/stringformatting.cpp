// stringformatting.cpp
//
// see `file_header_text.txt` for licensing & contact info. If you can't find that file, then assume you're NOT allowed to do whatever you wanted to do.
#include "stdafx.h"

#pragma once


#ifndef WDS_STRINGFORMATTING_CPP_INCLUDED
#define WDS_STRINGFORMATTING_CPP_INCLUDED


WDS_FILE_INCLUDE_MESSAGE

#include "stringformatting.h"
#include "globalhelpers.h"


namespace {

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

	//maximum representable integral component of a double SEEMS to be 15 characters long, so we need at least 17
	//The compiler will automatically inline if /Ob2 is on, so we'll ask anyways.
	_Success_( SUCCEEDED( return ) ) inline HRESULT CStyle_FormatDouble( _In_ const DOUBLE d, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_formatted_double, _In_range_( 17, 64 ) const rsize_t strSize, _Out_ rsize_t& chars_written ) {
		rsize_t chars_remaining = 0;
		const HRESULT fmt_res = StringCchPrintfExW( psz_formatted_double, strSize, NULL, &chars_remaining, 0, L"%.1f", d );
		ASSERT( SUCCEEDED( fmt_res ) );
		if ( SUCCEEDED( fmt_res ) ) {
			chars_written = ( strSize - chars_remaining );
			ASSERT( wcslen( psz_formatted_double ) == chars_written );
			return fmt_res;
			}
		WDS_ASSERT_EXPECTED_STRING_FORMAT_FAILURE_HRESULT( fmt_res );
		WDS_STRSAFE_E_INVALID_PARAMETER_HANDLER( fmt_res, "StringCchPrintfExW" );
		if ( fmt_res == STRSAFE_E_INSUFFICIENT_BUFFER ) {
			chars_written = strSize;
			return fmt_res;
			}
		chars_written = 0;
		return fmt_res;
		}

	_Success_( SUCCEEDED( return ) ) inline HRESULT CStyle_FormatLongLongHuman_0( WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_formatted_LONGLONG_HUMAN, _In_range_( 8, 64 ) const rsize_t strSize, _Out_ rsize_t& chars_written ) {
		ASSERT( strSize > 2 );
		if ( strSize > 2 ) {
			psz_formatted_LONGLONG_HUMAN[ 0 ] = L'0';
			psz_formatted_LONGLONG_HUMAN[ 1 ] = 0;
			chars_written = 1;
			ASSERT( wcslen( psz_formatted_LONGLONG_HUMAN ) == chars_written );
			return S_OK;
			}
		chars_written = 0;
		return STRSAFE_E_INSUFFICIENT_BUFFER;
		}


	_Success_( SUCCEEDED( return ) ) HRESULT CStyle_FormatLongLongHuman_B( WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_formatted_LONGLONG_HUMAN, _In_range_( 8, 64 ) const rsize_t strSize, _Out_ rsize_t& chars_written, _In_ const DOUBLE B ) {
		size_t remaining_chars = 0;
		const HRESULT res = StringCchPrintfExW( psz_formatted_LONGLONG_HUMAN, strSize, NULL, &remaining_chars, 0, L"%i Bytes", static_cast<INT>( B ) );
		ASSERT( SUCCEEDED( res ) );
		if ( SUCCEEDED( res ) ) {
			ASSERT( strSize >= remaining_chars );
			chars_written = ( strSize - remaining_chars );
			return res;
			}
		WDS_ASSERT_EXPECTED_STRING_FORMAT_FAILURE_HRESULT( res );
		WDS_STRSAFE_E_INVALID_PARAMETER_HANDLER( res, "StringCchPrintfExW" );
		//chars_written = strSize;
		wds_fmt::write_BAD_FMT( psz_formatted_LONGLONG_HUMAN, chars_written );
		//worst case, need ~( 28 + 6 )
		//handle_stack_insufficient_buffer( strSize, 34u,  )
		return res;
		}

	_Success_( SUCCEEDED( return ) ) _Pre_satisfies_( chars_written == 0 )
	HRESULT CStyle_FormatLongLongHuman_KB( WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_formatted_LONGLONG_HUMAN, _In_range_( 23, 64 ) const rsize_t strSize, _Out_ rsize_t& chars_written, _In_ const DOUBLE B, _In_ const DOUBLE KB ) {
		const rsize_t number_formatted_buffer_size = 19;
		_Null_terminated_ wchar_t buffer[ number_formatted_buffer_size ] = { 0 };
		rsize_t buffer_chars_written = 0;
		const HRESULT res = CStyle_FormatDouble( KB + B / WDS_INT_BASE, buffer, number_formatted_buffer_size, buffer_chars_written );
		ASSERT( SUCCEEDED( res ) );
		if ( !SUCCEEDED( res ) ) {
			WDS_ASSERT_EXPECTED_STRING_FORMAT_FAILURE_HRESULT( res );
			WDS_STRSAFE_E_INVALID_PARAMETER_HANDLER( res, "wds_fmt::CStyle_FormatDouble" );
			wds_fmt::write_BAD_FMT( psz_formatted_LONGLONG_HUMAN, chars_written );
			return res;
			}
		
		ASSERT( wcslen( buffer ) == buffer_chars_written );
		rsize_t chars_remaining = 0;
		const HRESULT fmt_res = StringCchPrintfExW( psz_formatted_LONGLONG_HUMAN, strSize, NULL, &chars_remaining, 0, L"%s KB", buffer );
		ASSERT( SUCCEEDED( fmt_res ) );
		if ( SUCCEEDED( fmt_res ) ) {
			chars_written = ( strSize - chars_remaining );
			ASSERT( wcslen( psz_formatted_LONGLONG_HUMAN ) == chars_written );
			return fmt_res;
			}
		WDS_ASSERT_EXPECTED_STRING_FORMAT_FAILURE_HRESULT( fmt_res );
		WDS_STRSAFE_E_INVALID_PARAMETER_HANDLER( fmt_res, "StringCchPrintfExW" );

		if ( fmt_res == STRSAFE_E_INSUFFICIENT_BUFFER ) {
			chars_written = strSize;
			ASSERT( chars_written == strSize );
			return fmt_res;
			}

		chars_written = 0;
		wds_fmt::write_BAD_FMT( psz_formatted_LONGLONG_HUMAN, chars_written );
		return fmt_res;
		}

	_Success_( SUCCEEDED( return ) ) HRESULT CStyle_FormatLongLongHuman_MB( WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_formatted_LONGLONG_HUMAN, _In_range_( 23, 64 ) const rsize_t strSize, _Out_ rsize_t& chars_written, _In_ const DOUBLE KB, _In_ const DOUBLE MB ) {
		const rsize_t number_formatted_buffer_size = 19;
		_Null_terminated_ wchar_t buffer[ number_formatted_buffer_size ] = { 0 };
		rsize_t buffer_chars_written = 0;
		const HRESULT res = CStyle_FormatDouble( MB + KB / WDS_INT_BASE, buffer, number_formatted_buffer_size, buffer_chars_written );
		ASSERT( SUCCEEDED( res ) );
		if ( !SUCCEEDED( res ) ) {
			WDS_ASSERT_EXPECTED_STRING_FORMAT_FAILURE_HRESULT( res );
			WDS_STRSAFE_E_INVALID_PARAMETER_HANDLER( res, "wds_fmt::CStyle_FormatDouble" );
			wds_fmt::write_BAD_FMT( psz_formatted_LONGLONG_HUMAN, chars_written );
			return res;
			}
		
		ASSERT( wcslen( buffer ) == buffer_chars_written );
		rsize_t chars_remaining = 0;
		const HRESULT fmt_res = StringCchPrintfExW( psz_formatted_LONGLONG_HUMAN, strSize, NULL, &chars_remaining, 0, L"%s MB", buffer );
		ASSERT( SUCCEEDED( fmt_res ) );
		if ( SUCCEEDED( fmt_res ) ) {
			chars_written = ( strSize - chars_remaining );
			ASSERT( wcslen( psz_formatted_LONGLONG_HUMAN ) == chars_written );
			return fmt_res;
			}
		WDS_ASSERT_EXPECTED_STRING_FORMAT_FAILURE_HRESULT( fmt_res );
		WDS_STRSAFE_E_INVALID_PARAMETER_HANDLER( fmt_res, "StringCchPrintfExW" );
		if ( fmt_res == STRSAFE_E_INSUFFICIENT_BUFFER ) {
			chars_written = strSize;
			ASSERT( chars_written == strSize );
			return fmt_res;
			}
		chars_written = 0;
		wds_fmt::write_BAD_FMT( psz_formatted_LONGLONG_HUMAN, chars_written );
		return fmt_res;
		}

	_Success_( SUCCEEDED( return ) ) HRESULT CStyle_FormatLongLongHuman_GB( WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_formatted_LONGLONG_HUMAN, _In_range_( 8, 64 ) const rsize_t strSize, _Out_ rsize_t& chars_written, _In_ const DOUBLE MB, _In_ const DOUBLE GB ) {
		const rsize_t number_formatted_buffer_size = 19;
		_Null_terminated_ wchar_t buffer[ number_formatted_buffer_size ] = { 0 };
		rsize_t buffer_chars_written = 0;
		const HRESULT res = CStyle_FormatDouble( GB + MB / WDS_INT_BASE, buffer, number_formatted_buffer_size, buffer_chars_written );
		ASSERT( SUCCEEDED( res ) );
		if ( !SUCCEEDED( res ) ) {
			WDS_ASSERT_EXPECTED_STRING_FORMAT_FAILURE_HRESULT( res );
			WDS_STRSAFE_E_INVALID_PARAMETER_HANDLER( res, "wds_fmt::CStyle_FormatDouble" );
			wds_fmt::write_BAD_FMT( psz_formatted_LONGLONG_HUMAN, chars_written );
			return res;
			}
		
		ASSERT( wcslen( buffer ) == buffer_chars_written );
		rsize_t chars_remaining = 0;
		const HRESULT fmt_res = StringCchPrintfExW( psz_formatted_LONGLONG_HUMAN, strSize, NULL, &chars_remaining, 0, L"%s GB", buffer );
		ASSERT( SUCCEEDED( fmt_res ) );
		if ( SUCCEEDED( fmt_res ) ) {
			chars_written = ( strSize - chars_remaining );
			ASSERT( wcslen( psz_formatted_LONGLONG_HUMAN ) == chars_written );
			return fmt_res;
			}
		WDS_ASSERT_EXPECTED_STRING_FORMAT_FAILURE_HRESULT( fmt_res );
		WDS_STRSAFE_E_INVALID_PARAMETER_HANDLER( fmt_res, "StringCchPrintfExW" );
		if ( fmt_res == STRSAFE_E_INSUFFICIENT_BUFFER ) {
			chars_written = strSize;
			ASSERT( chars_written == strSize );
			return fmt_res;
			}
		chars_written = 0;
		wds_fmt::write_BAD_FMT( psz_formatted_LONGLONG_HUMAN, chars_written );
		return fmt_res;
		}

	_Success_( SUCCEEDED( return ) ) HRESULT CStyle_FormatLongLongHuman_TB( WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_formatted_LONGLONG_HUMAN, _In_range_( 8, 64 ) const rsize_t strSize, _Out_ rsize_t& chars_written, _In_ const DOUBLE GB, _In_ const DOUBLE TB ) {
		const rsize_t number_formatted_buffer_size = 19;
		_Null_terminated_ wchar_t buffer[ number_formatted_buffer_size ] = { 0 };
		rsize_t buffer_chars_written = 0;
		const HRESULT res = CStyle_FormatDouble( TB + GB / WDS_INT_BASE, buffer, number_formatted_buffer_size, buffer_chars_written );
		ASSERT( SUCCEEDED( res ) );
		if ( !SUCCEEDED( res ) ) {
			WDS_ASSERT_EXPECTED_STRING_FORMAT_FAILURE_HRESULT( res );
			WDS_STRSAFE_E_INVALID_PARAMETER_HANDLER( res, "wds_fmt::CStyle_FormatDouble" );
			wds_fmt::write_BAD_FMT( psz_formatted_LONGLONG_HUMAN, chars_written );
			return res;
			}
		ASSERT( wcslen( buffer ) == buffer_chars_written );
		rsize_t chars_remaining = 0;
		const HRESULT fmt_res = StringCchPrintfExW( psz_formatted_LONGLONG_HUMAN, strSize, NULL, &chars_remaining, 0, L"%s TB", buffer );
		ASSERT( SUCCEEDED( fmt_res ) );
		if ( SUCCEEDED( fmt_res ) ) {
			chars_written = ( strSize - chars_remaining );
			ASSERT( wcslen( psz_formatted_LONGLONG_HUMAN ) == chars_written );
			return fmt_res;
			}
		WDS_ASSERT_EXPECTED_STRING_FORMAT_FAILURE_HRESULT( fmt_res );
		WDS_STRSAFE_E_INVALID_PARAMETER_HANDLER( fmt_res, "StringCchPrintfExW" );
		if ( fmt_res == STRSAFE_E_INSUFFICIENT_BUFFER ) {
			chars_written = strSize;
			ASSERT( chars_written == strSize );
			return fmt_res;
			}
		chars_written = 0;
		wds_fmt::write_BAD_FMT( psz_formatted_LONGLONG_HUMAN, chars_written );
		return fmt_res;
		}


	_Success_( SUCCEEDED( return ) ) HRESULT CStyle_FormatLongLongHuman( _In_ std::uint64_t n, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_formatted_LONGLONG_HUMAN, _In_range_( 19, 64 ) const rsize_t strSize, _Out_ rsize_t& chars_written ) {
		//MAX value of a LONGLONG is 19 digits
		const DOUBLE B  = static_cast<INT>( n % WDS_INT_BASE );
		n /= WDS_INT_BASE;
		const DOUBLE KB = static_cast<INT>( n % WDS_INT_BASE );
		n /= WDS_INT_BASE;
		const DOUBLE MB = static_cast<INT>( n % WDS_INT_BASE );
		n /= WDS_INT_BASE;
		const DOUBLE GB = static_cast<INT>( n % WDS_INT_BASE );
		n /= WDS_INT_BASE;
		const DOUBLE TB = static_cast<INT>( n );

		if (      ( TB != 0 ) || ( GB == WDS_INT_BASE - 1 ) && ( MB >= WDS_HALF_BASE ) ) {
			return CStyle_FormatLongLongHuman_TB( psz_formatted_LONGLONG_HUMAN, strSize, chars_written, GB, TB );
			}
		else if ( ( GB != 0 ) || ( MB == WDS_INT_BASE - 1 ) && ( KB >= WDS_HALF_BASE ) ) {
			return CStyle_FormatLongLongHuman_GB( psz_formatted_LONGLONG_HUMAN, strSize, chars_written, MB, GB );
			}
		else if ( ( MB != 0 ) || ( KB == WDS_INT_BASE - 1 ) && (  B >= WDS_HALF_BASE ) ) {
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

	}


_Success_( SUCCEEDED( return ) ) HRESULT wds_fmt::FormatBytes( _In_ const std::uint64_t n, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_formatted_bytes, _In_range_( 38, 64 ) const rsize_t strSize, _Out_ rsize_t& chars_written, _On_failure_( _Post_valid_ ) rsize_t& size_needed ) noexcept {
	const auto res = CStyle_FormatLongLongHuman( n, psz_formatted_bytes, strSize, chars_written );
	ASSERT( SUCCEEDED( res ) );
	if ( SUCCEEDED( res ) ) {
		return res;
		}
	WDS_ASSERT_EXPECTED_STRING_FORMAT_FAILURE_HRESULT( res );
	WDS_STRSAFE_E_INVALID_PARAMETER_HANDLER( res, "CStyle_FormatLongLongHuman" );
	wds_fmt::write_BAD_FMT( psz_formatted_bytes, chars_written );
	size_needed = ( strSize * 2 );
	return res;
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
	auto res = CStyle_FormatLongLongHuman( n, psz_formatted_longlong, strSize, chars_written );
	if ( SUCCEEDED( res ) ) {
		return psz_formatted_longlong;
		}
	wds_fmt::write_BAD_FMT( psz_formatted_longlong, chars_written );
	return psz_formatted_longlong;
	}


//This is an error handling function, and is intended to be called rarely!
__declspec(noinline)
void wds_fmt::write_BAD_FMT( _Out_writes_z_( 8 ) _Pre_writable_size_( 8 ) _Post_readable_size_( 8 ) PWSTR pszFMT, _Out_ rsize_t& chars_written ) noexcept {
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



#endif