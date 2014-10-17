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
//#include ".\treelistcontrol.h"

//#include "windirstat.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

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
	const LONG NODE_HEIGHT = 24;	// Height of IDB_NODES
	const LONG INDENT_WIDTH = 18;

	const LONG HOTNODE_CX = 9;		// Size and position of the +/- buttons
	const LONG HOTNODE_CY = 9;
	const LONG HOTNODE_X = 0;

	std::vector<const CTreeListItem *> buildVectorOfPaths( _In_ const CTreeListItem* item ) {
		std::vector<const CTreeListItem *> path;
		auto p = item;
		while ( p != NULL ) {
			path.emplace_back( p );
			p = p->GetParent( );
			}
		return path;
		}


}

CTreeListItem::CTreeListItem( CTreeListItem&& in ) {
	m_parent = std::move( in.m_parent );
	m_vi = std::move( in.m_vi );
	
	//Invalidate the remaining pointers
	in.m_parent = NULL;
	in.m_vi = NULL;
	
	}

CTreeListItem::~CTreeListItem( ) {
	delete m_vi;
	m_vi = NULL;
	m_parent = NULL;
	}

bool CTreeListItem::DrawSubitem( _In_ _In_range_( 0, INT_MAX ) const ENUM_COL subitem, _In_ CDC* pdc, _In_ CRect rc, _In_ const UINT state, _Out_opt_ INT* width, _Inout_ INT* focusLeft ) const {
	ASSERT_VALID( pdc );
	ASSERT( ( focusLeft != NULL ) && ( subitem >= 0 ) );

	if ( subitem != 0 ) {
		return false;
		}
#ifdef _DEBUG
	bool wasNull = ( width == NULL );
#endif

	auto rcNode = rc;
	CRect rcPlusMinus;
	auto TreeListControl = GetTreeListControl( );
	ASSERT( TreeListControl != NULL );
	if ( TreeListControl != NULL ) {
		ASSERT( this != NULL );
		if ( width != NULL ) {
			*width = rc.Width( );
			}
		TreeListControl->DrawNode( pdc, rcNode, rcPlusMinus, this );//pass subitem to drawNode?
		}
	
	auto rcLabel = rc;
	rcLabel.left = rcNode.right;
#ifdef DRAW_ICONS
	auto MyImageList = GetMyImageList( );
	ASSERT( MyImageList != NULL );
	if ( ( TreeListControl != NULL ) && ( MyImageList != NULL ) ) {
		DrawLabel( TreeListControl , MyImageList, pdc, rcLabel, state, width, focusLeft, false );
		}
#else
	if ( TreeListControl != NULL ) {
		DrawLabel( TreeListControl, nullptr, pdc, rcLabel, state, width, focusLeft, false );
		}
#endif
	if ( width != NULL ) {
#ifdef _DEBUG
		ASSERT( !wasNull );
#endif
		*width = rcLabel.Width( );
		}
	else {
		SetPlusMinusRect( rcPlusMinus - rc.TopLeft( ) );
		SetTitleRect( rcLabel - rc.TopLeft( ) );
		}
	return true;
	}

#ifdef DRAW_ICONS
INT CTreeListItem::GetImage( ) const {
	ASSERT( IsVisible( ) );
	if ( m_vi->image == -1 ) {
		m_vi->image = GetImageToCache( );
		}
	return m_vi->image;
	}

void CTreeListItem::UncacheImage( ) {
	if ( IsVisible( ) ) {
		m_vi->image = -1;
		}
	}
#endif


_Success_( return != -1 ) INT CTreeListItem::GetScrollPosition( ) {
	auto TreeListControl = GetTreeListControl( );
	if ( TreeListControl != NULL ) {
		return TreeListControl->GetItemScrollPosition( this );
		}
	ASSERT( TreeListControl != NULL );//BADBADBAD
	return -1;
	}

