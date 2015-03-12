// TreeListControl.cpp	- Implementation of CTreeListItem and CTreeListControl
//
// see `file_header_text.txt` for licensing & contact info.

#pragma once

#include "stdafx.h"

#ifndef WDS_TREELISTCONTROL_CPP
#define WDS_TREELISTCONTROL_CPP

#pragma message( "Including `" __FILE__ "`..." )

//encourage inter-procedural optimization (and class-hierarchy analysis!)
//#include "ownerdrawnlistcontrol.h"
#include "TreeListControl.h"
//#include "typeview.h"
//#include "SelectDrivesDlg.h"

#include "datastructures.h"

#include "macros_that_scare_small_children.h"

#include "globalhelpers.h"
#include "windirstat.h"
#include "mainframe.h"
#include "dirstatdoc.h"

#include "directory_enumeration.h" //for get_uncompressed_size


namespace {

	const LONG NODE_WIDTH = 15;		// Width of a node within IDB_NODES 
	const LONG INDENT_WIDTH = 18;
	const LONG HOTNODE_CX = 9;		// Size and position of the +/- buttons
	const LONG HOTNODE_CY = 9;
	const LONG HOTNODE_X = 0;

	std::vector<const CTreeListItem *> buildVectorOfPaths( _In_ const CTreeListItem* item ) {
		std::vector<const CTreeListItem *> path;
		auto p = item;
		while ( p != NULL ) {
			path.emplace_back( p );
			p = p->m_parent;
			}
		return path;
		}

	//Compare_FILETIME compiles to only 4 instructions, and is only called once, conditionally.
	//passing `lhs` & `rhs` by value cause WORSE codegen!
	inline const INT Compare_FILETIME( const FILETIME& lhs, const FILETIME& rhs ) {
		//duhh, there's a win32 function for this!
		return CompareFileTime( &lhs, &rhs );
		}


	//int __cdecl _compareProc_orig( const void* const p1, const void* const p2 ) {
	//	const auto item1 = * ( reinterpret_cast< const CTreeListItem* const* >( p1 ) );
	//	const auto item2 = * ( reinterpret_cast< const CTreeListItem* const* >( p2 ) );
	//	return item1->CompareS( item2, CTreeListItem::GetTreeListControl( )->m_sorting );
	//	}
	_Success_( return != NULL ) _Ret_maybenull_ _Pre_satisfies_( root_item->m_child_info.m_child_info_ptr != NULL )
	CTreeListItem* find_second_level_item_in_root_item( _In_ const CTreeListItem* const root_item, _In_ const std::vector<const CTreeListItem*>& path, _In_ const size_t steps_from_target ) {
		//CItemBranch* find_second_level_item_in_root_item( const CTreeListItem* const root_item, std::vector<CTreeListItem*>& path )
		ASSERT( root_item->m_child_info.m_child_info_ptr != nullptr );
		//ASSERT( root_item->m_childCount == root_item->m_child_info->m_childCount );
		for ( size_t i = 0; i < root_item->m_child_info.m_child_info_ptr->m_childCount; ++i ) {
			if ( &( root_item->m_child_info.m_child_info_ptr->m_children[ i ] ) == ( static_cast< const CTreeListItem* >( path.at( steps_from_target ) ) ) ) {
				return ( &( root_item->m_child_info.m_child_info_ptr->m_children[ i ] ) );
				}
			}
		return nullptr;
		}


	void select_and_show_experimental_algorithm( _In_ const CTreeListItem* const item, _In_ const std::vector<const CTreeListItem*>& path ) {
		//START new algorithm
		auto parent_ptr = item->m_parent;
		size_t steps_from_target = 0;

		while ( parent_ptr->m_parent != nullptr ) {
			parent_ptr = parent_ptr->m_parent;
			++steps_from_target;
			}

		const auto root_item = parent_ptr;
		//CItemBranch* child = nullptr;
		ASSERT( root_item->m_child_info.m_child_info_ptr != nullptr );
		if ( root_item->m_child_info.m_child_info_ptr == nullptr ) {
			displayWindowsMsgBoxWithMessage( L"select_and_show_experimental_algorithm, root_item->m_child_info is NULL! This should never happen! (terminating)" );
			std::terminate( );
			
			//so /analyze understands.
			abort( );
			return;
			}


		//The expression '_Param_(1)->m_child_info._Myptr!=0' is not true at this call. //But it IS!
#pragma warning(suppress: 28020)
		CTreeListItem* child = find_second_level_item_in_root_item( root_item, path, steps_from_target );

		//CItemBranch* find_second_level_item_in_root_item( const CTreeListItem* const root_item, std::vector<CTreeListItem*>& path )
		//for ( size_t i = 0; i < root_item->GetChildrenCount_( ); ++i ) {
		//	if ( &( static_cast< const CItemBranch* >( root_item )->m_children[ i ] ) == ( static_cast< const CItemBranch* >( path.at( steps_from_target ) ) ) ) {
		//		child = &( static_cast< const CItemBranch* >( root_item )->m_children[ i ] );
		//		break;
		//		}
		//	}

		ASSERT( child != NULL );
		--steps_from_target;
		do {
			ASSERT( child != NULL );
			if ( child == NULL ) {
				_CrtDbgBreak( );
				return;
				}
			ASSERT( child->m_child_info.m_child_info_ptr != nullptr );
			//ASSERT( child->m_childCount == child->m_child_info->m_childCount );
			for ( size_t i = 0; i < child->m_child_info.m_child_info_ptr->m_childCount; ++i ) {
				if ( &( child->m_child_info.m_child_info_ptr->m_children[ i ] ) == ( static_cast< const CTreeListItem* >( path.at( steps_from_target ) ) ) ) {
					child = &( child->m_child_info.m_child_info_ptr->m_children[ i ] );
					break;
					}
				}
			--steps_from_target;
			}
		while ( ( steps_from_target > 0 ) && ( child != static_cast< const CTreeListItem* >( item ) ) );

			TRACE( _T( "halted at: %s\r\n" ), child->m_name );

			//END new algorithm
		}

	//Compare_FILETIME_lessthan compiles to only 6 instructions, and is only called twice, conditionally.
	//When NOT inlined requires 5 more instructions at call site.
	//When inlined requires only 5 instructions (total) at call site.
	inline const bool Compare_FILETIME_lessthan( const FILETIME& t1, const FILETIME& t2 ) {
		//CompareFileTime returns -1 when first FILETIME is less than second FILETIME
		//Therefore: we can 'emulate' the `<` operator, by checking if ( CompareFileTime( &t1, &t2 ) == ( -1 ) );
		return ( CompareFileTime( &t1, &t2 ) == ( -1 ) );
		}

	// Sequence within IDB_NODES
	enum class ENUM_NODE {
		NODE_PLUS_SIBLING,
		NODE_PLUS_END,
		NODE_MINUS_SIBLING,
		NODE_MINUS_END,
		NODE_SIBLING,
		NODE_END,
		NODE_LINE
		};

	_Success_( SUCCEEDED( return ) ) HRESULT CStyle_FormatAttributes( _In_ const attribs& attr, _Out_writes_z_( strSize ) _Pre_writable_size_( strSize ) PWSTR psz_formatted_attributes, _In_range_( 6, 18 ) const rsize_t strSize, _Out_ rsize_t& chars_written  ) {
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
		rsize_t chars_remaining = 0;
		const HRESULT alt_errCode = StringCchPrintfExW( psz_formatted_attributes, strSize, NULL, &chars_remaining, 0, L"%s%s%s%s%s", ( ( attr.readonly ) ? L"R" : L"" ),  ( ( attr.hidden ) ? L"H" : L"" ),  ( ( attr.system ) ? L"S" : L"" ),  ( ( attr.compressed ) ? L"C" : L"" ), ( ( attr.encrypted ) ? L"E" : L"" ) );
		ASSERT( SUCCEEDED( alt_errCode ) );
		if ( SUCCEEDED( alt_errCode ) ) {
			ASSERT( strSize >= chars_remaining );
			chars_written = ( strSize - chars_remaining );
			ASSERT( wcslen( psz_formatted_attributes ) == chars_written );
			return alt_errCode;
			}
		chars_written = 0;
		WDS_ASSERT_EXPECTED_STRING_FORMAT_FAILURE_HRESULT( alt_errCode );
		WDS_STRSAFE_E_INVALID_PARAMETER_HANDLER( alt_errCode, "StringCchPrintfExW" );
		return alt_errCode;
		}

	struct compare_CTreeListItems final {
		compare_CTreeListItems( const CTreeListControl* const ctrl_in ) : ctrl( ctrl_in ) { }
		compare_CTreeListItems& operator=( compare_CTreeListItems& in ) = delete;
		bool operator()( const CTreeListItem* const lhs, const CTreeListItem* const rhs ) {
			const auto result = ( lhs->CompareS( rhs, ctrl->m_sorting ) < 0 );
			return result;
			}
		const CTreeListControl* const ctrl;
		};


	}




const bool CTreeListItem::set_plusminus_and_title_rects( _In_ const RECT rcLabel, _In_ const RECT rc_const  ) const {
	const POINT rc_top_left = { rc_const.left, rc_const.top };
	const RECT _rcPlusMinus = { 0, rcLabel.top, rcLabel.left, rcLabel.bottom };
	//const CRect rcLabel_( rcLabel );
	RECT temp_rect = _rcPlusMinus;
	VERIFY( ::OffsetRect( &temp_rect, -( rc_top_left.x ), -( rc_top_left.y ) ) );
	const RECT& new_plus_minus_rect = temp_rect;

	RECT test_rect = temp_rect;
	normalize_RECT( test_rect );

	SetPlusMinusRect( new_plus_minus_rect );

	RECT new_title_rect = rcLabel;
	VERIFY( ::OffsetRect( &new_title_rect, -( rc_top_left.x ), -( rc_top_left.y ) ) );
	SetTitleRect( new_title_rect );
	return true;
	}

