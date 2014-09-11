// sortinglistcontrol.cpp	- Implementation of CSortingListItem and CSortingListControl
//
// WinDirStat - Directory Statistics
// Copyright (C) 2003-2005 Bernhard Seifert
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
//#include ".\sortinglistcontrol.h"


//#include "windirstat.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////

CString CSortingListItem::GetText( _In_ const INT subitem ) const {
	// Dummy implementation
	CString s;
	s.Format( _T( "subitem %d" ), subitem );
	return s;
	}

INT CSortingListItem::Compare( _In_ const CSortingListItem *other, _In_ const INT subitem ) const {
/*
   Return value:
   <= -2:	this is less than other regardless of ascending flag
   -1:		this is less than other
   0:		this equals other
   +1:		this is greater than other
   >= +1:	this is greater than other regardless of ascending flag.
*/

	// Default implementation compares strings
	return signum( GetText( subitem ).CompareNoCase( other->GetText( subitem ) ) );
	}

INT CSortingListItem::CompareS( _In_ const CSortingListItem *other, _In_ const SSorting& sorting ) const {
	auto r = Compare( other, sorting.column1 );
	if ( abs( r ) < 2 && !sorting.ascending1 ) {
		r = -r;
		}
	
	if (r == 0 && sorting.column2 != sorting.column1) {
		r = Compare( other, sorting.column2 );
		if ( abs( r ) < 2 && !sorting.ascending2 ) {
			r = -r;
			}
		}
	return r;
	}

/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC( CSortingListControl, CListCtrl )

CSortingListControl::CSortingListControl( _In_z_ LPCTSTR name ) {
	m_name = name;
	m_indicatedColumn = -1;
	}

CSortingListControl::~CSortingListControl()
{
}

void CSortingListControl::LoadPersistentAttributes( ) {
	CArray<INT, INT> arr;
	arr.SetSize( GetHeaderCtrl( )->GetItemCount( ) );//Critical! else, we'll overrun the CArray in GetColumnOrderArray
	//arr.AssertValid( );
	auto arrSize = arr.GetSize( );

	auto res = GetColumnOrderArray( arr.GetData( ), arrSize );//TODO: BAD IMPLICIT CONVERSION HERE!!! BUGBUG FIXME
	ASSERT( res != 0 );
	CPersistence::GetColumnOrder( m_name, arr );
	auto res2 = SetColumnOrderArray( arrSize, arr.GetData( ) );//TODO: BAD IMPLICIT CONVERSION HERE!!! BUGBUG FIXME
	ASSERT( res2 != 0 );
	static_assert( sizeof( INT_PTR ) == sizeof( arrSize ), "Bad loop!" );
	for ( INT_PTR i = 0; i < arrSize; i++ ) {
		arr[ i ] = GetColumnWidth( i );
		}
	
	CPersistence::GetColumnWidths( m_name, arr );

	for ( INT_PTR i = 0; i < arrSize; i++ ) {
		// To avoid "insane" settings we set the column width to maximal twice the default width.
		auto maxWidth = GetColumnWidth( i ) * 2;
		
#pragma push_macro("min")
#undef min
		auto w = std::min( arr[ i ], maxWidth );
#pragma pop_macro("min")

		SetColumnWidth( i, w );
		}
	//arr.AssertValid( );
	// Not so good: CPersistence::GetSorting(m_name, GetHeaderCtrl()->GetItemCount(), m_sorting.column1, m_sorting.ascending1, m_sorting.column2, m_sorting.ascending2);
	// We refrain from saving the sorting because it is too likely, that users start up with insane settings and don't get it.
	}

void CSortingListControl::SavePersistentAttributes( ) {
	CArray<INT, INT> arr;
	//arr.AssertValid( );
	arr.SetSize( GetHeaderCtrl( )->GetItemCount( ) );//Critical! else, we'll overrun the CArray in GetColumnOrderArray

	auto res = GetColumnOrderArray( arr.GetData( ), arr.GetSize( ) );//TODO: BAD IMPLICIT CONVERSION HERE!!! BUGBUG FIXME
	ASSERT( res != 0 );
	CPersistence::SetColumnOrder( m_name, arr );

	for ( INT_PTR i = 0; i < arr.GetSize( ); i++ ) {
		arr[ i ] = GetColumnWidth( i );
		}
	CPersistence::SetColumnWidths( m_name, arr );
	//arr.AssertValid( );
	// Not so good: CPersistence::SetSorting(m_name, m_sorting.column1, m_sorting.ascending1, m_sorting.column2, m_sorting.ascending2);
	}

