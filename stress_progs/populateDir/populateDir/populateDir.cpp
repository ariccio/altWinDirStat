#include <assert.h>
#include <vector>
#include <string>
#include <Windows.h>
#include <cstdint>
//#include <exception>
#include <string>
//#include <strsafe.h>
#include <memory>
#include <algorithm>
#include <functional>
#include <random>
#include "Header.h"
#include <stdio.h>
#include <utility>


#ifdef NDEBUG
#define POPULATE_DIR_ASSERT_IF_DEBUG_ELSE_UNREFERENCED( var_name, cmp_op, cond ) UNREFERENCED_PARAMETER( var_name )
#else
#define POPULATE_DIR_ASSERT_IF_DEBUG_ELSE_UNREFERENCED( var_name, cmp_op, cond ) assert( ( ( var_name ) cmp_op ( cond ) ) )
#endif

#ifdef NDEBUG
#define TRACE_OUT_C_STYLE( x, fmt_spec ) wprintf( L"\r\n\t\t" L#x L" = `" L#fmt_spec L"` ", ##x )
#define TRACE_OUT_C_STYLE_ENDL( ) wprintf( L"\r\n" )
#else
#define TRACE_OUT_C_STYLE( x, fmt_spec ) assert( wprintf( L"\r\n\t\t" L#x L" = `" L#fmt_spec L"` ", ##x ) >= 0 )
#define TRACE_OUT_C_STYLE_ENDL( ) assert( wprintf( L"\r\n" ) >= 0 )
#endif


#ifndef WDS_WRITES_TO_STACK
#define WDS_WRITES_TO_STACK( strSize, chars_written ) _Out_writes_z_( strSize ) _Pre_writable_size_( strSize ) _Post_readable_size_( chars_written ) _Pre_satisfies_( strSize >= chars_written ) _Post_satisfies_( _Old_( chars_written ) <= chars_written )
#else
#error already defined!
#endif


//http://stackoverflow.com/questions/440133/how-do-i-create-a-random-alpha-numeric-string-in-c


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

//On returning E_FAIL, call GetLastError for details. That's not my idea!
_Success_( SUCCEEDED( return ) ) HRESULT CStyle_GetLastErrorAsFormattedMessage( WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_formatted_error, _In_range_( 128, 32767 ) const rsize_t strSize, _Out_ rsize_t& chars_written, const DWORD error ) {
	//const auto err = GetLastError( );
	const auto err = error;
	const auto ret = FormatMessageW( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, err, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), psz_formatted_error, static_cast< DWORD >( strSize ), NULL );
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



//static_assert( !SUCCEEDED( INVALID_HANDLE_VALUE ), "we got's a problem" );

void handle_invalid_handle_value( ) {
	const rsize_t err_buff_size = 512;
	const auto last_err = GetLastError( );
	wchar_t err_buff[ err_buff_size ] = { 0 };
	rsize_t chars_written = 0;
	const HRESULT err_res = CStyle_GetLastErrorAsFormattedMessage( err_buff, err_buff_size, chars_written, last_err );
	if ( SUCCEEDED( err_res ) ) {
		const auto wpf_res = wprintf( L"Error creating file: %s\r\n", err_buff );
		POPULATE_DIR_ASSERT_IF_DEBUG_ELSE_UNREFERENCED( wpf_res, >= , 0 );
		}
	else {
		const auto wpf_res = wprintf( L"Error creating file: %u (also, error getting error message)\r\n", last_err );
		POPULATE_DIR_ASSERT_IF_DEBUG_ELSE_UNREFERENCED( wpf_res, >= , 0 );
		}
	}

_At_( the_handle, _Pre_valid_ )
_At_( the_handle, _Post_invalid_ )
void close_single_handle( _In_ const HANDLE& the_handle ) {
	//"If the function succeeds, the return value is nonzero."
	const BOOL close_res = CloseHandle( the_handle );
	assert( close_res != 0 );
	if ( close_res != 0 ) {
		return;
		}
	const rsize_t err_buff_size = 512;
	wchar_t err_buff[ err_buff_size ] = { 0 };
	const auto last_err = GetLastError( );
	rsize_t chars_written = 0;
	const HRESULT err_res = CStyle_GetLastErrorAsFormattedMessage( err_buff, err_buff_size, chars_written, last_err );
	assert( SUCCEEDED( err_res ) );
	if ( SUCCEEDED( err_res ) ) {
		const auto wpf_res = wprintf( L"Error closing handle: %s\r\n", err_buff );
		POPULATE_DIR_ASSERT_IF_DEBUG_ELSE_UNREFERENCED( wpf_res, >= , 0 );
		}
	else {
		const auto wpf_res = wprintf( L"Error closing handle: %u (also, error getting error message)\r\n", last_err );
		POPULATE_DIR_ASSERT_IF_DEBUG_ELSE_UNREFERENCED( wpf_res, >= , 0 );
		}
	}