//CRect rc is NOT const here so that other virtual functions may modify it?
//DOES NOT draw self for NON-NAME columns!
//DRAWS self for NAME column!//This is because we need to draw indentation & the little boxes
//We also report our width because the list control DOES NOT know about our indentation (YET)
/*
E.g.:
 C:\Some_Root_Folder
 +--some_other_folder
 |--some_file
 ---yet_another_folder
  |--some_sub_file
^^
||This is the indentation/boxes. Drawing all other columns is trivial.

*/
bool CTreeListItem::DrawSubitem( RANGE_ENUM_COL const column::ENUM_COL subitem, _In_ CDC& pdc, _In_ RECT rc, _In_ const UINT state, _Out_opt_ INT* const width, _Inout_ INT* const focusLeft, _In_ const COwnerDrawnListCtrl* const list ) const {
	const RECT& rc_const = rc;
	ASSERT( ( focusLeft != NULL ) && ( subitem >= 0 ) );

	if ( subitem != column::COL_NAME ) {
		if ( width != NULL ) {
#ifdef DEBUG
			_CrtDbgBreak( );
#endif
			//Should never happen?
			*width = ( rc_const.right - rc_const.left );
			return false;
			}
		return false;
		}
	if ( width != NULL ) {
		*width = ( rc_const.right - rc_const.left );
		}

	RECT rcNode = rc_const;
	//tree_list_control->DrawNode( this, pdc, rcNode, rcPlusMinus );//pass subitem to drawNode?
	static_cast<const CTreeListControl* const>( list )->DrawNode( this, pdc, rcNode );//pass subitem to drawNode?
	RECT rcLabel = rc_const;
	rcLabel.left = rcNode.right;
	CFont* const list_font = list->GetFont( );
	const bool list_has_focus = list->HasFocus( );
	const bool list_is_show_selection_always = list->IsShowSelectionAlways( );
	const COLORREF list_highlight_text_color = list->GetHighlightTextColor( );
	const COLORREF list_highlight_color = list->GetHighlightColor( );
	const bool list_is_full_row_selection = list->m_showFullRowSelection;
	//list_has_focus, list_is_show_selection_always, list_highlight_text_color, list_highlight_color, list_is_full_row_selection

	DrawLabel( pdc, rcLabel, state, width, focusLeft, false, list_font, list_has_focus, list_is_show_selection_always, list_highlight_text_color, list_highlight_color, list_is_full_row_selection );
	if ( width != NULL ) {
		*width = ( rcLabel.right - rcLabel.left );
		set_plusminus_and_title_rects( rcLabel, rc_const );
		return true;
		}
	return set_plusminus_and_title_rects( rcLabel, rc_const );
	}

void CTreeListItem::childNotNull( _In_ CTreeListItem* const aTreeListChild, const size_t i ) {
	if ( ( i > m_vi->cache_sortedChildren.size( ) ) /*&& ( i > 0 )*/ ) {
		m_vi->cache_sortedChildren.resize( i + 1u );
		}
	else if ( i == m_vi->cache_sortedChildren.size( ) ) {
		m_vi->cache_sortedChildren.emplace_back( aTreeListChild );
		}
	else {
		ASSERT( i < m_vi->cache_sortedChildren.size( ) );
		ASSERT( m_vi->cache_sortedChildren.at( i ) == aTreeListChild );
		ASSERT( wcscmp( m_vi->cache_sortedChildren.at( i )->m_name, aTreeListChild->m_name ) == 0u );
		m_vi->cache_sortedChildren.at( i ) = aTreeListChild;
		}
	}

_Pre_satisfies_( this->m_vi._Myptr != nullptr )
void CTreeListItem::SortChildren( _In_ const CTreeListControl* const ctrl ) {
	ASSERT( IsVisible( ) );

	const auto thisBranch = static_cast<const CTreeListItem* >( this );

	m_vi->cache_sortedChildren = thisBranch->size_sorted_vector_of_children( );

	if ( !m_vi->cache_sortedChildren.empty( ) ) {
		//qsort( m_vi->cache_sortedChildren.data( ), m_vi->cache_sortedChildren.size( ) -1, sizeof( CTreeListItem * ), &_compareProc_orig );
		//compare_CTreeListItems comp_functor( GetTreeListControl( ) );
		//std::sort( m_vi->cache_sortedChildren.begin( ), m_vi->cache_sortedChildren.end( ), &_compareProc2 );
		std::sort( m_vi->cache_sortedChildren.begin( ), m_vi->cache_sortedChildren.end( ), compare_CTreeListItems( ctrl ) );
		}
	}

//std::uint64_t CTreeListItem::size_recurse_( ) const {
//	static_assert( std::is_same<decltype( std::declval<CTreeListItem>( ).size_recurse_( ) ), decltype( std::declval<CItemBranch>( ).size_recurse( ) )>::value , "The return type of CTreeListItem::size_recurse_ needs to be fixed!!" );
//	return static_cast< const CItemBranch* >( this )->size_recurse( );
//	}

INT CTreeListItem::GetSortAttributes( ) const {
	INT ret = 0;

	// We want to enforce the order RHSACE with R being the highest priority attribute and E being the lowest priority attribute.
	ret += ( m_attr.readonly   ) ? 1000000 : 0; // R
	ret += ( m_attr.hidden     ) ? 100000  : 0; // H
	ret += ( m_attr.system     ) ? 10000   : 0; // S
	ret += ( m_attr.compressed ) ? 100     : 0; // C
	ret += ( m_attr.encrypted  ) ? 10      : 0; // E

	return ( ( m_attr.invalid ) ? 0 : ret );
	}


