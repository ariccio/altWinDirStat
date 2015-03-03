// item.cpp	- Implementation of CItemBranch
//
// see `file_header_text.txt` for licensing & contact info.

#pragma once

#include "stdafx.h"

#ifndef WDS_ITEM_CPP
#define WDS_ITEM_CPP

//encourage inter-procedural optimization (and class-hierarchy analysis!)
#include "ownerdrawnlistcontrol.h"
#include "TreeListControl.h"
#include "item.h"
#include "typeview.h"


#include "globalhelpers.h"
#include "options.h"
#include "windirstat.h"

#ifdef _DEBUG
#include "dirstatdoc.h"
#endif

namespace {
	//Compare_FILETIME compiles to only 4 instructions, and is only called once, conditionally.
	//passing `lhs` & `rhs` by value cause WORSE codegen!
	inline const INT Compare_FILETIME( const FILETIME& lhs, const FILETIME& rhs ) {
		//duhh, there's a win32 function for this!
		return CompareFileTime( &lhs, &rhs );
		}

	//Compare_FILETIME_lessthan compiles to only 6 instructions, and is only called twice, conditionally.
	//When NOT inlined requires 5 more instructions at call site.
	//When inlined requires only 5 instructions (total) at call site.
	inline const bool Compare_FILETIME_lessthan( const FILETIME& t1, const FILETIME& t2 ) {
		//CompareFileTime returns -1 when first FILETIME is less than second FILETIME
		//Therefore: we can 'emulate' the `<` operator, by checking if ( CompareFileTime( &t1, &t2 ) == ( -1 ) );
		return ( CompareFileTime( &t1, &t2 ) == ( -1 ) );
		}

