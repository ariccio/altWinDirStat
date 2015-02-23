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

		//if ( Compare_FILETIME_cast( lhs, rhs ) ) {
		//	return -1;
		//	}
		//else if ( ( lhs.dwLowDateTime == rhs.dwLowDateTime ) && ( lhs.dwHighDateTime == rhs.dwHighDateTime ) ) {
		//	return 0;
		//	}
		//return 1;
		}

	//Compare_FILETIME_lessthan compiles to only 6 instructions, and is only called twice, conditionally.
	//When NOT inlined requires 5 more instructions at call site.
	//When inlined requires only 5 instructions (total) at call site.
	inline const bool Compare_FILETIME_lessthan( const FILETIME& t1, const FILETIME& t2 ) {
		//CompareFileTime returns -1 when first FILETIME is less than second FILETIME
		//Therefore: we can 'emulate' the `<` operator, by checking if ( CompareFileTime( &t1, &t2 ) == ( -1 ) );
		return ( CompareFileTime( &t1, &t2 ) == ( -1 ) );
	
		//const auto u1 = reinterpret_cast<const ULARGE_INTEGER&>( t1 );
		//const auto u2 = reinterpret_cast<const ULARGE_INTEGER&>( t2 );
		//return ( u1.QuadPart < u2.QuadPart );
		}

	}


CItemBranch::CItemBranch( const std::uint64_t size, const FILETIME time, const DWORD attr, const bool done, _In_ CItemBranch* const parent, _In_z_ _Readable_elements_( length ) PCWSTR const name, const std::uint16_t length ) : m_size{ size }, m_rect{ 0, 0, 0, 0 }, m_lastChange( time ), m_childCount{ 0 }, CTreeListItem{ std::move( name ), std::move( length ), std::move( parent ) } {
	//m_vi( nullptr );
	SetAttributes( attr );
	m_attr.m_done = done;
	//m_name = std::move( name );
	}

