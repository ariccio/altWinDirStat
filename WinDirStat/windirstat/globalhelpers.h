// globalhelpers.h	- Declaration of global helper functions
//
// see `file_header_text.txt` for licensing & contact info.

#pragma once

#include "stdafx.h"

#ifndef WDS_GLOBALHELPERS_H
#define WDS_GLOBALHELPERS_H



//must be inline, else compiler bitches about ODR!
//should really just be a void* and a size_t?
template<typename type_struct_to_memset>
inline void memset_zero_struct( type_struct_to_memset& the_struct ) {
	static_assert( std::is_pod<type_struct_to_memset>::value, "can't memset a non-pod struct!" );
	static_assert( !std::is_polymorphic<type_struct_to_memset>::value, "can't memset a polymorphic type!" );
	static_assert( std::is_standard_layout<type_struct_to_memset>::value, "can't memset a non-standard layout struct!" );
	memset( &the_struct, 0, sizeof( the_struct ) );
	}

//must be inline, else compiler bitches about ODR!
template<typename type_struct_to_init>
inline type_struct_to_init zero_init_struct( ) {
	static_assert( std::is_pod<type_struct_to_init>::value, "can't memset a non-pod struct!" );
	static_assert( !std::is_polymorphic<type_struct_to_init>::value, "can't memset a polymorphic type!" );
	static_assert( std::is_standard_layout<type_struct_to_init>::value, "can't memset a non-standard layout struct!" );
	static_assert( std::is_trivially_default_constructible<type_struct_to_init>::value, "can't memset a struct that isn't trivially default constructable!" );
	static_assert( std::is_trivially_copyable<type_struct_to_init>::value, "might have trouble returning a non-trivially-copyable item by value. You've been warned!" );
	type_struct_to_init the_struct;
	//memset( &the_struct, 0, sizeof( the_struct ) );
	memset_zero_struct( the_struct );
	return the_struct;
	}


struct Children_String_Heap_Manager {

	//TODO: inline these?
	Children_String_Heap_Manager( );
	Children_String_Heap_Manager( const rsize_t number_of_characters_needed );
	Children_String_Heap_Manager& operator=( const Children_String_Heap_Manager& in ) = delete;
	Children_String_Heap_Manager( const Children_String_Heap_Manager& in ) = delete;

	void reset( const rsize_t number_of_characters_needed );

	_Success_( SUCCEEDED( return ) )
	const HRESULT copy_name_str_into_buffer( _Pre_invalid_ _Post_z_ _Post_readable_size_( new_name_length ) wchar_t*& new_name_ptr, _In_ _In_range_( 0, UINT16_MAX ) const rsize_t& new_name_length, const std::wstring& name );

	_Field_size_part_( m_buffer_size, m_buffer_filled ) std::unique_ptr<wchar_t[ ]> m_string_buffer;
	size_t m_buffer_size;
	size_t m_buffer_filled;

	};


struct QPC_timer {
	QPC_timer( );
	void begin( );
	void end( );
	const double total_time_elapsed( ) const;
	QPC_timer& operator=( const QPC_timer& in ) = delete;

	const std::int64_t m_frequency;
	std::int64_t m_start;
	std::int64_t m_end;
	};

_Pre_satisfies_( handle != INVALID_HANDLE_VALUE )
_At_( handle, _Post_invalid_ )
_At_( handle, _Pre_valid_ )
void close_handle( const HANDLE handle );

_Pre_satisfies_( rect.left > rect.right ) _Post_satisfies_( rect.left <= rect.right )
inline void normalize_RECT_left_right( _Inout_ RECT& rect );

_Pre_satisfies_( rect.top > rect.bottom ) _Post_satisfies_( rect.top <= rect.bottom )
inline void normalize_RECT_top_bottom( _Inout_ RECT& rect );

_Post_satisfies_( rect.left <= rect.right ) _Post_satisfies_( rect.top <= rect.bottom )
void normalize_RECT( _Inout_ RECT& rect );

void error_getting_pointer_to( _In_z_ PCWSTR const function_name );
void test_if_null_funcptr( void* func_ptr, _In_z_ PCWSTR const function_name );

void InitializeCriticalSection_wrapper( _Pre_invalid_ _Post_valid_ _Out_ CRITICAL_SECTION& cs );

void DeleteCriticalSection_wrapper( _Pre_valid_ _Post_invalid_ CRITICAL_SECTION& cs );

//On returning E_FAIL, call GetLastError for details. That's not my idea!
_Success_( SUCCEEDED( return ) ) HRESULT CStyle_GetLastErrorAsFormattedMessage( WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_formatted_error, _In_range_( 128, 32767 ) const rsize_t strSize, _Out_ rsize_t& chars_written, const DWORD error = GetLastError( ) );