void CTreeListItem::SetScrollPosition( _In_ _In_range_( 0, INT_MAX ) const INT top ) {
	auto TreeListControl = GetTreeListControl( );
	if ( TreeListControl != NULL ) {
		TreeListControl->SetItemScrollPosition( this, top );
		}
	ASSERT( TreeListControl != NULL );
	}


_Pre_satisfies_( this->m_vi != NULL ) void CTreeListItem::SortChildren( ) {
	ASSERT( IsVisible( ) );
	m_vi->sortedChildren.reserve( GetChildrenCount( ) );
	auto childCount = GetChildrenCount( );
	for ( size_t i = 0; i < childCount; i++ ) {
		auto aTreeListChild = GetTreeListChild( i );

		if ( aTreeListChild != NULL ) {
			if ( ( i > m_vi->sortedChildren.size( ) ) && ( i > 0 ) ) {
				m_vi->sortedChildren.resize( i + 1 );
				}
			else if ( (!m_vi->sortedChildren.empty())&& (i == m_vi->sortedChildren.size())) {
				m_vi->sortedChildren.emplace_back( aTreeListChild );
				}
			else if ( m_vi->sortedChildren.empty( ) && ( i == 0 ) ) {
				m_vi->sortedChildren.emplace_back( aTreeListChild );
				}
			else {
				ASSERT( i < m_vi->sortedChildren.size( ) );
				m_vi->sortedChildren.at( i ) = aTreeListChild;
				}
			}
		ASSERT( aTreeListChild != NULL );
		}
	if ( !m_vi->sortedChildren.empty( ) ) {
		//qsort( m_vi->sortedChildren.at( 0 ), m_vi->sortedChildren.size( ) -1, sizeof( CTreeListItem * ), &_compareProc );
		std::sort( m_vi->sortedChildren.begin( ), m_vi->sortedChildren.end( ), &_compareProc2 );
		//std::sort( m_vi->sortedChildren.begin( ), m_vi->sortedChildren.end( ), TreeListItemSortStruct( ) );
		m_vi->sortedChildren.shrink_to_fit( );
		}
	}


bool CTreeListItem::_compareProc2( CTreeListItem* lhs, CTreeListItem* rhs ) {
	auto TreeListCtrl = GetTreeListControl( );
	if ( TreeListCtrl != NULL ) {
		auto result = lhs->CompareS( rhs, TreeListCtrl->m_sorting ) < 0;
		return result;
		}
	else {
		ASSERT( false );
		return lhs->CompareS( rhs, SSorting( ) ) > 0;//else, fall back to some default behavior.
		}
	}

//INT __cdecl CTreeListItem::_compareProc( _In_ const void *p1, _In_ const void *p2 ) {
//	const auto item1 = *( const CTreeListItem** ) p1;
//	const auto item2 = *( const CTreeListItem** ) p2;
//	const auto TreeListCtrl = GetTreeListControl( );
//	if ( TreeListCtrl != NULL ) {
//		return item1->CompareS( item2, TreeListCtrl->m_sorting );
//		}
//	else {
//		ASSERT( false );
//		return item1->CompareS( item2, SSorting( ) );//else, fall back to some default behavior.
//		}
//	}

_Success_( return != NULL ) _Must_inspect_result_ _Ret_maybenull_ CTreeListItem* CTreeListItem::GetSortedChild( _In_ const size_t i ) {
	ASSERT( m_vi != NULL );
	if ( m_vi != NULL ) {
		if ( m_vi->sortedChildren.empty( ) ) {
			return NULL;
			}
		else {
			return m_vi->sortedChildren.at( i );
			}
		}
	return NULL;
	}

INT CTreeListItem::Compare( _In_ const CSortingListItem* const baseOther, _In_ const INT subitem ) const {
	const auto other = static_cast<const CTreeListItem *>( baseOther );
	if ( other == NULL ) {
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
		return CompareSibling( other, subitem );
		}
	if ( GetIndent( ) < other->GetIndent( ) ) {
		return Compare( other->m_parent, subitem );
		}
	else if ( GetIndent( ) > other->GetIndent( ) ) {
		return m_parent->Compare( other, subitem );
		}
	else {
		return m_parent->Compare( other->m_parent, subitem );
		}
	}

