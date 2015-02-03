// TreeListControl.cpp	- Implementation of CTreeListItem and CTreeListControl
//
// WinDirStat - Directory Statistics
// Copyright (C) 2003-2004 Bernhard Seifert
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// Author: bseifert@users.sourceforge.net, bseifert@daccord.net
//
// Last modified: $Date$

#include "stdafx.h"


//encourage inter-procedural optimization (and class-hierarchy analysis!)
#include "ownerdrawnlistcontrol.h"
#include "TreeListControl.h"
#include "item.h"
#include "typeview.h"
#include "SelectDrivesDlg.h"


#include "globalhelpers.h"
#include "windirstat.h"
#include "mainframe.h"
#include "dirstatdoc.h"



namespace
{

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



	//int __cdecl _compareProc_orig( const void* const p1, const void* const p2 ) {
	//	const auto item1 = * ( reinterpret_cast< const CTreeListItem* const* >( p1 ) );
	//	const auto item2 = * ( reinterpret_cast< const CTreeListItem* const* >( p2 ) );
	//	return item1->CompareS( item2, CTreeListItem::GetTreeListControl( )->m_sorting );
	//	}

}

struct compare_CTreeListItems {
	compare_CTreeListItems( const CTreeListControl* const ctrl_in ) : ctrl( ctrl_in ) { }
	compare_CTreeListItems& operator=( compare_CTreeListItems& in ) = delete;
	bool operator()( const CTreeListItem* const lhs, const CTreeListItem* const rhs ) {
		const auto result = ( lhs->CompareS( rhs, ctrl->m_sorting ) < 0 );
		return result;
		}
	const CTreeListControl* const ctrl;
	};

//CRect rc is NOT const here so that other virtual functions may modify it?
bool CTreeListItem::DrawSubitem( RANGE_ENUM_COL const column::ENUM_COL subitem, _In_ CDC& pdc, _In_ RECT rc, _In_ const UINT state, _Out_opt_ INT* const width, _Inout_ INT* const focusLeft, _In_ const COwnerDrawnListCtrl* const list ) const {
	//ASSERT_VALID( pdc );
	ASSERT( ( focusLeft != NULL ) && ( subitem >= 0 ) );

	//if ( subitem != 0 ) {
	if ( subitem != column::COL_NAME ) {
		if ( width != NULL ) {
#ifdef DEBUG
			_CrtDbgBreak( );
#endif
			//Should never happen?
			*width = ( rc.right - rc.left );
			}
		return false;
		}

	RECT rcNode = rc;
	if ( width != NULL ) {
		*width = ( rc.right - rc.left );
		}
	//const auto tree_list_control = GetTreeListControl( );
	//tree_list_control->DrawNode( this, pdc, rcNode, rcPlusMinus );//pass subitem to drawNode?
	static_cast<const CTreeListControl* const>( list )->DrawNode( this, pdc, rcNode );//pass subitem to drawNode?
	
	RECT rcLabel = rc;
	rcLabel.left = rcNode.right;
	DrawLabel( list, pdc, rcLabel, state, width, focusLeft, false );
	if ( width != NULL ) {
		*width = ( rcLabel.right - rcLabel.left );
		}
	else {
		
		const POINT rc_top_left = { rc.left, rc.top };
		const RECT _rcPlusMinus = { 0, 0, 0, 0 };

		//const CRect rcLabel_( rcLabel );
		RECT temp_rect = _rcPlusMinus;
		VERIFY( ::OffsetRect( &temp_rect, -( rc_top_left.x ), -( rc_top_left.y ) ) );
		const RECT& new_plus_minus_rect = temp_rect;

		SetPlusMinusRect( new_plus_minus_rect );

		RECT new_title_rect = rcLabel;
		VERIFY( ::OffsetRect( &new_title_rect, -( rc_top_left.x ), -( rc_top_left.y ) ) );
		SetTitleRect( new_title_rect );
		}
	return true;
	}

