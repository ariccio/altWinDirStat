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
#include "windirstat.h"
#include ".\treelistcontrol.h"

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
	const UINT NODE_WIDTH = 15;		// Width of a node within IDB_NODES 
	const UINT NODE_HEIGHT = 24;	// Height of IDB_NODES
	const UINT INDENT_WIDTH = 18;

	const UINT HOTNODE_CX = 9;		// Size and position of the +/- buttons
	const UINT HOTNODE_CY = 9;
	const UINT HOTNODE_X = 0;

}

CTreeListItem::CTreeListItem( ) {
	m_parent = NULL;
	m_vi = NULL;
	}

CTreeListItem::~CTreeListItem( ) {
	if ( m_vi != NULL && ( !IsVisible( ) ) ) {
		delete m_vi;
		m_vi = NULL;
		}
	if ( m_parent != NULL ) {
		//delete m_parent;//causes stack overflow! (understandably)
		}
	if ( m_vi != NULL ) {
		delete m_vi;
		m_vi = NULL;
		}
	}

bool CTreeListItem::DrawSubitem( _In_ const INT subitem, _In_ CDC *pdc, _In_ CRect rc, _In_ const UINT state, _Inout_ INT *width, _Inout_ INT *focusLeft ) const {
	ASSERT_VALID( pdc );
	ASSERT( focusLeft != NULL );
	ASSERT( &width != NULL );
	ASSERT( &focusLeft != NULL );
	ASSERT( subitem >= 0 );

	if ( subitem != 0 ) {
		return false;
		}

	CRect rcNode = rc;
	CRect rcPlusMinus;
	GetTreeListControl( )->DrawNode( pdc, rcNode, rcPlusMinus, this, width );//pass subitem to drawNode? 
	CRect rcLabel = rc;
	rcLabel.left = rcNode.right;
	auto TreeListControl = GetTreeListControl( );
	auto MyImageList = GetMyImageList( );
	if ( ( TreeListControl != NULL ) && ( MyImageList != NULL ) ) {
		DrawLabel( GetTreeListControl( ), GetMyImageList( ), pdc, rcLabel, state, width, focusLeft, false );
		}
	else {
		ASSERT( false );
		}
	if ( width != NULL ) {
		*width = rcLabel.right - rc.left;
		}
	else {
		SetPlusMinusRect( rcPlusMinus - rc.TopLeft( ) );
		SetTitleRect( rcLabel - rc.TopLeft( ) );
		}
	return true;
	}

CString CTreeListItem::GetText( _In_ const INT /*subitem*/ ) const {
	return _T("test"); 
	}

INT CTreeListItem::GetImage( ) const {
	ASSERT( IsVisible( ) );
	if ( m_vi->image == -1 ) {
		m_vi->image = GetImageToCache( );
		}
	return m_vi->image;
	}

void CTreeListItem::DrawPacman( _In_ CDC *pdc, _In_ const CRect& rc, _In_ const COLORREF bgColor ) const {
	ASSERT_VALID( pdc );
	ASSERT( IsVisible( ) );
	m_vi->pacman.SetBackgroundColor( bgColor );
	m_vi->pacman.Draw( pdc, rc );
	}

void CTreeListItem::StartPacman( _In_ const bool start ) {
	if ( IsVisible( ) ) {
#ifdef _DEBUG
		//these are REALLY noisy.
		if ( start ) {
			//TRACE(_T("Starting a Pacman...\r\n") );
			}
		else {
			//TRACE( _T( "Stopping a Pacman...\r\n" ) );
			}
#endif
		m_vi->pacman.Start( start );
		}
	}

bool CTreeListItem::DrivePacman( _In_ const LONGLONG readJobs ) {
	if ( !IsVisible( ) ) {
		return false;
		}
#ifdef _DEBUG
	bool ret = m_vi->pacman.Drive( readJobs );
	//TRACE( _T( "DrivePacman returning readJobs: %lld, bool: %d\r\n" ), readJobs, ret );
	return ret;
#else
	return m_vi->pacman.Drive( readJobs );
#endif
	}

