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
#include "sortinglistcontrol.h"
#include "ownerdrawnlistcontrol.h"
#include "options.h"
#include "globalhelpers.h"

//#include "windirstat.h"
//#ifdef _DEBUG
//#define new DEBUG_NEW
//#endif


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


namespace {
	static INT CALLBACK _CompareFunc( _In_ const LPARAM lParam1, _In_ const LPARAM lParam2, _In_ const LPARAM lParamSort ) {
		const auto sorting = reinterpret_cast<const SSorting*>( lParamSort );
		return reinterpret_cast<const COwnerDrawnListItem*>( lParam1 )->CompareS( reinterpret_cast<const COwnerDrawnListItem*>( lParam2 ), *sorting );
		}

	}

/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC( CSortingListControl, CListCtrl )

void CSortingListControl::LoadPersistentAttributes( ) {
	
	
	const auto itemCount = static_cast<size_t>( GetHeaderCtrl( )->GetItemCount( ) );
	
#ifdef DEBUG
	CArray<INT, INT> arr;
	arr.SetSize( itemCount );//Critical! else, we'll overrun the CArray in GetColumnOrderArray
	TRACE( _T( "%s arr size set to: %i\r\n" ), m_name, static_cast<int>( itemCount ) );
	arr.AssertValid( );
	const auto arrSize = arr.GetSize( );
	ASSERT( arrSize == itemCount );
#endif

	const rsize_t countArray = 10;
	
	if ( countArray <= itemCount ) {
		TRACE( _T( "%i <= %i !!!! Something is REALLY wrong!!!\r\n" ), static_cast<int>( countArray ), static_cast<int>( itemCount ) );
		displayWindowsMsgBoxWithMessage( std::wstring( L"countArray <= itemCount !!!! Something is REALLY wrong!!!" ) );
		std::terminate( );
		}

	ASSERT( countArray > itemCount );
	
	INT fuck_CArray[ countArray ] = { 0 };

#ifdef DEBUG
	const auto res = GetColumnOrderArray( arr.GetData( ), itemCount );
	ENSURE( res != 0 );
#endif

	const auto res_2 = GetColumnOrderArray( fuck_CArray, itemCount );
	ENSURE( res_2 != 0 );
	CPersistence::GetColumnOrder( m_name, fuck_CArray, itemCount );

//#ifdef DEBUG
//	CPersistence::GetColumnOrder( m_name, arr );
//	ASSERT( arr.GetSize( ) == itemCount );
//	for ( size_t i = 0; i < itemCount; ++i ) {
//		ASSERT( arr[ i ] == fuck_CArray[ i ] );
//		}
//#endif

	const auto res2 = SetColumnOrderArray( itemCount, fuck_CArray );
	ENSURE( res2 != 0 );

	for ( size_t i = 0; i < itemCount; i++ ) {
		fuck_CArray[ i ] = GetColumnWidth( static_cast<int>( i ) );
		}
	CPersistence::GetColumnWidths( m_name, fuck_CArray, itemCount );
//#ifdef DEBUG
//	CPersistence::GetColumnWidths( m_name, arr );
//	ASSERT( arr.GetSize( ) == itemCount );
//	for ( size_t i = 0; i < itemCount; ++i ) {
//		ASSERT( arr[ i ] == fuck_CArray[ i ] );
//		}
//#endif

	for ( size_t i = 0; i < itemCount; i++ ) {
		// To avoid "insane" settings we set the column width to maximal twice the default width.
		const auto maxWidth = GetColumnWidth( static_cast<int>( i ) ) * 2;
		
#pragma push_macro("min")
#undef min
		//auto w = std::min( arr[ i ], maxWidth );
		const auto w = std::min( fuck_CArray[ i ], maxWidth );
#pragma pop_macro("min")

		VERIFY( SetColumnWidth( static_cast<int>( i ), w ) );
		}
	// Not so good: CPersistence::GetSorting(m_name, GetHeaderCtrl()->GetItemCount(), m_sorting.column1, m_sorting.ascending1, m_sorting.column2, m_sorting.ascending2);
	// We refrain from saving the sorting because it is too likely, that users start up with insane settings and don't get it.
	}

void CSortingListControl::AddExtendedStyle( _In_ const DWORD     exStyle ) {
	SetExtendedStyle( GetExtendedStyle( ) bitor exStyle );
	}

void CSortingListControl::RemoveExtendedStyle( _In_ const DWORD     exStyle ) {
	SetExtendedStyle( GetExtendedStyle( ) bitand ~exStyle );
	}