void CTreeListItem::childNotNull( _In_ CItemBranch* const aTreeListChild, const size_t i ) {
	
	//ASSERT( m_vi->cache_sortedChildren.at( i )->GetText( column::COL_NAME ).compare( aTreeListChild->GetText( column::COL_NAME ) ) == 0 );
	if ( ( i > m_vi->cache_sortedChildren.size( ) ) /*&& ( i > 0 )*/ ) {
		m_vi->cache_sortedChildren.resize( i + 1u );
		//_CrtDbgBreak( );
		}
	else if ( i == m_vi->cache_sortedChildren.size( ) ) {
		m_vi->cache_sortedChildren.emplace_back( aTreeListChild );
		}
	else {
		ASSERT( i < m_vi->cache_sortedChildren.size( ) );
		//TRACE( _T( "m_vi->cache_sortedChildren.at( i ): %s\r\n" ), m_vi->cache_sortedChildren.at( i )->GetText( column::COL_NAME ).c_str( ) );
		//TRACE( _T( "aTreeListChild: %s\r\n" ), aTreeListChild->GetText( column::COL_NAME ).c_str( ) );
		ASSERT( m_vi->cache_sortedChildren.at( i ) == aTreeListChild );
		//ASSERT( m_vi->cache_sortedChildren.at( i )->GetText( column::COL_NAME ).compare( aTreeListChild->GetText( column::COL_NAME ) ) == 0 );
		ASSERT( wcscmp( m_vi->cache_sortedChildren.at( i )->m_name.get( ), aTreeListChild->m_name.get( ) ) == 0u );
		m_vi->cache_sortedChildren.at( i ) = aTreeListChild;
		}
	}

_Pre_satisfies_( this->m_vi._Myptr != nullptr )
void CTreeListItem::SortChildren( _In_ const CTreeListControl* const ctrl ) {
	ASSERT( IsVisible( ) );

	const auto thisBranch = static_cast<const CItemBranch* >( this );

	m_vi->cache_sortedChildren = thisBranch->size_sorted_vector_of_children( );

	if ( !m_vi->cache_sortedChildren.empty( ) ) {
		//qsort( m_vi->cache_sortedChildren.data( ), m_vi->cache_sortedChildren.size( ) -1, sizeof( CTreeListItem * ), &_compareProc_orig );
		//compare_CTreeListItems comp_functor( GetTreeListControl( ) );
		//std::sort( m_vi->cache_sortedChildren.begin( ), m_vi->cache_sortedChildren.end( ), &_compareProc2 );
		std::sort( m_vi->cache_sortedChildren.begin( ), m_vi->cache_sortedChildren.end( ), compare_CTreeListItems( ctrl ) );
		}
	}

std::uint64_t CTreeListItem::size_recurse_( ) const {
	static_assert( std::is_same<decltype( std::declval<CTreeListItem>( ).size_recurse_( ) ), decltype( std::declval<CItemBranch>( ).size_recurse( ) )>::value , "The return type of CTreeListItem::size_recurse_ needs to be fixed!!" );
	return static_cast< const CItemBranch* >( this )->size_recurse( );
	}

_Ret_range_( 0, UINT32_MAX ) 
std::uint32_t CTreeListItem::GetChildrenCount_( ) const {
	static_assert( std::is_same<decltype( std::declval<CTreeListItem>( ).GetChildrenCount_( ) ), decltype( std::declval<CItemBranch>( ).m_childCount )>::value , "The return type of CTreeListItem::GetChildrenCount_ needs to be fixed!!" );
	return static_cast< const CItemBranch* >( this )->m_childCount;
	}

_Ret_maybenull_
CItemBranch* CTreeListItem::children_ptr( ) const {
	return static_cast< const CItemBranch* >( this )->m_children.get( );
	}