_Pre_satisfies_( subitem == column::COL_PERCENTAGE ) _Success_( SUCCEEDED( return ) )
HRESULT CItemBranch::WriteToStackBuffer_COL_PERCENTAGE( RANGE_ENUM_COL const column::ENUM_COL subitem, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const {
	//auto res = StringCchPrintfW( psz_text, strSize, L"%.1f%%", ( GetFraction( ) * static_cast<DOUBLE>( 100 ) ) );
#ifndef DEBUG
	UNREFERENCED_PARAMETER( subitem );
#endif
	ASSERT( subitem == column::COL_PERCENTAGE );
	size_t chars_remaining = 0;
	const auto percentage = ( GetFraction( ) * static_cast< DOUBLE >( 100 ) );
	ASSERT( percentage <= 100.00 );
	const HRESULT res = StringCchPrintfExW( psz_text, strSize, NULL, &chars_remaining, 0, L"%.1f%%", percentage );
	if ( res == STRSAFE_E_INSUFFICIENT_BUFFER ) {
		chars_written = strSize;
		sizeBuffNeed = 64;//Generic size needed.
		return res;
		}
	else if ( ( res != STRSAFE_E_INSUFFICIENT_BUFFER ) && ( FAILED( res ) ) ) {
		chars_written = 0;
		return res;
		}
	ASSERT( SUCCEEDED( res ) );
	if ( SUCCEEDED( res ) ) {
		chars_written = ( strSize - chars_remaining );
		ASSERT( chars_written == wcslen( psz_text ) );
		return res;
		}
	return res;
	}

_Pre_satisfies_( subitem == column::COL_NTCOMPRESS ) _Success_( SUCCEEDED( return ) )
HRESULT CItemBranch::WriteToStackBuffer_COL_NTCOMPRESS( RANGE_ENUM_COL const column::ENUM_COL subitem, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const {
	//auto res = StringCchPrintfW( psz_text, strSize, L"%.1f%%", ( GetFraction( ) * static_cast<DOUBLE>( 100 ) ) );
#ifndef DEBUG
	UNREFERENCED_PARAMETER( subitem );
#endif
	ASSERT( subitem == column::COL_NTCOMPRESS );
	size_t chars_remaining = 0;
	if ( !( m_attr.compressed ) ) {
		//do nothing
		return StringCchPrintfExW( psz_text, strSize, NULL, &chars_remaining, 0, L"" );
		}
	if ( m_children != nullptr ) {
		//do nothing
		return StringCchPrintfExW( psz_text, strSize, NULL, &chars_remaining, 0, L"" );
		}

	ASSERT( m_vi != nullptr );
	const auto percentage = ( m_vi->ntfs_compression_ratio * static_cast< DOUBLE >( 100 ) );
	ASSERT( percentage <= 100.00 );
	const HRESULT res = StringCchPrintfExW( psz_text, strSize, NULL, &chars_remaining, 0, L"%.1f%%", percentage );
	if ( res == STRSAFE_E_INSUFFICIENT_BUFFER ) {
		chars_written = strSize;
		sizeBuffNeed = 64;//Generic size needed.
		return res;
		}
	else if ( ( res != STRSAFE_E_INSUFFICIENT_BUFFER ) && ( FAILED( res ) ) ) {
		chars_written = 0;
		return res;
		}
	ASSERT( SUCCEEDED( res ) );
	if ( SUCCEEDED( res ) ) {
		chars_written = ( strSize - chars_remaining );
		ASSERT( chars_written == wcslen( psz_text ) );
		return res;
		}
	return res;
	}


_Pre_satisfies_( subitem == column::COL_SUBTREETOTAL ) _Success_( SUCCEEDED( return ) )
HRESULT CItemBranch::WriteToStackBuffer_COL_SUBTREETOTAL( RANGE_ENUM_COL const column::ENUM_COL subitem, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const {
#ifndef DEBUG
	UNREFERENCED_PARAMETER( subitem );
#endif
	ASSERT( subitem == column::COL_SUBTREETOTAL );
	const HRESULT res = wds_fmt::FormatBytes( size_recurse( ), psz_text, strSize, chars_written, sizeBuffNeed );
	if ( res == STRSAFE_E_INSUFFICIENT_BUFFER ) {
		chars_written = strSize;
		sizeBuffNeed = ( ( 64 > sizeBuffNeed ) ? 64 : sizeBuffNeed );//Generic size needed.
		return res;
		}
	else if ( ( res != STRSAFE_E_INSUFFICIENT_BUFFER ) && ( FAILED( res ) ) ) {
		chars_written = 0;
		return res;
		}
	ASSERT( SUCCEEDED( res ) );
	ASSERT( chars_written == wcslen( psz_text ) );
	return res;
	}

_Pre_satisfies_( ( subitem == column::COL_FILES ) || ( subitem == column::COL_ITEMS ) ) _Success_( SUCCEEDED( return ) )
HRESULT CItemBranch::WriteToStackBuffer_COL_FILES( RANGE_ENUM_COL const column::ENUM_COL subitem, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const {
#ifndef DEBUG
	UNREFERENCED_PARAMETER( subitem );
#endif
	ASSERT( ( subitem == column::COL_FILES ) || ( subitem == column::COL_ITEMS ) );
	const auto number_to_format = files_recurse( );
	const HRESULT num_fmt_Res = wds_fmt::CStyle_GetNumberFormatted( number_to_format, psz_text, strSize, chars_written );

	if ( SUCCEEDED( num_fmt_Res ) ) {
		return num_fmt_Res;
		}
	sizeBuffNeed = ( ( strSize > 64 ) ? ( strSize * 2 ) : 128 );
	return num_fmt_Res;
	}

_Pre_satisfies_( subitem == column::COL_LASTCHANGE ) _Success_( SUCCEEDED( return ) )
HRESULT CItemBranch::WriteToStackBuffer_COL_LASTCHANGE( RANGE_ENUM_COL const column::ENUM_COL subitem, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, _Out_ _On_failure_( _Post_valid_ ) rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const {
#ifndef DEBUG
	UNREFERENCED_PARAMETER( subitem );
#endif
	ASSERT( subitem == column::COL_LASTCHANGE );
	const HRESULT res = wds_fmt::CStyle_FormatFileTime( FILETIME_recurse( ), psz_text, strSize, chars_written );
	if ( SUCCEEDED( res ) ) {
		sizeBuffNeed = SIZE_T_ERROR;
		return S_OK;
		}
	ASSERT( SUCCEEDED( res ) );
	chars_written = { 0u };
	sizeBuffNeed = { 48u };

	//_CrtDbgBreak( );//not handled yet.
	return STRSAFE_E_INVALID_PARAMETER;
	}

_Pre_satisfies_( subitem == column::COL_ATTRIBUTES ) _Success_( SUCCEEDED( return ) )
HRESULT CItemBranch::WriteToStackBuffer_COL_ATTRIBUTES( RANGE_ENUM_COL const column::ENUM_COL subitem, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const {
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


COLORREF CItemBranch::ItemTextColor( ) const {
	if ( m_attr.invalid ) {
		//return GetItemTextColor( true );
		return RGB( 0xFF, 0x00, 0x00 );
		}
	if ( m_attr.compressed ) {
		return RGB( 0x00, 0x00, 0xFF );
		}
	else if ( m_attr.encrypted ) {
		return GetApp( )->m_altEncryptionColor;
		}
	//ASSERT( GetItemTextColor( true ) == default_item_text_color( ) );
	//return GetItemTextColor( true ); // The rest is not colored
	return default_item_text_color( ); // The rest is not colored
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

//Unconditionally called only ONCE, so we ask for inlining.
//Encodes the attributes to fit (in) 1 byte
inline void CItemBranch::SetAttributes( _In_ const DWORD attr ) {
	if ( attr == INVALID_FILE_ATTRIBUTES ) {
		m_attr.invalid = true;
		return;
		}
	m_attr.readonly   = ( ( attr bitand FILE_ATTRIBUTE_READONLY      ) != 0 );
	m_attr.hidden     = ( ( attr bitand FILE_ATTRIBUTE_HIDDEN        ) != 0 );
	m_attr.system     = ( ( attr bitand FILE_ATTRIBUTE_SYSTEM        ) != 0 );
	m_attr.compressed = ( ( attr bitand FILE_ATTRIBUTE_COMPRESSED    ) != 0 );
	m_attr.encrypted  = ( ( attr bitand FILE_ATTRIBUTE_ENCRYPTED     ) != 0 );
	m_attr.reparse    = ( ( attr bitand FILE_ATTRIBUTE_REPARSE_POINT ) != 0 );
	m_attr.invalid    = false;
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

RECT CItemBranch::TmiGetRectangle( ) const {
	return BuildRECT( m_rect );
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

	//if ( m_vi != nullptr ) {
	//	//if ( m_vi->sizeCache != UINT64_ERROR ) {
	//	//	m_vi->sizeCache = UINT64_ERROR;
	//	//	m_vi->sizeCache = size_recurse( );
	//	//	}
	//	}
	if ( m_size == UINT64_ERROR ) {
		const auto children_size = m_childCount;
		const auto child_array = m_children.get( );
		for ( size_t i = 0; i < children_size; ++i ) {
			( child_array + i )->refresh_sizeCache( );
			}

		m_size = compute_size_recurse( );
		}
	}

_Ret_range_( 0, UINT64_MAX )
std::uint64_t CItemBranch::compute_size_recurse( ) const {
	std::uint64_t total = 0;

	const auto childCount = m_childCount;
	const auto child_array = m_children.get( );
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
	return total;
	}

_Ret_range_( 0, UINT64_MAX )
std::uint64_t CItemBranch::size_recurse( ) const {
	static_assert( std::is_same<decltype( std::declval<CItemBranch>( ).size_recurse( ) ), decltype( std::declval<CItemBranch>( ).m_size )>::value , "The return type of CItemBranch::size_recurse needs to be fixed!!" );
	
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
//_Pre_satisfies_( this->m_type == IT_FILE )
_Pre_satisfies_( this->m_children._Myptr == nullptr ) 
PCWSTR const CItemBranch::CStyle_GetExtensionStrPtr( ) const {
	ASSERT( m_name_length < ( MAX_PATH + 1 ) );

	PCWSTR const resultPtrStr = PathFindExtensionW( m_name );
	ASSERT( resultPtrStr != '\0' );
	return resultPtrStr;
	}

//_Pre_satisfies_( this->m_type == IT_FILE )
_Pre_satisfies_( this->m_children._Myptr == nullptr )
_Success_( SUCCEEDED( return ) )
HRESULT CItemBranch::CStyle_GetExtension( WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_extension, const rsize_t strSize, _Out_ rsize_t& chars_written ) const {
	psz_extension[ 0 ] = 0;

	PWSTR resultPtrStr = PathFindExtensionW( m_name );
	ASSERT( resultPtrStr != '\0' );
	if ( resultPtrStr != '\0' ) {
		size_t extLen = 0;
		const HRESULT res_1 = StringCchLengthW( resultPtrStr, MAX_PATH, &extLen );
		if ( FAILED( res_1 ) ) {
			psz_extension[ 0 ] = 0;
			chars_written = 0;
			return res_1;
			}
		if ( extLen > ( strSize ) ) {
			psz_extension[ 0 ] = 0;
			chars_written = 0;
			return STRSAFE_E_INSUFFICIENT_BUFFER;
			}
		const HRESULT res_2 = StringCchCopyW( psz_extension, strSize, resultPtrStr );
		if ( SUCCEEDED( res_2 ) ){
			chars_written = extLen;
			return res_2;
			}
#ifdef DEBUG
		if ( SUCCEEDED( res_2 ) ) {
			ASSERT( GetExtension( ).compare( psz_extension ) == 0 );
			return res_2;
			}
#endif
		return res_2;
		}

	psz_extension[ 0 ] = 0;
	chars_written = 0;
	return STRSAFE_E_INVALID_PARAMETER;//some generic error
	}

//_Pre_satisfies_( this->m_type == IT_FILE )
_Pre_satisfies_( this->m_children._Myptr == nullptr ) 
const std::wstring CItemBranch::GetExtension( ) const {
	//if ( m_type == IT_FILE ) {
	if ( m_children == nullptr ) {
		PWSTR const resultPtrStr = PathFindExtensionW( m_name );
		ASSERT( resultPtrStr != 0 );
		if ( resultPtrStr != '\0' ) {
			return resultPtrStr;
			}
		PCWSTR const i = wcsrchr( m_name, L'.' );

		if ( i == NULL ) {
			return _T( "." );
			}
		return std::wstring( i );
		}
	return std::wstring( L"" );
	}


void CItemBranch::TmiSetRectangle( _In_ const RECT& rc ) const {
	ASSERT( ( rc.right + 1 ) >= rc.left );
	ASSERT( rc.bottom >= rc.top );
	m_rect.left   = static_cast<short>( rc.left );
	m_rect.top    = static_cast<short>( rc.top );
	m_rect.right  = static_cast<short>( rc.right );
	m_rect.bottom = static_cast<short>( rc.bottom );
	}


_Ret_range_( 0, 33000 ) DOUBLE CItemBranch::averageNameLength( ) const {
	const auto myLength = static_cast<DOUBLE>( m_name_length );
	DOUBLE childrenTotal = 0;
	
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
void CItemBranch::stdRecurseCollectExtensionData_FILE( _Inout_ std::unordered_map<std::wstring, SExtensionRecord>& extensionMap ) const {
	const size_t extensionPsz_size = 48;
	_Null_terminated_ wchar_t extensionPsz[ extensionPsz_size ] = { 0 };
	rsize_t chars_written = 0;
	HRESULT res = CStyle_GetExtension( extensionPsz, extensionPsz_size, chars_written );
	if ( SUCCEEDED( res ) ) {
		auto& value = extensionMap[ extensionPsz ];
		if ( value.files == 0 ) {
			value.ext = extensionPsz;
			value.ext.shrink_to_fit( );
			}
		++( value.files );
		value.bytes += m_size;
		return;
		}
	//use an underscore to avoid name conflict with _DEBUG build
	auto ext_ = GetExtension( );
	ext_.shrink_to_fit( );
	TRACE( _T( "Extension len: %i ( bigger than buffer! )\r\n\toffending extension:\r\n %s\r\n" ), ext_.length( ), ext_.c_str( ) );
	auto& value = extensionMap[ ext_ ];
	if ( value.files == 0 ) {
		value.ext = ext_;
		value.ext.shrink_to_fit( );
		}
	++( value.files );
	extensionMap[ ext_ ].bytes += m_size;

	}


void CItemBranch::stdRecurseCollectExtensionData( _Inout_ std::unordered_map<std::wstring, SExtensionRecord>& extensionMap ) const {
	//if ( m_type == IT_FILE ) {
	if ( m_children == nullptr ) {
		stdRecurseCollectExtensionData_FILE( extensionMap );
		return;
		}
	const auto childCount = m_childCount;
	const auto local_m_children = m_children.get( );
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



#else

#endif