#include <Windows.h>
#include <memory>
#include <string>
#include <numeric>
#include <cstdint>
#include <stdlib.h>
#include <strsafe.h>
#include <iso646.h>
#include <wctype.h>
#include <stdio.h>
#include <exception>

struct Children_String_Heap_Manager {
	Children_String_Heap_Manager& operator=( const Children_String_Heap_Manager& in ) = delete;
	Children_String_Heap_Manager( const rsize_t number_of_characters_needed ) : m_buffer_size( number_of_characters_needed ), m_buffer_filled( 0 ), m_string_buffer( new wchar_t[ number_of_characters_needed ] ) { }
	~Children_String_Heap_Manager( ) = default;
	_Field_size_part_( m_buffer_size, m_buffer_filled ) std::unique_ptr<wchar_t[ ]> m_string_buffer;
	const size_t m_buffer_size;
	size_t m_buffer_filled;

	_Success_( SUCCEEDED( return ) )
	const HRESULT copy_name_str_into_buffer( _Pre_invalid_ _Post_z_ _Post_readable_size_( new_name_length ) wchar_t*& new_name_ptr, _In_ _In_range_( 0, UINT16_MAX ) const rsize_t& new_name_length, const std::wstring& name ) {
		if ( !( new_name_length < UINT16_MAX ) ) {
			throw std::logic_error( "too long!" );
			}
		new_name_ptr = ( m_string_buffer.get( ) + m_buffer_filled );
		if ( !( ( m_buffer_filled + new_name_length ) < m_buffer_size ) ) {
			throw std::logic_error( "Bad arithmetic!" );
			}
		m_buffer_filled += new_name_length;

		PWSTR pszend = NULL;

		const rsize_t buffer_space_remaining = ( m_buffer_size - m_buffer_filled + new_name_length );

		rsize_t chars_remaining = ( buffer_space_remaining );
		const HRESULT res = StringCchCopyExW( new_name_ptr, ( buffer_space_remaining ), name.c_str( ), &pszend, &chars_remaining, 0 );
		if ( !( SUCCEEDED( res ) ) ) {
			throw std::logic_error( "Failed!" );
			}
		if ( SUCCEEDED( res ) ) {

			if ( !( wcslen( new_name_ptr ) == new_name_length ) ) {
				throw std::logic_error( "Bad length!" );
				}
			if ( !( wcscmp( new_name_ptr, name.c_str( ) ) == 0 ) ) {
				throw std::logic_error( "bad string!" );
				}
			const auto da_ptrdiff = ( std::ptrdiff_t( pszend ) - std::ptrdiff_t( new_name_ptr ) );
			if ( !( ( da_ptrdiff / sizeof( wchar_t ) ) == new_name_length ) ) {
				throw std::logic_error( "Bad length!" );
				}
			return res;
			}
		throw std::logic_error( "Copy of name_str into Children_String_Heap_Manager failed!!!" );
		}
	};


void some_func( ) {
	Children_String_Heap_Manager my_manager( 10023u );
		{
		const std::wstring some_test_str( L"hello, yo!" );

		const rsize_t length_1 = some_test_str.length( );

		PWSTR allocated_str_1 = nullptr;

		const HRESULT res_1 = my_manager.copy_name_str_into_buffer( allocated_str_1, length_1, some_test_str );
		if ( !SUCCEEDED( res_1 ) ) {
			throw std::runtime_error( "Failed!" );
			}
		wprintf( L"string in memory buffer: %s\r\n", allocated_str_1 );
		}
		{
		const std::wstring some_other_str( L"goodbye, yo!" );

		const rsize_t length_2 = some_other_str.length( );

		PWSTR allocated_str_2 = nullptr;
		const HRESULT res_2 = my_manager.copy_name_str_into_buffer( allocated_str_2, length_2, some_other_str );
		if ( !SUCCEEDED( res_2 ) ) {
			throw std::runtime_error( "Failed!" );
			}

		wprintf( L"string in memory buffer: %s\r\n", allocated_str_2 );
		}

		{
		std::wstring some_damned_long_str;
		const rsize_t some_large_number = 10000u;
		some_damned_long_str.reserve( some_large_number );
		for ( size_t i = 0; i < some_large_number; ++i ) {
			some_damned_long_str.append( L"a" );
			}
		const rsize_t length_3 = some_damned_long_str.length( );
		PWSTR allocated_str_3 = nullptr;
		const HRESULT res_3 = my_manager.copy_name_str_into_buffer( allocated_str_3, length_3, some_damned_long_str );
		if ( !SUCCEEDED( res_3 ) ) {
			throw std::runtime_error( "Failed!" );
			}

		wprintf( L"string in memory buffer: %s\r\n", allocated_str_3 );
		}
	PWSTR someStr = my_manager.m_string_buffer.get( );
	if ( someStr != nullptr ) {
		wprintf( L"string in raw buffer: %s\r\n", someStr );
		while ( someStr < ( my_manager.m_string_buffer.get( ) + my_manager.m_buffer_filled - 2 ) ) {
			if ( ( *someStr ) == 0 ) {
				wprintf( L"string in raw buffer: %s\r\n", ( someStr + 1 ) );
				++someStr;
				}
			else {
				++someStr;
				}
			}
		}
	}

int main( ) {
	some_func( );
	}