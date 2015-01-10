// OwnerDrawnListControl.cpp	- Implementation of COwnerDrawnListItem and COwnerDrawnListCtrl
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


#pragma once

#include "stdafx.h"

//#include "treemap.h"		// CColorSpace
#include "ownerdrawnlistcontrol.h"
#include "globalhelpers.h"
#include "options.h"


namespace {
	static INT CALLBACK _CompareFunc( _In_ const LPARAM lParam1, _In_ const LPARAM lParam2, _In_ const LPARAM lParamSort ) {
		const auto sorting = reinterpret_cast<const SSorting*>( lParamSort );
		return reinterpret_cast<const COwnerDrawnListItem*>( lParam1 )->CompareS( reinterpret_cast<const COwnerDrawnListItem*>( lParam2 ), *sorting );
		}

	}



INT COwnerDrawnListItem::CompareS( _In_ const COwnerDrawnListItem* const other, _In_ const SSorting& sorting ) const {
	if ( sorting.column1 == column::COL_NAME ) {
		const auto sort_result = signum( wcscmp( m_name, other->m_name ) );
		
		if ( sort_result != 0 ) {
			return sort_result;
			}
		}

	auto r_1 = compare_interface( other, sorting.column1 );
	if ( abs( r_1 ) < 2 && !sorting.ascending1 ) {
		r_1 = -r_1;
		}
	auto r_2 = r_1;

	if ( r_2 == 0 && sorting.column2 != sorting.column1 ) {
		r_2 = compare_interface( other, sorting.column2 );
		
		if ( abs( r_2 ) < 2 && !sorting.ascending2 ) {
			r_2 = -r_2;
			}
		}
	return r_2;
	}

void COwnerDrawnListItem::DrawHighlightSelectBackground( _In_ const CRect& rcLabel, _In_ const CRect& rc, _In_ const COwnerDrawnListCtrl* const list, _In_ CDC& pdc, _Inout_ COLORREF& textColor ) const {
	// Color for the text in a highlighted item (usually white)
	textColor = list->GetHighlightTextColor( );

	auto selection = rcLabel;
	// Depending on "FullRowSelection" style
	if ( list->m_showFullRowSelection ) {
		selection.right = rc.right;
		}
	// Fill the selection rectangle background (usually dark blue)
	pdc.FillSolidRect( selection, list->GetHighlightColor( ) );
	
	}

void COwnerDrawnListItem::AdjustLabelForMargin( _In_ const CRect& rcRest, _Inout_ CRect& rcLabel ) const {
	rcLabel.InflateRect( LABEL_INFLATE_CX, 0 );
	rcLabel.top    = rcRest.top + static_cast<LONG>( LABEL_Y_MARGIN );
	rcLabel.bottom = rcRest.bottom - static_cast<LONG>( LABEL_Y_MARGIN );
	}

void COwnerDrawnListItem::DrawLabel( _In_ COwnerDrawnListCtrl* const list, _In_ CDC& pdc, _In_ CRect& rc, _In_ const UINT state, _Out_opt_ INT* const width, _Inout_ INT* const focusLeft, _In_ const bool indent ) const {
	/*
	  Draws an item label (icon, text) in all parts of the WinDirStat view. The rest is drawn by DrawItem()
	  */

	const auto tRc = rc;
	auto rcRest = rc;
	// Increase indentation according to tree-level
	if ( indent ) {
		rcRest.left += GENERAL_INDENT;
		}

	CSelectObject sofont( pdc, *( list->GetFont( ) ) );

	rcRest.DeflateRect( TEXT_X_MARGIN, 0 );

	auto rcLabel = rcRest;
	pdc.DrawTextW( m_name, static_cast<int>( m_name_length ), rcLabel, DT_SINGLELINE | DT_VCENTER | DT_WORD_ELLIPSIS | DT_CALCRECT | DT_NOPREFIX | DT_NOCLIP );//DT_CALCRECT modifies rcLabel!!!

	AdjustLabelForMargin( rcRest, rcLabel );

	CSetBkMode bk( pdc, TRANSPARENT );
	auto textColor = GetSysColor( COLOR_WINDOWTEXT );

	if ( width == NULL && ( state bitand ODS_SELECTED ) != 0 && ( list->HasFocus( ) || list->IsShowSelectionAlways( ) ) ) {
		DrawHighlightSelectBackground( rcLabel, rc, list, pdc, textColor );
		}
	else {
		textColor = item_text_color( ); // Use the color designated for this item. This is currently only for encrypted and compressed items
		}

	// Set text color for device context
	CSetTextColor stc( pdc, textColor );

	if ( width == NULL ) {
		pdc.DrawTextW( m_name, static_cast<int>( m_name_length ), rcRest, DT_SINGLELINE | DT_VCENTER | DT_WORD_ELLIPSIS | DT_NOPREFIX | DT_NOCLIP );
		}

	rcLabel.InflateRect( 1, 1 );

	*focusLeft = rcLabel.left;

	if ( ( ( state bitand ODS_FOCUS ) != 0 ) && list->HasFocus( ) && ( width == NULL ) && ( !list->m_showFullRowSelection ) ) {
		pdc.DrawFocusRect( rcLabel );
		}


	rcLabel.left = rc.left;
	rc = rcLabel;
	if ( width != NULL ) {
		*width = ( rcLabel.Width( ) ) + 5; // +5 because GENERAL_INDENT?
		}
	}

void COwnerDrawnListItem::DrawSelection( _In_ const COwnerDrawnListCtrl* const list, _In_ CDC& pdc, _Inout_ CRect rc, _In_ const UINT state ) const {
	//ASSERT_VALID( pdc );//has already been verified by all callers!!
	ASSERT( list != NULL );
	if ( ( !list->m_showFullRowSelection ) ) {
		return;
		}
	if ( ( !list->HasFocus( ) ) && ( !list->IsShowSelectionAlways( ) ) ) {
		return;
		}
	if ( ( state bitand ODS_SELECTED ) == 0 ) {
		return;
		}

	rc.DeflateRect( 0, LABEL_Y_MARGIN );
	pdc.FillSolidRect( rc, list->GetHighlightColor( ) );
	}

/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC( COwnerDrawnListCtrl, CListCtrl )

