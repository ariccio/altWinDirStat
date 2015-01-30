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


//encourage inter-procedural optimization (and class-heirarchy analysis!)
#include "ownerdrawnlistcontrol.h"
#include "TreeListControl.h"
#include "item.h"
#include "typeview.h"


#include "globalhelpers.h"
#include "windirstat.h"
#include "mainframe.h"
#include "dirstatdoc.h"

namespace
{
	// Sequence within IDB_NODES
	enum 
	{
		NODE_PLUS_SIBLING,
		NODE_PLUS_END,
		NODE_MINUS_SIBLING,
		NODE_MINUS_END,
		NODE_SIBLING,
		NODE_END,
		NODE_LINE
	};
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

	int __cdecl _compareProc_orig( const void* const p1, const void* const p2 ) {
		const auto item1 = * ( reinterpret_cast< const CTreeListItem* const* >( p1 ) );
		const auto item2 = * ( reinterpret_cast< const CTreeListItem* const* >( p2 ) );
		return item1->CompareS( item2, CTreeListItem::GetTreeListControl( )->m_sorting );
		}

}

//CTreeListItem::~CTreeListItem( ) {
//	//delete m_vi;
//	//m_vi = { NULL };
//	//m_parent = { NULL };
//	}

//CRect rc is NOT const here so that other virtual functions may modify it?
bool CTreeListItem::DrawSubitem( RANGE_ENUM_COL const column::ENUM_COL subitem, _In_ CDC& pdc, _In_ RECT rc, _In_ const UINT state, _Out_opt_ INT* const width, _Inout_ INT* const focusLeft ) const {
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
	RECT rcPlusMinus;
	if ( width != NULL ) {
		*width = ( rc.right - rc.left );
		}
	const auto tree_list_control = GetTreeListControl( );
	tree_list_control->DrawNode( this, pdc, rcNode, rcPlusMinus );//pass subitem to drawNode?
	
	auto rcLabel = rc;
	rcLabel.left = rcNode.right;
	DrawLabel( tree_list_control, pdc, rcLabel, state, width, focusLeft, false );
	if ( width != NULL ) {
		*width = ( rcLabel.right - rcLabel.left );
		}
	else {
		CRect _rc( rc );
		const CPoint rc_top_left = _rc.TopLeft( );
		const CRect _rcPlusMinus;
		const RECT new_plus_minus_rect = ( _rcPlusMinus - rc_top_left );
		const CRect rcLabel_( rcLabel );


		SetPlusMinusRect( new_plus_minus_rect );

		const RECT new_title_rect = ( rcLabel_ - rc_top_left );
		SetTitleRect( new_title_rect );
		}
	return true;
	}

void CTreeListItem::childNotNull( CItemBranch* const aTreeListChild, const size_t i ) {
	
	//ASSERT( m_vi->cache_sortedChildren.at( i )->GetText( column::COL_NAME ).compare( aTreeListChild->GetText( column::COL_NAME ) ) == 0 );
	if ( ( i > m_vi->cache_sortedChildren.size( ) ) && ( i > 0 ) ) {
		m_vi->cache_sortedChildren.resize( i + 1 );
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
		ASSERT( wcscmp( m_vi->cache_sortedChildren.at( i )->m_name.get( ), aTreeListChild->m_name.get( ) ) == 0 );
		m_vi->cache_sortedChildren.at( i ) = aTreeListChild;
		}
	}

_Pre_satisfies_( this->m_vi._Myptr != nullptr )
void CTreeListItem::SortChildren( ) {
	ASSERT( IsVisible( ) );

	const auto thisBranch = static_cast<const CItemBranch* >( this );

	//auto children_vec = thisBranch->size_sorted_vector_of_children( );	
	//m_vi->cache_sortedChildren = std::move( children_vec );

	m_vi->cache_sortedChildren = thisBranch->size_sorted_vector_of_children( );

	if ( !m_vi->cache_sortedChildren.empty( ) ) {
		//qsort( m_vi->cache_sortedChildren.data( ), m_vi->cache_sortedChildren.size( ) -1, sizeof( CTreeListItem * ), &_compareProc_orig );
		
		std::sort( m_vi->cache_sortedChildren.begin( ), m_vi->cache_sortedChildren.end( ), &_compareProc2 );
		////std::sort( m_vi->cache_sortedChildren.begin( ), m_vi->cache_sortedChildren.end( ), TreeListItemSortStruct( ) );
		//m_vi->cache_sortedChildren.shrink_to_fit( );
		}
	}

