// ownerdrawnlistcontrol.h	- Declaration of COwnerDrawnListCtrl and COwnerDrawnListItem
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

#ifndef OWNERDRAWNLISTCONTROL_H
#define OWNERDRAWNLISTCONTROL_H

#pragma once
#include "stdafx.h"
#include "mainframe.h"


class COwnerDrawnListItem;
class COwnerDrawnListCtrl;


// COwnerDrawnListItem. An item in a COwnerDrawnListCtrl. Some columns (subitems) may be owner drawn (DrawSubitem() returns true), COwnerDrawnListCtrl draws the texts (GetText()) of all others.
// DrawLabel() draws a standard label (width image, text, selection and focus rect)
class COwnerDrawnListItem {
public:

	COwnerDrawnListItem( _In_z_ _Readable_elements_( length ) PCWSTR name, const std::uint16_t length ) : m_name( name ), m_name_length( length ) { }
	COwnerDrawnListItem( const COwnerDrawnListItem& in ) = delete;
	COwnerDrawnListItem& operator=( const COwnerDrawnListItem& in ) = delete;
	COwnerDrawnListItem( ) = default;
	virtual ~COwnerDrawnListItem( ) = default;

	INT          compare_interface            ( _In_ const COwnerDrawnListItem* const other, RANGE_ENUM_COL const column::ENUM_COL subitem ) const {
		return Compare( other, subitem );
		}