DOUBLE CTreeListItem::GetFraction( ) const {
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


//4,294,967,295  (4294967295 ) is the maximum number of files in an NTFS filesystem according to http://technet.microsoft.com/en-us/library/cc781134(v=ws.10).aspx
_Ret_range_( 0, 4294967295 )
std::uint32_t CTreeListItem::files_recurse( ) const {
	static_assert( std::is_same<decltype( std::declval<CTreeListItem>( ).files_recurse( ) ), decltype( std::declval<CTreeListItem>( ).m_child_info.m_child_info_ptr->m_childCount )>::value , "The return type of CItemBranch::files_recurse needs to be fixed!!" );

	if ( m_child_info.m_child_info_ptr == nullptr ) {
		ASSERT( m_child_info.m_child_info_ptr == nullptr );
		return 1;
		}
	std::uint32_t total = 0;
	static_assert( std::is_same<decltype( total ), decltype( std::declval<CTreeListItem>( ).m_child_info.m_child_info_ptr->m_childCount )>::value , "The type of total needs to be fixed!!" );
	

	ASSERT( m_child_info.m_child_info_ptr != nullptr );
	//ASSERT( m_childCount == m_child_info->m_childCount );

	const auto childCount = m_child_info.m_child_info_ptr->m_childCount;
	const auto my_m_children = m_child_info.m_child_info_ptr->m_children.get( );
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


//Sometimes I just need to COMPARE the extension with a string. So, instead of copying/screwing with string internals, I'll just return a pointer to the substring.
_Pre_satisfies_( this->m_child_info.m_child_info_ptr == nullptr ) 
PCWSTR const CTreeListItem::CStyle_GetExtensionStrPtr( ) const {
	ASSERT( m_name_length < ( MAX_PATH + 1 ) );

	PCWSTR const resultPtrStr = PathFindExtensionW( m_name );
	ASSERT( resultPtrStr != '\0' );
	return resultPtrStr;
	}


std::vector<CTreeListItem*> CTreeListItem::size_sorted_vector_of_children( ) const {
	std::vector<CTreeListItem*> children;
	if ( m_child_info.m_child_info_ptr == nullptr ) {
		//ASSERT( m_childCount == 0 );
		ASSERT( m_child_info.m_child_info_ptr == nullptr );
		return children;
		}
	ASSERT( m_child_info.m_child_info_ptr != nullptr );
	//ASSERT( m_childCount == m_child_info->m_childCount );

	const auto child_count = m_child_info.m_child_info_ptr->m_childCount;
	children.reserve( child_count );
	const auto local_m_children = m_child_info.m_child_info_ptr->m_children.get( );
	if ( m_child_info.m_child_info_ptr->m_children != nullptr ) {
		//Not vectorized: 1200, loop contains data dependencies
		for ( size_t i = 0; i < child_count; ++i ) {
			children.emplace_back( local_m_children + i );
			}
		}
#ifdef DEBUG
	else {
		//ASSERT( m_childCount == 0 );
		}
#endif
	//TODO: qsort is bleh
	qsort( children.data( ), static_cast< const size_t >( children.size( ) ), sizeof( CTreeListItem* ), &CItem_compareBySize );
	//std::sort( children.begin( ), children.end( ), [] ( const CTreeListItem* const lhs, const CTreeListItem* const rhs ) { return static_cast< const CTreeListItem* >( lhs )->size_recurse( ) < static_cast< const CTreeListItem* >( rhs )->size_recurse( ); } );
	return children;
	}

_Ret_range_( 0, UINT64_MAX )
std::uint64_t CTreeListItem::size_recurse( ) const {
	static_assert( std::is_same<decltype( std::declval<CTreeListItem>( ).size_recurse( ) ), decltype( std::declval<CTreeListItem>( ).m_size )>::value , "The return type of CItemBranch::size_recurse needs to be fixed!!" );
	//ASSERT( m_size != UINT64_ERROR );
	//if ( m_type == IT_FILE ) {
	if ( m_child_info.m_child_info_ptr == nullptr ) {
		//ASSERT( m_childCount == 0 );
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




//_Ret_range_( 0, UINT32_MAX ) 
//std::uint32_t CTreeListItem::GetChildrenCount_( ) const {
//	static_assert( std::is_same<decltype( std::declval<CTreeListItem>( ).GetChildrenCount_( ) ), decltype( std::declval<CTreeListItem>( ).m_childCount )>::value , "The return type of CTreeListItem::GetChildrenCount_ needs to be fixed!!" );
//	return m_childCount;
//	}

//_Ret_maybenull_
//CTreeListItem* CTreeListItem::children_ptr( ) const {
//	return static_cast< const CTreeListItem* >( this )->m_children.get( );
//	}

_Success_( return != NULL ) _Must_inspect_result_ _Ret_maybenull_
CTreeListItem* CTreeListItem::GetSortedChild( _In_ const size_t i ) const {
	ASSERT( m_vi != nullptr );
	ASSERT( !( m_vi->cache_sortedChildren.empty( ) ) );
	if ( m_vi != nullptr ) {
		if ( !( m_vi->cache_sortedChildren.empty( ) ) ) {
			return m_vi->cache_sortedChildren.at( i );
			}
		return NULL;
		}
	return NULL;
	}

void CTreeListItem::TmiSetRectangle( _In_ const RECT& rc ) const {
	ASSERT( ( rc.right + 1 ) >= rc.left );
	ASSERT( rc.bottom >= rc.top );
	m_rect.left   = static_cast<short>( rc.left );
	m_rect.top    = static_cast<short>( rc.top );
	m_rect.right  = static_cast<short>( rc.right );
	m_rect.bottom = static_cast<short>( rc.bottom );
	}

const COLORREF CTreeListItem::Concrete_ItemTextColor( ) const {
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


inline INT CTreeListItem::concrete_compare( _In_ const CTreeListItem* const other, RANGE_ENUM_COL const column::ENUM_COL subitem ) const {
	if ( other == this ) {
		return 0;
		}
	if ( m_parent == NULL ) {
		return -2;
		}
	if ( other->m_parent == NULL ) {
		return 2;
		}
	if ( m_parent == other->m_parent ) {
		const auto thisBranch = static_cast< const CTreeListItem* >( this );//ugly, I know
		return thisBranch->CompareSibling( other, subitem );
		}
	const auto my_indent = GetIndent( );
	const auto other_indent = other->GetIndent( );
	if ( my_indent < other_indent ) {
		return concrete_compare( other->m_parent, subitem );
		}
	else if ( my_indent > other_indent ) {
		return m_parent->concrete_compare( other, subitem );
		}
	return m_parent->concrete_compare( other->m_parent, subitem );
	}

INT CTreeListItem::Compare( _In_ const COwnerDrawnListItem* const baseOther, RANGE_ENUM_COL const column::ENUM_COL subitem ) const {
	const auto other = static_cast<const CTreeListItem *>( baseOther );
	return concrete_compare( other, subitem );
	}

FILETIME CTreeListItem::FILETIME_recurse( ) const {
	if ( m_child_info.m_child_info_ptr == nullptr ) {
		return m_lastChange;
		}
	const auto my_m_children = m_child_info.m_child_info_ptr->m_children.get( );
	if ( my_m_children == nullptr ) {
		ASSERT( m_child_info.m_child_info_ptr == nullptr );
		//ASSERT( m_childCount == 0 );
		return m_lastChange;
		}
	ASSERT( m_child_info.m_child_info_ptr != nullptr );
	//ASSERT( m_childCount == m_child_info->m_childCount );
	auto ft = zero_init_struct<FILETIME>( );
	if ( Compare_FILETIME_lessthan( ft, m_lastChange ) ) {
		ft = m_lastChange;
		}
	//ASSERT( m_childCount == m_child_info->m_childCount );

	const auto childCount = m_child_info.m_child_info_ptr->m_childCount;
	
	//Not vectorized: 1304, loop includes assignments of different sizes
	for ( size_t i = 0; i < childCount; ++i ) {
		const auto ft_child = ( my_m_children + i )->FILETIME_recurse( );
		if ( Compare_FILETIME_lessthan( ft, ft_child ) ) {
			ft = ft_child;
			}
		}
	return ft;
	}



_Success_( return < child_count ) _Pre_satisfies_( child_count > 0 )
size_t CTreeListItem::FindSortedChild( _In_ const CTreeListItem* const child, _In_ const size_t child_count ) const {
	ASSERT( child_count > 0u );
	//ASSERT( child_count == m_childCount );
	ASSERT( m_child_info.m_child_info_ptr != nullptr );
	for ( size_t i = 0u; i < child_count; i++ ) {
		if ( child == GetSortedChild( i ) ) {
			return i;
			}
		}
	return child_count;
	}

_Pre_satisfies_( this->m_parent != NULL )
bool CTreeListItem::HasSiblings( ) const {
	if ( m_parent == NULL ) {
		return false;
		}
	ASSERT( m_parent->m_child_info.m_child_info_ptr != nullptr );
	const auto count = m_parent->m_child_info.m_child_info_ptr->m_childCount;
	if ( count < 2u ) {
		ASSERT( count == 1u );
		return false;
		}
	ASSERT( count >= 2u );
	ASSERT( count > 0 );
	return true;
	}

void CTreeListItem::SetVisible( _In_ const bool next_state_visible ) const {
	if ( next_state_visible ) {
		m_vi.reset( new VISIBLEINFO );
		m_vi->isExpanded = false;
		if ( m_parent == NULL ) {
			m_vi->indent = 0;
			}
		else {
			ASSERT( m_parent != NULL );
			m_vi->indent = m_parent->GetIndent( ) + 1;
			if ( m_attr.compressed ) {
				auto uncompressed_size_temp = get_uncompressed_file_size( this );
				if ( uncompressed_size_temp == 0 ) {
					uncompressed_size_temp = m_size;
					}
				if ( uncompressed_size_temp == UINT64_ERROR ) {
					uncompressed_size_temp = m_size;
					}
				if ( uncompressed_size_temp != 0 ) {
					const auto uncompressed_size = uncompressed_size_temp;
					const auto ratio = ( static_cast< const double >( m_size ) / static_cast< const double >( uncompressed_size ) );
					m_vi->ntfs_compression_ratio = ratio;
					}
				else {
					m_vi->ntfs_compression_ratio = 1;
					}
				}
			}
		m_vi->isExpanded = false;
		return;
		}
	ASSERT( m_vi != nullptr );
	m_vi.reset( );
	}

std::wstring CTreeListItem::GetPath( ) const {
	std::wstring pathBuf;
	pathBuf.reserve( MAX_PATH );
	UpwardGetPathWithoutBackslash( pathBuf );
	ASSERT( wcslen( m_name ) == m_name_length );
	ASSERT( wcslen( m_name ) < 33000 );
	ASSERT( pathBuf.length( ) < 33000 );
	return pathBuf;
	}

//_Pre_satisfies_( this->m_type == IT_FILE )
_Pre_satisfies_( this->m_child_info.m_child_info_ptr == nullptr ) 
void CTreeListItem::stdRecurseCollectExtensionData_FILE( _Inout_ std::unordered_map<std::wstring, minimal_SExtensionRecord>& extensionMap ) const {
	ASSERT( m_child_info.m_child_info_ptr == nullptr );

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

void CTreeListItem::stdRecurseCollectExtensionData( _Inout_ std::unordered_map<std::wstring, minimal_SExtensionRecord>& extensionMap ) const {
	//if ( m_type == IT_FILE ) {
	if ( m_child_info.m_child_info_ptr == nullptr ) {
		stdRecurseCollectExtensionData_FILE( extensionMap );
		return;
		}

	ASSERT( m_child_info.m_child_info_ptr != nullptr );
	//ASSERT( m_childCount == m_child_info->m_childCount );
	const auto childCount = m_child_info.m_child_info_ptr->m_childCount;
	const auto local_m_children = m_child_info.m_child_info_ptr->m_children.get( );
	//todo: Iterate over the heapmanager items instead
	//Not vectorized: 1200, loop contains data dependencies
	for ( size_t i = 0; i < childCount; ++i ) {
		( local_m_children + i )->stdRecurseCollectExtensionData( extensionMap );
		}
	}


_Pre_satisfies_( subitem == column::COL_PERCENTAGE ) _Success_( SUCCEEDED( return ) )
const HRESULT CTreeListItem::WriteToStackBuffer_COL_PERCENTAGE( RANGE_ENUM_COL const column::ENUM_COL subitem, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const {
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
const HRESULT CTreeListItem::WriteToStackBuffer_COL_NTCOMPRESS( RANGE_ENUM_COL const column::ENUM_COL subitem, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const {
	//auto res = StringCchPrintfW( psz_text, strSize, L"%.1f%%", ( GetFraction( ) * static_cast<DOUBLE>( 100 ) ) );
#ifndef DEBUG
	UNREFERENCED_PARAMETER( subitem );
#endif
	ASSERT( subitem == column::COL_NTCOMPRESS );
	ASSERT( strSize > 5u );
	
	if ( !( m_attr.compressed ) ) {
		return WriteToStackBuffer_do_nothing( psz_text, strSize, sizeBuffNeed, chars_written );
		}
	if ( m_child_info.m_child_info_ptr != nullptr ) {
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
const HRESULT CTreeListItem::WriteToStackBuffer_COL_SUBTREETOTAL( RANGE_ENUM_COL const column::ENUM_COL subitem, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const {
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
const HRESULT CTreeListItem::WriteToStackBuffer_COL_FILES( RANGE_ENUM_COL const column::ENUM_COL subitem, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const {
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
const HRESULT CTreeListItem::WriteToStackBuffer_COL_LASTCHANGE( RANGE_ENUM_COL const column::ENUM_COL subitem, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, _Out_ _On_failure_( _Post_valid_ ) rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const {
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
const HRESULT CTreeListItem::WriteToStackBuffer_COL_ATTRIBUTES( RANGE_ENUM_COL const column::ENUM_COL subitem, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const {
#ifndef DEBUG
	UNREFERENCED_PARAMETER( subitem );
#endif
	ASSERT( subitem == column::COL_ATTRIBUTES );
	const HRESULT res = CStyle_FormatAttributes( m_attr, psz_text, strSize, chars_written );
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
HRESULT CTreeListItem::Text_WriteToStackBuffer( RANGE_ENUM_COL const column::ENUM_COL subitem, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, _On_failure_( _Post_valid_ ) rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const {
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
				return WriteToStackBuffer_default( subitem, psz_text, strSize, sizeBuffNeed, chars_written, L"CTreeListItem::" );
	}
	}


INT CTreeListItem::CompareSibling( _In_ const CTreeListItem* const tlib, _In_ _In_range_( 0, INT32_MAX ) const column::ENUM_COL subitem ) const {
	auto const other = static_cast< const CTreeListItem* >( tlib );
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
				return ( ( ( m_vi != nullptr ) && ( other->m_vi != nullptr ) ) ? ( signum( m_vi->ntfs_compression_ratio - other->m_vi->ntfs_compression_ratio ) ) : 0 );
			case column::COL_LASTCHANGE:
				return Compare_FILETIME( FILETIME_recurse( ), other->FILETIME_recurse( ) );

			case column::COL_ATTRIBUTES:
				return signum( GetSortAttributes( ) - other->GetSortAttributes( ) );
			default:
				ASSERT( false );
				return 666;
		}
	}

void CTreeListItem::refresh_sizeCache( ) {
	//if ( m_type == IT_FILE ) {
	if ( m_child_info.m_child_info_ptr == nullptr ) {
		//ASSERT( m_childCount == 0 );
		ASSERT( m_child_info.m_child_info_ptr == nullptr );
		ASSERT( m_size < UINT64_ERROR );
		return;
		}
	if ( m_size == UINT64_ERROR ) {
		ASSERT( m_child_info.m_child_info_ptr != nullptr );
		//ASSERT( m_child_info->m_childCount == m_childCount );

		const auto children_size = m_child_info.m_child_info_ptr->m_childCount;
		const auto child_array = m_child_info.m_child_info_ptr->m_children.get( );
		for ( size_t i = 0; i < children_size; ++i ) {
			( child_array + i )->refresh_sizeCache( );
			}

		//---
		std::uint64_t total = 0;

		ASSERT( m_child_info.m_child_info_ptr != nullptr );
		//ASSERT( m_child_info->m_childCount == m_childCount );

		const auto childCount = m_child_info.m_child_info_ptr->m_childCount;
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


_Ret_range_( 0, 33000 ) DOUBLE CTreeListItem::averageNameLength( ) const {
	const auto myLength = static_cast<DOUBLE>( m_name_length );
	DOUBLE childrenTotal = 0;
	//TODO: take advantage of block heap allocation in this
	
	//if ( m_type != IT_FILE ) {
	if ( m_child_info.m_child_info_ptr != nullptr ) {
		ASSERT( m_child_info.m_child_info_ptr != nullptr );
		//ASSERT( m_child_info->m_childCount == m_childCount );
		
		const auto childCount = m_child_info.m_child_info_ptr->m_childCount;
		const auto my_m_children = m_child_info.m_child_info_ptr->m_children.get( );
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
	//ASSERT( m_childCount == 0 );
	return myLength;
	}

void CTreeListItem::UpwardGetPathWithoutBackslash( std::wstring& pathBuf ) const {
	auto myParent = GetParentItem( );
	if ( myParent != NULL ) {
		myParent->UpwardGetPathWithoutBackslash( pathBuf );
		}
#ifdef DEBUG
	auto guard_assert = WDS_SCOPEGUARD_INSTANCE( [ &] { ASSERT( wcslen( m_name ) == m_name_length ); } );
#endif
	ASSERT( wcslen( m_name ) < 33000 );
	if ( m_child_info.m_child_info_ptr == nullptr ) {
		//ASSERT( m_child_info->m_children == nullptr );
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

_Pre_satisfies_( this->m_vi._Myptr != nullptr ) 
RECT CTreeListItem::GetPlusMinusRect( ) const {
	ASSERT( IsVisible( ) );
	return BuildRECT( m_vi->rcPlusMinus );
	}

_Pre_satisfies_( this->m_vi._Myptr != nullptr )
RECT CTreeListItem::GetTitleRect( ) const {
	ASSERT( IsVisible( ) );
	return BuildRECT( m_vi->rcTitle );
	}

//RECT CTreeListItem::TmiGetRectangle( ) const {
//	return BuildRECT( m_rect );
//	}

//Unconditionally called only ONCE, so we ask for inlining.
//Encodes the attributes to fit (in) 1 byte
//void CTreeListItem::SetAttributes( _In_ const DWORD attr ) {
//	if ( attr == INVALID_FILE_ATTRIBUTES ) {
//		m_attr.invalid = true;
//		return;
//		}
//	m_attr.readonly   = ( ( attr bitand FILE_ATTRIBUTE_READONLY      ) != 0 );
//	m_attr.hidden     = ( ( attr bitand FILE_ATTRIBUTE_HIDDEN        ) != 0 );
//	m_attr.system     = ( ( attr bitand FILE_ATTRIBUTE_SYSTEM        ) != 0 );
//	m_attr.compressed = ( ( attr bitand FILE_ATTRIBUTE_COMPRESSED    ) != 0 );
//	m_attr.encrypted  = ( ( attr bitand FILE_ATTRIBUTE_ENCRYPTED     ) != 0 );
//	m_attr.reparse    = ( ( attr bitand FILE_ATTRIBUTE_REPARSE_POINT ) != 0 );
//	m_attr.invalid    = false;
//	}


IMPLEMENT_DYNAMIC( CTreeListControl, COwnerDrawnListCtrl )

_Pre_satisfies_( ( parent + 1 ) < index ) _Ret_range_( -1, INT_MAX ) 
int CTreeListControl::collapse_parent_plus_one_through_index( _In_ const CTreeListItem* thisPath, const int index, _In_range_( 0, INT_MAX ) const int parent ) {
	for ( int k = ( parent + 1 ); k < index; k++ ) {
		if ( !CollapseItem( k ) ) {
			break;
			}
		//We need to move UP the hierarchy, so we need to collapse items we're moving UP FROM
		}
	return FindListItem( thisPath );
	//index = FindTreeItem( thisPath );
	}


void CTreeListControl::adjustColumnSize( _In_ const CTreeListItem* const item_at_index ) {
	static_assert( column::COL_NAME == 0, "GetSubItemWidth used to accept an INT as the second parameter. The value of zero, I believe, should be COL_NAME" );
	static_assert( std::is_convertible<column::ENUM_COL, int>::value, "we're gonna need to do this!" );
	static_assert( std::is_convertible<std::underlying_type<column::ENUM_COL>::type, int>::value, "we're gonna need to do this!" );

	const auto w = GetSubItemWidth( item_at_index, column::COL_NAME ) + 5;
	ASSERT( w == ( CListCtrl::GetStringWidth( item_at_index->m_name ) + 15 ) );
	const auto colWidth = CListCtrl::GetColumnWidth( static_cast<int>( column::COL_NAME ) );
	if ( colWidth < w ) {
		VERIFY( CListCtrl::SetColumnWidth( 0, w + colWidth ) );
		}
	}

void CTreeListControl::expand_item_no_scroll_then_doWhateverJDoes( _In_ const CTreeListItem* const pathZero, _In_range_( 0, INT_MAX ) const int parent ) {
	//auto j = FindTreeItem( pathZero );
	TRACE( _T( "doing whatever j does....\r\n" ) );
	const auto item_index = FindListItem( pathZero );
	if ( item_index == -1 ) {
		ASSERT( parent >= 0 );
		//ExpandItem( parent, false );
		ExpandItemNoScroll( parent );

		//FindListItem is const, doesn't affect anything.
		//j = FindListItem( pathZero );//TODO: j?
		}
	}

void CTreeListControl::expand_item_then_scroll_to_it( _In_ const CTreeListItem* const pathZero, _In_range_( 0, INT_MAX ) const int index, _In_ const bool showWholePath ) {
	expand_item_no_scroll_then_doWhateverJDoes( pathZero, index );
	ASSERT( index >= 0 );
	const auto item_at_index = GetItem( index );
	ASSERT( item_at_index != NULL );
	if ( item_at_index != NULL ) {
		adjustColumnSize( item_at_index );
		}
	if ( showWholePath ) {
		TRACE( _T( "Ensuring item is visible by scrolling to it...\r\n" ) );
		VERIFY( CListCtrl::EnsureVisible( 0, false ) );
		}
	SelectItem( index );
	}

INT CTreeListControl::find_item_then_show_first_try_failed( _In_ const CTreeListItem* const thisPath, const int i ) {
	TRACE( _T( "Searching %s ( this path element ) for next path element...not found! Expanding %I64d...\r\n" ), thisPath->m_name, i );
	ExpandItemNoScroll( i );
		
	//we expect to find the item on the second try.
	const INT index = FindListItem( thisPath );
	TRACE( _T( "Set index to %i\r\n" ), index );
	ASSERT( index != -1 );
	return index;
	}

void CTreeListControl::find_item_then_show( _In_ const CTreeListItem* const thisPath, const int i, int& parent, _In_ const bool showWholePath, _In_ const CTreeListItem* const target_item_in_path ) {
	//auto index = FindTreeItem( thisPath );
	auto index = FindListItem( thisPath );
	if ( index == -1 ) {
		index = find_item_then_show_first_try_failed( thisPath, i );
		if ( index == -1 ) {
			TRACE( _T( "Logic error! Item not found!\r\n" ) );
			displayWindowsMsgBoxWithMessage( L"Logic error! Item not found!" );
			parent = index;
			return;
			}
		}
	else {
		//if we've found the item, then we should close anything that we opened in the process?
		TRACE( _T( "Searching %s for next path element...found! path.at( %I64d ), index: %i\r\n" ), thisPath->m_name, std::int64_t( i ), index );
		index = collapse_parent_plus_one_through_index( thisPath, index, parent );
		TRACE( _T( "Collapsing items [%i, %i), new index %i. Item count: %i\r\n" ), ( parent + 1 ), index, index, GetItemCount( ) );
		}
	ASSERT( target_item_in_path != NULL );
	parent = index;
	ASSERT( index != -1 );
	//if target_item_in_path is found, then we expand (the item?), adjust the name column width, and scroll to it.
	if ( index != -1 ) {
		expand_item_then_scroll_to_it( target_item_in_path, index, showWholePath );
		}
	}

void CTreeListControl::SelectAndShowItem( _In_ const CTreeListItem* const item, _In_ const bool showWholePath ) {
	/*
		Here's what's going on in this function:
		The user has clicked an item in the treemap (colored squares at the bottom of the screen).
		Build vector of paths returns each path component as a pointer, which would look like:
		This last component is the ROOT |
		V
		`{ L"test_how_a_really_fucking_horrible_expression_in_SequoiaView_is_parsed.txt", L"altWinDirStat", L"GitHub", L"C:\Users\Alexander Riccio\Documents" };`

		We don't care about the same kind of path components that the file system does, only those in our tree. This particular scan was run on `C:\Users\Alexander Riccio\Documents`, which is the **root object**, and thus it's display name is `C:\Users\Alexander Riccio\Documents`

		Now, we start at the last item in this vector, and the first item in the tree list.

		STEP 1:

		TreeList:
		C:\Users\Alexander Riccio\Documents [<- we are here]
		+   GitHub
		+   SomeOtherFolder
		+   no_I_do_not_care_about_naming_conventions_I_use_whatever_works
		+   My_documents_folder_is_full_of_crap
		+   etc...[<- not an actual folder on my computer, thankfully]

		vector of paths:
		`{ L"test_how_a_really_fucking_horrible_expression_in_SequoiaView_is_parsed.txt", L"altWinDirStat", L"GitHub", L"C:\Users\Alexander Riccio\Documents" };`
		^
		|[we are here]

		STEP 2:

		Now we "expand" the root item (it already is, so no work necessary), and look for the next path component.

		TreeList:
		C:\Users\Alexander Riccio\Documents
		+   GitHub [<- we are here]
		+   SomeOtherFolder
		+   no_I_do_not_care_about_naming_conventions_I_use_whatever_works
		+   My_documents_folder_is_full_of_crap
		+   etc...

		vector of paths:
		`{ L"test_how_a_really_fucking_horrible_expression_in_SequoiaView_is_parsed.txt", L"altWinDirStat", L"GitHub", L"C:\Users\Alexander Riccio\Documents" };`
		^
		|[we are here]

		STEP 3:

		Now we expand L"GitHub", and search for L"altWinDirStat" therein.

		TreeList:
		C:\Users\Alexander Riccio\Documents
		+   GitHub
		+   altWinDirStat [<- we are here]
		+   asio
		+   boost.afio
		+   FFmpeg
		+   FileFindBench
		+   HopperScripts
		+   i7z
		+   kicad-source-mirror
		+   OBS
		+   pattern
		+   pythonz
		+   RTClib
		+   SALExamples
		+   study.py
		+   subbrute
		+   termsaver
		+   update-pip-packages
		+   vlc
		+   webcam-pulse-detector
		+   SomeOtherFolder
		+   no_I_do_not_care_about_naming_conventions_I_use_whatever_works
		+   My_documents_folder_is_full_of_crap
		+   etc...

		vector of paths:
		`{ L"test_how_a_really_fucking_horrible_expression_in_SequoiaView_is_parsed.txt", L"altWinDirStat", L"GitHub", L"C:\Users\Alexander Riccio\Documents" };`
		^
		|[we are here]

		STEP 4:

		Now we expand L"altWinDirStat", and search for L"test_how_a_really_fucking_horrible_expression_in_SequoiaView_is_parsed.txt" therein.

		TreeList:
		C:\Users\Alexander Riccio\Documents
		+   GitHub
		+   altWinDirStat
		+   .git
		+   filesystem-docs-n-stuff
		+   stress_progs
		+   WinDirStat
		+   all_sorts_of_other_crap
		test_how_a_really_fucking_horrible_expression_in_SequoiaView_is_parsed.txt [<- we are here]
		+   asio
		+   boost.afio
		+   FFmpeg
		+   FileFindBench
		+   HopperScripts
		+   i7z
		+   kicad-source-mirror
		+   OBS
		+   pattern
		+   pythonz
		+   RTClib
		+   SALExamples
		+   study.py
		+   subbrute
		+   termsaver
		+   update-pip-packages
		+   vlc
		+   webcam-pulse-detector
		+   SomeOtherFolder
		+   no_I_do_not_care_about_naming_conventions_I_use_whatever_works
		+   My_documents_folder_is_full_of_crap
		+   etc...

		vector of paths:
		`{ L"test_how_a_really_fucking_horrible_expression_in_SequoiaView_is_parsed.txt", L"altWinDirStat", L"GitHub", L"C:\Users\Alexander Riccio\Documents" };`
		^
		|[we are here]


		...and we're done! Yay!


		Which is also why this needs to be refactored.

		*/


	//This function is VERY finicky. Be careful.
	CWnd::SetRedraw( FALSE );
	const auto path = buildVectorOfPaths( item );

#ifdef DEBUG
	for ( size_t inner = 0; inner < path.size( ); ++inner ) {
		ASSERT( path.at( inner )->m_name != NULL );
		TRACE( _T( "path component %I64u: `%s` (%p)\r\n" ), std::uint64_t( inner ), path.at( inner )->m_name, path.at( inner ) );
		}
#endif

	//select_and_show_experimental_algorithm( item, path );

	auto parent = 0;
	for ( auto i = static_cast<std::int64_t>( path.size( ) - 1 ); i >= 0; --i ) {//Iterate downwards, root first, down each matching parent, until we find item
		const auto thisPath = path.at( static_cast<size_t>( i ) );
		ASSERT( thisPath != NULL );
		if ( thisPath == NULL ) {
			continue;
			}
		ASSERT( static_cast<std::uint64_t>( i ) < INT_MAX );
		find_item_then_show( thisPath, static_cast< int >( i ), parent, showWholePath, path.at( 0 ) );
		}
	CWnd::SetRedraw( TRUE );
	}

void CTreeListControl::InitializeNodeBitmaps( ) const {
	
	m_bmNodes0.Detach( );
	m_bmNodes1.Detach( );
	m_bmNodes0.DeleteObject( );
	m_bmNodes1.DeleteObject( );

	COLORMAP cm = { RGB( 255, 0, 255 ), 0 };
	
	cm.to = m_windowColor;
	VERIFY( m_bmNodes0.LoadMappedBitmap( IDB_NODES, 0, &cm, 1 ) );
	cm.to = m_stripeColor;
	VERIFY( m_bmNodes1.LoadMappedBitmap( IDB_NODES, 0, &cm, 1 ) );
	}

BEGIN_MESSAGE_MAP(CTreeListControl, COwnerDrawnListCtrl)
	ON_WM_CONTEXTMENU()
	ON_WM_MEASUREITEM_REFLECT()
	ON_WM_LBUTTONDOWN()
	ON_WM_SETFOCUS()
	ON_WM_KEYDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_DESTROY()
END_MESSAGE_MAP()

_Pre_satisfies_( item->m_vi._Myptr != nullptr ) _Success_( return )
const bool CTreeListControl::DrawNodeNullWidth( _In_ const CTreeListItem* const item, _In_ CDC& pdc, _In_ const RECT& rcRest, _In_ CDC& dcmem, _In_ const UINT ysrc ) const {
	bool didBitBlt = false;
	auto ancestor = item;
	//Not vectorized: 1304, loop includes assignments of different sizes
	for ( auto indent = ( item->GetIndent( ) - 2 ); indent >= 0; --indent ) {
		if ( ancestor == NULL ) {
			//continue;
			//break;
			return false;
			}
		ancestor = ancestor->m_parent;
		if ( ancestor == NULL ) {
			continue;
			}
		if ( ancestor->HasSiblings( ) ) {
			ASSERT_VALID( &dcmem );
			//god, I hate code like this.
			VERIFY( pdc.BitBlt(
								( static_cast<int>( rcRest.left ) + indent * static_cast<int>( INDENT_WIDTH ) ),
								static_cast<int>( rcRest.top ),
								static_cast<int>( NODE_WIDTH ),
								static_cast<int>( NODE_HEIGHT ),
								&dcmem,
								( NODE_WIDTH * static_cast<int>( ENUM_NODE::NODE_LINE ) ),
								static_cast<int>( ysrc ),
								SRCCOPY
								)
					);
			didBitBlt = true;
			}
		}
	return didBitBlt;
	}

void CTreeListControl::SelectItem( _In_ const CTreeListItem* const item ) {
	//auto i = FindTreeItem( item );
	const auto i = FindListItem( item );
	if ( i != -1 ) {
		SelectItem( i );
		}
	}


_Must_inspect_result_ _Success_( return != -1 )  _Ret_range_( 0, INT_MAX )
INT CTreeListControl::GetSelectedItem( ) const {
	auto pos = CListCtrl::GetFirstSelectedItemPosition( );
	if ( pos == NULL ) {
		return -1;
		}
	//`pos` is an _Inout_ here, so it can't be const above.
	return CListCtrl::GetNextSelectedItem( pos );
	}
void CTreeListControl::handle_OnContextMenu( CPoint pt ) const {
	const auto i = GetSelectedItem( );
	if ( i == -1 ) {
		TRACE( _T( "OnContextMenu failed to get a valid selected item! returning early....\r\n" ) );
		return;
		}

	const auto item = GetItem( i );
	const auto thisHeader = CListCtrl::GetHeaderCtrl( );
	const auto rc = GetWholeSubitemRect( i, 0, thisHeader );
	if ( item == NULL ) {
		displayWindowsMsgBoxWithMessage( L"GetItem returned NULL!" );
		return;
		}
	RECT trect = item->GetTitleRect( );
	VERIFY( ::OffsetRect( &trect, rc.left, rc.top ) );
	//RECT rcTitle = CRect( item->GetTitleRect( ) ) + rc.TopLeft( );
	//ASSERT( CRect( rcTitle ) == CRect( trect ) );
	CMenu menu;
	VERIFY( menu.LoadMenuW( IDR_POPUPLIST ) );
	const auto sub = menu.GetSubMenu( 0 );
	PrepareDefaultMenu( static_cast<CTreeListItem*>( item ), sub );

	const RECT& rcTitle = trect;
	// Show pop-up menu and act accordingly. The menu shall not overlap the label but appear horizontally at the cursor position,  vertically under (or above) the label.
	// TrackPopupMenuEx() behaves in the desired way, if we exclude the label rectangle extended to full screen width.

	TPMPARAMS tp;
	tp.cbSize = sizeof( tp );
	tp.rcExclude = rcTitle;
	CWnd::ClientToScreen( &tp.rcExclude );

	RECT desktop;
	CWnd::GetDesktopWindow( )->GetWindowRect( &desktop );

	tp.rcExclude.left = desktop.left;
	tp.rcExclude.right = desktop.right;

	const INT overlap = 2;	// a little vertical overlapping
	tp.rcExclude.top += overlap;
	tp.rcExclude.bottom -= overlap;

	VERIFY( sub->TrackPopupMenuEx( TPM_LEFTALIGN | TPM_LEFTBUTTON, pt.x, pt.y, AfxGetMainWnd( ), &tp ) );
	}

void CTreeListControl::OnContextMenu( CWnd* /*pWnd*/, CPoint pt ) {
	handle_OnContextMenu( pt );
	}

void CTreeListControl::SelectItem( _In_ _In_range_( 0, INT_MAX ) const INT i ) {
	TRACE( _T( "Selecting item: %i\r\n" ), i );
	VERIFY( CListCtrl::SetItemState( i, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED ) );
	VERIFY( CListCtrl::EnsureVisible( i, false ) );
	}

void CTreeListControl::PrepareDefaultMenu( _In_ const CTreeListItem* const item, _Out_ CMenu* const menu ) const {
	//if ( item->m_type == IT_FILE ) {
	if ( item->m_child_info.m_child_info_ptr == nullptr ) {
		VERIFY( menu->DeleteMenu( 0, MF_BYPOSITION ) );	// Remove "Expand/Collapse" item
		VERIFY( menu->DeleteMenu( 0, MF_BYPOSITION ) );	// Remove separator
		}
	else {
		//const auto command = MAKEINTRESOURCEW( item->IsExpanded( ) && item->HasChildren( ) ? IDS_COLLAPSE : IDS_EXPAND );
		const auto command = ( item->IsExpanded( ) && item->HasChildren( ) ? L"Co&llapse" : L"E&xpand" );
		VERIFY( menu->ModifyMenuW( ID_POPUP_TOGGLE, MF_BYCOMMAND | MF_STRING, ID_POPUP_TOGGLE, command ) );
		VERIFY( menu->SetDefaultItem( ID_POPUP_TOGGLE, false ) );
		}
	}

void CTreeListControl::OnSetFocus( _In_ CWnd* pOldWnd ) {
	CWnd::OnSetFocus( pOldWnd );
	m_frameptr->SetLogicalFocus( LOGICAL_FOCUS::LF_DIRECTORYLIST );
	}

//void CTreeListControl::SysColorChanged( ) {
//	InitializeColors( );
//	InitializeNodeBitmaps( );
//	}

BOOL CTreeListControl::CreateEx( _In_ const DWORD dwExStyle, _In_ DWORD dwStyle, _In_ const RECT& rect, _In_ CWnd* pParentWnd, _In_ const UINT nID ) {
	InitializeNodeBitmaps( );

	dwStyle or_eq ( LVS_OWNERDRAWFIXED bitor LVS_SINGLESEL );
	VERIFY( CListCtrl::CreateEx( dwExStyle, dwStyle, rect, pParentWnd, nID ) );
	return true;
	}


_Must_inspect_result_ _Success_( return != NULL ) _Ret_maybenull_
CTreeListItem* CTreeListControl::GetItem( _In_ _In_range_( 0, INT_MAX ) const int i ) const {
	ASSERT( i < CListCtrl::GetItemCount( ) );
	const auto itemCount = CListCtrl::GetItemCount( );
	if ( i < itemCount ) {
		return reinterpret_cast< CTreeListItem* >( CListCtrl::GetItemData( i ) );
		}
	return NULL;
	}

void CTreeListControl::SetRootItem( _In_opt_ const CTreeListItem* const root ) {
	VERIFY( CListCtrl::DeleteAllItems( ) );
	if ( root == NULL ) {
		return;
		}
	CWnd::SetRedraw( FALSE );
	InsertItem( root, 0 );
	CWnd::SetRedraw( TRUE );

	//ExpandItem( static_cast<int>( 0 ), true );//otherwise ambiguous call - is it a NULL pointer?
	ExpandItemAndScroll( 0 );//otherwise ambiguous call - is it a NULL pointer?
	
	}

void CTreeListControl::InsertItem( _In_ const CTreeListItem* const item, _In_ _In_range_( 0, INT32_MAX ) const INT_PTR i ) {
	COwnerDrawnListCtrl::InsertListItem( i, item );
	item->SetVisible( true );
	}

int CTreeListControl::EnumNode( _In_ const CTreeListItem* const item ) const {
	
	if ( ( item->m_child_info.m_child_info_ptr != nullptr ) && ( item->m_child_info.m_child_info_ptr->m_childCount > 0 ) ) {
		//ASSERT( ( item->m_child_info->m_childCount > 0 ) == ( item->m_childCount > 0 ) );
		ASSERT( item->m_child_info.m_child_info_ptr != nullptr );
		ASSERT( item->m_child_info.m_child_info_ptr->m_childCount > 0 );

		if ( item->HasSiblings( ) ) {
			if ( item->IsExpanded( ) ) {
				return static_cast<int>( ENUM_NODE::NODE_MINUS_SIBLING );
				}
			return static_cast<int>( ENUM_NODE::NODE_PLUS_SIBLING );
			}
		if ( item->IsExpanded( ) ) {
			return static_cast<int>( ENUM_NODE::NODE_MINUS_END );
			}
		return static_cast<int>( ENUM_NODE::NODE_PLUS_END );
		}
	if ( item->HasSiblings( ) ) {
		return static_cast<int>( ENUM_NODE::NODE_SIBLING );
		}
	return static_cast<int>( ENUM_NODE::NODE_END );
	}

RECT CTreeListControl::DrawNode_Indented( _In_ const CTreeListItem* const item, _In_ CDC& pdc, _Inout_ RECT& rc, _Inout_ RECT& rcRest ) const {
	//bool didBitBlt = false;
	RECT rcPlusMinus;
	rcRest.left += 3;
	CDC dcmem;
	VERIFY( dcmem.CreateCompatibleDC( &pdc ) );
	CSelectObject sonodes( dcmem, ( IsItemStripeColor( item ) ? m_bmNodes1 : m_bmNodes0 ) );
	const auto ysrc = ( NODE_HEIGHT / 2 ) - ( m_rowHeight / 2 );
	const bool didBitBlt = DrawNodeNullWidth( item, pdc, rcRest, dcmem, ysrc );
	rcRest.left += ( item->GetIndent( ) - 1 ) * INDENT_WIDTH;
	const auto node = EnumNode( item );
	ASSERT_VALID( &dcmem );
	if ( !didBitBlt ) {//Else we'd double BitBlt?
		VERIFY( pdc.BitBlt( static_cast<int>( rcRest.left ), static_cast<int>( rcRest.top ), static_cast<int>( NODE_WIDTH ), static_cast<int>( NODE_HEIGHT ), &dcmem, ( NODE_WIDTH * node ), static_cast<int>( ysrc ), SRCCOPY ) );
		}
	rcPlusMinus.left    = rcRest.left      + HOTNODE_X;
	rcPlusMinus.right   = rcPlusMinus.left + HOTNODE_CX;
	rcPlusMinus.top     = rcRest.top       + ( rcRest.bottom - rcRest.top )/ 2 - HOTNODE_CY / 2 - 1;
	rcPlusMinus.bottom  = rcPlusMinus.top  + HOTNODE_CY;
			
	rcRest.left += NODE_WIDTH;
	//VERIFY( dcmem.DeleteDC( ) );
	rc.right = rcRest.left;
	return rcPlusMinus;
	}

RECT CTreeListControl::DrawNode( _In_ const CTreeListItem* const item, _In_ CDC& pdc, _Inout_ RECT& rc ) const {
	//ASSERT_VALID( pdc );
	RECT rcRest = rc;
	
	rcRest.left += GENERAL_INDENT;
	if ( item->GetIndent( ) > 0 ) {
		return DrawNode_Indented( item, pdc, rc, rcRest );
		}
	RECT rcPlusMinus;
	rcPlusMinus.bottom = 0;
	rcPlusMinus.left   = 0;
	rcPlusMinus.right  = 0;
	rcPlusMinus.top    = 0;
	rc.right = rcRest.left;
	return rcPlusMinus;
	}

void CTreeListControl::OnLButtonDown( UINT nFlags, CPoint point ) {
	m_lButtonDownItem = -1;

	auto hti = zero_init_struct<LVHITTESTINFO>( );

	hti.pt = point;

	const auto i = HitTest( &hti );
	if ( i == -1 ) {
		return;
		}

	if ( hti.iSubItem != 0 ) {
		CWnd::OnLButtonDown( nFlags, point );
		return;
		}
	const auto thisHeader = CListCtrl::GetHeaderCtrl( );
	const auto rc = GetWholeSubitemRect( i, 0, thisHeader );

	const POINT temp = { rc.left, rc.top };
	//given temp  == {   0, 84 };
	//given point == { 200, 89 };
	//pt == { 200, 5 };

	WTL::CPoint pt = ( point - temp );

	const auto item = GetItem( i );

	m_lButtonDownItem = i;
	
	ASSERT( item != NULL );
	if ( item == NULL ) {
		return;
		}

	const RECT plus_minus_rect = item->GetPlusMinusRect( );
	//if ( CRect( item->GetPlusMinusRect( ) ).PtInRect( pt ) ) {
	if ( ::PtInRect( &plus_minus_rect, pt ) ) {
		m_lButtonDownOnPlusMinusRect = true;
		ToggleExpansion( i );
		return;
		}
	m_lButtonDownOnPlusMinusRect = false;
	CWnd::OnLButtonDown( nFlags, point );
	}

void CTreeListControl::OnLButtonDblClk( UINT nFlags, CPoint point ) {
	CWnd::OnLButtonDblClk( nFlags, point );

	if ( m_lButtonDownItem == -1 ) {
		return;
		}

	if ( m_lButtonDownOnPlusMinusRect ) {
		ToggleExpansion( m_lButtonDownItem );
		}
	else {
		OnItemDoubleClick( m_lButtonDownItem );
		}
	}

void CTreeListControl::ToggleExpansion( _In_ _In_range_( 0, INT_MAX ) const INT i ) {
	const auto item_at_i = GetItem( i );
	ASSERT( item_at_i != NULL );
	if ( item_at_i == NULL ) {
		TRACE( _T( "Can't toggle the expansion of a NULL item!! ( item # %i )\r\n" ), i );
		return;
		}
	//SetRedraw( FALSE );
	if ( item_at_i->IsExpanded( ) ) {
		VERIFY( CollapseItem( i ) );
		//SetRedraw( TRUE );
		return;
		}
	//ExpandItem( i, true );
	ExpandItemAndScroll( i );
	//SetRedraw( TRUE );
	}

int CTreeListControl::countItemsToDelete( _In_ const CTreeListItem* const item, bool& selectNode, _In_ _In_range_( 0, INT_MAX ) const int& i ) {
	int todelete = 0;
	const auto itemCount = CListCtrl::GetItemCount( );
	const auto count_to_start_at = ( i + 1 );
	for ( int k = count_to_start_at; k < itemCount; k++ ) {
		const auto child = GetItem( k );
		ASSERT( child != NULL );
		if ( child != NULL ) {
			if ( child->GetIndent( ) <= item->GetIndent( ) ) {
				break;
				}
			}
		if ( CListCtrl::GetItemState( k, LVIS_SELECTED ) == LVIS_SELECTED ) {
			selectNode = true;
			}
		todelete++;
		}
	TRACE( _T( "Need to delete %i items from %s\r\n" ), todelete, item->m_name );
	return todelete;
	}

_Success_( return == true ) bool CTreeListControl::CollapseItem( _In_ _In_range_( 0, INT_MAX ) const int i ) {
	auto const item = GetItem( i );
	ASSERT( item != NULL );
	if ( item == NULL ) {
		TRACE( _T( "Can't collapse a NULL item!\r\n" ) );
		return false;
		}
	if ( !item->IsExpanded( ) ) {
		TRACE( _T( "ERROR: Collapsing item %i: %s...it's not expanded!\r\n" ), i, item->m_name );
		return false;
		}
	TRACE( _T( "Collapsing item %i: %s\r\n" ), i, item->m_name );
	//WTL::CWaitCursor wc;
	CWnd::SetRedraw( FALSE );
	
	bool selectNode = false;
	const auto item_number_to_delete = ( i + 1 );
	const auto todelete = countItemsToDelete( item, selectNode, i );
	for ( INT m = 0; m < todelete; m++ ) {
		
#ifdef DEBUG
		const auto local_var = GetItem( item_number_to_delete );
		ASSERT( local_var != NULL );
		if ( local_var != NULL ) {
			ASSERT( item_number_to_delete == FindListItem( local_var ) );
			TRACE( _T( "deleting item %i (%i/%i), %s\r\n" ), ( item_number_to_delete ), m, todelete, local_var->m_name );
			}
		else {
			TRACE( _T( "deleting item %i (%i/%i), %s\r\n" ), ( item_number_to_delete ), m, todelete, L"ERROR: NULL POINTER!" );
			}
		ASSERT( local_var->GetIndent( ) > item->GetIndent( ) );
#endif
		DeleteItem( item_number_to_delete );
		}
	item->SetExpanded( false );
	if ( selectNode ) {
		SelectItem( i );
		}

	CWnd::SetRedraw( TRUE );
	const auto item_count = CListCtrl::GetItemCount( );
#ifdef DEBUG
	const auto local_var = GetItem( i );
	TRACE( _T( "Redrawing items %i (`%s`) to %i....\r\n" ), i, ( ( local_var != NULL ) ? local_var->m_name : L"" ), ( item_count ) );

#endif
	VERIFY( CListCtrl::RedrawItems( i, item_count ) );
	//VERIFY( RedrawItems( i, item_count + 1 ) );
	//VERIFY( RedrawItems( 0, item_count + 1 ) );
	TRACE( _T( "Collapsing item succeeded!\r\n" ) );
	//GetDocument( )->UpdateAllViews( NULL, UpdateAllViews_ENUM::HINT_NULL );
	GetDocument( )->UpdateAllViews( NULL, UpdateAllViews_ENUM::HINT_SHOWNEWSELECTION );
	return true;
	}

bool CTreeListControl::SelectedItemCanToggle( ) const {
	const auto i = GetSelectedItem( );
	if ( i == -1 ) {
		return false;
		}
	const auto item = GetItem( i );
	ASSERT( item != NULL );
	if ( ( item != NULL ) && ( item->m_child_info.m_child_info_ptr != nullptr ) ) {
		//ASSERT( ( item->m_childCount > 0 ) == ( item->m_child_info->m_childCount > 0 ) );
		return ( item->m_child_info.m_child_info_ptr->m_childCount > 0 );
		}
	
	return false;
	}

void CTreeListControl::ToggleSelectedItem( ) {
	const auto i = GetSelectedItem( );
	ASSERT( i != -1 );
	if ( i != -1 ) {
		ToggleExpansion( i );
		}
	}

void CTreeListControl::insertItemsAdjustWidths( _In_ const CTreeListItem* const item, _In_ _In_range_( 1, SIZE_T_MAX ) const size_t count, _Inout_ _Out_range_( 0, INT_MAX ) INT& maxwidth, _In_ const bool scroll, _In_ _In_range_( 0, INT_MAX ) const INT_PTR i ) {
	if ( count == 0 ) {
		return;
		}

	ASSERT( count >= 1 );

	//Not vectorized: 1304, loop includes assignments of different sizes
	for ( size_t c = 0; c < count; c++ ) {
		//ASSERT( count == item->m_childCount );
		const auto child = item->GetSortedChild( c );//m_vi->cache_sortedChildren[i];
		ASSERT( child != NULL );
		if ( child != NULL ) {
			InsertItem( child, i + static_cast<INT_PTR>( 1 ) + static_cast<INT_PTR>( c ) );
			if ( scroll ) {
				const auto w = GetSubItemWidth( child, column::COL_NAME );
				if ( w > maxwidth ) {
					ASSERT( w >= 0 );
					if ( w >= 0 ) {
						maxwidth = w;
						}
					}
				}
			}
		}
	ASSERT( maxwidth >= 0 );
	if ( maxwidth < 0 ) {
		maxwidth = 0;
		}
	}

void CTreeListControl::OnItemDoubleClick ( _In_ _In_range_( 0, INT_MAX ) const int i ) {
	const auto item = static_cast< const CTreeListItem* >( GetItem( i ) );
	if ( item != NULL ) {
		//if ( item->m_type == IT_FILE ) {
		if ( item->m_child_info.m_child_info_ptr == nullptr ) {
			TRACE( _T( "User double-clicked %s in TreeListControl! Opening Item!\r\n" ), item->GetPath( ).c_str( ) );
			ASSERT( m_pDocument == GetDocument( ) );
			ASSERT( m_pDocument != NULL );
			return m_pDocument->OpenItem( *item );
			}
		TRACE( _T( "User double-clicked %s in TreeListControl - it's not a file, so I'll toggle expansion for that item.\r\n" ), item->GetPath( ).c_str( ) );
		}
	ASSERT( item != NULL );
	ToggleExpansion( i );
	}

void CTreeListControl::ExpandItemInsertChildren( _In_ const CTreeListItem* const item, _In_ _In_range_( 0, INT32_MAX ) const INT_PTR i, _In_ const bool scroll ) {
	static_assert( column::COL_NAME == 0, "GetSubItemWidth used to accept an INT as the second parameter. The value of zero, I believe, should be COL_NAME" );
	//static_assert( COL_NAME__ == 0,       "GetSubItemWidth used to accept an INT as the second parameter. The value of zero, I believe, should be COL_NAME" );
	auto maxwidth = GetSubItemWidth( item, column::COL_NAME );

	ASSERT( maxwidth == ( CListCtrl::GetStringWidth( item->m_name ) + 10 ) );

	if ( item->m_child_info.m_child_info_ptr == nullptr ) {
		TRACE( _T( "item `%s` has a child count of ZERO! Not expanding! \r\n" ), item->m_name );
		return;
		}

	ASSERT( item->m_child_info.m_child_info_ptr != nullptr );
	//ASSERT( item->m_childCount == item->m_child_info->m_childCount );
	
	const auto count = item->m_child_info.m_child_info_ptr->m_childCount;
	if ( count == 0 ) {
		TRACE( _T( "item `%s` has a child count of ZERO! Not expanding! \r\n" ), item->m_name );
		return;
		}
	const auto myCount  = static_cast<size_t>( CListCtrl::GetItemCount( ) );
	TRACE( _T( "Expanding %s! Must insert %i items!\r\n" ), item->m_name, count );
	SetItemCount( static_cast<INT>( ( count >= myCount) ? ( count + 1 ) : ( myCount + 1 ) ) );
	
	insertItemsAdjustWidths( item, count, maxwidth, scroll, i );
	
	if ( scroll && ( CListCtrl::GetColumnWidth( static_cast<int>( column::COL_NAME ) ) < maxwidth ) ) {
		VERIFY( CListCtrl::SetColumnWidth( static_cast<int>( column::COL_NAME ), maxwidth ) );
		}
	}

void CTreeListControl::ExpandItem( _In_ _In_range_( 0, INT_MAX ) const int i, _In_ const bool scroll ) {
	auto const item = GetItem( i );
	ASSERT( item != NULL );
	if ( item == NULL ) {
		TRACE( _T( "Can expand a NULL item!! ( item # %i, scroll: %s )\r\n" ), i, ( scroll ? L"YES" : L"NO" ) );
		return;
		}
	if ( item->IsExpanded( ) ) {
		TRACE( _T( "Item is already expanded!! ( item # %i, scroll: %s )\r\n" ), i, ( scroll ? L"YES" : L"NO" ) );
		return;
		}

	WTL::CWaitCursor wc; // TODO: smart WaitCursor. In CollapseItem(), too.

#ifdef DEBUG
	const auto qpf = ( static_cast<DOUBLE>( 1 ) / static_cast<DOUBLE>( help_QueryPerformanceFrequency( ).QuadPart ) );
	const auto qpc_1 = help_QueryPerformanceCounter( );
#endif

	item->SortChildren( this );

	ExpandItemInsertChildren( item, i, scroll );

	item->SetExpanded( true );

#ifdef DEBUG
	const auto qpc_2 = help_QueryPerformanceCounter( );
	const auto timing = ( qpc_2.QuadPart - qpc_1.QuadPart ) * qpf;
	TRACE( _T( "Inserting items ( expansion ) took %f!\r\n" ), timing );
#endif

	item->SortChildren( this );
	VERIFY( RedrawItems( i, i ) );

#ifdef DEBUG
	const auto qpc_3 = help_QueryPerformanceCounter( );
	const auto timing_2 = ( qpc_3.QuadPart - qpc_2.QuadPart ) * qpf;
	TRACE( _T( "Inserting items (sort/redraw) took %f!\r\n" ), timing_2 );
#endif

	if ( scroll ) {
		// Scroll up so far, that i is still visible and the first child becomes visible, if possible.
		VERIFY( CListCtrl::EnsureVisible( i, false ) );
		}
	}

void CTreeListControl::handle_VK_LEFT( _In_ const CTreeListItem* const item, _In_ _In_range_( 0, INT32_MAX ) const int i ) {
	if ( item->IsExpanded( ) ) {
		VERIFY( CollapseItem( i ) );
		}
	else if ( item->m_parent != NULL ) {
		SelectItem( item->m_parent );
		}
	}

void CTreeListControl::handle_VK_RIGHT( _In_ const CTreeListItem* const item, _In_ _In_range_( 0, INT_MAX ) const int i ) {
	
	//ASSERT( ( item->m_childCount > 0 ) == ( item->m_child_info->m_childCount > 0 ) );
	//ASSERT( item->m_childCount == item->m_child_info->m_childCount );

	if ( !item->IsExpanded( ) ) {
		//ExpandItem( i, true );
		CWnd::SetRedraw( FALSE );
		ExpandItemAndScroll( i );
		CWnd::SetRedraw( TRUE );
		}
	else if ( item->m_child_info.m_child_info_ptr != nullptr ) {

		const auto sortedItemAtZero = item->GetSortedChild( 0 );
		if ( sortedItemAtZero != NULL ){
			SelectItem( sortedItemAtZero );
			}
		}
	}

void CTreeListControl::handle_VK_TAB( const UINT nChar, const UINT nRepCnt, const UINT nFlags ) {
	if ( m_frameptr->GetTypeView( ) != NULL ) {
		TRACE( _T( "TAB pressed! Focusing on extension list!\r\n" ) );
		m_frameptr->MoveFocus( LOGICAL_FOCUS::LF_EXTENSIONLIST );
		return CWnd::OnKeyDown( nChar, nRepCnt, nFlags );
		}
	TRACE( _T( "TAB pressed! No extension list! Setting Null focus!\r\n" ) );
	m_frameptr->MoveFocus( LOGICAL_FOCUS::LF_NONE );
	return CWnd::OnKeyDown( nChar, nRepCnt, nFlags );
	}

void CTreeListControl::handle_VK_ESCAPE( const UINT nChar, const UINT nRepCnt, const UINT nFlags ) {
	TRACE( _T( "ESCAPE pressed! Null focus!\r\n" ) );
	m_frameptr->MoveFocus( LOGICAL_FOCUS::LF_NONE );
	return CWnd::OnKeyDown( nChar, nRepCnt, nFlags );
	}

void CTreeListControl::handle_remaining_keys( const UINT nChar, const UINT nRepCnt, const UINT nFlags ) {
	//docs for GetNextItem, nItem: "Index of the item to begin the searching with, or -1 to find the first item that matches the specified flags. The specified item itself is excluded from the search."
	//docs for GetNextItem, return: "The index of the next item if successful, or -1 otherwise."
	const auto i = GetNextItem( -1, LVNI_FOCUSED );
	if ( i == ( -1 ) ) {
		return CWnd::OnKeyDown( nChar, nRepCnt, nFlags );
		}
	const auto item = GetItem( i );
	ASSERT( item != NULL );
	if ( item == NULL ) {
		return CWnd::OnKeyDown( nChar, nRepCnt, nFlags );
		}
	if ( nChar == VK_LEFT ) {
		return handle_VK_LEFT( item, i );
		}
	if ( nChar == VK_RIGHT ) {
		return handle_VK_RIGHT( item, i );
		}

	WDS_ASSERT_NEVER_REACHED( );
	return CWnd::OnKeyDown( nChar, nRepCnt, nFlags );
	}

void CTreeListControl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) {
	if ( nChar == VK_TAB ) {
		return handle_VK_TAB( nChar, nRepCnt, nFlags );
		}
	if ( nChar == VK_ESCAPE ) {
		return handle_VK_ESCAPE( nChar, nRepCnt, nFlags );
		}
	if ( ( nChar != VK_LEFT ) && ( nChar != VK_RIGHT ) ) {
		return CWnd::OnKeyDown( nChar, nRepCnt, nFlags );
		}

	return handle_remaining_keys( nChar, nRepCnt, nFlags );
	}

_Pre_satisfies_( !isDone ) void CTreeListControl::OnChildAdded( _In_opt_ const CTreeListItem* const parent, _In_ CTreeListItem* const child, _In_ const bool isDone ) {
	if ( parent == NULL ) {
		ASSERT( m_pDocument == GetDocument( ) );
		if ( m_pDocument == GetDocument( ) ) {
			ASSERT( m_pDocument->m_rootItem.get( ) == child );
			}
		
		SetRootItem( child );
		return;
		}

	if ( !parent->IsVisible( ) ) {
		return;
		}
	//const auto p = FindTreeItem( parent );
	const auto p = FindListItem( parent );
	ASSERT( p != -1 );

	if ( parent->IsExpanded( ) ) {
		InsertItem( child, p + 1 );
		if ( isDone ) {
			VERIFY( CListCtrl::RedrawItems( p, p ) );
			Sort( );
			}
		}
	else {
		VERIFY( CListCtrl::RedrawItems( p, p ) );
		}
	}

void CTreeListControl::Sort( ) {
	const auto countItems = CListCtrl::GetItemCount( );
	//Not vectorized: 1200, loop contains data dependencies
	for ( int i = 0; i < countItems; i++ ) {//convert to ranged for?
		const auto Item = GetItem( i );
		ASSERT( Item != NULL );
		if ( Item == NULL ) {
			continue;
			}
		if ( Item->IsExpanded( ) ) {
			Item->SortChildren( this );
			}
		}
	COwnerDrawnListCtrl::SortItems( );
	}

void CTreeListControl::EnsureItemVisible( _In_ const CTreeListItem* const item ) {
	ASSERT( item != NULL );
	if ( item == NULL ) {
		return;
		}
	//const auto i = FindTreeItem( item );
	const auto i = FindListItem( item );
	ASSERT( i != ( -1 ) );
	if ( i == -1 ) {
		return;
		}
	VERIFY( CListCtrl::EnsureVisible( i, false ) );
	}

INT __cdecl CItem_compareBySize( _In_ _Points_to_data_ const void* const p1, _In_ _Points_to_data_ const void* const p2 ) {
	const auto size1 = ( *( reinterpret_cast< const CTreeListItem * const* const >( p1 ) ) )->size_recurse( );
	const auto size2 = ( *( reinterpret_cast< const CTreeListItem * const* const >( p2 ) ) )->size_recurse( );
	return signum( static_cast<std::int64_t>( size2 ) - static_cast<std::int64_t>( size1 ) ); // biggest first// TODO: Use 2nd sort column (as set in our TreeListView?)
	}

//_At_( return, _Writable_bytes_( bytes_allocated ) )
//_Ret_notnull_ children_heap_block_allocation* allocate_enough_memory_for_children_block( _In_ const std::uint32_t number_of_children, _Out_ size_t& bytes_allocated ) {
//	const rsize_t base_memory_size_in_bytes = ( sizeof( decltype( children_heap_block_allocation::m_childCount ) ) + sizeof( Children_String_Heap_Manager ) );
//	
//	
//	const rsize_t size_of_a_single_child_in_bytes = sizeof( CTreeListItem );
//	const size_t size_of_children_needed_in_bytes = ( size_of_a_single_child_in_bytes * static_cast<size_t>( number_of_children + 1 ) );
//
//	const size_t total_size_needed = ( base_memory_size_in_bytes + size_of_children_needed_in_bytes );
//	void* const memory_block = malloc( total_size_needed );
//	if ( memory_block == NULL ) {
//		displayWindowsMsgBoxWithMessage( L"can't allocate enough memory for children block! (aborting)" );
//		std::terminate( );
//
//		//shut analyze up.
//		abort( );
//		}
//	bytes_allocated = total_size_needed;
//	children_heap_block_allocation* const new_block = static_cast< children_heap_block_allocation* const>( memory_block );
//	new_block->m_childCount = number_of_children;
//	return new_block;
//	}


#else

#endif