_Must_inspect_result_ COwnerDrawnListItem* CSortingListControl::GetSortingListItem( _In_ const INT i ) {
	return reinterpret_cast<COwnerDrawnListItem *>( GetItemData( i ) );
	}


void CSortingListControl::SortItems( ) {
	VERIFY( CListCtrl::SortItems( &_CompareFunc, reinterpret_cast<DWORD_PTR>( &m_sorting ) ) );
	auto hditem =  zeroInitHDITEM( );

	auto thisHeaderCtrl = GetHeaderCtrl( );
	CString text;
	hditem.mask       = HDI_TEXT;
	hditem.pszText    = text.GetBuffer( 260 );//http://msdn.microsoft.com/en-us/library/windows/desktop/bb775247(v=vs.85).aspx specifies 260
	hditem.cchTextMax = 260;

	if ( m_indicatedColumn != -1 ) {
		VERIFY( thisHeaderCtrl->GetItem( m_indicatedColumn, &hditem ) );
		text.ReleaseBuffer( );
		text           = text.Mid( 2 );
		hditem.pszText = text.GetBuffer( 260 );
		VERIFY( thisHeaderCtrl->SetItem( m_indicatedColumn, &hditem ) );
		text.ReleaseBuffer( );
		}

	hditem.pszText = text.GetBuffer( 260 );
	VERIFY( thisHeaderCtrl->GetItem( m_sorting.column1, &hditem ) );
	text.ReleaseBuffer( );
	text = ( m_sorting.ascending1 ? _T( "< " ) : _T( "> " ) ) + text;
	hditem.pszText = text.GetBuffer( 260 );
	VERIFY( thisHeaderCtrl->SetItem( m_sorting.column1, &hditem ) );
	m_indicatedColumn = m_sorting.column1;
	text.ReleaseBuffer( );
	}


void CSortingListControl::SavePersistentAttributes( ) {
	
	const rsize_t col_array_size = 128;
	int col_array[ col_array_size ] = { 0 };

	const auto itemCount = GetHeaderCtrl( )->GetItemCount( );

	ENSURE( itemCount < col_array_size );

	const auto get_res = GetColumnOrderArray( col_array, itemCount );

	ENSURE( get_res != 0 );

#ifdef DEBUG
	CArray<INT, INT> arr;
	arr.SetSize( GetHeaderCtrl( )->GetItemCount( ) );//Critical! else, we'll overrun the CArray in GetColumnOrderArray
	auto res = GetColumnOrderArray( arr.GetData( ), static_cast<int>( arr.GetSize( ) ) );//TODO: BAD IMPLICIT CONVERSION HERE!!! BUGBUG FIXME
	ENSURE( res != 0 );
	
	for ( int i = 0; i < arr.GetSize( ); ++i ) {
		ASSERT( arr[ i ] == col_array[ i ] );
		}
#endif

	//CPersistence::SetColumnOrder( m_name, arr );

	CPersistence::SetColumnOrder( m_name, col_array, itemCount );

	for ( INT_PTR i = 0; i < itemCount; i++ ) {
		col_array[ i ] = GetColumnWidth( static_cast<int>( i ) );
		}
	CPersistence::SetColumnWidths( m_name, col_array, itemCount );
	}


void CSortingListControl::InsertListItem( _In_ const INT_PTR i, _In_ const COwnerDrawnListItem* const item ) {
	auto lvitem = partInitLVITEM( );

	lvitem.mask = LVIF_TEXT | LVIF_PARAM;
	lvitem.iItem   = static_cast<int>( i );
	lvitem.pszText = LPSTR_TEXTCALLBACKW;
	lvitem.iImage  = I_IMAGECALLBACK;
	lvitem.lParam  = reinterpret_cast< LPARAM >( item );

	VERIFY( i == CListCtrl::InsertItem( &lvitem ) );

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

			auto ret = StringCchCopyW( di->item.pszText, static_cast<rsize_t>( di->item.cchTextMax ), item->GetText( static_cast<column::ENUM_COL>( di->item.iSubItem ) ).c_str( ) );
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
	const auto col = static_cast<column::ENUM_COL>( phdr->iItem );
	if ( col == m_sorting.column1 ) {
		m_sorting.ascending1 =  ! m_sorting.ascending1;
		}
	else {
		SetSorting( col, true ); //GetAscendingDefault( col ) == true, unconditionally
		}
	SortItems( );
	}

void CSortingListControl::OnDestroy( ) {
	SavePersistentAttributes( );
	CListCtrl::OnDestroy( );
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
