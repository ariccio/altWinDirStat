#include "stdafx.h"

#pragma once

#ifndef WDS_CHILDREN_HEAP_MANAGER_CPP_INCLUDED
#define WDS_CHILDREN_HEAP_MANAGER_CPP_INCLUDED


#pragma message( "Including `" __FILE__ "`..." )

#include  "ChildrenHeapManager.h"
#include "globalhelpers.h"
#include "TreeListControl.h"
#include "macros_that_scare_small_children.h"

_Success_( SUCCEEDED( return ) )
const HRESULT Children_String_Heap_Manager_Impl::copy_name_str_into_buffer( _Pre_invalid_ _Post_z_ _Post_readable_size_( new_name_length ) wchar_t*& new_name_ptr, _In_ _In_range_( 0, UINT16_MAX ) const rsize_t& new_name_length, const std::wstring& name ) {
	ASSERT( m_buffer_size > m_buffer_filled );
	ASSERT( new_name_length < UINT16_MAX );
	ASSERT( new_name_length == ( name.length( ) + 1 ) );
	new_name_ptr = ( ( this->m_string_buffer ) + m_buffer_filled );
	ASSERT( ( m_buffer_filled + new_name_length ) < m_buffer_size );
	m_buffer_filled += new_name_length;

	PWSTR pszend = NULL;

	

	//god this is ugly.
	const rsize_t buffer_space_remaining = ( m_buffer_size - m_buffer_filled + new_name_length );

	ASSERT( m_buffer_size >= buffer_space_remaining );

	rsize_t chars_remaining = buffer_space_remaining;
	const HRESULT res = StringCchCopyExW( new_name_ptr, ( buffer_space_remaining ), name.c_str( ), &pszend, &chars_remaining, 0 );
	ASSERT( SUCCEEDED( res ) );
	if ( SUCCEEDED( res ) ) {
#ifdef DEBUG
		ASSERT( wcslen( new_name_ptr ) == ( new_name_length - 1u ) );
		ASSERT( wcscmp( new_name_ptr, name.c_str( ) ) == 0 );
		const auto da_ptrdiff = ( std::ptrdiff_t( pszend ) - std::ptrdiff_t( new_name_ptr ) );
		ASSERT( ( da_ptrdiff / sizeof( wchar_t ) ) == ( new_name_length - 1u ) );
#endif
		return res;
		}
	m_buffer_filled -= new_name_length;
	new_name_ptr = ( ( this->m_string_buffer ) + m_buffer_filled );
	WDS_ASSERT_EXPECTED_STRING_FORMAT_FAILURE_HRESULT( res );
	WDS_STRSAFE_E_INVALID_PARAMETER_HANDLER( res, "StringCchCopyExW" );
	displayWindowsMsgBoxWithMessage( L"Copy of name_str into Children_String_Heap_Manager failed!!!" );
	std::terminate( );
	return res;
	}


child_info::child_info( _In_ const rsize_t number_of_characters_needed, _In_ const rsize_t child_count ) : m_name_pool { number_of_characters_needed }, m_childCount { 0u }, m_children { new CTreeListItem[ child_count ] } { }


child_info::~child_info( ) = default;

#else

#endif