COwnerDrawnListCtrl::COwnerDrawnListCtrl( _In_z_ PCWSTR name, _In_range_( 0, UINT_MAX ) const UINT rowHeight ) : m_persistent_name( name ), m_indicatedColumn( -1 ), m_rowHeight( rowHeight ), m_showGrid( false ), m_showStripes( false ), m_showFullRowSelection( false ) {
	ASSERT( rowHeight > 0 );
	InitializeColors( );
	}


//COwnerDrawnListCtrl::~COwnerDrawnListCtrl( ) { }

COLORREF COwnerDrawnListItem::default_item_text_color( ) const {
	return GetSysColor( COLOR_WINDOWTEXT );
	}

COLORREF COwnerDrawnListItem::item_text_color( ) const {
	return ItemTextColor( );
	}

bool COwnerDrawnListItem::DrawSubitem_( RANGE_ENUM_COL const column::ENUM_COL subitem, _In_ CDC& pdc, _In_ CRect rc, _In_ const UINT state, _Out_opt_ INT* const width, _Inout_ INT* const focusLeft ) const {
	return DrawSubitem( subitem, pdc, rc, state, width, focusLeft );
	}

INT COwnerDrawnListItem::compare_interface( _In_ const COwnerDrawnListItem* const other, RANGE_ENUM_COL const column::ENUM_COL subitem ) const {
	return Compare( other, subitem );
	}

//INT COwnerDrawnListItem::Compare( _In_ const COwnerDrawnListItem* const other, RANGE_ENUM_COL const column::ENUM_COL subitem ) const {
///*
//	Return value:
//	<= -2:	this is less than other regardless of ascending flag
//	-1:		this is less than other
//	0:		this equals other
//	+1:		this is greater than other
//	>= +1:	this is greater than other regardless of ascending flag.
//*/
//
//	// Default implementation compares strings
//	return signum( GetText( subitem ).compare( other->GetText( subitem ) ) );
//
//	}



//std::wstring COwnerDrawnListItem::GetText( RANGE_ENUM_COL const column::ENUM_COL subitem ) const {
//	return Text( subitem );
//	}