bool CTreeListItem::_compareProc2( const CTreeListItem* const lhs, const CTreeListItem* const rhs ) {
	auto result = lhs->CompareS( rhs, GetTreeListControl( )->m_sorting ) < 0;
	return result;
	}

std::uint64_t CTreeListItem::size_recurse_( ) const {
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
	if ( GetIndent( ) < other->GetIndent( ) ) {
		return Compare( other->m_parent, subitem );
		}
	else if ( GetIndent( ) > other->GetIndent( ) ) {
		return m_parent->Compare( other, subitem );
		}
	return m_parent->Compare( other->m_parent, subitem );
	}

size_t CTreeListItem::FindSortedChild( _In_ const CTreeListItem* const child ) const {
	const auto childCount = GetChildrenCount_( );
	ASSERT( childCount > 0 );
	for ( size_t i = 0; i < childCount; i++ ) {
		if ( child == GetSortedChild( i ) ) {
			return i;
			}
		}
	return childCount; 
	}

_Pre_satisfies_( this->m_parent != NULL )
bool CTreeListItem::HasSiblings( ) const {
	if ( m_parent == NULL ) {
		return false;
		}
	const auto count = m_parent->GetChildrenCount_( );
	if ( count < 2 ) {
		ASSERT( count == 1 );
		return false;
		}
	const auto i = m_parent->FindSortedChild( this );
	return ( i <= ( count - 1 ) );//return true if `i` is in valid range
	}

void CTreeListItem::SetVisible( _In_ const bool next_state_visible ) const {
	if ( next_state_visible ) {
		if ( m_vi != nullptr ) {
			//delete m_vi;
			m_vi.reset( );
			//m_vi = new VISIBLEINFO;
			//m_vi = { NULL };
			}
		ASSERT( m_vi == nullptr );
		m_vi.reset( new VISIBLEINFO );
		m_vi->isExpanded = false;
		if ( m_parent == NULL ) {
			m_vi->indent = 0;
			}
		else {
			auto Parent = m_parent;
			if ( Parent != NULL ) {
				m_vi->indent = Parent->GetIndent( ) + 1;
				}
			ASSERT( Parent != NULL );
			}
		m_vi->isExpanded = false;
		
		//m_vi->sizeCache = UINT64_ERROR;
		//Eww.
		//m_vi->sizeCache = static_cast< const CItemBranch* >( this )->size_recurse( );
		}
	else {
		ASSERT( m_vi != nullptr );
		//delete m_vi;
		//m_vi = { NULL };
		m_vi.reset( );
		}
	}

_Ret_notnull_ CTreeListControl* CTreeListItem::GetTreeListControl( ) {
	// As we only have 1 TreeListControl and want to economize memory, we simple made the TreeListControl global.
	const auto tlc = CTreeListControl::GetTheTreeListControl( );
	ASSERT( tlc != NULL );
	if ( tlc == NULL ) {
		displayWindowsMsgBoxWithMessage( L"Serious error in CTreeListItem::GetTreeListControl: tlc == NULL, This should never happen!(aborting)" );
		//throw std::logic_error( "This should never happen!" );
		std::terminate( );
		
		//need to 'call' abort because `/analyze` doesn't understand that std::terminate DOES NOT RETURN!
		abort( );
		}
	return tlc;
	}

_Pre_satisfies_( this->m_vi._Myptr != nullptr ) 
CRect CTreeListItem::GetPlusMinusRect( ) const {
	ASSERT( IsVisible( ) );
	return BuildCRect( m_vi->rcPlusMinus );
	}

_Pre_satisfies_( this->m_vi._Myptr != nullptr )
CRect CTreeListItem::GetTitleRect( ) const {
    ASSERT( IsVisible( ) );
    return BuildCRect( m_vi->rcTitle );
    }