void handle_failed_to_create_event( const DWORD last_err ) {
	const rsize_t err_buff_size = 512;
	wchar_t err_buff[ err_buff_size ] = { 0 };
	rsize_t chars_written = 0;
	const HRESULT err_res = CStyle_GetLastErrorAsFormattedMessage( err_buff, err_buff_size, chars_written, last_err );
	if ( SUCCEEDED( err_res ) ) {
		const auto wpf_res = wprintf( L"Error creating event: %s\r\n", err_buff );
		POPULATE_DIR_ASSERT_IF_DEBUG_ELSE_UNREFERENCED( wpf_res, >= , 0 );
		}
	else {
		const auto wpf_res = wprintf( L"Error creating event: %u (also, error getting error message)\r\n", last_err );
		POPULATE_DIR_ASSERT_IF_DEBUG_ELSE_UNREFERENCED( wpf_res, >= , 0 );
		}
	}

void handle_failed_to_create_event( const DWORD last_err, _In_ const HANDLE& fileHandle ) {
	handle_failed_to_create_event( last_err );
	close_single_handle( fileHandle );
	}


void handle_failed_to_create_event_already_exists( ) {
	const auto wpf_res = wprintf( L"Error creating event: event already exists!\r\n" );
	POPULATE_DIR_ASSERT_IF_DEBUG_ELSE_UNREFERENCED( wpf_res, >= , 0 );
	}


void handle_failed_to_create_event_already_exists( _In_ const HANDLE& fileHandle ) {
	handle_failed_to_create_event_already_exists( );
	close_single_handle( fileHandle );
	}


_Pre_satisfies_( fileHandle != INVALID_HANDLE_VALUE )
_Pre_satisfies_( handle_event != NULL )
_At_( handle_event, _Pre_valid_ )
_At_( fileHandle, _Pre_valid_ )
_At_( handle_event, _Post_invalid_ )
_At_( fileHandle, _Post_invalid_ )
void close_handles( const HANDLE& handle_event, const HANDLE& fileHandle ) {
	assert( fileHandle != INVALID_HANDLE_VALUE );
	assert( handle_event != NULL );
	close_single_handle( handle_event );
	close_single_handle( fileHandle );
	}

void handle_error_getting_overlapped_result( ) {
	const rsize_t err_buff_size = 512;
	const auto get_overlapped_result_error = GetLastError( );
	assert( get_overlapped_result_error != ERROR_IO_INCOMPLETE );
	wchar_t err_buff[ err_buff_size ] = { 0 };
	rsize_t chars_written = 0;
	const HRESULT err_res = CStyle_GetLastErrorAsFormattedMessage( err_buff, err_buff_size, chars_written, get_overlapped_result_error );
	if ( SUCCEEDED( err_res ) ) {
		const auto wpf_res = wprintf( L"Error getting overlapped result: %s\r\n", err_buff );
		POPULATE_DIR_ASSERT_IF_DEBUG_ELSE_UNREFERENCED( wpf_res, >= , 0 );
		}
	else {
		const auto wpf_res = wprintf( L"Error getting overlapped result: %u, (also, error getting error message)\r\n", get_overlapped_result_error );
		POPULATE_DIR_ASSERT_IF_DEBUG_ELSE_UNREFERENCED( wpf_res, >= , 0 );
		}
	//close_handles( handle_event, fileHandle );
	}

