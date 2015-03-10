#pragma once

#include "stdafx.h"

#ifndef WDS_CHILDREN_HEAP_MANAGER_H_INCLUDED
#define WDS_CHILDREN_HEAP_MANAGER_H_INCLUDED


#pragma message( "Including `" __FILE__ "`..." )

//See N4188
struct Children_String_Heap_Manager {
	Children_String_Heap_Manager& operator=( const Children_String_Heap_Manager& in ) = delete;
	Children_String_Heap_Manager( const Children_String_Heap_Manager& in ) = delete;

	//TODO: inline these?
	__forceinline Children_String_Heap_Manager( ) : m_buffer_size { 0u }, m_buffer_filled { 0u }, m_string_buffer { nullptr } { }
	__forceinline Children_String_Heap_Manager( const rsize_t number_of_characters_needed ) : m_buffer_size( number_of_characters_needed ), m_buffer_filled( 0 ), m_string_buffer( new wchar_t[ number_of_characters_needed ] ) { }

	__forceinline void reset( const rsize_t number_of_characters_needed ) {
		m_buffer_filled = 0u;
		m_string_buffer.reset( new wchar_t[ number_of_characters_needed ] );
		m_buffer_size = number_of_characters_needed;
		}

	_Success_( SUCCEEDED( return ) )
	const HRESULT copy_name_str_into_buffer( _Pre_invalid_ _Post_z_ _Post_readable_size_( new_name_length ) wchar_t*& new_name_ptr, _In_ _In_range_( 0, UINT16_MAX ) const rsize_t& new_name_length, const std::wstring& name );

	_Field_size_part_( m_buffer_size, m_buffer_filled ) std::unique_ptr<wchar_t[ ]> m_string_buffer;
	size_t m_buffer_size;
	size_t m_buffer_filled;

	};



#else

#endif