void CSortingListControl::AddExtendedStyle( _In_ const DWORD exStyle ) {
	SetExtendedStyle( GetExtendedStyle( ) | exStyle );
	}

void CSortingListControl::RemoveExtendedStyle( _In_ const DWORD exStyle ) {
	SetExtendedStyle( GetExtendedStyle( ) & ~exStyle );
	}

const SSorting& CSortingListControl::GetSorting( ) const {
	return m_sorting;
	}

void CSortingListControl::SetSorting( _In_ const SSorting& sorting ) {
	m_sorting = sorting;
	}

void CSortingListControl::SetSorting( _In_ const INT sortColumn1, _In_ const bool ascending1, _In_ const INT sortColumn2, _In_ const bool ascending2 ) {
	m_sorting.column1    = std::int8_t( sortColumn1 );
	m_sorting.ascending1 = ascending1;
	m_sorting.column2    = std::int8_t( sortColumn2 );
	m_sorting.ascending2 = ascending2;
	}

void CSortingListControl::SetSorting( _In_ const INT sortColumn, _In_ const bool ascending ) {
	
	m_sorting.ascending2 = m_sorting.ascending1;
	m_sorting.column1    = std::int8_t( sortColumn );
	m_sorting.column2    = m_sorting.column1;
	m_sorting.ascending1 = ascending;
	}

void CSortingListControl::InsertListItem( _In_ const INT i, _In_ const CSortingListItem* item ) {
	auto lvitem = partInitLVITEM( );

	lvitem.mask = LVIF_TEXT | LVIF_PARAM;
	//if ( HasImages( ) ) {//HasImages( ) == false, unconditionally
	//	lvitem.mask |= LVIF_IMAGE;
	//	}

	lvitem.iItem   = i;
	lvitem.pszText = LPSTR_TEXTCALLBACK;
	lvitem.iImage  = I_IMAGECALLBACK;
	lvitem.lParam  = reinterpret_cast< LPARAM >( item );

	VERIFY( i == CListCtrl::InsertItem( &lvitem ) );//int CListCtrl::InsertItem(_In_ const LVITEM* pItem) {
	                                                //    ASSERT(::IsWindow(m_hWnd));
	                                                //    return (int)
	                                                //    ::SendMessage(m_hWnd, LVM_INSERTITEM, 0, (LPARAM)pItem); //<<---------This is the slow part!
	                                                //}
	}

_Must_inspect_result_ CSortingListItem *CSortingListControl::GetSortingListItem( _In_ const INT i ) {
	return ( CSortingListItem * ) GetItemData( i );
	}

void CSortingListControl::SortItems( ) {
	VERIFY( CListCtrl::SortItems( &_CompareFunc, ( DWORD_PTR ) &m_sorting ) );
	auto hditem =  zeroInitHDITEM( );

	auto thisHeaderCtrl = GetHeaderCtrl( );
	CString text;
	hditem.mask       = HDI_TEXT;
	hditem.pszText    = text.GetBuffer( 260 );//http://msdn.microsoft.com/en-us/library/windows/desktop/bb775247(v=vs.85).aspx specifies 260
	hditem.cchTextMax = 260;

	if ( m_indicatedColumn != -1 ) {		
		thisHeaderCtrl->GetItem( m_indicatedColumn, &hditem );
		text.ReleaseBuffer( );
		text           = text.Mid( 2 );
		hditem.pszText = text.GetBuffer( 260 );
		thisHeaderCtrl->SetItem( m_indicatedColumn, &hditem );
		text.ReleaseBuffer( );
		}

	hditem.pszText = text.GetBuffer( 260 );
	thisHeaderCtrl->GetItem( m_sorting.column1, &hditem );
	text.ReleaseBuffer( );
	text = ( m_sorting.ascending1 ? _T( "< " ) : _T( "> " ) ) + text;
	hditem.pszText = text.GetBuffer( 260 );
	thisHeaderCtrl->SetItem( m_sorting.column1, &hditem );
	m_indicatedColumn = m_sorting.column1;
	}