void handle_error_writing_file( const DWORD write_file_error, _In_ const HANDLE& handle_event, _In_ const HANDLE& fileHandle ) {
	const rsize_t err_buff_size = 512;
	wchar_t err_buff[ err_buff_size ] = { 0 };
	rsize_t chars_written = 0;
	const HRESULT err_res = CStyle_GetLastErrorAsFormattedMessage( err_buff, err_buff_size, chars_written, write_file_error );
	if ( SUCCEEDED( err_res ) ) {
		const auto wpf_res = wprintf( L"Error writing file: %s\r\n", err_buff );
		POPULATE_DIR_ASSERT_IF_DEBUG_ELSE_UNREFERENCED( wpf_res, >= , 0 );
		}
	else {
		const auto wpf_res = wprintf( L"Error writing file: %u, (also, error getting error message)\r\n", write_file_error );
		POPULATE_DIR_ASSERT_IF_DEBUG_ELSE_UNREFERENCED( wpf_res, >= , 0 );
		}
	UNREFERENCED_PARAMETER( handle_event );
	UNREFERENCED_PARAMETER( fileHandle );
	//close_handles( handle_event, fileHandle );
	}

_Pre_satisfies_( overlapped_io_struct.hEvent != NULL )
void write_file_not_eq_TRUE( _In_ const HANDLE& fileHandle, _In_ const HANDLE& handle_event, _Inout_ OVERLAPPED& overlapped_io_struct, _In_ const DWORD write_file_error, _In_ const DWORD buffer_size ) {
	//const DWORD write_file_error = GetLastError( );
	if ( write_file_error == ERROR_IO_PENDING ) {
		DWORD bytes_transferred = 0;
		assert( overlapped_io_struct.hEvent != NULL );
		
		static_assert( INFINITE != 0, "" );

		//`If [GetOverlappedResultEx] fails, the return value is zero. To get extended error information, call GetLastError.`
		const BOOL overlapped_result = GetOverlappedResultEx( fileHandle, &overlapped_io_struct, &bytes_transferred, INFINITE, FALSE );
		
		if ( overlapped_result == 0 ) {
			handle_error_getting_overlapped_result( );
			return;
			}

		//if overlapped result is nonzero, then we should have a full buffer?
		assert( bytes_transferred == buffer_size );
		if ( bytes_transferred != buffer_size ) {
			handle_error_getting_overlapped_result( );
			return;
			}

		//wprintf( L"WriteFile succeeded! bytes written: %u\r\n", bytes_transferred );
		return;
		}
	//failed
	handle_error_writing_file( write_file_error, handle_event, fileHandle );

	return;
	}

const DWORD string_buffer_size_in_bytes( _In_ const std::wstring& newStr ) {
	static_assert( sizeof( wchar_t ) == sizeof( std::wstring::traits_type::char_type ), "" );
	return ( static_cast< DWORD >( newStr.size( ) ) * static_cast< DWORD >( sizeof( std::wstring::traits_type::char_type ) ) );
	}

OVERLAPPED init_overlapped( _In_ const HANDLE& handle_event ) {
	OVERLAPPED overlapped_io_struct = { 0 };
	overlapped_io_struct.Offset = 0;
	overlapped_io_struct.hEvent = handle_event;
	overlapped_io_struct.Internal = STATUS_PENDING;
	return overlapped_io_struct;
	}

void do_overlapped_write( _In_ const std::wstring& newStr, _In_ const HANDLE& fileHandle, _In_ const HANDLE& handle_event ) {
	auto overlapped_io_struct = init_overlapped( handle_event );

	const auto data_buffer_size = string_buffer_size_in_bytes( newStr );
	
	const BOOL val = WriteFile( fileHandle, newStr.c_str( ), data_buffer_size, NULL, &overlapped_io_struct );
	if ( val != TRUE ) {
		assert( val == 0 );
		const DWORD write_file_error = GetLastError( );
		write_file_not_eq_TRUE( fileHandle, handle_event, overlapped_io_struct, write_file_error, data_buffer_size );
		//close_handles( handle_event, fileHandle );
		return;
		}
	const auto wpf_res = wprintf( L"Successfully wrote to file %s, Bytes written: %u\r\n", newStr.c_str( ), overlapped_io_struct.Offset );
	POPULATE_DIR_ASSERT_IF_DEBUG_ELSE_UNREFERENCED( wpf_res, >= , 0 );
	//close_handles( handle_event, fileHandle );
	}

_Success_( return )
const bool create_event( _Out_ HANDLE& event_handle_in_buffer ) {
	event_handle_in_buffer = CreateEventW( NULL, TRUE, FALSE, NULL );
	const auto last_err = GetLastError( );
	if ( event_handle_in_buffer == NULL ) {
		handle_failed_to_create_event( last_err );
		return false;
		}
	if ( last_err == ERROR_ALREADY_EXISTS ) {
		handle_failed_to_create_event_already_exists( );
		return false;
		}
	return true;
	}