//Unused?
//_Success_( SUCCEEDED( return ) ) HRESULT GetFullPathName_WriteToStackBuffer( _In_z_ PCWSTR const relativePath, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_full_path, _In_range_( 128, 512 ) const DWORD strSize, _Out_ rsize_t& chars_written );

//Unused?
//_Success_( return ) bool MyQueryDosDevice           ( _In_z_ PCWSTR const drive, _Out_ _Post_z_ wchar_t ( &info )[ 512u ] );


//Unused?
//std::wstring dynamic_GetFullPathName( _In_z_ PCWSTR const relativePath );

void unexpected_strsafe_invalid_parameter_handler( _In_z_ PCSTR const strsafe_func_name, _In_z_ PCSTR const file_name_in, _In_z_ PCSTR const func_name_in, _In_ _In_range_( 0, INT_MAX ) const int line_number_in );

void handle_stack_insufficient_buffer( _In_ const rsize_t str_size, _In_ const rsize_t generic_size_needed, _Out_ rsize_t& size_buff_need, _Out_ rsize_t& chars_written );

//WinDirStat string-formatting functions
namespace wds_fmt {

	_Success_( SUCCEEDED( return ) ) HRESULT CStyle_FormatLongLongHuman ( _In_ std::uint64_t n,       WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_formatted_LONGLONG_HUMAN, _In_range_( 19, 64 ) const rsize_t strSize, _Out_ rsize_t& chars_written );

	//maximum representable integral component of a double SEEMS to be 15 characters long, so we need at least 17
	_Success_( SUCCEEDED( return ) ) HRESULT CStyle_FormatDouble        ( _In_ const DOUBLE d,        WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_formatted_double, _In_range_( 17, 64 ) const rsize_t strSize, _Out_ rsize_t& chars_written );

	_Success_( SUCCEEDED( return ) ) HRESULT FormatBytes                ( _In_ const std::uint64_t n, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_formatted_bytes, _In_range_( 38, 64 ) const rsize_t strSize, _Out_ rsize_t& chars_written, _On_failure_( _Post_valid_ ) rsize_t& size_needed );

	_Success_( SUCCEEDED( return ) ) HRESULT CStyle_FormatFileTime  ( _In_ const FILETIME t,    _Out_writes_z_( strSize ) _Pre_writable_size_( strSize ) PWSTR psz_formatted_datetime, _In_range_( 128, 2048 ) const rsize_t strSize, _Out_ rsize_t& chars_written );

	_Success_( SUCCEEDED( return ) ) HRESULT CStyle_FormatAttributes( _In_ const attribs& attr, _Out_writes_z_( strSize ) _Pre_writable_size_( strSize ) PWSTR psz_formatted_attributes, _In_range_( 6, 18 ) const rsize_t strSize, _Out_ rsize_t& chars_written  );

	_Success_( SUCCEEDED( return ) ) HRESULT CStyle_GetNumberFormatted( const std::int64_t number, _Pre_writable_size_( strSize ) PWSTR psz_formatted_number, _In_range_( 21, 64 ) const rsize_t strSize, _Out_ rsize_t& chars_written );

	std::wstring FormatBytes( _In_ const std::uint64_t n, const bool humanFormat );
	

	void write_MEM_INFO_ERR ( _Out_writes_z_( 13 ) _Pre_writable_size_( 13 ) PWSTR psz_formatted_usage );

	void write_RAM_USAGE    ( _Out_writes_z_( 12 ) _Pre_writable_size_( 13 ) PWSTR psz_ram_usage );
	
	void write_bad_fmt_msg  ( _Out_writes_z_( 41 ) _Pre_writable_size_( 42 ) _Post_readable_size_( chars_written ) PWSTR psz_fmt_msg, _Out_ rsize_t& chars_written );
	
	void write_BAD_FMT      ( _Out_writes_z_( 8 )  _Pre_writable_size_( 8 ) _Post_readable_size_( 8 ) PWSTR pszFMT, _Out_ rsize_t& chars_written );
	
	//Unused?
	//void FormatVolumeName   ( _In_ const std::wstring& rootPath, _In_z_ PCWSTR const volumeName, _Out_ _Post_z_ _Pre_writable_size_( MAX_PATH + 1u ) PWSTR formatted_volume_name );
	}