INT CALLBACK CSortingListControl::_CompareFunc( _In_ const LPARAM lParam1, _In_ const LPARAM lParam2, _In_ const LPARAM lParamSort ) {
	auto item1 = reinterpret_cast< const CSortingListItem*>( lParam1 );
	auto item2 = reinterpret_cast< const CSortingListItem*>( lParam2 );
	auto sorting       = reinterpret_cast<const SSorting*>( lParamSort );
#ifdef DEBUG
	auto result = item1->CompareS( item2, *sorting );
	return result;
#else
	return item1->CompareS( item2, *sorting );
#endif
	}

BEGIN_MESSAGE_MAP(CSortingListControl, CListCtrl)
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnLvnGetdispinfo)
	ON_NOTIFY(HDN_ITEMCLICKA, 0, OnHdnItemclick)
	ON_NOTIFY(HDN_ITEMCLICKW, 0, OnHdnItemclick)
	ON_NOTIFY(HDN_ITEMDBLCLICKA, 0, OnHdnItemdblclick)
	ON_NOTIFY(HDN_ITEMDBLCLICKW, 0, OnHdnItemdblclick)
	ON_WM_DESTROY()
END_MESSAGE_MAP()

void CSortingListControl::OnLvnGetdispinfo( NMHDR *pNMHDR, LRESULT *pResult ) {
	static_assert( sizeof( NMHDR* ) == sizeof( NMLVDISPINFO* ), "some size issues. Good luck with that cast!" );
	ASSERT( ( pNMHDR != NULL ) && ( pResult != NULL ) );
	auto di = reinterpret_cast< NMLVDISPINFO* >( pNMHDR );
	*pResult = 0;
	auto item = reinterpret_cast<CSortingListItem*>( di->item.lParam );
	ASSERT( item != NULL );
	if ( item != NULL ) {
		if ( ( di->item.mask & LVIF_TEXT ) != 0 ) {
			auto ret = lstrcpyn( di->item.pszText, item->GetText( di->item.iSubItem ), di->item.cchTextMax ); //BUGBUG TODO FIXME AHHHHH lstrcpyn is security liability!
			ASSERT( ret != NULL );
			if ( ret == NULL ) {
				AfxCheckMemory( );
				AfxMessageBox( _T( "lstrcpyn returned NULL!!!!" ), 0, 0 );
				throw 666;
				}
			}

		if ( ( di->item.mask & LVIF_IMAGE ) != 0 ) {
			di->item.iImage = item->GetImage( );
			}
		}
	ASSERT( item != NULL );
	}

void CSortingListControl::OnHdnItemclick( NMHDR *pNMHDR, LRESULT *pResult ) {
	auto phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);
	*pResult = 0;
	auto col = phdr->iItem;
	if ( col == m_sorting.column1 ) {
		m_sorting.ascending1 =  ! m_sorting.ascending1;
		}
	else {
		SetSorting( col, true ); //GetAscendingDefault( col ) == true, unconditionally
		}
	SortItems( );
	}

void CSortingListControl::OnHdnItemdblclick( NMHDR *pNMHDR, LRESULT *pResult ) {
	OnHdnItemclick( pNMHDR, pResult );
	}

void CSortingListControl::OnDestroy( ) {
	SavePersistentAttributes();
	CListCtrl::OnDestroy();
	}

// $Log$
// Revision 1.5  2005/04/10 16:49:30  assarbad
// - Some smaller fixes including moving the resource string version into the rc2 files
//
// Revision 1.4  2004/12/31 16:01:42  bseifert
// Bugfixes. See changelog 2004-12-31.
//
// Revision 1.3  2004/11/05 16:53:07  assarbad
// Added Date and History tag where appropriate.
//