_Must_inspect_result_ _Success_( SUCCEEDED( return ) )
HRESULT COwnerDrawnListItem::GetText_WriteToStackBuffer( RANGE_ENUM_COL const column::ENUM_COL subitem, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, _Inout_ rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const {
	const HRESULT res = Text_WriteToStackBuffer( subitem, psz_text, strSize, sizeBuffNeed, chars_written );
#ifdef DEBUG
	if ( SUCCEEDED( res ) ) {
		const auto len_dat_str = wcslen( psz_text );
		ASSERT( chars_written == len_dat_str );
		}
#endif
	return res;
	}

void COwnerDrawnListCtrl::OnColumnsInserted( ) {
	/*
	  This method MUST be called BEFORE the Control is shown.
	*/
	// The pacmen shall not draw over our header control.
	VERIFY( ModifyStyle( 0, WS_CLIPCHILDREN ) );

	// Where does the 1st Item begin vertically?
	if ( GetItemCount( ) > 0 ) {
		CRect rc;
		VERIFY( GetItemRect( 0, rc, LVIR_BOUNDS ) );
		m_yFirstItem = rc.top;
		}
	else {
		InsertItem( 0, _T( "_tmp" ), 0 );
		CRect rc;
		VERIFY( GetItemRect( 0, rc, LVIR_BOUNDS ) );
		VERIFY( DeleteItem( 0 ) );
		m_yFirstItem = rc.top;
		}

	LoadPersistentAttributes( );
	}

COLORREF COwnerDrawnListCtrl::GetItemSelectionTextColor( _In_ _In_range_( 0, INT_MAX ) const INT i ) const {
	auto selected = ( GetItemState( i, LVIS_SELECTED ) & LVIS_SELECTED ) != 0;
	if ( selected && m_showFullRowSelection && ( HasFocus( ) || IsShowSelectionAlways( ) ) ) {
		return GetHighlightTextColor( );
		}
	return GetSysColor( COLOR_WINDOWTEXT );
	}

_Must_inspect_result_ _Success_( return != NULL ) _Ret_maybenull_
COwnerDrawnListItem* COwnerDrawnListCtrl::GetItem( _In_ _In_range_( 0, INT_MAX ) const int i ) const {
	ASSERT( i < GetItemCount( ) );
	const auto itemCount = GetItemCount( );
	if ( i < itemCount ) {
		return reinterpret_cast< COwnerDrawnListItem* >( GetItemData( static_cast<int>( i ) ) );
		}
	return NULL;
	}

void COwnerDrawnListCtrl::SetSorting( RANGE_ENUM_COL const column::ENUM_COL sortColumn, _In_ const bool ascending ) {
	m_sorting.ascending2 = m_sorting.ascending1;
	m_sorting.column1    = sortColumn;
	m_sorting.column2    = m_sorting.column1;
	m_sorting.ascending1 = ascending;
	}

void COwnerDrawnListCtrl::ShowFullRowSelection( _In_ const bool show ) {
	m_showFullRowSelection = show;
	if ( IsWindow( m_hWnd ) ) {
		InvalidateRect( NULL );
		}
	}

void COwnerDrawnListCtrl::ShowGrid( _In_ const bool show ) {
	m_showGrid = show;
	if ( IsWindow( m_hWnd ) ) {
		InvalidateRect( NULL );
		}
	}

void COwnerDrawnListCtrl::ShowStripes( _In_ const bool show ) {
	m_showStripes = show;
	if ( IsWindow( m_hWnd ) ) {
		InvalidateRect( NULL );
		}
	}


_Success_( return != -1 ) _Ret_range_( -1, INT_MAX ) INT COwnerDrawnListCtrl::FindListItem( _In_ const COwnerDrawnListItem* const item ) const {

	auto fi   = zeroInitLVFINDINFO( );
	fi.flags  = LVFI_PARAM;
	fi.lParam = reinterpret_cast<LPARAM>( item );

	auto i = static_cast<INT>( FindItem( &fi ) );

	return i;
	}

void COwnerDrawnListCtrl::InitializeColors( ) {
	// I try to find a good contrast to COLOR_WINDOW (usually white or light grey).
	// This is a result of experiments. 

	const DOUBLE diff      = 0.07;		// Try to alter the brightness by diff.
	const DOUBLE threshold = 1.04;	// If result would be brighter, make color darker.
	m_windowColor          = GetSysColor( COLOR_WINDOW );

	auto b = CColorSpace::GetColorBrightness( m_windowColor );

	if ( b + diff > threshold ) {
		b -= diff;
		}
	else {
		b += diff;
		if ( b > 1.0 ) {
			b = 1.0;
			}
		}

#ifdef COLOR_DEBUGGING
	TRACE( _T( "Setting m_stripeColor to CColorSpace::MakeBrightColor( m_windowColor: %ld, b: %f )\r\n" ), m_windowColor, b );
#endif
	m_stripeColor = CColorSpace::MakeBrightColor( m_windowColor, b );
#ifdef COLOR_DEBUGGING
	TRACE( _T( "m_stripeColor: %ld\r\n" ), m_stripeColor );
#endif
	}

_Success_( SUCCEEDED( return ) )
HRESULT COwnerDrawnListCtrl::drawSubItem_stackbuffer( _In_ const COwnerDrawnListItem* const item, _In_ CRect& rcText, const int& align, _In_ _In_range_( 0, INT_MAX ) const column::ENUM_COL subitem, _In_ CDC& dcmem, _Out_ rsize_t& sizeNeeded ) const {
	const rsize_t subitem_text_size = 128;
	wchar_t psz_subitem_formatted_text[ subitem_text_size ] = { 0 };
	//rsize_t sizeNeeded = 0;
	rsize_t chars_written = 0;

	const HRESULT res = item->GetText_WriteToStackBuffer( subitem, psz_subitem_formatted_text, subitem_text_size, sizeNeeded, chars_written );
	if ( SUCCEEDED( res ) ) {
		dcmem.DrawTextW( psz_subitem_formatted_text, static_cast<int>( chars_written ), rcText, DT_SINGLELINE | DT_VCENTER | DT_WORD_ELLIPSIS | DT_NOPREFIX | DT_NOCLIP | static_cast<UINT>( align ) );
		return res;
		}
	if ( ( MAX_PATH * 2 ) > sizeNeeded ) {
		const rsize_t subitem_text_size_2 = ( MAX_PATH * 2 );
		wchar_t psz_subitem_formatted_text_2[ subitem_text_size_2 ] = { 0 };
		rsize_t chars_written_2 = 0;
		const HRESULT res_2 = item->GetText_WriteToStackBuffer( subitem, psz_subitem_formatted_text_2, subitem_text_size_2, sizeNeeded, chars_written_2 );
		if ( SUCCEEDED( res_2 ) ) {
			dcmem.DrawTextW( psz_subitem_formatted_text_2, static_cast<int>( chars_written_2 ), rcText, DT_SINGLELINE | DT_VCENTER | DT_WORD_ELLIPSIS | DT_NOPREFIX | DT_NOCLIP | static_cast<UINT>( align ) );
			return res;
			}
		//goto DoDrawSubItemBecauseItCannotDrawItself_drawText_dynamic_memory;
		//DrawText_dynamic( item, rcText, align, subitem, dcmem );
		return res;
		}
	ASSERT( !SUCCEEDED( res ) );
	return res;
	}

void COwnerDrawnListCtrl::DoDrawSubItemBecauseItCannotDrawItself( _In_ const COwnerDrawnListItem* const item, _In_ _In_range_( 0, INT_MAX ) const column::ENUM_COL subitem, _In_ CDC& dcmem, _In_ CRect& rcDraw, _In_ const PDRAWITEMSTRUCT& pdis, _In_ const bool showSelectionAlways, _In_ const bool bIsFullRowSelection, const std::vector<bool>& is_right_aligned_cache ) const {
	item->DrawSelection( this, dcmem, rcDraw, pdis->itemState );
	auto rcText = rcDraw;
	rcText.DeflateRect( TEXT_X_MARGIN, 0 );
	CSetBkMode bk( dcmem, TRANSPARENT );
	CSelectObject sofont( dcmem, *( GetFont( ) ) );
	
	//const auto align = IsColumnRightAligned( subitem ) ? DT_RIGHT : DT_LEFT;
	const auto align = is_right_aligned_cache[ static_cast<size_t>( subitem ) ] ? DT_RIGHT : DT_LEFT;

	// Get the correct color in case of compressed or encrypted items
	auto textColor = item->item_text_color( );

	if ( ( pdis->itemState bitand ODS_SELECTED ) && ( showSelectionAlways || HasFocus( ) ) && ( bIsFullRowSelection ) ) {
		textColor = GetItemSelectionTextColor( static_cast<INT>( pdis->itemID ) );
		}

	CSetTextColor tc( dcmem, textColor );

	if ( subitem == column::COL_NAME ) {
		//fastpath. No work to be done!
		dcmem.DrawTextW( item->m_name, static_cast< int >( item->m_name_length ), rcText, DT_SINGLELINE | DT_VCENTER | DT_WORD_ELLIPSIS | DT_NOPREFIX | DT_NOCLIP | static_cast< UINT >( align ) );
		return;
		}

	rsize_t size_needed = 0;
	const HRESULT stackbuffer_draw_res = drawSubItem_stackbuffer( item, rcText, align, subitem, dcmem, size_needed );
	if ( !SUCCEEDED( stackbuffer_draw_res ) ) {
		DrawText_dynamic( item, rcText, align, subitem, dcmem, size_needed );
		}
	}

_Pre_satisfies_( subitem != column::COL_NAME )
void COwnerDrawnListCtrl::DrawText_dynamic( _In_ const COwnerDrawnListItem* const item, _In_ CRect& rcText, const int& align, _In_ _In_range_( 0, INT_MAX ) const column::ENUM_COL subitem, _In_ CDC& dcmem, _In_ const rsize_t size_needed ) const {
	ASSERT( size_needed < 33000 );
	std::unique_ptr<wchar_t[ ]> buffer ( new wchar_t[ size_needed + 2 ] );
	SecureZeroMemory( buffer.get( ), ( ( size_needed + 2 ) * sizeof( wchar_t ) ) );

	rsize_t new_size_needed = 0;
	rsize_t chars_written = 0;
	const HRESULT res = item->GetText_WriteToStackBuffer( subitem, buffer.get( ), size_needed, new_size_needed, chars_written );
	if ( !SUCCEEDED( res ) ) {
		abort( );
		}
	// Draw the (sub)item text
	//const auto s( item->GetText( subitem ) );
	//dcmem.DrawTextW( s.c_str( ), static_cast<int>( s.length( ) ), rcText, DT_SINGLELINE | DT_VCENTER | DT_WORD_ELLIPSIS | DT_NOPREFIX | DT_NOCLIP | static_cast< UINT >( align ) );
	dcmem.DrawTextW( buffer.get( ), static_cast<int>( chars_written ), rcText, DT_SINGLELINE | DT_VCENTER | DT_WORD_ELLIPSIS | DT_NOPREFIX | DT_NOCLIP | static_cast< UINT >( align ) );
	}

void COwnerDrawnListCtrl::DrawItem( _In_ PDRAWITEMSTRUCT pdis ) {
	const auto item = reinterpret_cast< COwnerDrawnListItem *> ( pdis->itemData );
	const auto pdc = CDC::FromHandle( pdis->hDC );
	const auto bIsFullRowSelection = m_showFullRowSelection;
	ASSERT_VALID( pdc );
	CRect rcItem( pdis->rcItem );
	if ( m_showGrid ) {
		rcItem.bottom--;
		rcItem.right--;
		}

	CDC dcmem; //compiler seems to vectorize this!

	VERIFY( dcmem.CreateCompatibleDC( pdc ) );
	CBitmap bm;
	VERIFY( bm.CreateCompatibleBitmap( pdc, ( rcItem.Width( ) ), ( rcItem.Height( ) ) ) );
	CSelectObject sobm( dcmem, bm );

	dcmem.FillSolidRect( rcItem - rcItem.TopLeft( ), GetItemBackgroundColor( static_cast<INT>( pdis->itemID ) ) ); //NOT vectorized!

	const bool drawFocus = ( pdis->itemState bitand ODS_FOCUS ) != 0 && HasFocus( ) && bIsFullRowSelection; //partially vectorized

	std::vector<INT> order;
	//std::vector<column::ENUM_COL> orderVec;
	
	const bool showSelectionAlways = IsShowSelectionAlways( );
	const auto thisHeaderCtrl = GetHeaderCtrl( );//HORRENDOUSLY slow. Pessimisation of memory access, iterates (with a for loop!) over a map. MAXIMUM branch prediction failures! Maximum Bad Speculation stalls!

	//orderVec.reserve( static_cast<size_t>( thisHeaderCtrl->GetItemCount( ) ) );
	const auto resize_size = thisHeaderCtrl->GetItemCount( );
	if ( resize_size == -1 ) {
		std::terminate( );
		}
	order.resize( static_cast<size_t>( resize_size ) );
	ASSERT( order.GetSize( ) < 10 );
	VERIFY( thisHeaderCtrl->GetOrderArray( order.data( ), static_cast<int>( order.size( ) ) )) ;

#ifdef DEBUG
	for ( INT i = 0; i < order.GetSize( ) - 1; ++i ) {
		if ( i != order[ i ] ) {
			TRACE( _T( "order[%i]: %i \r\n" ), i, order[ i ] );
			}
		}
#endif
	const auto thisLoopSize = order.size( );
	if ( is_right_aligned_cache.empty( ) ) {
		
		is_right_aligned_cache.reserve( static_cast<size_t>( thisLoopSize ) );
		for ( size_t i = 0; i < thisLoopSize; ++i ) {
			is_right_aligned_cache.push_back( IsColumnRightAligned( i ) );
			}
		}
	auto rcFocus = rcItem;
	rcFocus.DeflateRect( 0, LABEL_Y_MARGIN - 1 );

	
	for ( size_t i = 0; i < thisLoopSize; i++ ) {
		//iterate over columns, properly populate fields.
		ASSERT( order[ i ] == i );
		static_assert( std::is_convertible< INT, std::underlying_type<column::ENUM_COL>::type>::value, "" );
		const auto subitem = static_cast<column::ENUM_COL>( order[ i ] );
		const auto rc = GetWholeSubitemRect( static_cast<INT>( pdis->itemID ), subitem );
		CRect rcDraw = rc - rcItem.TopLeft( );
		INT focusLeft = rcDraw.left;
		if ( !item->DrawSubitem_( subitem, dcmem, rcDraw, pdis->itemState, NULL, &focusLeft ) ) {//if DrawSubItem returns true, item draws self. Therefore `!item->DrawSubitem` is true when item DOES NOT draw self
			DoDrawSubItemBecauseItCannotDrawItself( item, subitem, dcmem, rcDraw, pdis, showSelectionAlways, bIsFullRowSelection, is_right_aligned_cache );
			}

		if ( focusLeft > rcDraw.left ) {
			if ( drawFocus && i > 0 ) {
				pdc->DrawFocusRect( rcFocus );
				}
			rcFocus.left = focusLeft;
			}
		rcFocus.right = rcDraw.right;
		VERIFY( pdc->BitBlt( ( rcItem.left + rcDraw.left ), ( rcItem.top + rcDraw.top ), ( rcDraw.Width( ) ), ( rcDraw.Height( ) ), &dcmem, rcDraw.left, rcDraw.top, SRCCOPY ) );
		}
	if ( drawFocus ) {
		pdc->DrawFocusRect( rcFocus );
		}
	//VERIFY( dcmem.DeleteDC( ) );
	}

bool COwnerDrawnListCtrl::IsColumnRightAligned( _In_ const INT col ) const {
	auto hditem = zeroInitHDITEM( );
	hditem.mask   = HDI_FORMAT;
	VERIFY( GetHeaderCtrl( )->GetItem( col, &hditem ) );
	return ( hditem.fmt bitand HDF_RIGHT ) != 0;
	}

CRect COwnerDrawnListCtrl::GetWholeSubitemRect( _In_ const INT item, _In_ const INT subitem ) const {
	CRect rc;
	if ( subitem == 0 ) {
		// Special case column 0:
		// If we did GetSubItemRect(item 0, LVIR_LABEL, rc) and we have an image list, then we would get the rectangle excluding the image.
		HDITEM hditem = zeroInitHDITEM( );

		hditem.mask = HDI_WIDTH;
		VERIFY( GetHeaderCtrl( )->GetItem( 0, &hditem ) );

		VERIFY( GetItemRect( item, rc, LVIR_LABEL ) );
		rc.left = rc.right - hditem.cxy;
		}
	else {
		VERIFY( GetSubItemRect( item, subitem, LVIR_LABEL, rc ) );
		}

	if ( m_showGrid ) {
		rc.right--;
		rc.bottom--;
		}
	return rc;
	}

_Success_( return >= 0 ) _Ret_range_( 0, INT_MAX ) _On_failure_( _Ret_range_( -1, -1 ) )
INT COwnerDrawnListCtrl::GetSubItemWidth( _In_ const COwnerDrawnListItem* const item, _In_ _In_range_( 0, INT_MAX ) const column::ENUM_COL subitem ) const {
	if ( item == NULL ) {
		return -1;
		}
	INT width = 0;

	CClientDC dc( const_cast< COwnerDrawnListCtrl* >( this ) );
	CRect rc( 0, 0, 1000, 1000 );
	
	INT dummy = rc.left;
	if ( item->DrawSubitem_( subitem, dc, rc, 0, &width, &dummy ) ) {
		//ASSERT( item )
		return width;
		}

	if ( subitem == column::COL_NAME ) {
		//column::COL_NAME requires very little work!
		if ( item->m_name_length == 0 ) {
			return 0;
			}
		CSelectObject sofont( dc, *( GetFont( ) ) );
		const auto align = IsColumnRightAligned( subitem ) ? DT_RIGHT : DT_LEFT;
		dc.DrawTextW( item->m_name, static_cast<int>( item->m_name_length ), rc, DT_SINGLELINE | DT_VCENTER | DT_CALCRECT | DT_NOPREFIX | DT_NOCLIP | static_cast<UINT>( align ) );
		rc.InflateRect( TEXT_X_MARGIN, 0 );
		return rc.Width( );
		}


	const rsize_t subitem_text_size = 128;
	wchar_t psz_subitem_formatted_text[ subitem_text_size ] = { 0 };
	rsize_t sizeNeeded = 0;
	rsize_t chars_written = 0;

	const HRESULT res_1 = item->GetText_WriteToStackBuffer( subitem, psz_subitem_formatted_text, subitem_text_size, sizeNeeded, chars_written );
	if ( !SUCCEEDED( res_1 ) ) {
		ASSERT( sizeNeeded < 33000 );
		std::unique_ptr<wchar_t[ ]> buffer ( new wchar_t[ sizeNeeded + 2 ] );
		SecureZeroMemory( buffer.get( ), ( ( sizeNeeded + 2 ) * sizeof( wchar_t ) ) );

		rsize_t new_size_needed = 0;
		rsize_t chars_written_2 = 0;
		const HRESULT res_2 = item->GetText_WriteToStackBuffer( subitem, buffer.get( ), sizeNeeded, new_size_needed, chars_written_2 );
		if ( !SUCCEEDED( res_2 ) ) {
			abort( );
			}
		if ( chars_written_2 == 0 ) {
			return 0;
			}
		CSelectObject sofont( dc, *( GetFont( ) ) );
		const auto align = IsColumnRightAligned( subitem ) ? DT_RIGHT : DT_LEFT;
		dc.DrawTextW( buffer.get( ), static_cast<int>( chars_written_2 ), rc, DT_SINGLELINE | DT_VCENTER | DT_CALCRECT | DT_NOPREFIX | DT_NOCLIP | static_cast<UINT>( align ) );

		rc.InflateRect( TEXT_X_MARGIN, 0 );
		return rc.Width( );
		}

	if ( chars_written == 0 ) {
		return 0;
		}

	//const auto s( item->GetText( subitem ) );
	//if ( s.empty( ) ) {
	//	return 0;
	//	}

	CSelectObject sofont( dc, *( GetFont( ) ) );
	const auto align = IsColumnRightAligned( subitem ) ? DT_RIGHT : DT_LEFT;
	dc.DrawTextW( psz_subitem_formatted_text, static_cast<int>( chars_written ), rc, DT_SINGLELINE | DT_VCENTER | DT_CALCRECT | DT_NOPREFIX | DT_NOCLIP | static_cast<UINT>( align ) );

	rc.InflateRect( TEXT_X_MARGIN, 0 );
	return rc.Width( );
	}

void COwnerDrawnListCtrl::buildArrayFromItemsInHeaderControl( _In_ _Pre_readable_size_( capacity ) const int* const columnOrder, _Out_ _Pre_writable_size_( capacity ) _Post_readable_size_( readable ) int* vertical, _In_ const rsize_t capacity, _Out_ rsize_t& readable, _In_ const CHeaderCtrl* header_ctrl ) const {
	//vertical.SetSize( GetHeaderCtrl( )->GetItemCount( ) + 1 );
	//ASSERT( columnOrder.GetSize( ) >= GetHeaderCtrl( )->GetItemCount( ) );
	ASSERT( capacity >= header_ctrl->GetItemCount( ) );
	//TRACE( _T( "columnOrder size: %i\r\n" ), int( columnOrder.GetSize( ) ) );
	readable = 0;


	auto x   = -GetScrollPos( SB_HORZ );
	auto hdi = zeroInitHDITEM( );

	hdi.mask = HDI_WIDTH;
	const auto header_ctrl_item_count = header_ctrl->GetItemCount( );
	if ( header_ctrl_item_count <= 0 ) {
		std::terminate( );
		}
	if ( static_cast<rsize_t>( header_ctrl_item_count ) > capacity ) {
		std::terminate( );
		}
	for ( INT i = 0; i < header_ctrl_item_count; i++ ) {
		VERIFY( header_ctrl->GetItem( columnOrder[ i ], &hdi ) );
		x += hdi.cxy;
		vertical[ i ] = x;
		++readable;
		}
	}

BEGIN_MESSAGE_MAP(COwnerDrawnListCtrl, CListCtrl)
	ON_NOTIFY(HDN_DIVIDERDBLCLICKA, 0, OnHdnDividerdblclick)
	ON_NOTIFY(HDN_DIVIDERDBLCLICKW, 0, OnHdnDividerdblclick)
	ON_NOTIFY(HDN_ITEMCLICKA, 0, OnHdnItemclick)
	ON_NOTIFY(HDN_ITEMCLICKW, 0, OnHdnItemclick)
	ON_NOTIFY(HDN_ITEMDBLCLICKA, 0, OnHdnItemdblclick)
	ON_NOTIFY(HDN_ITEMDBLCLICKW, 0, OnHdnItemdblclick)
	ON_NOTIFY(HDN_ITEMCHANGINGA, 0, OnHdnItemchanging)
	ON_NOTIFY(HDN_ITEMCHANGINGW, 0, OnHdnItemchanging)
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnLvnGetdispinfo)
	ON_WM_ERASEBKGND()
	ON_WM_VSCROLL()
	ON_WM_SHOWWINDOW()
	ON_WM_DESTROY()