//void file_handle_not_invalid_handle_value( _In_ const std::wstring& newStr, _In_ const HANDLE& fileHandle ) {
//	const HANDLE handle_event = create_event( );
//	if ( handle_event == NULL ) {
//		close_single_handle( fileHandle );
//		return;
//		}
//
//	do_overlapped_write( newStr, fileHandle, handle_event );
//	close_handles( handle_event, fileHandle );
//	
//	//DWORD dummy;
//	//GetHandleInformation( fileHandle, &dummy );
//	return;
//	}

_Success_( return == true )
const bool open_file( _In_ const std::wstring newStr, _Out_ HANDLE& file_handle_in_buffer ) {
	const HANDLE fileHandle = CreateFileW( newStr.c_str( ), GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL );
	if ( fileHandle == INVALID_HANDLE_VALUE ) {
		handle_invalid_handle_value( );
		return false;
		}
	file_handle_in_buffer = fileHandle;
	return true;
	}

_Success_( return == true )
bool single_file( _In_ const std::wstring newStr, HANDLE& file_handle_in_buffer, HANDLE& event_handle_in_buffer ) {
	const auto file_handle_valid = open_file( newStr, file_handle_in_buffer );
	if ( !file_handle_valid ) {
		return false;
		}

	const bool handle_event_valid = create_event( event_handle_in_buffer );
	if ( !handle_event_valid ) {
		close_single_handle( file_handle_in_buffer );
		return false;
		}

	do_overlapped_write( newStr, file_handle_in_buffer, event_handle_in_buffer );
	close_handles( event_handle_in_buffer, file_handle_in_buffer );

	//DWORD dummy;
	//GetHandleInformation( fileHandle, &dummy );

	//file_handle_not_invalid_handle_value( newStr, fileHandle );
	return true;
	}

void fillDir( _In_ std::wstring theDir, _In_ const size_t iterations ) {
	///http://stackoverflow.com/a/12468109
	const char alnumChars_arr[ ] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z' };

	const std::vector<char> alnumChars( alnumChars_arr, ( alnumChars_arr + sizeof( alnumChars_arr ) - 1 ) );
	std::default_random_engine rng( std::random_device { }( ) );
	std::uniform_int_distribution<> dist( 6, int( sizeof( alnumChars ) - 1 ) );

	const auto randchar = [ alnumChars, &dist, &rng ] ( ) { return alnumChars[ dist( rng ) ]; };

	size_t iterations_so_far = 0;
	const auto retval = SetCurrentDirectoryW( theDir.c_str( ) );
	if ( !retval ) {
		const auto wpf_res = wprintf( L"SetCurrentDirectoryW( %s ) failed!\r\n", theDir.c_str( ) );
		POPULATE_DIR_ASSERT_IF_DEBUG_ELSE_UNREFERENCED( wpf_res, >= , 0 );
		return;
		}

	//std::vector<OVERLAPPED> overlapped_struct_buffer;
	auto overlapped_struct_buffer = std::make_unique<OVERLAPPED[ ]>( iterations );
	auto file_handle_buffer = std::make_unique<HANDLE[ ]>( iterations );
	auto event_handle_buffer = std::make_unique<HANDLE[ ]>( iterations );
	auto string_buffer = std::make_unique<std::wstring[ ]>( iterations );

	std::generate( &( string_buffer[ 0 ] ), &( string_buffer[ iterations - 1 ] ), [ &] ( ) { return random_string( dist( rng ), randchar ); } );

	for ( size_t i = 0; i < iterations; ++i ) {
		TRACE_OUT_C_STYLE( string_buffer[ i ].c_str( ), %s );
		}


	for ( size_t i = 0; i < iterations; ++i ) {
		++iterations_so_far;
		single_file( string_buffer[ i ], file_handle_buffer[ i ], event_handle_buffer[ i ] );
		}
	//fixWCout( );
	const auto fixed_iters_so_far = static_cast< std::uint64_t >( iterations_so_far );
	TRACE_OUT_C_STYLE( fixed_iters_so_far, %I64u );
	TRACE_OUT_C_STYLE_ENDL( );

	}