size_t CTreeListItem::FindSortedChild( _In_ const CTreeListItem* const child ) {
	const auto childCount = GetChildrenCount( );
	ASSERT( childCount > 0 );
	for ( size_t i = 0; i < childCount; i++ ) {
		if ( child == GetSortedChild( i ) ) {
			return i;
			}
		}
	return childCount; 
	}

_Success_( return != NULL ) _Must_inspect_result_ _Ret_maybenull_ CTreeListItem* CTreeListItem::GetParent( ) const {
	return m_parent;
	}

_Pre_satisfies_( this->m_parent != NULL ) bool CTreeListItem::HasSiblings( ) const {
	if ( m_parent == NULL ) {
		return false;
		}
	if ( m_parent->GetChildrenCount( ) == 1 ) {
		return false;
		}
	if ( m_parent->GetChildrenCount( ) == 0 ) {
		ASSERT( false );
		}
	const auto i = m_parent->FindSortedChild( this );
	return ( i <= ( m_parent->GetChildrenCount( ) - 1 ) );//return true if `i` is in valid range
	}

bool CTreeListItem::HasChildren( ) const {
	return GetChildrenCount( ) > 0;
	}

bool CTreeListItem::IsExpanded( ) const {
	ASSERT( IsVisible( ) );
	return m_vi->isExpanded; 
	}

void CTreeListItem::SetExpanded( _In_ const bool expanded ) {
	ASSERT( IsVisible( ) );
	m_vi->isExpanded = expanded;
	}

bool CTreeListItem::IsVisible( ) const {
	return ( m_vi != NULL );
	}

void CTreeListItem::SetVisible( _In_ const bool next_state_visible ) {
	if ( next_state_visible ) {
		ASSERT( ( !IsVisible( ) ) && ( m_vi == NULL ) );
		m_vi = new VISIBLEINFO;
		if ( GetParent( ) == NULL ) {
			m_vi->indent = 0;
			}
		else {
			auto Parent = GetParent( );
			if ( Parent != NULL ) {
				m_vi->indent = Parent->GetIndent( ) + 1;
				}
			ASSERT( Parent != NULL );
			}
#ifdef DRAW_ICONS
		m_vi->image = -1;
#endif
		m_vi->isExpanded = false;
		}
	else {
		ASSERT( IsVisible( ) );
		delete m_vi;
		m_vi = NULL;
		}
	}

std::int16_t CTreeListItem::GetIndent( ) const {
	ASSERT( IsVisible( ) );
	return m_vi->indent;
	}

CRect CTreeListItem::GetPlusMinusRect( ) const {
	ASSERT( IsVisible( ) );
	return BuildCRect( m_vi->rcPlusMinus );
	}

void CTreeListItem::SetPlusMinusRect( _In_ const CRect& rc ) const {
	ASSERT( IsVisible( ) );
	m_vi->rcPlusMinus = SRECT( rc );
	}

CRect CTreeListItem::GetTitleRect( ) const {
	ASSERT( IsVisible( ) );
	return BuildCRect( m_vi->rcTitle );
	}

void CTreeListItem::SetTitleRect( _In_ const CRect& rc ) const {
	ASSERT( IsVisible( ) );
	m_vi->rcTitle = SRECT( rc );
	}

_Must_inspect_result_ _Ret_maybenull_ CTreeListControl* CTreeListItem::GetTreeListControl( ) {
	// As we only have 1 TreeListControl and want to economize memory, we simple made the TreeListControl global.
	return CTreeListControl::GetTheTreeListControl( );
	}

/////////////////////////////////////////////////////////////////////////////
// CTreeListControl

CTreeListControl* CTreeListControl::_theTreeListControl;

_Must_inspect_result_ _Ret_maybenull_ _Pre_satisfies_( _theTreeListControl != NULL ) CTreeListControl* CTreeListControl::GetTheTreeListControl( ) {
	ASSERT(_theTreeListControl != NULL);
	return _theTreeListControl;
	}