END_MESSAGE_MAP()

void COwnerDrawnListCtrl::handle_EraseBkgnd( _In_ CDC* pDC ) {
	// We should recalculate m_yFirstItem here (could have changed e.g. when the XP-Theme changed).
	if ( GetItemCount( ) > 0 ) {
		CRect rc;
		VERIFY( GetItemRect( GetTopIndex( ), rc, LVIR_BOUNDS ) );
		m_yFirstItem = rc.top;
		}
	// else: if we did the same thing as in OnColumnsCreated(), we get repaint problems.

	const COLORREF gridColor = RGB( 212, 208, 200 );

	CRect rcClient;
	GetClientRect( rcClient );

	CRect rcHeader;
	const auto header_ctrl = GetHeaderCtrl( );
	header_ctrl->GetWindowRect( rcHeader );
	ScreenToClient( rcHeader );

	auto rcBetween  = rcClient;// between header and first item
	rcBetween.top    = rcHeader.bottom;
	rcBetween.bottom = m_yFirstItem;
	pDC->FillSolidRect( rcBetween, gridColor );

	//I fucking HATE CArray!
	//CArray<INT, INT> columnOrder;
	//columnOrder.SetSize( GetHeaderCtrl( )->GetItemCount( ) );
	//ASSERT( columnOrder.GetSize( ) < 10 );

	const rsize_t column_buf_size = 10;
	
	const auto header_ctrl_item_count = header_ctrl->GetItemCount( );

	ASSERT( header_ctrl_item_count < column_buf_size );
	if ( header_ctrl_item_count > column_buf_size ) {
		//too many columns!
		std::terminate( );
		}
	int column_order[ column_buf_size ] = { 0 };

	VERIFY( GetColumnOrderArray( column_order, header_ctrl_item_count ) );

	//I fucking HATE CArray!
	//CArray<INT, INT> vertical;
	int vertical_buf[ column_buf_size ] = { 0 };
	rsize_t vertical_readable = 0;
	buildArrayFromItemsInHeaderControl( column_order, vertical_buf, column_buf_size, vertical_readable, header_ctrl );
	
	//ASSERT( vertical.GetSize( ) < column_buf_size );
	ASSERT( vertical_readable < column_buf_size );

	if ( m_showGrid ) {
		CPen pen( PS_SOLID, 1, gridColor );
		const CSelectObject sopen( *pDC, pen );

		const auto rowHeight = m_rowHeight;
		for ( auto y = ( m_yFirstItem + static_cast<LONG>( rowHeight ) - 1 ); y < rcClient.bottom; y += static_cast<LONG>( rowHeight ) ) {
			ASSERT( rowHeight == m_rowHeight );
			pDC->MoveTo( rcClient.left, static_cast<INT>( y ) );
			VERIFY( pDC->LineTo( rcClient.right, static_cast<INT>( y ) ) );
			}

		//const auto verticalSize = vertical.GetSize( );
		const auto verticalSize = vertical_readable;
		for ( size_t i = 0; i < verticalSize; i++ ) {
			//ASSERT( verticalSize == vertical.GetSize( ) );
			pDC->MoveTo( ( vertical_buf[ i ] - 1 ), rcClient.top );
			VERIFY( pDC->LineTo( ( vertical_buf[ i ] - 1 ), rcClient.bottom ) );
			}
		}

	const auto bgcolor    = GetSysColor( COLOR_WINDOW );
	const int  gridWidth  = ( m_showGrid ? 1 : 0 );
	const auto lineCount  = GetCountPerPage( ) + 1;
	const auto firstItem  = GetTopIndex( );
	const auto lastItem   = min( firstItem + lineCount, GetItemCount( ) ) - 1;

	ASSERT( GetItemCount( ) == 0 || firstItem < GetItemCount( ) );
	ASSERT( GetItemCount( ) == 0 || lastItem < GetItemCount( ) );
	ASSERT( GetItemCount( ) == 0 || lastItem >= firstItem );

	const auto itemCount = ( lastItem - firstItem + 1 );

	CRect fill;
	fill.left   = vertical_buf[ vertical_readable - 1 ];
	fill.right  = rcClient.right;
	fill.top    = m_yFirstItem;
	fill.bottom = fill.top + static_cast<LONG>( m_rowHeight ) - static_cast<LONG>( gridWidth );
	for ( INT i = 0; i < itemCount; i++ ) {
		pDC->FillSolidRect( fill, bgcolor );
		fill.OffsetRect( 0, static_cast<int>( m_rowHeight ) );
		}

	const auto rowHeight = m_rowHeight;
	auto top = fill.top;
	while ( top < rcClient.bottom ) {
		fill.top    = top;
		fill.bottom = top + static_cast<LONG>( m_rowHeight ) - static_cast<LONG>( gridWidth );
		
		INT left = 0;
		//auto verticalSize = vertical.GetSize( );
		const auto verticalSize = vertical_readable;
		for ( size_t i = 0; i < verticalSize; i++ ) {
			//ASSERT( verticalSize == vertical.GetSize( ) );
			fill.left = left;
			fill.right = vertical_buf[ i ] - gridWidth;
			pDC->FillSolidRect( fill, bgcolor );
			left = vertical_buf[ i ];
			}
		fill.left  = left;
		fill.right = rcClient.right;
		pDC->FillSolidRect( fill, bgcolor );

		ASSERT( rowHeight == m_rowHeight );
		top += rowHeight;
		}
	}