_Success_( return != NULL ) _Must_inspect_result_ _Ret_maybenull_
CTreeListItem* CTreeListItem::GetSortedChild( _In_ const size_t i ) const {
	ASSERT( m_vi != nullptr );
	if ( m_vi != nullptr ) {
		if ( !( m_vi->cache_sortedChildren.empty( ) ) ) {
			return m_vi->cache_sortedChildren.at( i );
			}
		}
	return NULL;
	}

INT CTreeListItem::Compare( _In_ const COwnerDrawnListItem* const baseOther, RANGE_ENUM_COL const column::ENUM_COL subitem ) const {
	const auto other = static_cast<const CTreeListItem *>( baseOther );
	if ( other == NULL ) {
		ASSERT( false );
		displayWindowsMsgBoxWithMessage( L"CTreeListItem::Compare passed a NULL `other`! This should never happen!" );
		std::terminate( );
		return 666;
		}
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
		const auto thisBranch = static_cast< const CItemBranch* >( this );//ugly, I know
		return thisBranch->CompareSibling( other, subitem );
		}
	const auto my_indent = GetIndent( );
	const auto other_indent = other->GetIndent( );
	if ( my_indent < other_indent ) {
		return Compare( other->m_parent, subitem );
		}
	else if ( my_indent > other_indent ) {
		return m_parent->Compare( other, subitem );
		}
	return m_parent->Compare( other->m_parent, subitem );
	}

_Success_( return < child_count )
size_t CTreeListItem::FindSortedChild( _In_ const CTreeListItem* const child, _In_ const size_t child_count ) const {
	ASSERT( child_count > 0u );
	ASSERT( child_count == GetChildrenCount_( ) );
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
	const auto count = m_parent->GetChildrenCount_( );
	if ( count < 2u ) {
		ASSERT( count == 1u );
		return false;
		}
	ASSERT( count >= 2u );
	ASSERT( count > 0 );
	//const auto child_count = GetChildrenCount_( );
	if ( count > 0 ) {
		//do we even need to find it?
#ifdef DEBUG
		//this is OBSCENELY slow in debug builds.
		//const auto i = m_parent->FindSortedChild( this, count );
		//ASSERT( i < count );
#endif
		return true;//return true if `i` is in valid range ( it was found )
		}
	ASSERT( count == 0 );
	return false;
	}

void CTreeListItem::SetVisible( _In_ const bool next_state_visible ) const {
	if ( next_state_visible ) {
		if ( m_vi != nullptr ) {
			m_vi.reset( );
			}
		ASSERT( m_vi == nullptr );
		m_vi.reset( new VISIBLEINFO );
		m_vi->isExpanded = false;
		if ( m_parent == NULL ) {
			m_vi->indent = 0;
			}
		else {
			ASSERT( m_parent != NULL );
			m_vi->indent = m_parent->GetIndent( ) + 1;
			}
		m_vi->isExpanded = false;
		
		//m_vi->sizeCache = UINT64_ERROR;
		//Eww.
		//m_vi->sizeCache = static_cast< const CItemBranch* >( this )->size_recurse( );
		}
	else {
		ASSERT( m_vi != nullptr );
		m_vi.reset( );
		}
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
	ASSERT( w == ( GetStringWidth( item_at_index->m_name.get( ) ) + 15 ) );
	const auto colWidth = GetColumnWidth( static_cast<int>( column::COL_NAME ) );
	if ( colWidth < w ) {
		VERIFY( SetColumnWidth( 0, w + colWidth ) );
		}
	}

void CTreeListControl::expand_item_no_scroll_then_doWhateverJDoes( _In_ const CTreeListItem* const pathZero, _In_range_( 0, INT_MAX ) const int parent ) {
	//auto j = FindTreeItem( pathZero );
	TRACE( _T( "doing whatever j does....\r\n" ) );
	auto j = FindListItem( pathZero );
	if ( j == -1 ) {
		ASSERT( parent >= 0 );
		//ExpandItem( parent, false );
		ExpandItemNoScroll( parent );
		j = FindListItem( pathZero );//TODO: j?
		}
	}

