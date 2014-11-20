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
#include ".\sortinglistcontrol.h"


//#include "windirstat.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


//INT CSortingListItem::CompareS( _In_ const COwnerDrawnListItem* const other, _In_ const SSorting& sorting ) const {
//	auto r = Compare( other, sorting.column1 );
//	if ( abs( r ) < 2 && !sorting.ascending1 ) {
//		r = -r;
//		}
//	
//	if (r == 0 && sorting.column2 != sorting.column1) {
//		r = Compare( other, sorting.column2 );
//		if ( abs( r ) < 2 && !sorting.ascending2 ) {
//			r = -r;
//			}
//		}
//	return r;
//	}

/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC( CSortingListControl, CListCtrl )

void CSortingListControl::LoadPersistentAttributes( ) {
	CArray<INT, INT> arr;
	arr.SetSize( GetHeaderCtrl( )->GetItemCount( ) );//Critical! else, we'll overrun the CArray in GetColumnOrderArray
	//arr.AssertValid( );
	auto arrSize = arr.GetSize( );

	auto res = GetColumnOrderArray( arr.GetData( ), static_cast<int>( arrSize ) );//TODO: BAD IMPLICIT CONVERSION HERE!!! BUGBUG FIXME
	ENSURE( res != 0 );
	CPersistence::GetColumnOrder( m_name, arr );
	auto res2 = SetColumnOrderArray( static_cast<int>( arrSize ), arr.GetData( ) );//TODO: BAD IMPLICIT CONVERSION HERE!!! BUGBUG FIXME
	ENSURE( res2 != 0 );
	static_assert( sizeof( INT_PTR ) == sizeof( arrSize ), "Bad loop!" );
	for ( INT_PTR i = 0; i < arrSize; i++ ) {
		arr[ i ] = GetColumnWidth( static_cast<int>( i ) );
		}
	
	CPersistence::GetColumnWidths( m_name, arr );

	for ( INT_PTR i = 0; i < arrSize; i++ ) {
		// To avoid "insane" settings we set the column width to maximal twice the default width.
		auto maxWidth = GetColumnWidth( static_cast<int>( i ) ) * 2;
		
#pragma push_macro("min")
#undef min
		auto w = std::min( arr[ i ], maxWidth );
#pragma pop_macro("min")

		SetColumnWidth( static_cast<int>( i ), w );
		}
	// Not so good: CPersistence::GetSorting(m_name, GetHeaderCtrl()->GetItemCount(), m_sorting.column1, m_sorting.ascending1, m_sorting.column2, m_sorting.ascending2);
	// We refrain from saving the sorting because it is too likely, that users start up with insane settings and don't get it.
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

BEGIN_MESSAGE_MAP(CSortingListControl, CListCtrl)
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnLvnGetdispinfo)
	ON_NOTIFY(HDN_ITEMCLICKA, 0, OnHdnItemclick)
	ON_NOTIFY(HDN_ITEMCLICKW, 0, OnHdnItemclick)
	ON_NOTIFY(HDN_ITEMDBLCLICKA, 0, OnHdnItemdblclick)
	ON_NOTIFY(HDN_ITEMDBLCLICKW, 0, OnHdnItemdblclick)
	ON_WM_DESTROY()
END_MESSAGE_MAP()

void CSortingListControl::OnLvnGetdispinfo( NMHDR *pNMHDR, LRESULT *pResult ) {
	static_assert( sizeof( NMHDR* ) == sizeof( NMLVDISPINFOW* ), "some size issues. Good luck with that cast!" );
	ASSERT( ( pNMHDR != NULL ) && ( pResult != NULL ) );
	auto di = reinterpret_cast< NMLVDISPINFOW* >( pNMHDR );
	*pResult = 0;
	auto item = reinterpret_cast<COwnerDrawnListItem*>( di->item.lParam );
	ASSERT( item != NULL );
	if ( item != NULL ) {
		if ( ( di->item.mask bitand LVIF_TEXT ) != 0 ) {

			auto ret = StringCchCopyW( di->item.pszText, static_cast<rsize_t>( di->item.cchTextMax ), item->GetText( di->item.iSubItem ).c_str( ) );
			if ( !( SUCCEEDED( ret ) ) ) {
				if ( ret == STRSAFE_E_INVALID_PARAMETER ) {
					//auto msgBxRet = ::MessageBoxW( NULL, _T( "STRSAFE_E_INVALID_PARAMETER" ), _T( "Error" ), MB_OK );
					displayWindowsMsgBoxWithMessage( std::move( std::wstring( L"STRSAFE_E_INVALID_PARAMETER" ) ) );
					}
				if ( ret == STRSAFE_E_INSUFFICIENT_BUFFER ) {
					//auto msgBxRet = ::MessageBoxW( NULL, _T( "STRSAFE_E_INSUFFICIENT_BUFFER" ), _T( "Error" ), MB_OK );
					displayWindowsMsgBoxWithMessage( std::move( std::wstring( L"STRSAFE_E_INSUFFICIENT_BUFFER" ) ) );
					}
				}
			}

		}
	ASSERT( item != NULL );
	}

void CSortingListControl::OnHdnItemclick( NMHDR *pNMHDR, LRESULT *pResult ) {
	const auto phdr = reinterpret_cast<LPNMHEADERW>(pNMHDR);
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