BOOL COwnerDrawnListCtrl::OnEraseBkgnd( CDC* pDC ) {
	ASSERT_VALID( pDC );
	ASSERT( GetHeaderCtrl( )->GetItemCount( ) > 0 );
	TRACE( _T( "COwnerDrawnListCtrl::OnEraseBkgnd!\r\n" ) );
	handle_EraseBkgnd( pDC );
	return true;
	}

void COwnerDrawnListCtrl::AddExtendedStyle( _In_ const DWORD exStyle ) {
	SetExtendedStyle( GetExtendedStyle( ) bitor exStyle );
	}

void COwnerDrawnListCtrl::OnDestroy( ) {
	SavePersistentAttributes( );
	CListCtrl::OnDestroy( );
	}

void COwnerDrawnListCtrl::SortItems( ) {
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

	//goddamnit, static_assert is AWESOME when combined with template metaprogramming!
	static_assert( std::is_convertible<std::underlying_type<column::ENUM_COL>::type, std::int8_t>::value, "m_sorting.column1 MUST be convertible to an ENUM_COL!" );
	m_indicatedColumn = static_cast<std::int8_t>( m_sorting.column1 );
	text.ReleaseBuffer( );
	}


void COwnerDrawnListCtrl::SavePersistentAttributes( ) {
	
	const rsize_t col_array_size = 128;
	int col_array[ col_array_size ] = { 0 };

	const auto itemCount = GetHeaderCtrl( )->GetItemCount( );

	if ( !( itemCount < col_array_size ) ) {
		std::terminate( );
		}

	const auto get_res = GetColumnOrderArray( col_array, itemCount );

	if ( get_res == 0 ) {
		std::terminate( );
		}

//#ifdef DEBUG
//	CArray<INT, INT> arr;
//	arr.SetSize( GetHeaderCtrl( )->GetItemCount( ) );//Critical! else, we'll overrun the CArray in GetColumnOrderArray
//	auto res = GetColumnOrderArray( arr.GetData( ), static_cast<int>( arr.GetSize( ) ) );//TODO: BAD IMPLICIT CONVERSION HERE!!! BUGBUG FIXME
//	if ( res == 0 ) {
//		std::terminate( );
//		}
//	
//	for ( int i = 0; i < arr.GetSize( ); ++i ) {
//		ASSERT( arr[ i ] == col_array[ i ] );
//		}
//#endif


	CPersistence::SetColumnOrder( m_persistent_name, col_array, static_cast<rsize_t>( itemCount ) );

	for ( INT_PTR i = 0; i < itemCount; i++ ) {
		col_array[ i ] = GetColumnWidth( static_cast<int>( i ) );
		}
	CPersistence::SetColumnWidths( m_persistent_name, col_array, static_cast<rsize_t>( itemCount ) );
	}