void usage( ) {
	const auto wpf_res_1 = wprintf( L"no arguments supplied, displaying usage.\r\n" );
	const auto wpf_res_2 = wprintf( L"usage:  `\"C:\\path\\to\\a\\directory\\to\\be\\filled\\with\\junk\" some_number_of_junk_files_to_create`\r\n" );
	const auto wpf_res_3 = wprintf( L"example: \"C:\\Users\\Alexander Riccio\\Documents\\test_junk_dir\\cpp_junk\" 150\r\n" );
	POPULATE_DIR_ASSERT_IF_DEBUG_ELSE_UNREFERENCED( wpf_res_1, >= , 0 );
	POPULATE_DIR_ASSERT_IF_DEBUG_ELSE_UNREFERENCED( wpf_res_2, >= , 0 );
	POPULATE_DIR_ASSERT_IF_DEBUG_ELSE_UNREFERENCED( wpf_res_3, >= , 0 );
	}

void trace_args( _In_ _In_range_( 0, INT_MAX ) const int& argc, _In_count_( argc ) _Readable_elements_( argc ) _Deref_prepost_z_ const wchar_t* const argv[ ] ) {
	const auto wpf_res_4 = wprintf( L"arguments passed: \r\n" );
	POPULATE_DIR_ASSERT_IF_DEBUG_ELSE_UNREFERENCED( wpf_res_4, >= , 0 );
	for ( int i = 0; i < argc; ++i ) {
		const auto wpf_res_5 = wprintf( L"\ti: %i", i );
		POPULATE_DIR_ASSERT_IF_DEBUG_ELSE_UNREFERENCED( wpf_res_5, >= , 0 );
		TRACE_OUT_C_STYLE( argv[ i ], %s );
		TRACE_OUT_C_STYLE_ENDL( );
		}
	}

void too_few_args( _In_ _In_range_( 0, INT_MAX ) const int& argc ) {
	const auto wpf_res_6 = wprintf( L"You passed %i arguments. Please pass more. (directory to fill, number of files)\r\n", argc );
	POPULATE_DIR_ASSERT_IF_DEBUG_ELSE_UNREFERENCED( wpf_res_6, >= , 0 );
	}

void wrap_filldir( _In_ _In_range_( 0, INT_MAX ) const int& argc, _In_count_( argc ) _Readable_elements_( argc ) _Deref_prepost_z_ const wchar_t* const argv[ ], _In_ const size_t number_files ) {
	UNREFERENCED_PARAMETER( argc );
	const auto qpc_1 = help_QueryPerformanceCounter( );

	fillDir( argv[ 1 ], number_files );
	
	const auto qpc_2 = help_QueryPerformanceCounter( );
	const auto qpf = help_QueryPerformanceFrequency( );
	const auto timing = ( static_cast< double >( qpc_2.QuadPart - qpc_1.QuadPart ) * ( static_cast< double >( 1.0 ) / static_cast< double >( qpf.QuadPart ) ) );
	
	const auto wpf_res_7 = wprintf( L"total time: %f\r\n", timing );
	POPULATE_DIR_ASSERT_IF_DEBUG_ELSE_UNREFERENCED( wpf_res_7, >= , 0 );
	
	const auto num_files_per_second = ( static_cast< double >( number_files ) / timing );
	const auto wpf_res_8 = wprintf( L"number of files per second: %f\r\n", num_files_per_second );
	POPULATE_DIR_ASSERT_IF_DEBUG_ELSE_UNREFERENCED( wpf_res_8, >= , 0 );
	}

int wmain( _In_ _In_range_( 0, INT_MAX ) int argc, _In_count_( argc ) _Readable_elements_( argc ) _Deref_prepost_z_ wchar_t* argv[ ] ) {
	if ( argc == 1 ) {
		usage( );
		return 0;
		}

	trace_args( argc, argv );

	assert( argv[ argc ] == NULL );
	TRACE_OUT_C_STYLE_ENDL( );
	if ( argc < 2 ) {
		too_few_args( argc );
		return -1;
		}

	const std::wstring number_files_str( argv[ 2 ] );
	std::uint64_t number_files_temp = 0;
	try {
		number_files_temp = std::stoull( number_files_str );
		}
	catch ( const std::exception& e ) {
		const auto pf_res = printf( "Exception thrown while converting argv[ 2 ] (`%S`) to std::uint64_t! Exception: %s\r\n", argv[ 2 ], e.what( ) );
		POPULATE_DIR_ASSERT_IF_DEBUG_ELSE_UNREFERENCED( pf_res, >= , 0 );
		return 666;
		}

	const auto number_files = static_cast<size_t>( number_files_temp );
	wrap_filldir( argc, argv, number_files );
	return 0;
	}