	_Success_( SUCCEEDED( return ) )
	const HRESULT WriteToStackBuffer_do_nothing( WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) {
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

	}


//CItemBranch::CItemBranch( const std::uint64_t size, const FILETIME time, const DWORD attr, const bool done, _In_ CItemBranch* const parent, _In_z_ _Readable_elements_( length ) PCWSTR const name, const std::uint16_t length ) : m_size{ size }, m_lastChange( time ), m_childCount{ 0u }, CTreeListItem{ std::move( name ), std::move( length ), std::move( parent ), std::move( attr ), std::move( done ) } { }

_Pre_satisfies_( subitem == column::COL_PERCENTAGE ) _Success_( SUCCEEDED( return ) )
const HRESULT CItemBranch::WriteToStackBuffer_COL_PERCENTAGE( RANGE_ENUM_COL const column::ENUM_COL subitem, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const {
	//auto res = StringCchPrintfW( psz_text, strSize, L"%.1f%%", ( GetFraction( ) * static_cast<DOUBLE>( 100 ) ) );
#ifndef DEBUG
	UNREFERENCED_PARAMETER( subitem );
#endif
	ASSERT( subitem == column::COL_PERCENTAGE );
	size_t chars_remaining = 0;
	const auto percentage = ( GetFraction( ) * static_cast< DOUBLE >( 100 ) );
	ASSERT( percentage <= 100.00 );
	const HRESULT res = StringCchPrintfExW( psz_text, strSize, NULL, &chars_remaining, 0, L"%.1f%%", percentage );
	ASSERT( SUCCEEDED( res ) );
	if ( SUCCEEDED( res ) ) {
		chars_written = ( strSize - chars_remaining );
		ASSERT( chars_written == wcslen( psz_text ) );
		return res;
		}
	WDS_ASSERT_EXPECTED_STRING_FORMAT_FAILURE_HRESULT( res );
	if ( res == STRSAFE_E_INSUFFICIENT_BUFFER ) {
		handle_stack_insufficient_buffer( strSize, 64u, sizeBuffNeed, chars_written );
		return res;
		}
	WDS_STRSAFE_E_INVALID_PARAMETER_HANDLER( res, "StringCchPrintFExW" );
	chars_written = 0;
	return res;
	}

_Pre_satisfies_( subitem == column::COL_NTCOMPRESS ) _Success_( SUCCEEDED( return ) )
const HRESULT CItemBranch::WriteToStackBuffer_COL_NTCOMPRESS( RANGE_ENUM_COL const column::ENUM_COL subitem, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const {
	//auto res = StringCchPrintfW( psz_text, strSize, L"%.1f%%", ( GetFraction( ) * static_cast<DOUBLE>( 100 ) ) );
#ifndef DEBUG
	UNREFERENCED_PARAMETER( subitem );
#endif
	ASSERT( subitem == column::COL_NTCOMPRESS );
	ASSERT( strSize > 5u );
	
	if ( !( m_attr.compressed ) ) {
		return WriteToStackBuffer_do_nothing( psz_text, strSize, sizeBuffNeed, chars_written );
		}
	if ( m_children != nullptr ) {
		return WriteToStackBuffer_do_nothing( psz_text, strSize, sizeBuffNeed, chars_written );
		}

	size_t chars_remaining = 0;
	ASSERT( m_vi != nullptr );
	const auto percentage = ( m_vi->ntfs_compression_ratio * static_cast< DOUBLE >( 100 ) );
	ASSERT( percentage <= 100.00 );
	const HRESULT res = StringCchPrintfExW( psz_text, strSize, NULL, &chars_remaining, 0, L"%.1f%%", percentage );
	ASSERT( SUCCEEDED( res ) );
	if ( SUCCEEDED( res ) ) {
		chars_written = ( strSize - chars_remaining );
		ASSERT( chars_written == wcslen( psz_text ) );
		return res;
		}
	WDS_ASSERT_EXPECTED_STRING_FORMAT_FAILURE_HRESULT( res );
	if ( res == STRSAFE_E_INSUFFICIENT_BUFFER ) {
		handle_stack_insufficient_buffer( strSize, 64u, sizeBuffNeed, chars_written );
		return res;
		}
	chars_written = 0;
	WDS_STRSAFE_E_INVALID_PARAMETER_HANDLER( res, "StringCchPrintFExW" );
	return res;
	}


_Pre_satisfies_( subitem == column::COL_SUBTREETOTAL ) _Success_( SUCCEEDED( return ) )
const HRESULT CItemBranch::WriteToStackBuffer_COL_SUBTREETOTAL( RANGE_ENUM_COL const column::ENUM_COL subitem, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const {
#ifndef DEBUG
	UNREFERENCED_PARAMETER( subitem );
#endif
	ASSERT( subitem == column::COL_SUBTREETOTAL );
	const HRESULT res = wds_fmt::FormatBytes( size_recurse( ), psz_text, strSize, chars_written, sizeBuffNeed );
	ASSERT( SUCCEEDED( res ) );
	if ( SUCCEEDED( res ) ) {
		ASSERT( chars_written == wcslen( psz_text ) );
		return res;
		}
	WDS_ASSERT_EXPECTED_STRING_FORMAT_FAILURE_HRESULT( res );
	if ( res == STRSAFE_E_INSUFFICIENT_BUFFER ) {
		handle_stack_insufficient_buffer( strSize, 64u, sizeBuffNeed, chars_written );
		return res;
		}
	chars_written = 0;
	WDS_STRSAFE_E_INVALID_PARAMETER_HANDLER( res, "wds_fmt::FormatBytes" );
	return res;
	}

_Pre_satisfies_( ( subitem == column::COL_FILES ) || ( subitem == column::COL_ITEMS ) ) _Success_( SUCCEEDED( return ) )
const HRESULT CItemBranch::WriteToStackBuffer_COL_FILES( RANGE_ENUM_COL const column::ENUM_COL subitem, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const {
#ifndef DEBUG
	UNREFERENCED_PARAMETER( subitem );
#endif
	ASSERT( ( subitem == column::COL_FILES ) || ( subitem == column::COL_ITEMS ) );
	const auto number_to_format = files_recurse( );
	const HRESULT res = wds_fmt::CStyle_GetNumberFormatted( number_to_format, psz_text, strSize, chars_written );
	ASSERT( SUCCEEDED( res ) );
	if ( SUCCEEDED( res ) ) {
		ASSERT( chars_written == wcslen( psz_text ) );
		return res;
		}

	WDS_ASSERT_EXPECTED_STRING_FORMAT_FAILURE_HRESULT( res );

	if ( res == STRSAFE_E_INSUFFICIENT_BUFFER ) {
		handle_stack_insufficient_buffer( strSize, 64u, sizeBuffNeed, chars_written );
		return res;
		}
	chars_written = 0;
	return res;
	}

_Pre_satisfies_( subitem == column::COL_LASTCHANGE ) _Success_( SUCCEEDED( return ) )
const HRESULT CItemBranch::WriteToStackBuffer_COL_LASTCHANGE( RANGE_ENUM_COL const column::ENUM_COL subitem, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, _Out_ _On_failure_( _Post_valid_ ) rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const {
#ifndef DEBUG
	UNREFERENCED_PARAMETER( subitem );
#endif
	ASSERT( subitem == column::COL_LASTCHANGE );
	const HRESULT res = wds_fmt::CStyle_FormatFileTime( FILETIME_recurse( ), psz_text, strSize, chars_written );
	ASSERT( SUCCEEDED( res ) );
	if ( SUCCEEDED( res ) ) {
		sizeBuffNeed = SIZE_T_ERROR;
		return S_OK;
		}
	if ( res == STRSAFE_E_INSUFFICIENT_BUFFER ) {
		handle_stack_insufficient_buffer( strSize, 64u, sizeBuffNeed, chars_written );
		return res;
		}


	WDS_ASSERT_EXPECTED_STRING_FORMAT_FAILURE_HRESULT( res );

	chars_written = { 0u };
	sizeBuffNeed = { 48u };

	//_CrtDbgBreak( );//not handled yet.
	return STRSAFE_E_INVALID_PARAMETER;
	}

_Pre_satisfies_( subitem == column::COL_ATTRIBUTES ) _Success_( SUCCEEDED( return ) )
const HRESULT CItemBranch::WriteToStackBuffer_COL_ATTRIBUTES( RANGE_ENUM_COL const column::ENUM_COL subitem, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const {
#ifndef DEBUG
	UNREFERENCED_PARAMETER( subitem );
#endif
	ASSERT( subitem == column::COL_ATTRIBUTES );
	const HRESULT res = wds_fmt::CStyle_FormatAttributes( m_attr, psz_text, strSize, chars_written );
	ASSERT( SUCCEEDED( res ) );
	if ( !SUCCEEDED( res ) ) {
		sizeBuffNeed = { 8u };//Generic size needed, overkill;
		chars_written = { 0u };
		//_CrtDbgBreak( );//not handled yet.
		return res;
		}
	ASSERT( chars_written == wcslen( psz_text ) );
	return res;
	}

_Must_inspect_result_ _Success_( SUCCEEDED( return ) )
HRESULT CItemBranch::Text_WriteToStackBuffer( RANGE_ENUM_COL const column::ENUM_COL subitem, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, _On_failure_( _Post_valid_ ) rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const {
	ASSERT( subitem != column::COL_NAME );
	if ( subitem == column::COL_NAME ) {
		displayWindowsMsgBoxWithMessage( L"GetText_WriteToStackBuffer was called for column::COL_NAME!!! This should never happen!!!!" );
		std::terminate( );
		}
	switch ( subitem )
	{
			case column::COL_PERCENTAGE:
				return WriteToStackBuffer_COL_PERCENTAGE( subitem, psz_text, strSize, sizeBuffNeed, chars_written );
			case column::COL_SUBTREETOTAL:
				return WriteToStackBuffer_COL_SUBTREETOTAL( subitem, psz_text, strSize, sizeBuffNeed, chars_written );
			case column::COL_ITEMS:
				return WriteToStackBuffer_COL_FILES( subitem, psz_text, strSize, sizeBuffNeed, chars_written );
			case column::COL_NTCOMPRESS:
				return WriteToStackBuffer_COL_NTCOMPRESS( subitem, psz_text, strSize, sizeBuffNeed, chars_written );
			case column::COL_LASTCHANGE:
				return WriteToStackBuffer_COL_LASTCHANGE( subitem, psz_text, strSize, sizeBuffNeed, chars_written );
			case column::COL_ATTRIBUTES:
				return WriteToStackBuffer_COL_ATTRIBUTES( subitem, psz_text, strSize, sizeBuffNeed, chars_written );
			case column::COL_NAME:
			default:
				return WriteToStackBuffer_default( subitem, psz_text, strSize, sizeBuffNeed, chars_written, L"CItemBranch::" );
	}
	}



INT CItemBranch::CompareSibling( _In_ const CTreeListItem* const tlib, _In_ _In_range_( 0, INT32_MAX ) const column::ENUM_COL subitem ) const {
	auto const other = static_cast< const CItemBranch* >( tlib );
	switch ( subitem ) {
			case column::COL_NAME:
				return signum( wcscmp( m_name, other->m_name ) );
			case column::COL_PERCENTAGE:
				return signum( GetFraction( ) - other->GetFraction( ) );
			case column::COL_SUBTREETOTAL:
				return signum( static_cast<std::int64_t>( size_recurse( ) ) - static_cast<std::int64_t>( other->size_recurse( ) ) );
			case column::COL_ITEMS:
				return signum( static_cast<std::int64_t>( files_recurse( ) ) - static_cast<std::int64_t>( other->files_recurse( ) ) );
			case column::COL_NTCOMPRESS:
				return signum( m_vi->ntfs_compression_ratio - other->m_vi->ntfs_compression_ratio );
			case column::COL_LASTCHANGE:
				return Compare_FILETIME( FILETIME_recurse( ), other->FILETIME_recurse( ) );

			case column::COL_ATTRIBUTES:
				return signum( GetSortAttributes( ) - other->GetSortAttributes( ) );
			default:
				ASSERT( false );
				return 666;
		}
	}

std::vector<CTreeListItem*> CItemBranch::size_sorted_vector_of_children( ) const {
	std::vector<CTreeListItem*> children;
	const auto child_count = m_childCount;
	children.reserve( child_count );
	const auto local_m_children = m_children.get( );
	if ( m_children != nullptr ) {
		//Not vectorized: 1200, loop contains data dependencies
		for ( size_t i = 0; i < child_count; ++i ) {
			children.emplace_back( local_m_children + i );
			}
		}
#ifdef DEBUG
	else {
		ASSERT( m_childCount == 0 );
		}
#endif
	//TODO: qsort is bleh
	qsort( children.data( ), static_cast< const size_t >( children.size( ) ), sizeof( CTreeListItem* ), &CItem_compareBySize );
	//std::sort( children.begin( ), children.end( ), [] ( const CTreeListItem* const lhs, const CTreeListItem* const rhs ) { return static_cast< const CItemBranch* >( lhs )->size_recurse( ) < static_cast< const CItemBranch* >( rhs )->size_recurse( ); } );
	return children;
	}


INT CItemBranch::GetSortAttributes( ) const {
	INT ret = 0;

	// We want to enforce the order RHSACE with R being the highest priority attribute and E being the lowest priority attribute.
	ret += ( m_attr.readonly   ) ? 1000000 : 0; // R
	ret += ( m_attr.hidden     ) ? 100000  : 0; // H
	ret += ( m_attr.system     ) ? 10000   : 0; // S
	ret += ( m_attr.compressed ) ? 100     : 0; // C
	ret += ( m_attr.encrypted  ) ? 10      : 0; // E

	return ( ( m_attr.invalid ) ? 0 : ret );
	}

DOUBLE CItemBranch::GetFraction( ) const {
	const auto myParent = GetParentItem( );
	if ( myParent == NULL ) {
		return static_cast<DOUBLE>( 1.0 );//root item? must be whole!
		}
	const auto parentSize = myParent->size_recurse( );
	if ( parentSize == 0 ) {//root item?
		return static_cast<DOUBLE>( 1.0 );
		}
	const auto my_size = size_recurse( );
	ASSERT( my_size != UINT64_ERROR );
	ASSERT( my_size <= parentSize );
	return static_cast<DOUBLE>( my_size ) / static_cast<DOUBLE>( parentSize );
	}

std::wstring CItemBranch::GetPath( ) const {
	std::wstring pathBuf;
	pathBuf.reserve( MAX_PATH );
	UpwardGetPathWithoutBackslash( pathBuf );
	ASSERT( wcslen( m_name ) == m_name_length );
	ASSERT( wcslen( m_name ) < 33000 );
	ASSERT( pathBuf.length( ) < 33000 );
	return pathBuf;
	}

void CItemBranch::UpwardGetPathWithoutBackslash( std::wstring& pathBuf ) const {
	auto myParent = GetParentItem( );
	if ( myParent != NULL ) {
		myParent->UpwardGetPathWithoutBackslash( pathBuf );
		}
#ifdef DEBUG
	auto guard_assert = WDS_SCOPEGUARD_INSTANCE( [ &] { ASSERT( wcslen( m_name ) == m_name_length ); } );
#endif
	ASSERT( wcslen( m_name ) < 33000 );
	if ( m_children == nullptr ) {
		//ASSERT( m_parent != NULL );
		if ( m_parent != NULL ) {
			//WTF IS GOING ON HERE
			//TODO: BUGBUG: what is dis?
			if ( m_parent->m_parent != NULL ) {
				pathBuf += L'\\';
				pathBuf += m_name;
				return;
				}
			pathBuf += L'\\';
			pathBuf += m_name;
			return;
			}
		ASSERT( pathBuf.empty( ) );
		pathBuf = m_name;
		return;
		//ASSERT( false );
		//return;
		}
	if ( !pathBuf.empty( ) ) {
		if ( pathBuf.back( ) != L'\\' ) {//if pathBuf is empty, it's because we don't have a parent ( we're the root ), so we already have a "\\"
			pathBuf += L'\\';
			pathBuf += m_name;
			return;
			}
		pathBuf += m_name;
		return;
		}
	pathBuf += m_name;
	return;
	}


_Success_( return < SIZE_T_MAX )
size_t CItemBranch::findItemInChildren( const CItemBranch* const theItem ) const {
	const auto childrenSize = m_childCount;
	for ( size_t i = 0; i < childrenSize; ++i ) {
		if ( ( ( m_children.get( ) + i ) ) == theItem ) {
			return i;
			}
		}
	return SIZE_T_MAX;
	}


void CItemBranch::refresh_sizeCache( ) {
	//if ( m_type == IT_FILE ) {
	if ( m_children == nullptr ) {
		ASSERT( m_childCount == 0 );
		ASSERT( m_size < UINT64_ERROR );
		return;
		}
	if ( m_size == UINT64_ERROR ) {
		const auto children_size = m_childCount;
		const auto child_array = m_children.get( );
		for ( size_t i = 0; i < children_size; ++i ) {
			( child_array + i )->refresh_sizeCache( );
			}

		//---
		std::uint64_t total = 0;

		const auto childCount = m_childCount;
		const rsize_t stack_alloc_threshold = 128;
		if ( childCount < stack_alloc_threshold ) {
			std::uint64_t child_totals[ stack_alloc_threshold ];
			for ( size_t i = 0; i < childCount; ++i ) {
				child_totals[ i ] = ( child_array + i )->size_recurse( );
				}
			//loop vectorized!
			for ( size_t i = 0; i < childCount; ++i ) {
				ASSERT( total < ( UINT64_MAX / 2 ) );
				ASSERT( child_totals[ i ] < ( UINT64_MAX / 2 ) );
				total += child_totals[ i ];
				}
			}
		else {
			//Not vectorized: 1200, loop contains data dependencies
			for ( size_t i = 0; i < childCount; ++i ) {
				total += ( child_array + i )->size_recurse( );
				}
			}
		//return total;

		//---
		m_size = total;
		}
	}

_Ret_range_( 0, UINT64_MAX )
std::uint64_t CItemBranch::size_recurse( ) const {
	static_assert( std::is_same<decltype( std::declval<CItemBranch>( ).size_recurse( ) ), decltype( std::declval<CItemBranch>( ).m_size )>::value , "The return type of CItemBranch::size_recurse needs to be fixed!!" );
	//ASSERT( m_size != UINT64_ERROR );
	//if ( m_type == IT_FILE ) {
	if ( !m_children ) {
		ASSERT( m_childCount == 0 );
		if ( m_parent == NULL ) {
			return 0;
			}
		ASSERT( m_size < UINT64_ERROR );
		return m_size;
		}
	if ( m_size != UINT64_ERROR ) {
		return m_size;
		}
	WDS_ASSERT_NEVER_REACHED( );
	//ASSERT( m_size == UINT64_ERROR );
	//const auto total = compute_size_recurse( );
	//ASSERT( m_size == UINT64_ERROR );
	//m_size = total;
	//ASSERT( total < ( UINT64_MAX / 2 ) );
	//return total;
	return UINT64_ERROR;
	}


//4,294,967,295  (4294967295 ) is the maximum number of files in an NTFS filesystem according to http://technet.microsoft.com/en-us/library/cc781134(v=ws.10).aspx
_Ret_range_( 0, 4294967295 )
std::uint32_t CItemBranch::files_recurse( ) const {
	static_assert( std::is_same<decltype( std::declval<CItemBranch>( ).files_recurse( ) ), decltype( std::declval<CItemBranch>( ).m_childCount )>::value , "The return type of CItemBranch::files_recurse needs to be fixed!!" );

	if ( m_children == nullptr ) {
		return 1;
		}
	std::uint32_t total = 0;
	static_assert( std::is_same<decltype( total ), decltype( std::declval<CItemBranch>( ).m_childCount )>::value , "The type of total needs to be fixed!!" );
	
	const auto childCount = m_childCount;
	const auto my_m_children = m_children.get( );
	const rsize_t stack_alloc_threshold = 128;
	if ( childCount < stack_alloc_threshold ) {
		std::uint32_t child_totals[ stack_alloc_threshold ];
		for ( size_t i = 0; i < childCount; ++i ) {
			child_totals[ i ] = ( my_m_children + i )->files_recurse( );
			}

		//std::accumulate works nicely even though it includes iterators.
		//numeric(19) loop vectorized!
		total = std::accumulate( child_totals, ( child_totals + childCount ), static_cast<std::uint32_t>( 0 ) );
		++total;
		return total;
		}
	//Not vectorized: 1304, loop includes assignments of different sizes
	for ( size_t i = 0; i < childCount; ++i ) {
		total += ( my_m_children + i )->files_recurse( );
		}
	total += 1;
	return total;
	}

FILETIME CItemBranch::FILETIME_recurse( ) const {
	if ( m_children == nullptr ) {
		return m_lastChange;
		}
	auto ft = zero_init_struct<FILETIME>( );
	if ( Compare_FILETIME_lessthan( ft, m_lastChange ) ) {
		ft = m_lastChange;
		}
	
	const auto childCount = m_childCount;
	const auto my_m_children = m_children.get( );
	//Not vectorized: 1304, loop includes assignments of different sizes
	for ( size_t i = 0; i < childCount; ++i ) {
		const auto ft_child = ( my_m_children + i )->FILETIME_recurse( );
		if ( Compare_FILETIME_lessthan( ft, ft_child ) ) {
			ft = ft_child;
			}
		}
	return ft;
	}

//Sometimes I just need to COMPARE the extension with a string. So, instead of copying/screwing with string internals, I'll just return a pointer to the substring.
_Pre_satisfies_( this->m_children._Myptr == nullptr ) 
PCWSTR const CItemBranch::CStyle_GetExtensionStrPtr( ) const {
	ASSERT( m_name_length < ( MAX_PATH + 1 ) );

	PCWSTR const resultPtrStr = PathFindExtensionW( m_name );
	ASSERT( resultPtrStr != '\0' );
	return resultPtrStr;
	}


_Ret_range_( 0, 33000 ) DOUBLE CItemBranch::averageNameLength( ) const {
	const auto myLength = static_cast<DOUBLE>( m_name_length );
	DOUBLE childrenTotal = 0;
	//TODO: take advantage of block heap allocation in this
	
	//if ( m_type != IT_FILE ) {
	if ( m_children != nullptr ) {
		const auto childCount = m_childCount;
		const auto my_m_children = m_children.get( );
		const rsize_t stack_alloc_threshold = 128;
		if ( childCount < stack_alloc_threshold ) {
			DOUBLE children_totals[ stack_alloc_threshold ] = { 0 };
			for ( size_t i = 0; i < childCount; ++i ) {
				children_totals[ i ] = ( my_m_children + i )->averageNameLength( );
				}
			for ( size_t i = 0; i < childCount; ++i ) {
				childrenTotal += children_totals[ i ];
				}
			}
		else {
			//Not vectorized: 1200, loop contains data dependencies
			for ( size_t i = 0; i < childCount; ++i ) {
				childrenTotal += ( my_m_children + i )->averageNameLength( );
				}
			}
		return ( childrenTotal + myLength ) / static_cast<DOUBLE>( childCount + 1u );
		}
	ASSERT( m_childCount == 0 );
	return myLength;
	}

//_Pre_satisfies_( this->m_type == IT_FILE )
_Pre_satisfies_( this->m_children._Myptr == nullptr ) 
void CItemBranch::stdRecurseCollectExtensionData_FILE( _Inout_ std::unordered_map<std::wstring, minimal_SExtensionRecord>& extensionMap ) const {
	ASSERT( m_children == nullptr );

	PCWSTR const resultPtrStr = CStyle_GetExtensionStrPtr( );
	static_assert( std::is_same< std::decay<decltype(*m_name)>::type, wchar_t>::value, "Bad division below!" );
#ifdef DEBUG
	const auto alt_length = ( ( std::ptrdiff_t( m_name + m_name_length ) - std::ptrdiff_t( resultPtrStr ) ) / sizeof( wchar_t ) );
	ASSERT( wcslen( resultPtrStr ) == alt_length );
#endif
	//TRACE( _T( "Calculated length: %lld, actual length: %llu\r\n" ), LONGLONG( alt_length ), ULONGLONG( wcslen( resultPtrStr ) ) );
	auto& value = extensionMap[ resultPtrStr ];
	++( value.files );
	value.bytes += m_size;
	return;
	}


void CItemBranch::stdRecurseCollectExtensionData( _Inout_ std::unordered_map<std::wstring, minimal_SExtensionRecord>& extensionMap ) const {
	//if ( m_type == IT_FILE ) {
	if ( m_children == nullptr ) {
		stdRecurseCollectExtensionData_FILE( extensionMap );
		return;
		}
	const auto childCount = m_childCount;
	const auto local_m_children = m_children.get( );
	//todo: Iterate over the heapmanager items instead
	//Not vectorized: 1200, loop contains data dependencies
	for ( size_t i = 0; i < childCount; ++i ) {
		( local_m_children + i )->stdRecurseCollectExtensionData( extensionMap );
		}
	}

INT __cdecl CItem_compareBySize( _In_ _Points_to_data_ const void* const p1, _In_ _Points_to_data_ const void* const p2 ) {
	const auto size1 = ( *( reinterpret_cast< const CItemBranch * const* const >( p1 ) ) )->size_recurse( );
	const auto size2 = ( *( reinterpret_cast< const CItemBranch * const* const >( p2 ) ) )->size_recurse( );
	return signum( static_cast<std::int64_t>( size2 ) - static_cast<std::int64_t>( size1 ) ); // biggest first// TODO: Use 2nd sort column (as set in our TreeListView?)
	}


_At_( return, _Writable_bytes_( bytes_allocated ) )
_Ret_notnull_ children_heap_block_allocation* allocate_enough_memory_for_children_block( _In_ const std::uint32_t number_of_children, _Out_ size_t& bytes_allocated ) {
	const rsize_t base_memory_size_in_bytes = ( sizeof( decltype( children_heap_block_allocation::m_childCount ) ) + sizeof( Children_String_Heap_Manager ) );
	
	
	const rsize_t size_of_a_single_child_in_bytes = sizeof( CItemBranch );
	const size_t size_of_children_needed_in_bytes = ( size_of_a_single_child_in_bytes * static_cast<size_t>( number_of_children + 1 ) );

	const size_t total_size_needed = ( base_memory_size_in_bytes + size_of_children_needed_in_bytes );
	void* const memory_block = malloc( total_size_needed );
	if ( memory_block == NULL ) {
		displayWindowsMsgBoxWithMessage( L"can't allocate enough memory for children block! (aborting)" );
		std::terminate( );

		//shut analyze up.
		abort( );
		}
	bytes_allocated = total_size_needed;
	children_heap_block_allocation* const new_block = static_cast< children_heap_block_allocation* const>( memory_block );
	new_block->m_childCount = number_of_children;
	return new_block;
	}



#else

#endif