void COwnerDrawnListCtrl::LoadPersistentAttributes( ) {
	TRACE( _T( "Loading persisten attributes....\r\n" ) );
	
	const auto itemCount_default_type = GetHeaderCtrl( )->GetItemCount( );
	const auto itemCount = static_cast<size_t>( itemCount_default_type );
	

//#ifdef DEBUG
//	CArray<INT, INT> arr;
//	arr.SetSize( itemCount_default_type );//Critical! else, we'll overrun the CArray in GetColumnOrderArray
//	TRACE( _T( "%s arr size set to: %i\r\n" ), m_persistent_name, static_cast<int>( itemCount ) );
//	arr.AssertValid( );
//	const auto arrSize = arr.GetSize( );
//	ASSERT( arrSize == static_cast<INT_PTR>( itemCount_default_type ) );
//#endif

	const rsize_t countArray = 10;
	
	if ( countArray <= itemCount ) {
		TRACE( _T( "%i <= %i !!!! Something is REALLY wrong!!!\r\n" ), static_cast<int>( countArray ), static_cast<int>( itemCount ) );
		displayWindowsMsgBoxWithMessage( std::wstring( L"countArray <= itemCount !!!! Something is REALLY wrong!!!" ) );
		std::terminate( );
		}

	ASSERT( countArray > itemCount );
	
	INT col_order_array[ countArray ] = { 0 };

#ifdef DEBUG
	const auto res = GetColumnOrderArray( arr.GetData( ), itemCount_default_type );
	if ( res == 0 ) {
		std::terminate( );
		}
#endif

	const auto res_2 = GetColumnOrderArray( col_order_array, itemCount_default_type );
	if ( res_2 == 0 ) {
		std::terminate( );
		}

	CPersistence::GetColumnOrder( m_persistent_name, col_order_array, itemCount );

	const auto res2 = SetColumnOrderArray( static_cast<int>( itemCount ), col_order_array );
	if ( res2 == 0 ) {
		std::terminate( );
		}

	for ( size_t i = 0; i < itemCount; i++ ) {
		col_order_array[ i ] = GetColumnWidth( static_cast<int>( i ) );
		}
	CPersistence::GetColumnWidths( m_persistent_name, col_order_array, itemCount );

	for ( size_t i = 0; i < itemCount; i++ ) {
		// To avoid "insane" settings we set the column width to maximal twice the default width.
		const auto maxWidth = GetColumnWidth( static_cast<int>( i ) ) * 2;
		
#pragma push_macro("min")
#undef min
		//auto w = std::min( arr[ i ], maxWidth );
		const auto w = std::min( col_order_array[ i ], maxWidth );
#pragma pop_macro("min")

		VERIFY( SetColumnWidth( static_cast<int>( i ), w ) );
		}
	// We refrain from saving the sorting because it is too likely, that users start up with insane settings and don't get it.
	}