void CTreeListControl::expand_item_then_scroll_to_it( _In_ const CTreeListItem* const pathZero, _In_range_( 0, INT_MAX ) const int index, _In_ const bool showWholePath ) {
	expand_item_no_scroll_then_doWhateverJDoes( pathZero, index );
	ASSERT( index >= -1 );
	//void adjustColumnSize( CTreeListItem* item_at_index )
	const auto item_at_index = GetItem( index );
	ASSERT( item_at_index != NULL );
	if ( item_at_index != NULL ) {
		adjustColumnSize( item_at_index );
		}
	if ( showWholePath ) {
		VERIFY( EnsureVisible( 0, false ) );
		}
	SelectItem( index );
	}

INT CTreeListControl::find_item_then_show_first_try_failed( _In_ const CTreeListItem* const thisPath, const int i ) {
	TRACE( _T( "Searching %s ( this path element ) for next path element...not found! Expanding %I64d...\r\n" ), thisPath->m_name.get( ), i );
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
		TRACE( _T( "Searching %s for next path element...found! path.at( %I64d ), index: %i\r\n" ), thisPath->m_name.get( ), std::int64_t( i ), index );
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
	SetRedraw( FALSE );
	const auto path = buildVectorOfPaths( item );

#ifdef DEBUG
	for ( size_t inner = 0; inner < path.size( ); ++inner ) {
		ASSERT( path.at( inner )->m_name.get( ) != NULL );
		TRACE( _T( "path component %I64u: `%s` (%p)\r\n" ), std::uint64_t( inner ), path.at( inner )->m_name.get( ), path.at( inner ) );
		}
#endif

	//START new algorithm
	auto parent_ptr = item->m_parent;
	size_t steps_from_target = 0;
	
	while ( parent_ptr->m_parent != nullptr ) {
		parent_ptr = parent_ptr->m_parent;
		++steps_from_target;
		}

	const auto root_item = parent_ptr;
	CItemBranch* child = nullptr;

	for ( size_t i = 0; i < root_item->GetChildrenCount_( ); ++i ) {
		if ( &( static_cast< const CItemBranch* >( root_item )->m_children[ i ] ) == ( static_cast< const CItemBranch* >( path.at( steps_from_target ) ) ) ) {
			child = &( static_cast< const CItemBranch* >( root_item )->m_children[ i ] );
			break;
			}
		}

	ASSERT( child != NULL );
	if ( child == NULL ) {
		_CrtDbgBreak( );
		return;
		}
	--steps_from_target;
	do {
		ASSERT( child != NULL );
		if ( child == NULL ) {
			_CrtDbgBreak( );
			return;
			}
		for ( size_t i = 0; i < child->m_childCount; ++i ) {
			if ( &( child->m_children[ i ] ) == ( static_cast< const CItemBranch* >( path.at( steps_from_target ) ) ) ) {
				child = &( child->m_children[ i ] );
				break;
				}
			}


		//child_idx = child->FindSortedChild( path.at( steps_from_target ) );
		//ASSERT( child_idx < child->GetChildrenCount_( ) );
		//child = child->GetSortedChild( child_idx );
		--steps_from_target;
		} while ( ( steps_from_target > 0 ) && ( child != static_cast<const CItemBranch*>( item ) ) );

	TRACE( _T( "halted at: %s\r\n" ), child->m_name.get( ) );

	//END new algorithm

	auto parent = 0;
	for ( auto i = static_cast<std::int64_t>( path.size( ) - 1 ); i >= 0; --i ) {//Iterate downwards, root first, down each matching parent, until we find item
		const auto thisPath = path.at( static_cast<size_t>( i ) );
		if ( thisPath != NULL ) {
			ASSERT( static_cast<std::uint64_t>( i ) < INT_MAX );
			find_item_then_show( thisPath, static_cast<int>( i ), parent, showWholePath, path.at( 0 ) );
			}
		ASSERT( thisPath != NULL );
		}
	SetRedraw( TRUE );
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

void CTreeListControl::DrawNodeNullWidth( _In_ const CTreeListItem* const item, _In_ CDC& pdc, _In_ const RECT& rcRest, _Inout_ bool& didBitBlt, _In_ CDC& dcmem, _In_ const UINT ysrc ) const {
	auto ancestor = item;
	//Not vectorized: 1304, loop includes assignments of different sizes
	for ( auto indent = ( item->GetIndent( ) - 2 ); indent >= 0; indent-- ) {
		if ( ancestor != NULL ) {
			ancestor = ancestor->m_parent;
			if ( ancestor != NULL ) {
				if ( ancestor->HasSiblings( ) ) {
					ASSERT_VALID( &dcmem );
					VERIFY( pdc.BitBlt( ( static_cast<int>( rcRest.left ) + indent * static_cast<int>( INDENT_WIDTH ) ), static_cast<int>( rcRest.top ), static_cast<int>( NODE_WIDTH ), static_cast<int>( NODE_HEIGHT ), &dcmem, ( NODE_WIDTH * static_cast<int>( ENUM_NODE::NODE_LINE ) ), static_cast<int>( ysrc ), SRCCOPY ) );
					didBitBlt = true;
					}
				}
			}
		}
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
	auto pos = GetFirstSelectedItemPosition( );
	if ( pos == NULL ) {
		return -1;
		}
	//`pos` is an _Inout_ here, so it can't be const above.
	return GetNextSelectedItem( pos );
	}
void CTreeListControl::handle_OnContextMenu( CPoint pt ) const {
	const auto i = GetSelectedItem( );
	if ( i == -1 ) {
		TRACE( _T( "OnContextMenu failed to get a valid selected item! returning early....\r\n" ) );
		return;
		}

	const auto item = GetItem( i );
	const auto thisHeader = GetHeaderCtrl( );
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
	PrepareDefaultMenu( static_cast<CItemBranch*>( item ), sub );

	const RECT& rcTitle = trect;
	// Show pop-up menu and act accordingly. The menu shall not overlap the label but appear horizontally at the cursor position,  vertically under (or above) the label.
	// TrackPopupMenuEx() behaves in the desired way, if we exclude the label rectangle extended to full screen width.

	TPMPARAMS tp;
	tp.cbSize = sizeof( tp );
	tp.rcExclude = rcTitle;
	ClientToScreen( &tp.rcExclude );

	RECT desktop;
	GetDesktopWindow( )->GetWindowRect( &desktop );

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
	VERIFY( SetItemState( i, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED ) );
	VERIFY( EnsureVisible( i, false ) );
	}

void CTreeListControl::PrepareDefaultMenu( _In_ const CItemBranch* const item, _Out_ CMenu* const menu ) const {
	//if ( item->m_type == IT_FILE ) {
	if ( item->m_children == nullptr ) {
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
	ASSERT( GetMainFrame( ) == m_frameptr );
	m_frameptr->SetLogicalFocus( LOGICAL_FOCUS::LF_DIRECTORYLIST );
	}

void CTreeListControl::SysColorChanged( ) {
	InitializeColors( );
	InitializeNodeBitmaps( );
	}

BOOL CTreeListControl::CreateEx( _In_ const DWORD dwExStyle, _In_ DWORD dwStyle, _In_ const RECT& rect, _In_ CWnd* pParentWnd, _In_ const UINT nID ) {
	InitializeNodeBitmaps( );

	dwStyle or_eq ( LVS_OWNERDRAWFIXED bitor LVS_SINGLESEL );
	VERIFY( COwnerDrawnListCtrl::CreateEx( dwExStyle, dwStyle, rect, pParentWnd, nID ) );
	return true;
	}


_Must_inspect_result_ _Success_( return != NULL ) _Ret_maybenull_
CTreeListItem* CTreeListControl::GetItem( _In_ _In_range_( 0, INT_MAX ) const int i ) const {
	ASSERT( i < GetItemCount( ) );
	const auto itemCount = GetItemCount( );
	if ( i < itemCount ) {
		return reinterpret_cast< CTreeListItem* >( GetItemData( i ) );
		}
	return NULL;
	}

void CTreeListControl::SetRootItem( _In_opt_ const CTreeListItem* const root ) {
	VERIFY( DeleteAllItems( ) );
	if ( root != NULL ) {
		SetRedraw( FALSE );
		InsertItem( root, 0 );
		//ExpandItem( static_cast<int>( 0 ), true );//otherwise ambiguous call - is it a NULL pointer?
		ExpandItemAndScroll( 0 );//otherwise ambiguous call - is it a NULL pointer?
		SetRedraw( TRUE );
		}
	}

void CTreeListControl::InsertItem( _In_ const CTreeListItem* const item, _In_ _In_range_( 0, INT32_MAX ) const INT_PTR i ) {
	COwnerDrawnListCtrl::InsertListItem( i, item );
	item->SetVisible( true );
	}

int CTreeListControl::EnumNode( _In_ const CTreeListItem* const item ) const {
	if ( item->GetChildrenCount_( ) > 0 ) {
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

RECT CTreeListControl::DrawNode( _In_ const CTreeListItem* const item, _In_ CDC& pdc, _Inout_ RECT& rc ) const {
	//ASSERT_VALID( pdc );
	RECT rcRest = rc;
	RECT rcPlusMinus;
	bool didBitBlt = false;
	rcRest.left += GENERAL_INDENT;
	if ( item->GetIndent( ) > 0 ) {
		rcRest.left += 3;
		CDC dcmem;
		VERIFY( dcmem.CreateCompatibleDC( &pdc ) );
		CSelectObject sonodes( dcmem, ( IsItemStripeColor( item ) ? m_bmNodes1 : m_bmNodes0 ) );
		auto ysrc = ( NODE_HEIGHT / 2 ) - ( m_rowHeight / 2 );
		DrawNodeNullWidth( item, pdc, rcRest, didBitBlt, dcmem, ysrc );
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
		}
	else {
		rcPlusMinus.bottom = 0;
		rcPlusMinus.left   = 0;
		rcPlusMinus.right  = 0;
		rcPlusMinus.top    = 0;
		}
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
		COwnerDrawnListCtrl::OnLButtonDown( nFlags, point );
		return;
		}
	const auto thisHeader = GetHeaderCtrl( );
	const auto rc = GetWholeSubitemRect( i, 0, thisHeader );

	const POINT temp = { rc.left, rc.top };


	//WTL::CPoint pt = point - rc.TopLeft( );
	WTL::CPoint pt = ( point - temp );

	//WTL::CPoint pt_temp( ( point - temp ) );

	//ASSERT( pt == pt_temp );

	const auto item = GetItem( i );

	m_lButtonDownItem = i;
	if ( item != NULL ) {
		const RECT plus_minus_rect = item->GetPlusMinusRect( );
		//if ( CRect( item->GetPlusMinusRect( ) ).PtInRect( pt ) ) {
		if ( ::PtInRect( &plus_minus_rect, pt ) ) {
			m_lButtonDownOnPlusMinusRect = true;
			ToggleExpansion( i );
			}
		else {
			m_lButtonDownOnPlusMinusRect = false;
			COwnerDrawnListCtrl::OnLButtonDown( nFlags, point );
			}
		}
	ASSERT( item != NULL );
	}

void CTreeListControl::OnLButtonDblClk( UINT nFlags, CPoint point ) {
	COwnerDrawnListCtrl::OnLButtonDblClk( nFlags, point );

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
	if ( item_at_i != NULL ) {
		SetRedraw( FALSE );
		if ( item_at_i->IsExpanded( ) ) {
			VERIFY( CollapseItem( i ) );
			return;
			}
		//ExpandItem( i, true );
		ExpandItemAndScroll( i );
		SetRedraw( TRUE );
		}
	}

int CTreeListControl::countItemsToDelete( _In_ const CTreeListItem* const item, bool& selectNode, _In_ _In_range_( 0, INT_MAX ) const int& i ) {
	int todelete = 0;
	//void countItemsToDelete( bool& selectNode, const INT& i )
	const auto itemCount = GetItemCount( );
	for ( int k = i + 1; k < itemCount; k++ ) {
		const auto child = GetItem( k );
		if ( child != NULL ) {
			if ( child->GetIndent( ) <= item->GetIndent( ) ) {
				break;
				}
			}
		ASSERT( child != NULL );
		if ( GetItemState( k, LVIS_SELECTED ) == LVIS_SELECTED ) {
			selectNode = true;
			}
		todelete++;
		}
	return todelete;
	}

_Success_( return == true ) bool CTreeListControl::CollapseItem( _In_ _In_range_( 0, INT_MAX ) const int i ) {
	auto const item = GetItem( i );
	ASSERT( item != NULL );
	if ( item == NULL ) {
		return false;
		}
	if ( !item->IsExpanded( ) ) {
		TRACE( _T( "ERROR: Collapsing item %i: %s...it's not expanded!\r\n" ), i, item->m_name.get( ) );
		return false;
		}
	TRACE( _T( "Collapsing item %i: %s\r\n" ), i, item->m_name.get( ) );
	WTL::CWaitCursor wc;
	//LockWindowUpdate( );
	SetRedraw( FALSE );
	
	bool selectNode = false;
	auto todelete = countItemsToDelete( item, selectNode, i );
	for ( INT m = 0; m < todelete; m++ ) {
#ifdef DEBUG
		const auto local_var = GetItem( i + 1 );
		if ( local_var != NULL ) {
			TRACE( _T( "deleting item %i (%i/%i), %s\r\n" ), ( i + 1 ), m, todelete, local_var->m_name.get( ) );
			}
		else {
			TRACE( _T( "deleting item %i (%i/%i), %s\r\n" ), ( i + 1 ), m, todelete, L"ERROR: NULL POINTER!" );
			}
#endif
		DeleteItem( i + 1 );
		}
	item->SetExpanded( false );
	if ( selectNode ) {

		SelectItem( i );
		}

	SetRedraw( TRUE );
	//UnlockWindowUpdate( );
	VERIFY( RedrawItems( i, i ) );
	TRACE( _T( "Collapsing item succeeded!\r\n" ) );
	return true;
	}

bool CTreeListControl::SelectedItemCanToggle( ) const {
	const auto i = GetSelectedItem( );
	if ( i == -1 ) {
		return false;
		}
	const auto item = GetItem( i );
	if ( item != NULL ) {
		return ( item->GetChildrenCount_( ) > 0 );
		}
	ASSERT( item != NULL );
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
		ASSERT( count == item->GetChildrenCount_( ) );
		const auto child = item->GetSortedChild( c );//m_vi->cache_sortedChildren[i];
		ASSERT( child != NULL );
		if ( child != NULL ) {
			InsertItem( child, i + static_cast<INT_PTR>( 1 ) + static_cast<INT_PTR>( c ) );
			if ( scroll ) {
				const auto w = GetSubItemWidth( child, column::COL_NAME );
				//const auto predicted_str_width = ( GetStringWidth( child->m_name.get( ) ) + 10 );
				//ASSERT( w == predicted_str_width );
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
	const auto item = static_cast< const CItemBranch* >( GetItem( i ) );
	if ( item != NULL ) {
		//if ( item->m_type == IT_FILE ) {
		if ( item->m_children == nullptr ) {
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
	ASSERT( maxwidth == ( GetStringWidth( item->m_name.get( ) ) + 10 ) );
	const auto count    = item->GetChildrenCount_( );
	const auto myCount  = static_cast<size_t>( GetItemCount( ) );
	TRACE( _T( "Expanding %s! Must insert %i items!\r\n" ), item->m_name.get( ), count );
	SetItemCount( static_cast<INT>( ( count >= myCount) ? ( count + 1 ) : ( myCount + 1 ) ) );
	
	insertItemsAdjustWidths( item, count, maxwidth, scroll, i );
	
	if ( scroll && GetColumnWidth( 0 ) < maxwidth ) {
		VERIFY( SetColumnWidth( 0, maxwidth ) );
		}
	}

void CTreeListControl::ExpandItem( _In_ _In_range_( 0, INT_MAX ) const int i, _In_ const bool scroll ) {
	auto const item = GetItem( i );
	ASSERT( item != NULL );
	if ( item == NULL ) {
		return;
		}
	if ( item->IsExpanded( ) ) {
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
		//if ( item->GetChildrenCount_( ) > 0 ) {
		//	VERIFY( EnsureVisible( i, false ) );
		//	}
		VERIFY( EnsureVisible( i, false ) );
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
	if ( !item->IsExpanded( ) ) {
		//ExpandItem( i, true );
		SetRedraw( FALSE );
		ExpandItemAndScroll( i );
		SetRedraw( TRUE );
		}
	else if ( item->GetChildrenCount_( ) > 0 ) {
		const auto sortedItemAtZero = item->GetSortedChild( 0 );
		if ( sortedItemAtZero != NULL ){
			SelectItem( sortedItemAtZero );
			}
		}
	}


void CTreeListControl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) {
	if ( nChar == VK_TAB ) {
		ASSERT( GetMainFrame( ) == m_frameptr );
		if ( m_frameptr->GetTypeView( ) != NULL ) {
			TRACE( _T( "TAB pressed! Focusing on extension list!\r\n" ) );
			m_frameptr->MoveFocus( LOGICAL_FOCUS::LF_EXTENSIONLIST );
			}
		else {
			TRACE( _T( "TAB pressed! No extension list! Setting Null focus!\r\n" ) );
			m_frameptr->MoveFocus( LOGICAL_FOCUS::LF_NONE );
			}
		}
	else if ( nChar == VK_ESCAPE ) {
		ASSERT( GetMainFrame( ) == m_frameptr );
		TRACE( _T( "ESCAPE pressed! Null focus!\r\n" ) );
		m_frameptr->MoveFocus( LOGICAL_FOCUS::LF_NONE );
		}
	const auto i = GetNextItem( -1, LVNI_FOCUSED );
	if ( i != -1 ) {
		const auto item = GetItem( i );
		if ( item != NULL ) {
			switch ( nChar ) {
				case VK_LEFT:
					handle_VK_LEFT( item, i );
					return;

				case VK_RIGHT:
					handle_VK_RIGHT( item, i );
					return;
				}
			}
		ASSERT( item != NULL );
		}
	COwnerDrawnListCtrl::OnKeyDown( nChar, nRepCnt, nFlags );
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
			VERIFY( RedrawItems( p, p ) );
			Sort( );
			}
		}
	else {
		VERIFY( RedrawItems( p, p ) );
		}
	}

void CTreeListControl::Sort( ) {
	const auto countItems = GetItemCount( );
	//Not vectorized: 1200, loop contains data dependencies
	for ( int i = 0; i < countItems; i++ ) {//convert to ranged for?
		const auto Item = GetItem( i );
		if ( Item != NULL ) {
			if ( Item->IsExpanded( ) ) {
				Item->SortChildren( this );
				}
			}
		ASSERT( Item != NULL );
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
	VERIFY( EnsureVisible( i, false ) );
	}