/////////////////////////////////////////////////////////////////////////////
// CTreeListControl

CTreeListControl* CTreeListControl::_theTreeListControl;

IMPLEMENT_DYNAMIC( CTreeListControl, COwnerDrawnListCtrl )

_Pre_satisfies_( ( parent + 1 ) < index )
void CTreeListControl::collapse_parent_plus_one_through_index( _In_ const CTreeListItem* thisPath, _Inout_ _Out_range_( -1, INT_MAX ) int& index, _In_range_( 0, INT_MAX ) const int parent ) {
	for ( int k = ( parent + 1 ); k < index; k++ ) {
		if ( !CollapseItem( k ) ) {
			break;
			}
		}
	index = FindListItem( thisPath );
	//index = FindTreeItem( thisPath );
	}


void CTreeListControl::adjustColumnSize( _In_ const CTreeListItem* const item_at_index ) {
	static_assert( column::COL_NAME == 0, "GetSubItemWidth used to accept an INT as the second parameter. The value of zero, I believe, should be COL_NAME" );
	static_assert( std::is_convertible<column::ENUM_COL, int>::value, "we're gonna need to do this!" );
	static_assert( std::is_convertible<std::underlying_type<column::ENUM_COL>::type, int>::value, "we're gonna need to do this!" );

	const auto w = GetSubItemWidth( item_at_index, column::COL_NAME ) + 5;
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
		ASSERT( parent != -1 );
		ASSERT( parent >= 0 );
		//ExpandItem( parent, false );
		ExpandItemNoScroll( parent );
		j = FindListItem( pathZero );//TODO: j?
		}
	}

void CTreeListControl::pathZeroNotNull( _In_ const CTreeListItem* const pathZero, _In_range_( 0, INT_MAX ) const int index, _In_ const bool showWholePath ) {
	expand_item_no_scroll_then_doWhateverJDoes( pathZero, index );
	ASSERT( index != -1 );
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

void CTreeListControl::thisPathNotNull( _In_ const CTreeListItem* const thisPath, const int i, int& parent, _In_ const bool showWholePath, _In_ const CTreeListItem* const path ) {
	//auto index = FindTreeItem( thisPath );
	auto index = FindListItem( thisPath );
	if ( index == -1 ) {
		TRACE( _T( "Searching %s ( this path element ) for next path element...not found! Expanding %I64d...\r\n" ), thisPath->m_name.get( ), i );
		//ExpandItem( i, false );
		ExpandItemNoScroll( i );
		//index = FindTreeItem( thisPath );
		
		//we expect to find the item on the second try.
		index = FindListItem( thisPath );
		TRACE( _T( "Set index to %i\r\n" ), index );
		ASSERT( index != -1 );
		if ( index == -1 ) {
			TRACE( _T( "Logic error! Item not found!\r\n" ) );
			parent = index;
			return;
			}
		}
	else {
		//if we've found the item, then we should close anything that we opened in the process?
		TRACE( _T( "Searching %s for next path element...found! path.at( %I64d ), index: %i\r\n" ), thisPath->m_name.get( ), std::int64_t( i ), index );
		collapse_parent_plus_one_through_index( thisPath, index, parent );
		TRACE( _T( "Collapsing items [%i, %i), new index %i. Item count: %i\r\n" ), ( parent + 1 ), index, index, GetItemCount( ) );
		}
	parent = index;
	const auto pathZero = path;
	ASSERT( index != -1 );
	if ( pathZero != NULL ) {
		if ( index != -1 ) {
			pathZeroNotNull( pathZero, index, showWholePath );
			}
		}
	ASSERT( pathZero != NULL );

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
		TRACE( _T( "path component %I64u: `%s`\r\n" ), std::uint64_t( inner ), path.at( inner )->m_name.get( ) );
		}
#endif

	auto parent = 0;
	for ( auto i = static_cast<std::int64_t>( path.size( ) - 1 ); i >= 0; --i ) {//Iterate downwards, root first, down each matching parent, until we find item
		const auto thisPath = path.at( static_cast<size_t>( i ) );
		if ( thisPath != NULL ) {
			ASSERT( static_cast<std::uint64_t>( i ) < INT_MAX );
			thisPathNotNull( thisPath, static_cast<int>( i ), parent, showWholePath, path.at( 0 ) );
			}
		ASSERT( thisPath != NULL );
		}
	SetRedraw( TRUE );
	}