_Success_( SUCCEEDED( return ) ) const HRESULT allocate_and_copy_name_str( _Pre_invalid_ _Post_z_ _Post_readable_size_( new_name_length ) wchar_t*& new_name_ptr, _In_ _In_range_( 0, UINT16_MAX ) const rsize_t& new_name_length, const std::wstring& name );

                             //Unused?
                             //bool DriveExists       ( _In_z_ _In_reads_( path_len ) PCWSTR const path, _In_ _In_range_( 0, 4 ) const rsize_t path_len );

							 //Unused?
//_Success_( return != false ) bool GetVolumeName     ( _In_z_ PCWSTR const rootPath,    _Out_ _Post_z_ wchar_t ( &volumeName )[ MAX_PATH + 1u ]                        );
//_Success_( return != false ) bool GetVolumeName     ( _In_z_ PCWSTR const rootPath );

//void check8Dot3NameCreationAndNotifyUser( );

void displayWindowsMsgBoxWithError  ( const DWORD error = GetLastError( ) );

void displayWindowsMsgBoxWithMessage( const std::wstring message );
void displayWindowsMsgBoxWithMessage( const std::string message );
void displayWindowsMsgBoxWithMessage( PCWSTR const message );


//Unused?
//void MyGetDiskFreeSpace             ( _In_z_ PCWSTR const pszRootPath, _Out_ _Out_range_( 0, 18446744073709551615 ) std::uint64_t& total, _Out_ _Out_range_( 0, 18446744073709551615 ) std::uint64_t& unused   );

//void trace_helper(  )

const LARGE_INTEGER help_QueryPerformanceCounter( );
const LARGE_INTEGER help_QueryPerformanceFrequency( );

//Unused?
//std::wstring EncodeSelection( _In_ const RADIO radio, _In_ const std::wstring folder, _In_ const std::vector<std::wstring>& drives );

RECT BuildRECT( const SRECT& in );

_Pre_satisfies_( min_val < max_val )
_Post_satisfies_( min_val <= val )
_Post_satisfies_( val <= max_val )
void CheckMinMax( _Inout_ LONG& val, _In_ const LONG min_val, _In_ const LONG max_val );

_Pre_satisfies_( min_val < max_val )
_Post_satisfies_( min_val <= val )
_Post_satisfies_( val <= max_val )
void CheckMinMax( _Inout_ LONG& val, _In_ const INT min_val, _In_ const INT max_val );

_Pre_satisfies_( min_val < max_val )
_Post_satisfies_( min_val <= val )
_Post_satisfies_( val <= max_val )
void CheckMinMax( _Inout_ INT& val,  _In_ const INT min_val, _In_ const INT max_val );


//bool Compare_FILETIME_lessthan ( const FILETIME& t1,  const FILETIME& t2  ) ;
bool Compare_FILETIME_eq   ( const FILETIME& lhs, const FILETIME& rhs ) ;

//Compare_FILETIME compiles to only 6 instructions, and is only called once, conditionally.
//INT  Compare_FILETIME      ( const FILETIME& lhs, const FILETIME& rhs ) ;

void DistributeFirst( _Inout_ _Out_range_( 0, 255 ) INT& first, _Inout_ _Out_range_( 0, 255 ) INT& second, _Inout_ _Out_range_( 0, 255 ) INT& third ) ;
void NormalizeColor( _Inout_ _Out_range_( 0, 255 ) INT& red, _Inout_ _Out_range_( 0, 255 ) INT& green, _Inout_ _Out_range_( 0, 255 ) INT& blue ) ;

namespace CColorSpace {

	// Gives a color a defined brightness.
	//static COLORREF MakeBrightColor( _In_ const COLORREF color, _In_ _In_range_(0, 1) const DOUBLE brightness );
	COLORREF MakeBrightColor( _In_ const COLORREF color, _In_ _In_range_(0, 1) const DOUBLE brightness );
	}

#ifdef DEBUG

#ifdef COLOR_DEBUGGING
void trace_m_stripe_color_make_bright_color( _In_ const COLORREF m_windowColor, _In_ const DOUBLE b );
void trace_m_stripeColor( _In_ const COLORREF m_stripeColor );
#endif

void trace_on_destroy( _In_z_ PCWSTR const m_persistent_name );
void trace_prof_string( _In_z_ PCWSTR const section, _In_z_ PCWSTR const entry, _In_z_ PCWSTR const value );
void trace_no_vol_mnt( _In_z_ PCWSTR const volume );
void trace_fs_not_rea( _In_z_ PCWSTR const volume );
void trace_no_reparse( _In_z_ PCWSTR const volume );
void trace_GetVolumeNameForVolumeMountPoint_failed( _In_z_ PCWSTR const volume );
void trace_mntpt_found( _In_z_ PCWSTR const path, _In_z_ PCWSTR const volume );

void trace_full_path( _In_z_ PCWSTR const path );
#endif

#endif