INT CTreeListItem::GetScrollPosition( ) {
	auto TreeListControl = GetTreeListControl( );
	if ( TreeListControl != NULL ) {
		return TreeListControl->GetItemScrollPosition( this );
		}
	else {
		ASSERT( false );//BADBADBAD
		return -1;
		}
	}

void CTreeListItem::SetScrollPosition( _In_ const int top ) {
	ASSERT( top >= 0 );
	auto TreeListControl = GetTreeListControl( );
	if ( TreeListControl != NULL ) {
		TreeListControl->SetItemScrollPosition( this, top );
		}
	else {
		ASSERT( false );
		}
	}

void CTreeListItem::UncacheImage( ) {
	if ( IsVisible( ) ) {
		m_vi->image = -1;
		}
	}

void CTreeListItem::SortChildren( ) {
	ASSERT( IsVisible( ) );
	m_vi->sortedChildren.SetSize( GetChildrenCount( ) );
	auto childCount = GetChildrenCount( );
	for ( int i = 0; i < childCount; i++ ) {
		auto aTreeListChild = GetTreeListChild( i );
		if ( aTreeListChild != NULL ) {
			m_vi->sortedChildren[ i ] = aTreeListChild;
			}
		else {
			ASSERT( false );
			}
		}
	qsort( m_vi->sortedChildren.GetData( ), m_vi->sortedChildren.GetSize( ), sizeof( CTreeListItem * ), &_compareProc );
	}

INT __cdecl CTreeListItem::_compareProc( _In_ const void *p1, _In_ const void *p2 ) {
	CTreeListItem *item1 = *( CTreeListItem ** ) p1;
	CTreeListItem *item2 = *( CTreeListItem ** ) p2;
	return item1->CompareS( item2, GetTreeListControl( )->GetSorting( ) );
	}

_Must_inspect_result_ CTreeListItem *CTreeListItem::GetSortedChild( _In_ const INT i ) {
	ASSERT( i >= 0 );
	ASSERT( !( m_vi->sortedChildren.IsEmpty( ) ) );
	return m_vi->sortedChildren[ i ];
	}