IMPLEMENT_DYNAMIC( CTreeListControl, COwnerDrawnListControl )

_Pre_satisfies_( rowHeight % 2 == 0 ) CTreeListControl::CTreeListControl( UINT rowHeight ) : COwnerDrawnListControl( _T( "treelist" ), rowHeight ) {
	ASSERT( _theTreeListControl == NULL );
	_theTreeListControl = this;

	ASSERT( rowHeight <= NODE_HEIGHT );	// größer können wir nicht//"larger, we can not"?
	//ASSERT( rowHeight % 2 == 0 );			// muss gerade sein//"must be straight"?
	}

CTreeListControl::~CTreeListControl( ) {
	if ( _theTreeListControl != NULL ) {
		_theTreeListControl = NULL;
		}
	delete m_imageList;
	m_imageList = NULL;
	}

void CTreeListControl::SelectItem( _In_ _In_range_( 0, INT_MAX ) const INT i ) {
	SetItemState( i, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
	EnsureVisible( i, false );
	}

_Must_inspect_result_ _Success_( return != -1 ) INT CTreeListControl::GetSelectedItem( ) const {
	auto pos = GetFirstSelectedItemPosition( );
	if ( pos == NULL ) {
		return -1;
		}
	return GetNextSelectedItem( pos );
	}

void CTreeListControl::SelectItem( _In_ const CTreeListItem* item ) {
	auto i = FindTreeItem( item );
	if ( i != -1 ) {
		SelectItem( i );
		}
	}

BOOL CTreeListControl::CreateEx( _In_ const DWORD dwExStyle, _In_ DWORD dwStyle, _In_ const RECT& rect, _In_ CWnd* pParentWnd, _In_ const UINT nID ) {
	InitializeNodeBitmaps( );

	dwStyle |= LVS_OWNERDRAWFIXED | LVS_SINGLESEL;
	VERIFY( COwnerDrawnListControl::CreateEx( dwExStyle, dwStyle, rect, pParentWnd, nID ) );
	return true;
	}

void CTreeListControl::SysColorChanged( ) {
	COwnerDrawnListControl::SysColorChanged();
	InitializeNodeBitmaps();
	}

_Must_inspect_result_ _Success_( return != NULL ) _Ret_maybenull_ CTreeListItem* CTreeListControl::GetItem( _In_ _In_range_( 0, INT_MAX ) const INT_PTR i ) {
	auto itemCount = GetItemCount( );
	if ( i < itemCount ) {
		return reinterpret_cast< CTreeListItem * >( GetItemData( static_cast<int>( i ) ) );
		}
	return NULL;
	}

void CTreeListControl::SetRootItem( _In_opt_ CTreeListItem* root ) {
	DeleteAllItems( );
	if ( root != NULL ) {
		InsertItem( 0, root );
		ExpandItem( INT_PTR( 0 ) );//otherwise ambiguous call - is it a NULL pointer?
		}
	}

void CTreeListControl::DeselectAll( ) {
	auto itemCount = GetItemCount( );
	for ( INT i = 0; i < itemCount; i++ ) {
		SetItemState( i, 0, LVIS_SELECTED );
		}
	}

void CTreeListControl::SelectAndShowItem( _In_ const CTreeListItem* item, _In_ const bool showWholePath ) {
	//This function is VERY finicky. Be careful.
	SetRedraw( FALSE );
	auto path = buildVectorOfPaths( item );
	auto parent = 0;
	for ( auto i = std::int64_t( path.size( ) - 1 ); i >= 0; --i ) {//Iterate downwards, root first, down each matching parent, until we find item
		auto thisPath = path.at( static_cast<size_t>( i ) );
		if ( thisPath != NULL ) {
			auto index = FindTreeItem( thisPath );
			if ( index == -1 ) {
				TRACE( _T( "Searching %s ( this path element ) for next path element...not found! Expanding %I64d...\r\n" ), thisPath->GetText( 0 ), i );
				ExpandItem( i, false );
				index = FindTreeItem( thisPath );
				TRACE( _T( "Set index to %i\r\n" ), index );
				}
			else {
				TRACE( _T( "Searching %s for next path element...found! path.at( %I64d ), index: %i\r\n" ), thisPath->GetText( 0 ), i, index );
				auto newK = parent + 1;
				TRACE( _T( "Collapsing items [%i, %i), new index %i. Item count: %i\r\n" ), newK, index, index, GetItemCount( ) );
				for ( auto k = newK; k < index; k++ ) {
					if ( !CollapseItem( k ) ) {
						break;
						}
					}
				//auto tItem = FindTreeItem( thisPath );
				//ASSERT( index ==  tItem );
				index = FindTreeItem( thisPath );
				}
			parent = index;
			auto pathZero = path.at( 0 );
			if ( pathZero != NULL ) {
				auto j = FindTreeItem( pathZero );
				if ( j == -1 ) {
					ExpandItem( parent, false );
					j = FindTreeItem( pathZero );//TODO: j?
					}
				auto item_at_index = GetItem( index );
				ASSERT( item_at_index != NULL );
				if ( item_at_index != NULL ) {
					static_assert( COL_NAME == 0, "GetSubItemWidth used to accept an INT as the second parameter. The value of zero, I believe, should be COL_NAME" );
					auto w = GetSubItemWidth( item_at_index, ENUM_COL( 0 ) ) + 5;
					auto colWidth = GetColumnWidth( 0 );
					if ( colWidth < w ) {
						SetColumnWidth( 0, w + colWidth );
						}
					}
				if ( showWholePath ) {
					EnsureVisible( 0, false );
					}
				SelectItem( index );
				}
			ASSERT( pathZero != NULL );
			}
		ASSERT( thisPath != NULL );
		}
	SetRedraw( TRUE );
	}

void CTreeListControl::InitializeNodeBitmaps( ) {
	m_bmNodes0.DeleteObject( );
	m_bmNodes1.DeleteObject( );

	COLORMAP cm[ 1 ] = { { RGB( 255, 0, 255 ), 0 } };
	
	cm[ 0 ].to = m_windowColor;
	VERIFY( m_bmNodes0.LoadMappedBitmap( IDB_NODES, 0, cm, 1 ) );
	cm[ 0 ].to = m_stripeColor;
	VERIFY( m_bmNodes1.LoadMappedBitmap( IDB_NODES, 0, cm, 1 ) );
	}

void CTreeListControl::InsertItem( _In_ _In_range_( 0, INT_MAX ) const INT_PTR i, _In_ CTreeListItem* item ) {
	COwnerDrawnListControl::InsertListItem( i, item );
	item->SetVisible( true );
	}

void CTreeListControl::DeleteItem( _In_ _In_range_( 0, INT_MAX ) const INT i ) {
	ASSERT( i < GetItemCount( ) );
	auto anItem = GetItem( i );
	if ( anItem != NULL ) {
		anItem->SetExpanded( false );
		anItem->SetVisible( false );
		}
	COwnerDrawnListControl::DeleteItem( i );
	}

_Success_( return != -1 ) INT CTreeListControl::FindTreeItem( _In_ const CTreeListItem* item ) {
	return COwnerDrawnListControl::FindListItem( item );
	}

BEGIN_MESSAGE_MAP(CTreeListControl, COwnerDrawnListControl)
	ON_WM_MEASUREITEM_REFLECT()
	ON_WM_LBUTTONDOWN()
	ON_WM_KEYDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_DESTROY()
END_MESSAGE_MAP()

void CTreeListControl::DrawNodeNullWidth( _In_ CDC* pdc, _In_ CRect& rcRest, _In_ const CTreeListItem* const item, _Inout_ bool& didBitBlt, _In_ CDC& dcmem, _In_ unsigned int ysrc ) {
	auto ancestor = item;
	for ( auto indent = ( item->GetIndent( ) - 2 ); indent >= 0; indent-- ) {
		if ( ancestor != NULL ) {
			ancestor = ancestor->GetParent( );
			if ( ancestor != NULL ) {
				if ( ancestor->HasSiblings( ) ) {
					ASSERT_VALID( &dcmem );
					pdc->BitBlt( ( static_cast<int>( rcRest.left ) + indent * static_cast<int>( INDENT_WIDTH ) ), static_cast<int>( rcRest.top ), static_cast<int>( NODE_WIDTH ), static_cast<int>( NODE_HEIGHT ), &dcmem, ( NODE_WIDTH * NODE_LINE ), static_cast<int>( ysrc ), SRCCOPY );
					didBitBlt = true;
					}
				}
			}
		}
	}

int CTreeListControl::EnumNode( _In_ const CTreeListItem* const item ) {
	if ( item->HasChildren( ) ) {
		if ( item->HasSiblings( ) ) {
			if ( item->IsExpanded( ) ) {
				return NODE_MINUS_SIBLING;
				}
			return NODE_PLUS_SIBLING;
			}
		else {
			if ( item->IsExpanded( ) ) {
				return NODE_MINUS_END;
				}
			return NODE_PLUS_END;
			}
		}
	else {
		if ( item->HasSiblings( ) ) {
			return NODE_SIBLING;
			}
		return NODE_END;
		}
	}

void CTreeListControl::DrawNode( _In_ CDC* pdc, _In_ CRect& rc, _Inout_ CRect& rcPlusMinus, _In_ const CTreeListItem* item ) {
	ASSERT_VALID( pdc );
	CRect rcRest = rc;
	bool didBitBlt = false;
	rcRest.left += GENERAL_INDENT;
	if ( item->GetIndent( ) > 0 ) {
		rcRest.left += 3;
		CDC dcmem;
		dcmem.CreateCompatibleDC( pdc );
		CSelectObject sonodes( &dcmem, ( IsItemStripeColor( item ) ? &m_bmNodes1 : &m_bmNodes0 ) );
		auto ysrc = ( NODE_HEIGHT / 2 ) - ( m_rowHeight / 2 );
		DrawNodeNullWidth( pdc, rcRest, item, didBitBlt, dcmem, ysrc );
		rcRest.left += ( item->GetIndent( ) - 1 ) * INDENT_WIDTH;
		auto node = EnumNode( item );
		ASSERT_VALID( &dcmem );
		if ( !didBitBlt ) {//Else we'd double BitBlt?
			pdc->BitBlt( static_cast<int>( rcRest.left ), static_cast<int>( rcRest.top ), static_cast<int>( NODE_WIDTH ), static_cast<int>( NODE_HEIGHT ), &dcmem, ( NODE_WIDTH * node ), static_cast<int>( ysrc ), SRCCOPY );
			}
		rcPlusMinus.left    = rcRest.left      + HOTNODE_X;
		rcPlusMinus.right   = rcPlusMinus.left + HOTNODE_CX;
		rcPlusMinus.top     = rcRest.top       + ( rcRest.Height( ) )/ 2 - HOTNODE_CY / 2 - 1;
		rcPlusMinus.bottom  = rcPlusMinus.top  + HOTNODE_CY;
			
		rcRest.left += NODE_WIDTH;
	}
	rc.right = rcRest.left;
	}

void CTreeListControl::OnLButtonDown( UINT nFlags, CPoint point ) {
	m_lButtonDownItem = -1;

	auto hti = zeroInitLVHITTESTINFO( );

	hti.pt = point;

	auto i = HitTest( &hti );
	if ( i == -1 ) {
		return;
		}

	if ( hti.iSubItem != 0 ) {
		COwnerDrawnListControl::OnLButtonDown( nFlags, point );
		return;
		}

	auto rc = GetWholeSubitemRect( i, 0 );
	CPoint pt = point - rc.TopLeft( );

	auto item = GetItem( i );

	m_lButtonDownItem = i;
	if ( item != NULL ) {
		if ( item->GetPlusMinusRect( ).PtInRect( pt ) ) {
			m_lButtonDownOnPlusMinusRect = true;
			ToggleExpansion( i );
			}
		else {
			m_lButtonDownOnPlusMinusRect = false;
			COwnerDrawnListControl::OnLButtonDown( nFlags, point );
			}
		}
	ASSERT( item != NULL );
	}

void CTreeListControl::OnLButtonDblClk( UINT nFlags, CPoint point ) {
	COwnerDrawnListControl::OnLButtonDblClk( nFlags, point );

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
	auto item_at_i = GetItem( i );
	ASSERT( item_at_i != NULL );
	if ( item_at_i != NULL ) {
		if ( item_at_i->IsExpanded( ) ) {
			CollapseItem( i );
			return;
			}
		ExpandItem( i );
		}
	}

_Success_( return == true ) bool CTreeListControl::CollapseItem( _In_ _In_range_( 0, INT_MAX ) const INT i ) {
	auto item = GetItem( i );
	if ( item != NULL ) {
		if ( !item->IsExpanded( ) ) {
			return false;
			}
		}
	else {
		ASSERT( false );
		return false;
		}
	CWaitCursor wc;
	LockWindowUpdate( );

	bool selectNode = false;
	INT todelete = 0;
	auto itemCount = GetItemCount( );
	for ( INT k = i + 1; k < itemCount; k++ ) {
		auto child = GetItem( k );
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
	for ( INT m = 0; m < todelete; m++ ) {
		DeleteItem( i + 1 );
		}
	item->SetExpanded( false );
	if ( selectNode ) {
		SelectItem( i );
		}

	UnlockWindowUpdate( );
	RedrawItems( i, i );
	return true;
	}

INT CTreeListControl::GetItemScrollPosition (_In_ const CTreeListItem* const item ) {
	CRect rc;
	rc.bottom = NULL;
	rc.left   = NULL;
	rc.right  = NULL;
	rc.top    = NULL;
	VERIFY( GetItemRect( FindTreeItem( item ), rc, LVIR_BOUNDS ) );
	return rc.top;
	}

void CTreeListControl::SetItemScrollPosition( _In_ const CTreeListItem* const item, _In_ const INT top ) {
	auto old = GetItemScrollPosition( item );
	Scroll( CSize( 0, top - old ) );
	}

bool CTreeListControl::SelectedItemCanToggle( ) {
	auto i = GetSelectedItem( );
	if ( i == -1 ) {
		return false;
		}
	const auto item = GetItem( i );
	if ( item != NULL ) {
		return item->HasChildren( );
		}
	ASSERT( item != NULL );
	return false;
	}

void CTreeListControl::ToggleSelectedItem( ) {
	auto i = GetSelectedItem( );
	ASSERT( i != -1 );
	if ( i != -1 ) {
		ToggleExpansion( i );
		}
	}

void CTreeListControl::ExpandItem( _In_ CTreeListItem* item ) {
	ExpandItem( FindTreeItem( item ), false );
	}

void CTreeListControl::ExpandItemInsertChildren( _In_ _In_range_( 0, INT_MAX ) const INT_PTR i, _In_ const bool scroll, _In_ CTreeListItem* item ) {
	static_assert( COL_NAME == 0, "GetSubItemWidth used to accept an INT as the second parameter. The value of zero, I believe, should be COL_NAME" );
	auto maxwidth = GetSubItemWidth( item, ENUM_COL( 0 ) );
	auto count    = item->GetChildrenCount( );
	auto myCount  = size_t( GetItemCount( ) );
	TRACE( _T( "Expanding %s! Must insert %i items!\r\n" ), item->GetText( 0 ), count );
	SetItemCount( static_cast<INT>( ( count >= myCount) ? count + 1 : myCount + 1 ) );
	
	for ( size_t c = 0; c < count; c++ ) {
		ASSERT( count == item->GetChildrenCount( ) );
		auto child = item->GetSortedChild( c );//m_vi->sortedChildren[i];
		if ( child != NULL ) {
			InsertItem( i + static_cast<INT_PTR>( 1 ) + static_cast<INT_PTR>( c ), child );
			if ( scroll ) {
				auto w = GetSubItemWidth( child, ENUM_COL( 0 ) );//does drawing???
				if ( w > maxwidth ) {
					maxwidth = w;
					}
				}
			}
		ASSERT( child != NULL );
		}

	if ( scroll && GetColumnWidth( 0 ) < maxwidth ) {
		SetColumnWidth( 0, maxwidth );
		}
	}

void CTreeListControl::ExpandItem( _In_ _In_range_( 0, INT_MAX ) const INT_PTR i, _In_ const bool scroll ) {
	auto item = GetItem( i );
	if ( item == NULL ) {
		ASSERT( false );
		return;
		}
	if ( item->IsExpanded( ) ) {
		return;
		}

	CWaitCursor wc; // TODO: smart WaitCursor. In CollapseItem(), too.
	//LockWindowUpdate( );

	item->SortChildren( );

	ExpandItemInsertChildren( i, scroll, item );

	item->SetExpanded( true );
	item->SortChildren( );
	//UnlockWindowUpdate( );
	item->SortChildren( );
	RedrawItems( i, i );

	if ( scroll ) {
		// Scroll up so far, that i is still visible and the first child becomes visible, if possible.
		if ( item->GetChildrenCount( ) > 0 ) {
			EnsureVisible( i + 1, false );
			}
		EnsureVisible( i, false );
		}
	}

void CTreeListControl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) {
	auto i = GetNextItem( -1, LVNI_FOCUSED );
	if ( i != -1 ) {
		auto item = GetItem( i );
		if ( item != NULL ) {
			auto itemParent = item->GetParent( ) != NULL;
			switch ( nChar ) {
				case VK_LEFT:
					if ( item->IsExpanded( ) ) {
						CollapseItem( i );
						}
					else if ( itemParent != NULL ) {
						SelectItem( itemParent );
						}
					return;

				case VK_RIGHT:
					if ( !item->IsExpanded( ) ) {
						ExpandItem( i );
						}
					else if ( item->GetChildrenCount( ) > 0 ) {
						auto sortedItemAtZero = item->GetSortedChild( 0 );
						if ( sortedItemAtZero != NULL ){
							SelectItem( sortedItemAtZero );
							}
						}
					return;
				}
			}
		ASSERT( item != NULL );
		}
	COwnerDrawnListControl::OnKeyDown( nChar, nRepCnt, nFlags );
	}

_Pre_satisfies_( !isDone ) void CTreeListControl::OnChildAdded( _In_ const CTreeListItem* const parent, _In_ CTreeListItem* child, _In_ bool isDone ) {
	if ( !parent->IsVisible( ) ) {
		return;
		}
	auto p = FindTreeItem( parent );
	ASSERT( p != -1 );

	if ( parent->IsExpanded( ) ) {
		InsertItem( p + 1, child );
		if ( isDone ) {
			RedrawItems( p, p );
			Sort( );
			}
		}
	else {
		RedrawItems( p, p );
		}
	}

void CTreeListControl::Sort( ) {
	auto countItems = GetItemCount( );
	for ( INT i = 0; i < countItems; i++ ) {//convert to ranged for?
		auto Item = GetItem( i );
		if ( Item != NULL ) {
			if ( Item->IsExpanded( ) ) {
				Item->SortChildren( );
				}
			}
		ASSERT( Item != NULL );
		}
	COwnerDrawnListControl::SortItems( );
	}

void CTreeListControl::EnsureItemVisible( _In_ const CTreeListItem* item ) {
	if ( item == NULL ) {
		return;
		}
	auto i = FindTreeItem( item );
	if ( i == -1 ) {
		return;
		}
	EnsureVisible( i, false );
	}

void CTreeListControl::MeasureItem( PMEASUREITEMSTRUCT mis ) {
	mis->itemHeight = UINT( m_rowHeight );
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