void COwnerDrawnListCtrl::InsertListItem( _In_ _In_range_( 0, INT32_MAX ) const INT_PTR i, _In_ const COwnerDrawnListItem* const item ) {
	auto lvitem = partInitLVITEM( );

	lvitem.mask = LVIF_TEXT | LVIF_PARAM;
	lvitem.iItem   = static_cast<int>( i );
	lvitem.pszText = LPSTR_TEXTCALLBACKW;
	lvitem.iImage  = I_IMAGECALLBACK;
	lvitem.lParam  = reinterpret_cast< LPARAM >( item );

	VERIFY( i == CListCtrl::InsertItem( &lvitem ) );

	}


void COwnerDrawnListCtrl::OnHdnDividerdblclick( NMHDR* pNMHDR, LRESULT* pResult ) {
	CWaitCursor wc;
	ASSERT( pNMHDR != NULL );
	if ( pNMHDR != NULL ) {
		auto phdr = reinterpret_cast< LPNMHEADER >( pNMHDR );
		const INT subitem = phdr->iItem;
		AdjustColumnWidth( static_cast<column::ENUM_COL>( subitem ) );
		}
	ASSERT( pResult != NULL );
	if ( pResult != NULL ) {
		*pResult = 0;
		}
	}

void COwnerDrawnListCtrl::AdjustColumnWidth( RANGE_ENUM_COL const column::ENUM_COL col ) {
	CWaitCursor wc;

	INT width = 10;
	const auto itemCount = GetItemCount( );
	for ( INT i = 0; i < itemCount; i++ ) {
		ASSERT( itemCount == GetItemCount( ) );
		const auto item = GetItem( i );
		if ( item == NULL ) {
			std::terminate( );
			//`/analyze` is confused.
			return;
			}
		const auto w = GetSubItemWidth( item, col );
		if ( w > width ) {
			width = w;
			}
		}
	VERIFY( SetColumnWidth( col, width + 5 ) );
	}