	INT          CompareS                     ( _In_ const COwnerDrawnListItem* const other, _In_ const SSorting& sorting ) const {
		if ( sorting.column1 == column::COL_NAME ) {
			const auto sort_result = signum( wcscmp( m_name.get( ), other->m_name.get( ) ) );
		
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

	void         DrawSelection                ( _In_ const COwnerDrawnListCtrl* const list, _In_ CDC& pdc,       _In_ RECT rc, _In_ const UINT state                       ) const;


	bool         DrawSubitem_                 ( RANGE_ENUM_COL const column::ENUM_COL subitem, _In_ CDC& pdc, _In_ RECT rc, _In_ const UINT state, _Out_opt_ INT* const width, _Inout_ INT* const focusLeft ) const {
		return DrawSubitem( subitem, pdc, rc, state, width, focusLeft );
		}

	COLORREF    item_text_color( ) const {
		return ItemTextColor( );
		}

	COLORREF     default_item_text_color      ( ) const {
		return GetSysColor( COLOR_WINDOWTEXT );
		}
	
	_Must_inspect_result_ _Success_( SUCCEEDED( return ) ) 
	HRESULT      GetText_WriteToStackBuffer   ( RANGE_ENUM_COL const column::ENUM_COL subitem, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, _Out_ _On_failure_( _Post_valid_ )rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const {
		const HRESULT res = Text_WriteToStackBuffer( subitem, psz_text, strSize, sizeBuffNeed, chars_written );
	#ifdef DEBUG
		if ( SUCCEEDED( res ) ) {
			const auto len_dat_str = wcslen( psz_text );
			ASSERT( chars_written == len_dat_str );
			}
	#endif
		return res;
		}
protected:
	void         DrawLabel                    ( _In_ COwnerDrawnListCtrl* const list, _In_ CDC& pdc, _In_ CRect& rc, _In_ const UINT state, _Out_opt_ INT* const width, _Inout_ INT* const focusLeft, _In_ const bool indent ) const;
	
	void         DrawHighlightSelectBackground( _In_ const RECT& rcLabel, _In_ const RECT& rc, _In_ const COwnerDrawnListCtrl* const list, _In_ CDC& pdc, _Inout_ COLORREF& textColor ) const;


	void         AdjustLabelForMargin         ( _In_ const RECT& rcRest, _Inout_ CRect& rcLabel ) const {
		rcLabel.InflateRect( LABEL_INFLATE_CX, 0 );
		rcLabel.top    = rcRest.top + static_cast<LONG>( LABEL_Y_MARGIN );
		rcLabel.bottom = rcRest.bottom - static_cast<LONG>( LABEL_Y_MARGIN );
		}

private:
	virtual INT          Compare( _In_ const COwnerDrawnListItem* const other, RANGE_ENUM_COL const column::ENUM_COL subitem ) const = 0;

	_Must_inspect_result_ _Success_( SUCCEEDED( return ) )
	virtual HRESULT      Text_WriteToStackBuffer( RANGE_ENUM_COL const column::ENUM_COL subitem, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, _Out_ _On_failure_( _Post_valid_ ) rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const = 0;

	//CItemBranch is the only non-default behavior here!
	virtual COLORREF     ItemTextColor( ) const = 0;

	// Return value is true, if the item draws itself. width != NULL -> only determine width, do not draw. If focus rectangle shall not begin leftmost, set *focusLeft to the left edge of the desired focus rectangle.
	virtual bool         DrawSubitem            ( RANGE_ENUM_COL const column::ENUM_COL subitem, _In_ CDC& pdc, _In_ CRect rc, _In_ const UINT state, _Out_opt_ INT* const width, _Inout_ INT* const focusLeft ) const = 0;

	public:
	_Field_z_ _Field_size_( m_name_length ) std::unique_ptr<_Null_terminated_ const wchar_t[]> m_name;
	                                        std::uint16_t                                      m_name_length;
	};



// COwnerDrawnListCtrl. Must be report view. Deals with COwnerDrawnListItems.
// Can have a grid or not (own implementation, don't set LVS_EX_GRIDLINES). Flicker-free.
class COwnerDrawnListCtrl : public CListCtrl {
	DECLARE_DYNAMIC(COwnerDrawnListCtrl)
public:
	COwnerDrawnListCtrl          ( _In_z_ PCWSTR name, _In_range_( 0, UINT_MAX ) const UINT rowHeight );
	virtual ~COwnerDrawnListCtrl( ) { }

	COwnerDrawnListCtrl& operator=( const COwnerDrawnListCtrl& in ) = delete;
	COwnerDrawnListCtrl( const COwnerDrawnListCtrl& in ) = delete;

	void LoadPersistentAttributes        ( );
	void SavePersistentAttributes        ( );
	void SortItems                       ( );

	_Success_( return != -1 ) _Ret_range_( -1, INT_MAX )
	INT  FindListItem                        ( _In_ const COwnerDrawnListItem* const item   ) const;
	
	void InsertListItem                      ( _In_ _In_range_( 0, INT32_MAX ) const INT_PTR i, _In_ const COwnerDrawnListItem* const item );

	void AdjustColumnWidth                   ( RANGE_ENUM_COL const column::ENUM_COL col              );
	void OnColumnsInserted                   (                                              );
	void AddExtendedStyle                    ( _In_ const DWORD     exStyle );
	
	//COLORREF GetItemSelectionBackgroundColor ( _In_ _In_range_( 0, INT_MAX )   const INT i  ) const;

	COLORREF GetItemSelectionTextColor       ( _In_ _In_range_( 0, INT_MAX )   const INT i  ) const;
	
	CRect GetWholeSubitemRect                ( _In_ const INT item, _In_ const INT subitem  ) const;

	_Must_inspect_result_ _Success_( return != NULL ) _Ret_maybenull_
	COwnerDrawnListItem* GetItem             ( _In_ _In_range_( 0, INT_MAX )   const int i  ) const;

	void SetSorting( RANGE_ENUM_COL const column::ENUM_COL       sortColumn, _In_ const bool ascending );

	void ShowFullRowSelection( _In_ const bool show );

	void ShowGrid( _In_ const bool show );

	void ShowStripes( _In_ const bool show );


	COLORREF GetHighlightColor( ) const {
		if ( HasFocus( ) ) {
			return GetSysColor( COLOR_HIGHLIGHT );
			}
		return RGB( 190, 190, 190 );
		}
	
	_Success_( return != COLORREF( 0 ) )
	COLORREF GetHighlightTextColor( ) const {
		if ( HasFocus( ) ) {
			return GetSysColor( COLOR_HIGHLIGHTTEXT );
			}
		return RGB( 0, 0, 0 );
		}


	_Success_( return != COLORREF( 0 ) )
	COLORREF GetItemBackgroundColor( _In_ _In_range_( 0, UINT_MAX ) const UINT i ) const {
		return ( IsItemStripeColor( i ) ? m_stripeColor : m_windowColor );
		}

	static_assert( INT_MAX < UINT_MAX, "" );
	bool IsItemStripeColor( _In_ _In_range_( 0, UINT_MAX ) const UINT i ) const {
		return ( m_showStripes && ( i % 2 != 0 ) );
		}

	//_Success_( return != COLORREF( 0 ) )
	//COLORREF GetItemBackgroundColor( _In_ const COwnerDrawnListItem* const item ) const {
	//	const auto itemPos = FindListItem( item );
	//	if ( itemPos != -1 ) {
	//		return GetItemBackgroundColor( itemPos );
	//		}
	//	return COLORREF( 0 );
	//	}

	//_Success_( return != COLORREF( 0 ) )
	//COLORREF GetItemSelectionBackgroundColor( _In_ const COwnerDrawnListItem* const item ) const {
	//	const auto itemPos = FindListItem( item );
	//	if ( itemPos != -1 ) {
	//		return GetItemSelectionBackgroundColor( itemPos );
	//		}
	//	return COLORREF( 0 );
	//	}

	bool IsItemStripeColor( _In_ const COwnerDrawnListItem* const item ) const {
		const auto itemPos = FindListItem( item );
		if ( itemPos >= 0 ) {
#pragma warning(suppress: 28020)
			return IsItemStripeColor( static_cast<UINT>( itemPos ) );
			}
		return COLORREF( 0 );
		}
	bool HasFocus( ) const {
		return ::GetFocus( ) == m_hWnd;
		}
	bool IsShowSelectionAlways( ) const {
		return ( GetStyle( ) bitand LVS_SHOWSELALWAYS ) != 0;
		}

	bool AscendingDefault( RANGE_ENUM_COL const column::ENUM_COL column ) const {
		return GetAscendingDefault( column );
		}


private:
	
	//void OnVscroll( HWND hwnd, HWND hwndCtl, UINT code, int pos );

	// Overridables
	virtual bool GetAscendingDefault( _In_ const column::ENUM_COL column ) const = 0;

protected:

	virtual void DrawItem                    ( _In_ PDRAWITEMSTRUCT pdis                   ) override final;

	void         DoDrawSubItemBecauseItCannotDrawItself( _In_ const COwnerDrawnListItem* const item, _In_ _In_range_( 0, INT_MAX ) const column::ENUM_COL subitem, _In_ CDC& dcmem, _In_ RECT& rcDraw, _In_ const PDRAWITEMSTRUCT& pdis, _In_ const bool showSelectionAlways, _In_ const bool bIsFullRowSelection, const std::vector<bool>& is_right_aligned_cache ) const;

	_Success_( SUCCEEDED( return ) ) 
	HRESULT      drawSubItem_stackbuffer     ( _In_ const COwnerDrawnListItem* const item, _In_ RECT& rcText, const int& align, _In_ _In_range_( 0, INT_MAX ) const column::ENUM_COL subitem, _In_ CDC& dcmem, _On_failure_( _Post_valid_ ) rsize_t& sizeNeeded ) const;

	_Pre_satisfies_( subitem != column::COL_NAME )
	void         DrawText_dynamic            ( _In_ const COwnerDrawnListItem* const item, _In_ RECT& rcText, const int& align, _In_ _In_range_( 0, INT_MAX ) const column::ENUM_COL subitem, _In_ CDC& dcmem, _In_ const rsize_t size_needed ) const;

	void         InitializeColors            (                                              );
	bool         IsColumnRightAligned        ( _In_ const INT col                                ) const;
	
	_Success_( return >= 0 ) _Ret_range_( 0, INT_MAX ) _On_failure_( _Ret_range_( -1, -1 ) )
	INT          GetSubItemWidth             ( _In_ const COwnerDrawnListItem* const item, _In_ _In_range_( 0, INT_MAX ) const column::ENUM_COL subitem ) const;

public:
	                      CMainFrame* const m_frameptr;
	                      bool        m_showGrid             : 1; // Whether to draw a grid
	                      bool        m_showStripes          : 1; // Whether to show stripes
	                      bool        m_showFullRowSelection : 1; // Whether to draw full row selection

	_Field_range_( 0, UINT_MAX )
	                      UINT        m_rowHeight;                // Height of an item
	                      LONG        m_yFirstItem;               // Top of a first list item
	                      COLORREF    m_windowColor;              // The default background color if !m_showStripes
	                      COLORREF    m_stripeColor;              // The stripe color, used for every other item if m_showStripes
	            _Field_z_ PCWSTR      m_persistent_name;          // for persistence
						  SSorting    m_sorting;
	_Field_range_( 0, 8 ) std::int8_t m_indicatedColumn;
						  std::vector<bool> is_right_aligned_cache;


private:
	void buildArrayFromItemsInHeaderControl( _In_ _Pre_readable_size_( capacity ) const int* const columnOrder, _Out_ _Pre_writable_size_( capacity ) _Post_readable_size_( readable ) int* vertical, _In_ const rsize_t capacity, _Out_ rsize_t& readable, _In_ const CHeaderCtrl* header_ctrl ) const;
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnHdnDividerdblclick(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnHdnItemclick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnHdnItemdblclick( NMHDR* pNMHDR, LRESULT* pResult );
	//afx_msg void OnVscroll( HWND hwnd, HWND hwndCtl, UINT code, int pos );
	afx_msg void OnHdnItemchanging(NMHDR *pNMHDR, LRESULT *pResult);
	//afx_msg void OnSetRedraw( HWND hwnd, BOOL fRedraw );
	afx_msg void OnDestroy( );
	afx_msg void OnLvnGetdispinfo(NMHDR* pNMHDR, LRESULT* pResult);


private:
	void handle_EraseBkgnd( _In_ CDC* pDC );
	void handle_LvnGetdispinfo( _In_ NMHDR* pNMHDR, _In_ LRESULT* pResult );
	};

//need to explicitly ask for inlining else compiler bitches about ODR
inline void COwnerDrawnListItem::DrawHighlightSelectBackground( _In_ const RECT& rcLabel, _In_ const RECT& rc, _In_ const COwnerDrawnListCtrl* const list, _In_ CDC& pdc, _Inout_ COLORREF& textColor ) const {
	// Color for the text in a highlighted item (usually white)
	textColor = list->GetHighlightTextColor( );

	RECT selection = rcLabel;
	// Depending on "FullRowSelection" style
	if ( list->m_showFullRowSelection ) {
		selection.right = rc.right;
		}
	// Fill the selection rectangle background (usually dark blue)
	pdc.FillSolidRect( &selection, list->GetHighlightColor( ) );
	
	}

//need to explicitly ask for inlining else compiler bitches about ODR
inline void COwnerDrawnListItem::DrawLabel( _In_ COwnerDrawnListCtrl* const list, _In_ CDC& pdc, _In_ CRect& rc, _In_ const UINT state, _Out_opt_ INT* const width, _Inout_ INT* const focusLeft, _In_ const bool indent ) const {
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
	pdc.DrawTextW( m_name.get( ), static_cast<int>( m_name_length ), rcLabel, DT_SINGLELINE | DT_VCENTER | DT_WORD_ELLIPSIS | DT_CALCRECT | DT_NOPREFIX | DT_NOCLIP );//DT_CALCRECT modifies rcLabel!!!

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
		pdc.DrawTextW( m_name.get( ), static_cast<int>( m_name_length ), rcRest, DT_SINGLELINE | DT_VCENTER | DT_WORD_ELLIPSIS | DT_NOPREFIX | DT_NOCLIP );
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

//need to explicitly ask for inlining else compiler bitches about ODR
inline void COwnerDrawnListItem::DrawSelection( _In_ const COwnerDrawnListCtrl* const list, _In_ CDC& pdc, _In_ RECT rc, _In_ const UINT state ) const {
	ASSERT( list != NULL );
	if ( !list->m_showFullRowSelection ) {
		return;
		}
	if ( ( !list->HasFocus( ) ) && ( !list->IsShowSelectionAlways( ) ) ) {
		return;
		}
	if ( ( state bitand ODS_SELECTED ) == 0 ) {
		return;
		}

	/*
	inline void CRect::DeflateRect(
		_In_ int x,
		_In_ int y) throw()
	{
		::InflateRect(this, -x, -y);
	}
	*/

	::InflateRect( &rc, -0, -static_cast<int>( LABEL_Y_MARGIN ) );
	pdc.FillSolidRect( &rc, list->GetHighlightColor( ) );
	}

#else
#error ass
#endif



// $Log$
// Revision 1.9  2004/11/12 22:14:16  bseifert
// Eliminated CLR_NONE. Minor corrections.
//
// Revision 1.8  2004/11/12 00:47:42  assarbad
// - Fixed the code for coloring of compressed/encrypted items. Now the coloring spans the full row!
//
// Revision 1.7  2004/11/08 00:46:26  assarbad
// - Added feature to distinguish compressed and encrypted files/folders by color as in the Windows 2000/XP explorer.
//   Same rules apply. (Green = encrypted / Blue = compressed)
//
// Revision 1.6  2004/11/07 23:28:14  assarbad
// - Partial implementation for coloring of compressed/encrypted files
//
// Revision 1.5  2004/11/05 16:53:07  assarbad
// Added Date and History tag where appropriate.
//