void CTreeListControl::InitializeNodeBitmaps( ) {
	
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

void CTreeListControl::DeleteItem( _In_ _In_range_( 0, INT_MAX ) const INT i ) {
	ASSERT( i < GetItemCount( ) );
	auto const anItem = GetItem( i );
	if ( anItem != NULL ) {
		anItem->SetExpanded( false );
		anItem->SetVisible( false );
		//auto newVI = anItem->m_vi->rcTitle;
		}
	VERIFY( COwnerDrawnListCtrl::DeleteItem( i ) );
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

void CTreeListControl::DrawNodeNullWidth( _In_ const CTreeListItem* const item, _In_ CDC& pdc, _In_ const RECT& rcRest, _Inout_ bool& didBitBlt, _In_ CDC& dcmem, _In_ const UINT ysrc ) {
	auto ancestor = item;
	//Not vectorized: 1304, loop includes assignments of different sizes
	for ( auto indent = ( item->GetIndent( ) - 2 ); indent >= 0; indent-- ) {
		if ( ancestor != NULL ) {
			ancestor = ancestor->m_parent;
			if ( ancestor != NULL ) {
				if ( ancestor->HasSiblings( ) ) {
					ASSERT_VALID( &dcmem );
					VERIFY( pdc.BitBlt( ( static_cast<int>( rcRest.left ) + indent * static_cast<int>( INDENT_WIDTH ) ), static_cast<int>( rcRest.top ), static_cast<int>( NODE_WIDTH ), static_cast<int>( NODE_HEIGHT ), &dcmem, ( NODE_WIDTH * NODE_LINE ), static_cast<int>( ysrc ), SRCCOPY ) );
					didBitBlt = true;
					}
				}
			}
		}
	}

bool CTreeListControl::GetAscendingDefault( _In_ const column::ENUM_COL column ) const {
	switch ( column )
	{
		case column::COL_NAME:
		case column::COL_TOTAL:
		case column::COL_SUBTREETOTAL:
		case column::COL_ITEMS:
		case column::COL_FILES:
		case column::COL_LASTCHANGE:
		case column::COL_ATTRIBUTES:
			return true;
		default:
			ASSERT( false );
			return false;
	}
	//return ( column == column::COL_NAME || column == column::COL_LASTCHANGE );
	}


void CTreeListControl::SelectItem( _In_ const CTreeListItem* const item ) {
	//auto i = FindTreeItem( item );
	auto i = FindListItem( item );
	if ( i != -1 ) {
		SelectItem( i );
		}
	}


_Must_inspect_result_ _Success_( return != -1 ) INT CTreeListControl::GetSelectedItem( ) const {
	auto pos = GetFirstSelectedItemPosition( );
	if ( pos == NULL ) {
		return -1;
		}
	return GetNextSelectedItem( pos );
	}

void CTreeListControl::OnContextMenu( CWnd* /*pWnd*/, CPoint pt ) {
	auto i = GetSelectedItem( );
	
	if ( i == -1 ) {
		TRACE( _T( "OnContextMenu failed to get a valid selected item! returning early....\r\n" ) );
		return;
		}
	const auto item = GetItem( i );
	const auto thisHeader = GetHeaderCtrl( );
	auto rc = GetWholeSubitemRect( i, 0, thisHeader );
	if ( item == NULL ) {
		displayWindowsMsgBoxWithMessage( L"GetItem returned NULL!" );
		return;
		}
	/*
inline CRect CRect::operator+(_In_ POINT pt) const throw()
{
	CRect rect(*this);
	::OffsetRect(&rect, pt.x, pt.y);
	return rect;
}
	
	*/
	auto trect = item->GetTitleRect( );
	VERIFY( ::OffsetRect( trect, rc.left, rc.top ) );
	CRect rcTitle = item->GetTitleRect( ) + rc.TopLeft( );
	CMenu menu;
	VERIFY( menu.LoadMenuW( IDR_POPUPLIST ) );
	auto sub = menu.GetSubMenu( 0 );
	PrepareDefaultMenu( static_cast<CItemBranch*>( item ), sub );

	// Show popup menu and act accordingly. The menu shall not overlap the label but appear horizontally at the cursor position,  vertically under (or above) the label.
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

void CTreeListControl::SelectItem( _In_ _In_range_( 0, INT_MAX ) const INT i ) {
	VERIFY( SetItemState( i, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED ) );
	VERIFY( EnsureVisible( i, false ) );
	}

void CTreeListControl::PrepareDefaultMenu( _In_ const CItemBranch* const item, _Out_ CMenu* const menu ) {
	//if ( item->m_type == IT_FILE ) {
	if ( item->m_children == nullptr ) {
		VERIFY( menu->DeleteMenu( 0, MF_BYPOSITION ) );	// Remove "Expand/Collapse" item
		VERIFY( menu->DeleteMenu( 0, MF_BYPOSITION ) );	// Remove separator
		}
	else {
		const auto command = MAKEINTRESOURCEW( item->IsExpanded( ) && item->HasChildren( ) ? IDS_COLLAPSE : IDS_EXPAND );
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
	InitializeNodeBitmaps();
	}

BOOL CTreeListControl::CreateEx( _In_ const DWORD dwExStyle, _In_ DWORD dwStyle, _In_ const RECT& rect, _In_ CWnd* pParentWnd, _In_ const UINT nID ) {
	InitializeNodeBitmaps( );

	dwStyle |= LVS_OWNERDRAWFIXED | LVS_SINGLESEL;
	VERIFY( COwnerDrawnListCtrl::CreateEx( dwExStyle, dwStyle, rect, pParentWnd, nID ) );
	return true;
	}


_Must_inspect_result_ _Success_( return != NULL ) _Ret_maybenull_
CTreeListItem* CTreeListControl::GetItem( _In_ _In_range_( 0, INT_MAX ) const int i ) const {
	ASSERT( i < GetItemCount( ) );
	const auto itemCount = GetItemCount( );
	if ( i < itemCount ) {
		return reinterpret_cast< CTreeListItem* >( GetItemData( static_cast<int>( i ) ) );
		}
	return NULL;
	}



void CTreeListControl::SetRootItem( _In_opt_ const CTreeListItem* const root ) {
	VERIFY( DeleteAllItems( ) );
	if ( root != NULL ) {
		InsertItem( root, 0 );
		//ExpandItem( static_cast<int>( 0 ), true );//otherwise ambiguous call - is it a NULL pointer?
		ExpandItemAndScroll( static_cast<int>( 0 ) );//otherwise ambiguous call - is it a NULL pointer?
		}
	}

//_Success_( return != -1 ) _Ret_range_( -1, INT_MAX ) INT CTreeListControl::FindTreeItem( _In_ const CTreeListItem* const item ) const {
//	return COwnerDrawnListCtrl::FindListItem( item );
//	}

void CTreeListControl::InsertItem( _In_ const CTreeListItem* const item, _In_ _In_range_( 0, INT32_MAX ) const INT_PTR i ) {
	COwnerDrawnListCtrl::InsertListItem( i, item );
	item->SetVisible( true );
	}


int CTreeListControl::EnumNode( _In_ const CTreeListItem* const item ) const {
	if ( item->GetChildrenCount_( ) > 0 ) {
		if ( item->HasSiblings( ) ) {
			if ( item->IsExpanded( ) ) {
				return NODE_MINUS_SIBLING;
				}
			return NODE_PLUS_SIBLING;
			}
		if ( item->IsExpanded( ) ) {
			return NODE_MINUS_END;
			}
		return NODE_PLUS_END;
		}
	if ( item->HasSiblings( ) ) {
		return NODE_SIBLING;
		}
	return NODE_END;
	}

void CTreeListControl::DrawNode( _In_ const CTreeListItem* const item, _In_ CDC& pdc, _Inout_ RECT& rc, _Out_ RECT& rcPlusMinus ) {
	//ASSERT_VALID( pdc );
	RECT rcRest = rc;
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
	}

void CTreeListControl::OnLButtonDown( UINT nFlags, CPoint point ) {
	m_lButtonDownItem = -1;

	//auto hti = zeroInitLVHITTESTINFO( );
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
	WTL::CPoint pt = point - rc.TopLeft( );

	const auto item = GetItem( i );

	m_lButtonDownItem = i;
	if ( item != NULL ) {
		if ( item->GetPlusMinusRect( ).PtInRect( pt ) ) {
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
		if ( item_at_i->IsExpanded( ) ) {
			VERIFY( CollapseItem( i ) );
			return;
			}
		//ExpandItem( i, true );
		ExpandItemAndScroll( i );
		}
	}

INT CTreeListControl::countItemsToDelete( _In_ const CTreeListItem* const item, bool& selectNode, _In_ _In_range_( 0, INT_MAX ) const INT& i ) {
	INT todelete = 0;
	//void countItemsToDelete( bool& selectNode, const INT& i )
	const auto itemCount = GetItemCount( );
	for ( INT k = i + 1; k < itemCount; k++ ) {
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

_Success_( return == true ) bool CTreeListControl::CollapseItem( _In_ _In_range_( 0, INT_MAX ) const INT i ) {
	auto const item = GetItem( i );
	ASSERT( item != NULL );
	if ( item == NULL ) {
		return false;
		}
	if ( !item->IsExpanded( ) ) {
		return false;
		}
	WTL::CWaitCursor wc;
	//LockWindowUpdate( );
	SetRedraw( FALSE );
	
	bool selectNode = false;
	auto todelete = countItemsToDelete( item, selectNode, i );
	for ( INT m = 0; m < todelete; m++ ) {
		DeleteItem( i + 1 );
		}
	item->SetExpanded( false );
	if ( selectNode ) {
		SelectItem( i );
		}

	SetRedraw( TRUE );
	//UnlockWindowUpdate( );
	VERIFY( RedrawItems( i, i ) );
	return true;
	}

INT CTreeListControl::GetItemScrollPosition (_In_ const CTreeListItem* const item ) const {
	CRect rc;
	VERIFY( GetItemRect( FindListItem( item ), rc, LVIR_BOUNDS ) );
	//VERIFY( GetItemRect( FindTreeItem( item ), rc, LVIR_BOUNDS ) );
	return rc.top;
	}

void CTreeListControl::SetItemScrollPosition( _In_ const CTreeListItem* const item, _In_ const INT top ) {
	const auto old = GetItemScrollPosition( item );
	Scroll( WTL::CSize { 0, top - old } );
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

//void CTreeListControl::ExpandItem( _In_ const CTreeListItem* const item ) {
//	//const auto itemPos = FindTreeItem( item );
//	const auto itemPos = FindListItem( item );
//	ASSERT( itemPos != -1 );
//	if ( itemPos != -1 ) {
//		//ExpandItem( itemPos, false );
//		ExpandItemNoScroll( itemPos );
//		}
//	}

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

void CTreeListControl::OnItemDoubleClick ( _In_ _In_range_( 0, INT_MAX ) const INT i ) {
	const auto item = static_cast< const CItemBranch* >( GetItem( i ) );
	if ( item != NULL ) {
		//if ( item->m_type == IT_FILE ) {
		if ( item->m_children == nullptr ) {
			TRACE( _T( "User double-clicked %s in TreeListControl! Opening Item!\r\n" ), item->GetPath( ).c_str( ) );
			return GetDocument( )->OpenItem( *item );
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
	if ( item == NULL ) {
		ASSERT( false );
		return;
		}
	if ( item->IsExpanded( ) ) {
		return;
		}

	WTL::CWaitCursor wc; // TODO: smart WaitCursor. In CollapseItem(), too.
	SetRedraw( FALSE );
	//LockWindowUpdate( );
#ifdef PERF_DEBUG_SLEEP
	Sleep( 1000 );
#endif

#ifdef DEBUG
	auto qpf = ( DOUBLE( 1 ) / DOUBLE( help_QueryPerformanceFrequency( ).QuadPart ) );
	auto qpc_1 = help_QueryPerformanceCounter( );
#endif

	item->SortChildren( );

	ExpandItemInsertChildren( item, i, scroll );

	item->SetExpanded( true );

#ifdef DEBUG
	auto qpc_2 = help_QueryPerformanceCounter( );

	auto timing = ( qpc_2.QuadPart - qpc_1.QuadPart ) * qpf;
	TRACE( _T( "Inserting items ( expansion ) took %f!\r\n" ), timing );
#endif

	item->SortChildren( );

	//UnlockWindowUpdate( );
	//item->SortChildren( );

	//static cast to int is safe here, range of i should never be more than INT32_MAX
	VERIFY( RedrawItems( static_cast<int>( i ), static_cast<int>( i ) ) );

#ifdef DEBUG
	auto qpc_3 = help_QueryPerformanceCounter( );
	auto timing_2 = ( qpc_3.QuadPart - qpc_2.QuadPart ) * qpf;
	TRACE( _T( "Inserting items (sort/redraw) took %f!\r\n" ), timing_2 );
#endif

#ifdef PERF_DEBUG_SLEEP
	Sleep( 1000 );
#endif

	

	if ( scroll ) {
		// Scroll up so far, that i is still visible and the first child becomes visible, if possible.
		if ( item->GetChildrenCount_( ) > 0 ) {
			//static cast to int is safe here, range of i should never be more than INT32_MAX
			VERIFY( EnsureVisible( static_cast<int>( i ), false ) );
			}
		//static cast to int is safe here, range of i should never be more than INT32_MAX
		VERIFY( EnsureVisible( static_cast<int>( i ), false ) );
		}
	SetRedraw( TRUE );
	}

void CTreeListControl::handle_VK_LEFT( _In_ const CTreeListItem* const item, _In_ _In_range_( 0, INT32_MAX ) const int i ) {
	if ( item->IsExpanded( ) ) {
		VERIFY( CollapseItem( i ) );
		}
	//this used to be an ugly (and wrong) `itemParent != NULL`. Not sure how that got there.
	else if ( item->m_parent != NULL ) {
		SelectItem( item->m_parent );
		}

	}

void CTreeListControl::handle_VK_RIGHT( _In_ const CTreeListItem* const item, _In_ _In_range_( 0, INT_MAX ) const int i ) {
	if ( !item->IsExpanded( ) ) {
		//ExpandItem( i, true );
		ExpandItemAndScroll( i );
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
		auto item = GetItem( i );
		if ( item != NULL ) {
			//const auto itemParent = item->m_parent != NULL;
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
		ASSERT( GetDocument( )->m_rootItem.get( ) == child );
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
	for ( INT i = 0; i < countItems; i++ ) {//convert to ranged for?
		auto const Item = GetItem( i );
		if ( Item != NULL ) {
			if ( Item->IsExpanded( ) ) {
				Item->SortChildren( );
				}
			}
		ASSERT( Item != NULL );
		}
	COwnerDrawnListCtrl::SortItems( );
	}

void CTreeListControl::EnsureItemVisible( _In_ const CTreeListItem* const item ) {
	if ( item == NULL ) {
		return;
		}
	//const auto i = FindTreeItem( item );
	const auto i = FindListItem( item );
	if ( i == -1 ) {
		return;
		}
	VERIFY( EnsureVisible( i, false ) );
	}

// $Log$
// Revision 1.9  2004/11/13 08:17:07  bseifert
// Remove blanks in Unicode Configuration names.
//
// Revision 1.8  2004/11/12 22:14:16  bseifert
// Eliminated CLR_NONE. Minor corrections.
//
// Revision 1.7  2004/11/12 00:47:42  assarbad
// - Fixed the code for coloring of compressed/encrypted items. Now the coloring spans the full row!
//
// Revision 1.6  2004/11/08 00:46:26  assarbad
// - Added feature to distinguish compressed and encrypted files/folders by color as in the Windows 2000/XP explorer.
//   Same rules apply. (Green = encrypted / Blue = compressed)
//
// Revision 1.5  2004/11/05 16:53:07  assarbad
// Added Date and History tag where appropriate.
//