void COwnerDrawnListCtrl::OnVScroll( UINT nSBCode, UINT nPos, CScrollBar* pScrollBar ) {
	CListCtrl::OnVScroll(nSBCode, nPos, pScrollBar);

	// Owner drawn list controls with LVS_EX_GRIDLINES don't repaint correctly when scrolled (under Windows XP). So we force a complete repaint here.
	InvalidateRect( NULL );
	}

void COwnerDrawnListCtrl::OnHdnItemclick( NMHDR* pNMHDR, LRESULT* pResult ) {
	const auto phdr = reinterpret_cast<LPNMHEADERW>(pNMHDR);
	*pResult = 0;
	const auto col = static_cast<column::ENUM_COL>( phdr->iItem );
	if ( col == m_sorting.column1 ) {
		m_sorting.ascending1 =  ! m_sorting.ascending1;
		}
	else {
		//SetSorting( col, true ); //GetAscendingDefault( col ) == true, unconditionally
		SetSorting( col, AscendingDefault( col ) ); //GetAscendingDefault( col ) == true, unconditionally
		}
	SortItems( );
	}

void COwnerDrawnListCtrl::OnHdnItemdblclick( NMHDR* pNMHDR, LRESULT* pResult ) {
	OnHdnItemclick( pNMHDR, pResult );
	}

void COwnerDrawnListCtrl::OnHdnItemchanging( NMHDR* /*pNMHDR*/, LRESULT* pResult ) {
	// Unused: LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);
	Default( );
	InvalidateRect( NULL );
	ASSERT( pResult != NULL );
	if ( pResult != NULL ) {
		*pResult = 0;
		}
	}

void COwnerDrawnListCtrl::handle_LvnGetdispinfo( _In_ NMHDR* pNMHDR, _In_ LRESULT* pResult ) {
	auto di = reinterpret_cast< NMLVDISPINFOW* >( pNMHDR );
	*pResult = 0;
	auto item = reinterpret_cast<COwnerDrawnListItem*>( di->item.lParam );
	ASSERT( item != NULL );
	if ( item != NULL ) {
		if ( ( di->item.mask bitand LVIF_TEXT ) != 0 ) {
			if ( static_cast< column::ENUM_COL >( di->item.iSubItem ) == column::COL_NAME ) {
				//easy fastpath!
				if ( item->m_name == NULL ) {
					return;
					}
				size_t chars_remaining = 0;
				const HRESULT res = StringCchCopyExW( di->item.pszText, static_cast< rsize_t >( di->item.cchTextMax ), item->m_name, NULL, &chars_remaining, 0 );
				ASSERT( SUCCEEDED( res ) );
				if ( !SUCCEEDED( res ) ) {
					displayWindowsMsgBoxWithMessage( global_strings::COwnerDrawnListCtrl_handle_LvnGetdispinfo_err );
					std::terminate( );
					}
				return;
				}

			rsize_t chars_needed = 0;
			rsize_t chars_written = 0;
			const HRESULT text_res = item->GetText_WriteToStackBuffer( static_cast< column::ENUM_COL >( di->item.iSubItem ), di->item.pszText, static_cast< rsize_t >( di->item.cchTextMax ), chars_needed, chars_written );
			if ( !( SUCCEEDED( text_res ) ) ) {
				if ( text_res == STRSAFE_E_INVALID_PARAMETER ) {
					displayWindowsMsgBoxWithMessage( std::move( std::wstring( L"STRSAFE_E_INVALID_PARAMETER" ) ) );
					}
				if ( text_res == STRSAFE_E_INSUFFICIENT_BUFFER ) {
					displayWindowsMsgBoxWithMessage( std::move( std::wstring( L"STRSAFE_E_INSUFFICIENT_BUFFER" ) ) );
					}
				ASSERT( false );
				std::terminate( );
				}
			}
		}
	}

void COwnerDrawnListCtrl::OnLvnGetdispinfo( NMHDR* pNMHDR, LRESULT* pResult ) {
	static_assert( sizeof( NMHDR* ) == sizeof( NMLVDISPINFOW* ), "some size issues. Good luck with that cast!" );
	ASSERT( ( pNMHDR != NULL ) && ( pResult != NULL ) );
	handle_LvnGetdispinfo( pNMHDR, pResult );
	}


// $Log$
// Revision 1.13  2004/11/15 00:29:25  assarbad
// - Minor enhancement for the coloring of compressed/encrypted items when not in "select full row" mode
//
// Revision 1.12  2004/11/12 22:14:16  bseifert
// Eliminated CLR_NONE. Minor corrections.
//
// Revision 1.11  2004/11/12 00:47:42  assarbad
// - Fixed the code for coloring of compressed/encrypted items. Now the coloring spans the full row!
//
// Revision 1.10  2004/11/07 23:28:14  assarbad
// - Partial implementation for coloring of compressed/encrypted files
//
// Revision 1.9  2004/11/07 00:06:34  assarbad
// - Fixed minor bug with ampersand (details in changelog.txt)
//
// Revision 1.8  2004/11/05 16:53:07  assarbad
// Added Date and History tag where appropriate.
//