INT CTreeListItem::Compare( _In_ const CSortingListItem *baseOther, _In_ const INT subitem ) const {
	VERIFY( baseOther);
	auto other = ( CTreeListItem * ) baseOther;

	if ( other == NULL ) {
		return 666;
		}

	if ( other == this ) {
		return 0;
		}

	if ( m_parent == other->m_parent ) {
		return CompareSibling( other, subitem );
		}

	if ( m_parent == NULL ) {
		return -2;
		}
	
	if ( other->m_parent == NULL ) {
		return 2;
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

INT CTreeListItem::FindSortedChild(const CTreeListItem *child) 
{ 
	auto childCount = GetChildrenCount( );
	for ( int i = 0; i < childCount; i++ ) {
		if ( child == GetSortedChild( i ) ) {
			return i;
			}
		}
	
	ASSERT(false); 
	return 0; 
}
_Must_inspect_result_ CTreeListItem *CTreeListItem::GetParent() const
{ 
	if (this == NULL || m_parent == NULL ) {
		return NULL;
		}
	else if ( m_parent != NULL ) {
		return m_parent;
		}
	ASSERT( false );
	return NULL;
}
void CTreeListItem::SetParent(_In_ CTreeListItem *parent) 
{ 
	m_parent= parent; 
}
bool CTreeListItem::HasSiblings() const
{ 
	if ( m_parent == NULL ) {
		return false;
		}
	auto i = m_parent->FindSortedChild( this );
	return i < m_parent->GetChildrenCount( ) - 1;
}
bool CTreeListItem::HasChildren() const
{ 
	return GetChildrenCount( ) > 0;
}
bool CTreeListItem::IsExpanded() const
{
	ASSERT(IsVisible());
	return m_vi->isExpanded; 
}
void CTreeListItem::SetExpanded( _In_ const bool expanded )
{ 
	ASSERT( IsVisible( ) );
	m_vi->isExpanded = expanded;
}
bool CTreeListItem::IsVisible() const
{
	return (m_vi != NULL);
}
void CTreeListItem::SetVisible( _In_ const bool visible )
{ 
	if ( visible ) {
		ASSERT( !IsVisible( ) );
		m_vi = new VISIBLEINFO;
		if ( GetParent( ) == NULL ) {
			m_vi->indent = 0;
			}
		else {
			m_vi->indent = GetParent( )->GetIndent( ) + 1;
			}
		m_vi->image = -1;
		m_vi->isExpanded = false;
		}
	else {
		ASSERT( IsVisible( ) );
		if ( m_vi != NULL ) {
			delete m_vi;
			m_vi = NULL;
			}
		}
}
INT CTreeListItem::GetIndent() const
{
	ASSERT( IsVisible( ) );
	return m_vi->indent;
}
CRect CTreeListItem::GetPlusMinusRect() const
{ 
	ASSERT( IsVisible( ) );
	return m_vi->rcPlusMinus; 
}
void CTreeListItem::SetPlusMinusRect(_In_ const CRect& rc) const
{ 
	ASSERT( IsVisible( ) );
	m_vi->rcPlusMinus = rc;
}
CRect CTreeListItem::GetTitleRect() const
{ 
	ASSERT( IsVisible( ) );
	return m_vi->rcTitle; 
}
void CTreeListItem::SetTitleRect(_In_ const CRect& rc) const
{
	ASSERT( IsVisible( ) );
	m_vi->rcTitle = rc;
}

_Must_inspect_result_ CTreeListControl *CTreeListItem::GetTreeListControl()
{
	// As we only have 1 TreeListControl and want to economize memory, we simple made the TreeListControl global.
	return CTreeListControl::GetTheTreeListControl( );
}


/////////////////////////////////////////////////////////////////////////////
// CTreeListControl

CTreeListControl *CTreeListControl::_theTreeListControl;

_Must_inspect_result_ CTreeListControl *CTreeListControl::GetTheTreeListControl()
{
	ASSERT(_theTreeListControl != NULL);
	return _theTreeListControl;
}


IMPLEMENT_DYNAMIC(CTreeListControl, COwnerDrawnListControl)

CTreeListControl::CTreeListControl(INT rowHeight)
: COwnerDrawnListControl(_T("treelist"), rowHeight)
{
	ASSERT(_theTreeListControl == NULL);
	_theTreeListControl = this;

	ASSERT(rowHeight <= NODE_HEIGHT);	// größer können wir nicht
	ASSERT(rowHeight % 2 == 0);			// muss gerade sein
}

CTreeListControl::~CTreeListControl()
{
}

bool CTreeListControl::HasImages( ) const
{
	return true;
}
void CTreeListControl::MySetImageList( _In_opt_ CImageList *il)
{
	m_imageList = il;
}

void CTreeListControl::SelectItem( _In_ const INT i )
{
	SetItemState(i, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
	EnsureVisible(i, false);
}

INT CTreeListControl::GetSelectedItem( ) const
{
	POSITION pos= GetFirstSelectedItemPosition();
	if ( pos == NULL ) {
		return -1;
		}
	return GetNextSelectedItem(pos);
}

void CTreeListControl::SelectItem(_In_ const CTreeListItem *item)
{
	auto i = FindTreeItem( item );
	if ( i != -1 ) {
		SelectItem( i );
		}
}

BOOL CTreeListControl::CreateEx( _In_ const DWORD dwExStyle, _In_ DWORD dwStyle, _In_ const RECT& rect, _In_ CWnd* pParentWnd, _In_ const UINT nID )
{
	InitializeNodeBitmaps( );

	dwStyle |= LVS_OWNERDRAWFIXED | LVS_SINGLESEL;
	VERIFY( COwnerDrawnListControl::CreateEx( dwExStyle, dwStyle, rect, pParentWnd, nID ) );
	return true;
}

void CTreeListControl::SysColorChanged()
{
	COwnerDrawnListControl::SysColorChanged();
	InitializeNodeBitmaps();
}

_Must_inspect_result_ CTreeListItem *CTreeListControl::GetItem( _In_ const INT i )
{
	CTreeListItem *item = ( CTreeListItem * ) GetItemData( i );
	return item;
}

void CTreeListControl::SetRootItem( _In_opt_ CTreeListItem *root)
{
	DeleteAllItems( );
	if ( root != NULL ) {
		InsertItem( 0, root );
		ExpandItem( 0 );
		}
}

void CTreeListControl::DeselectAll()
{
	for ( int i = 0; i < GetItemCount( ); i++ ) {
		SetItemState( i, 0, LVIS_SELECTED );
		}
}

void CTreeListControl::SelectAndShowItem( _In_ const CTreeListItem *item, _In_ const bool showWholePath )
{
	if ( item == NULL ) {
		ASSERT( false );
		return;
		}
	CArray<const CTreeListItem *, const CTreeListItem *> path;
	const CTreeListItem *p = item;
	while ( p != NULL ) {
		path.Add( p );
#ifdef _DEBUG
		path.AssertValid( );
#endif
		p = p->GetParent( );
		}
	auto parent = 0;
	for ( auto i = ( path.GetUpperBound( ) - 1 ); i >= 0; --i ) {
		ASSERT( i <= path.GetUpperBound( ) );
		auto index = FindTreeItem( path[ i ] );
		if ( index == -1 ) {
			ASSERT( i <= path.GetUpperBound( ) );
			ExpandItem( i, false );
			index = FindTreeItem( path[ i ] );
			}
		else {
			for ( auto k = parent + 1; k < index; k++ ) {
				CollapseItem( k );
				index = FindTreeItem( path[ i ] );
				ASSERT( index >= 0 );
				}
			}
		parent = index;
		auto j = FindTreeItem( path[ 0 ] );
		if ( j == -1 ) {
			ASSERT( i < path.GetUpperBound( ) );
			ExpandItem( parent, false );
			j = FindTreeItem( path[ 0 ] );
		}
		auto w = GetSubItemWidth( GetItem( index ), 0 ) + 5;
		if ( GetColumnWidth( 0 ) < w ) {
			SetColumnWidth( 0, w );
			}
		if ( showWholePath ) {
			EnsureVisible( 0, false );
			}
		SelectItem( index );
		}
}

void CTreeListControl::OnItemDoubleClick( _In_ const INT i )
{
	ToggleExpansion( i );
}

void CTreeListControl::InitializeNodeBitmaps( ) {
	m_bmNodes0.DeleteObject( );
	m_bmNodes1.DeleteObject( );

	COLORMAP cm[1] = {	{ RGB(255,0,255), 0 } };
	
	cm[ 0 ].to = GetWindowColor( );
	VERIFY( m_bmNodes0.LoadMappedBitmap( IDB_NODES, 0, cm, 1 ) );
	cm[ 0 ].to = GetStripeColor( );
	VERIFY( m_bmNodes1.LoadMappedBitmap( IDB_NODES, 0, cm, 1 ) );
	}

void CTreeListControl::InsertItem( _In_ const INT i, _In_ CTreeListItem *item ) {
	COwnerDrawnListControl::InsertListItem( i, item );
	item->SetVisible( true );
	}

void CTreeListControl::DeleteItem( _In_ const INT i ) {
	ASSERT( i >= 0 );
	ASSERT( i < GetItemCount( ) );
	auto anItem = GetItem( i );
	if ( anItem != NULL ) {
		anItem->SetExpanded( false );
		anItem->SetVisible( false );
		}
	COwnerDrawnListControl::DeleteItem( i );
	}

INT CTreeListControl::FindTreeItem( _In_ const CTreeListItem *item ) {
	ASSERT( item != NULL );
	return COwnerDrawnListControl::FindListItem( item );
	}

BEGIN_MESSAGE_MAP(CTreeListControl, COwnerDrawnListControl)
	ON_WM_MEASUREITEM_REFLECT()
	ON_WM_LBUTTONDOWN()
	ON_WM_KEYDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_DESTROY()
END_MESSAGE_MAP()


void CTreeListControl::DrawNode( _In_ CDC *pdc, _In_ CRect& rc, _Inout_ CRect& rcPlusMinus, _In_ const CTreeListItem *item, _Inout_ INT *width ) {
	ASSERT_VALID( pdc );
	CRect rcRest = rc;
	bool didBitBlt = false;
	rcRest.left += GetGeneralLeftIndent( );
	if ( item->GetIndent( ) > 0 ) {
		rcRest.left += 3;
		CDC dcmem;
		dcmem.CreateCompatibleDC( pdc );
		CSelectObject sonodes( &dcmem, ( IsItemStripeColor( item ) ? &m_bmNodes1 : &m_bmNodes0 ) );
		auto ysrc = ( NODE_HEIGHT / 2 ) - ( GetRowHeight( ) / 2 );
		if ( width == NULL ) {
			auto ancestor = item;
			for ( auto indent = ( item->GetIndent( ) - 2 ); indent >= 0; indent-- ) {
				ancestor = ancestor->GetParent( );
				if ( ancestor->HasSiblings( ) ) {
					ASSERT_VALID( &dcmem );
					pdc->BitBlt( ( rcRest.left + indent * INDENT_WIDTH ), rcRest.top, NODE_WIDTH, NODE_HEIGHT, &dcmem, ( NODE_WIDTH * NODE_LINE ), ysrc, SRCCOPY );
					didBitBlt = true;
					}
				}
			}
		rcRest.left += ( item->GetIndent( ) - 1 ) * INDENT_WIDTH;
		if ( width == NULL ) {
			auto node = -1;
			if ( item->HasChildren( ) ) {
				if ( item->HasSiblings( ) ) {
					if ( item->IsExpanded( ) ) {
						node = NODE_MINUS_SIBLING;
						}
					else {
						node = NODE_PLUS_SIBLING;
						}
					}
				else {
					if ( item->IsExpanded( ) ) {
						node = NODE_MINUS_END;
						}
					else {
						node = NODE_PLUS_END;
						}
					}
				}
			else {
				if ( item->HasSiblings( ) ) {
					node = NODE_SIBLING;
					}
				else {
					node = NODE_END;
					}
				}
			ASSERT_VALID( &dcmem );
			if ( !didBitBlt ) {//Else we'd double BitBlt?
				pdc->BitBlt( rcRest.left, rcRest.top, NODE_WIDTH, NODE_HEIGHT, &dcmem, ( NODE_WIDTH * node ), ysrc, SRCCOPY );
				}
			rcPlusMinus.left    = rcRest.left      + HOTNODE_X;
			rcPlusMinus.right   = rcPlusMinus.left + HOTNODE_CX;
			rcPlusMinus.top     = rcRest.top       + ( rcRest.bottom - rcRest.top )/ 2 - HOTNODE_CY / 2 - 1;
			rcPlusMinus.bottom  = rcPlusMinus.top  + HOTNODE_CY;
			}
		rcRest.left += NODE_WIDTH;
	}
	rc.right = rcRest.left;
	if ( width != NULL ) {
		*width = rc.Width( );
		}
	}

void CTreeListControl::OnLButtonDown( UINT nFlags, CPoint point ) {
	m_lButtonDownItem = -1;

	LVHITTESTINFO hti = zeroInitLVHITTESTINFO( );

	hti.pt = point;

	int i = HitTest( &hti );
	if ( i == -1 ) {
		return;
		}

	if ( hti.iSubItem != 0 ) {
		COwnerDrawnListControl::OnLButtonDown( nFlags, point );
		return;
		}

	CRect rc = GetWholeSubitemRect( i, 0 );
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
	else {
		ASSERT( false );
		}
	}

void CTreeListControl::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	COwnerDrawnListControl::OnLButtonDblClk( nFlags, point );

	if ( m_lButtonDownItem == -1 ) {
		return;
		}

	if (m_lButtonDownOnPlusMinusRect) {
		ToggleExpansion( m_lButtonDownItem );
		}
	else {
		OnItemDoubleClick( m_lButtonDownItem );
		}
}

void CTreeListControl::ToggleExpansion( _In_ const INT i )
{
	if ( GetItem( i )->IsExpanded( ) ) {
		CollapseItem( i );
		}
	else {
		ExpandItem( i );
		}
}

void CTreeListControl::CollapseItem( _In_ const INT i ) {
	auto item = GetItem( i );
	if ( item != NULL ) {
		if ( !item->IsExpanded( ) ) {
			return;
			}
		}
	else {
		ASSERT( false );
		}
	CWaitCursor wc;
	LockWindowUpdate( );
	bool selectNode = false;
	int todelete = 0;
	for ( int k = i + 1; k < GetItemCount( ); k++ ) {
		auto child = GetItem( k );
		if ( ( item != NULL ) && ( child != NULL ) ) {
			if ( child->GetIndent( ) <= item->GetIndent( ) ) {
				break;
				}
			}
		else {
			ASSERT( false );
			}
		if ( GetItemState( k, LVIS_SELECTED ) == LVIS_SELECTED ) {
			selectNode = true;
			}
		todelete++;
		}
	for ( int m = 0; m < todelete; m++ ) {
		DeleteItem( i + 1 );
		}
	if ( item != NULL ) {
		item->SetExpanded( false );
		}
	else {
		ASSERT( false );
		}
	if ( selectNode ) {
		SelectItem( i );
		}
	UnlockWindowUpdate( );
	RedrawItems( i, i );
	}

INT CTreeListControl::GetItemScrollPosition(_In_ CTreeListItem *item) {
	CRect rc;
	rc.bottom = NULL;
	rc.left = NULL;
	rc.right = NULL;
	rc.top = NULL;
	VERIFY( GetItemRect( FindTreeItem( item ), rc, LVIR_BOUNDS ) );
	return rc.top;
	}

void CTreeListControl::SetItemScrollPosition( _In_ CTreeListItem *item, _In_ const INT top )
{
	auto old = GetItemScrollPosition( item );
	Scroll( CSize( 0, top - old ) );
}

bool CTreeListControl::SelectedItemCanToggle( ) {
	auto i = GetSelectedItem( );
	if ( i == -1 ) {
		return false;
		}
	const CTreeListItem *item = GetItem( i );
	if ( item != NULL ) {
		return item->HasChildren( );
		}
	else {
		ASSERT( false );
		return false;
		}
	}

void CTreeListControl::ToggleSelectedItem( ) {
	auto i = GetSelectedItem( );
	ASSERT( i != -1 );
	ToggleExpansion( i );
	}

void CTreeListControl::ExpandItem( _In_ CTreeListItem *item ) {
	ExpandItem( FindTreeItem( item ), false );
	}

void CTreeListControl::ExpandItem( _In_ const INT i, _In_ const bool scroll ) {
	CTreeListItem *item = GetItem( i );
	if ( item == NULL ) {
		return;
		}
	if ( item->IsExpanded( ) ) {
		return;
		}

	CWaitCursor wc; // TODO: smart WaitCursor. In CollapseItem(), too.
	LockWindowUpdate( );

	item->SortChildren( );

	auto maxwidth = GetSubItemWidth( item, 0 );
	auto count = item->GetChildrenCount();
	auto myCount = GetItemCount( );
	SetItemCount( ( count >= myCount) ? count : myCount );
	
	for ( int c = 0; c < item->GetChildrenCount( ); c++ ) {
		CTreeListItem *child = item->GetSortedChild( c );//m_vi->sortedChildren[i];
		InsertItem( i + 1 + c, child );
		if ( scroll ) {
			auto w = GetSubItemWidth( child, 0 );
			if ( w > maxwidth ) {
				maxwidth = w;
				}
			}
		}

	if ( scroll && GetColumnWidth( 0 ) < maxwidth ) {
		SetColumnWidth( 0, maxwidth );
		}

	item->SetExpanded( true );
	UnlockWindowUpdate( );
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
	int i = GetNextItem( -1, LVNI_FOCUSED );
	if ( i != -1 ) {
		CTreeListItem *item = GetItem( i );
		if ( item != NULL ) {
			switch ( nChar ) {
					case VK_LEFT:
						if ( item->IsExpanded( ) ) {
							CollapseItem( i );
							}
						else if ( item->GetParent( ) != NULL ) {
							SelectItem( item->GetParent( ) );
							}
						return;

					case VK_RIGHT:
						if ( !item->IsExpanded( ) ) {
							ExpandItem( i );
							}
						else if ( item->GetChildrenCount( ) > 0 ) {
							SelectItem( item->GetSortedChild( 0 ) );
							}
						return;
				}
			}
		else {
			ASSERT( false );
			}
	}
	COwnerDrawnListControl::OnKeyDown( nChar, nRepCnt, nFlags );
	}

void CTreeListControl::OnChildAdded( _In_ CTreeListItem *parent, _In_ CTreeListItem *child ) {
	if ( !parent->IsVisible( ) ) {
		//TRACE( _T("Child added, but parent not visible!\r\n" ) );
		return;
		}
	auto p = FindTreeItem( parent );
	ASSERT( p != -1 );

	if ( parent->IsExpanded( ) ) {
		InsertItem( p + 1, child );
		RedrawItems( p, p );
		Sort( );
		}
	else {
		RedrawItems( p, p );
		}
	}

void CTreeListControl::OnChildRemoved(_In_ CTreeListItem *parent, _In_ CTreeListItem *child)
{
	if ( !parent->IsVisible( ) ) {
		return;
		}

	auto p = FindTreeItem( parent );
	ASSERT( p != -1 );

	if ( parent->IsExpanded( ) ) {
		for ( auto i = 0; i < child->GetChildrenCount( ); i++ ) {//MAYBE I can pull GetChildrenCount out so compiler can vectorize
			OnChildRemoved( child, child->GetTreeListChild( i ) );
			}
		auto c = FindTreeItem( child );
		ASSERT( c != -1 );
		COwnerDrawnListControl::DeleteItem( c );
		parent->SortChildren( );
		}
	RedrawItems( p, p );
}

void CTreeListControl::OnRemovingAllChildren(_In_ CTreeListItem *parent)
{
	if ( !parent->IsVisible( ) ) {
		return;
		}
	
	auto p = FindTreeItem( parent );
	ASSERT( p != -1 );

	CollapseItem( p );
}

void CTreeListControl::Sort( ) {
	auto countItems = GetItemCount( );
	for ( int i = 0; i < countItems; i++ ) {//convert to ranged for?
		auto Item = GetItem( i );
		if ( Item != NULL ) {
			if ( Item->IsExpanded( ) ) {
				Item->SortChildren( );
				}
			}
		else {
			ASSERT( false );
			}
		}
	COwnerDrawnListControl::SortItems( );
	}

void CTreeListControl::EnsureItemVisible( _In_ const CTreeListItem *item ) {
	if ( item == NULL ) {
		return;
		}
	auto i = FindTreeItem( item );
	if ( i == -1 ) {
		return;
		}
	EnsureVisible( i, false );
	}

void CTreeListControl::MeasureItem( LPMEASUREITEMSTRUCT mis ) {
	mis->itemHeight = GetRowHeight( );
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
