// ownerdrawnlistcontrol.h	- Declaration of COwnerDrawnListControl and COwnerDrawnListItem
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
#else
#error ass
#endif

#pragma once
#include "stdafx.h"

//#include "sortinglistcontrol.h"

class COwnerDrawnListItem;
class COwnerDrawnListControl;


// COwnerDrawnListItem. An item in a COwnerDrawnListControl.
// Some columns (subitems) may be owner drawn (DrawSubitem() returns true), COwnerDrawnListControl draws the texts (GetText()) of all others.
// DrawLabel() draws a standard label (width image, text, selection and focus rect)
class COwnerDrawnListItem {
public:
	//COwnerDrawnListItem();
	//virtual ~COwnerDrawnListItem();


	virtual INT Compare( _In_ const COwnerDrawnListItem* const other, _In_ _In_range_( 0, 7 ) const INT subitem ) const {
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

	INT CompareS            ( _In_ const COwnerDrawnListItem* const other, _In_ const SSorting& sorting ) const;

	virtual CString GetText                  ( _In_range_( 0, 7 ) const INT subitem ) const = 0; // This text is drawn, if DrawSubitem returns false
	
	virtual COLORREF GetItemTextColor        ( ) const { return GetSysColor(COLOR_WINDOWTEXT); } // This color is used for the  current item
	
	// Return value is true, if the item draws itself. width != NULL -> only determine width, do not draw.
	// If focus rectangle shall not begin leftmost, set *focusLeft to the left edge of the desired focus rectangle.
	virtual bool DrawSubitem                 ( _In_ _In_range_( 0, 7 ) const ENUM_COL subitem,            _In_ CDC& pdc,     _In_ CRect rc, _In_ const UINT state, _Out_opt_ INT* const width, _Inout_ INT* const focusLeft ) const = 0;

	void DrawSelection                       ( _In_ const COwnerDrawnListControl* const list, _In_ CDC& pdc,       _Inout_ CRect rc, _In_ const UINT state                       ) const;
#ifdef DEBUG
	static int longestString;
#endif

protected:
	
	void DrawLabel                           ( _In_ COwnerDrawnListControl* const list, _In_opt_ CImageList* const il, _In_ CDC& pdc,              _In_ CRect& rc, _In_ const UINT state, _Out_opt_ INT* const width, _Inout_ INT* const focusLeft, _In_ const bool indent = true) const;
	void DrawPercentage                      ( _In_ CDC& pdc,                     _In_ CRect rc,       _In_ const DOUBLE fraction, _In_ const COLORREF color                                                             ) const;

	void DrawColorWithTransparentBackground( _In_ CRect& rcRest, _In_ CImageList* const il, _In_ CDC& pdc ) const;
	void DrawHighlightedItemSelectionBackground( _In_ const CRect& rcLabel, _In_ const CRect& rc, _In_ const COwnerDrawnListControl* const list, _In_ CDC& pdc, _Inout_ COLORREF& textColor ) const;
	void AdjustLabelForMargin( _In_ const CRect& rcRest, _Inout_ CRect& rcLabel ) const;
	};

static INT CALLBACK _CompareFunc( _In_ const LPARAM lParam1, _In_ const LPARAM lParam2, _In_ const LPARAM lParamSort ) {
	const auto sorting = reinterpret_cast<const SSorting*>( lParamSort );
	return ( reinterpret_cast< const COwnerDrawnListItem*>( lParam1 ) )->CompareS( ( reinterpret_cast< const COwnerDrawnListItem*>( lParam2 ) ), *sorting );
	}

//
// COwnerDrawnListControl. Must be report view. Deals with COwnerDrawnListItems.
// Can have a grid or not (own implementation, don't set LVS_EX_GRIDLINES). Flicker-free.
class COwnerDrawnListControl : public CSortingListControl {
	DECLARE_DYNAMIC(COwnerDrawnListControl)
public:
	COwnerDrawnListControl          ( _In_z_ PCWSTR name, _In_range_( 0, UINT_MAX ) UINT rowHeight );
	//virtual ~COwnerDrawnListControl ( );
	virtual void SysColorChanged( ) {
		InitializeColors( );
		}

	_Success_( return != -1 ) _Ret_range_( -1, INT_MAX ) INT FindListItem ( _In_ const COwnerDrawnListItem* const item   ) const;
	void AdjustColumnWidth                   ( _In_ const ENUM_COL col                     );
	void OnColumnsInserted                   (                                   );
	
	
	COLORREF GetItemSelectionBackgroundColor ( _In_ _In_range_( 0, INT_MAX )   const INT i  ) const;
	COLORREF GetItemSelectionTextColor       ( _In_ _In_range_( 0, INT_MAX )   const INT i  ) const;
	
	COwnerDrawnListItem* GetItem             ( _In_ _In_range_( 0, INT_MAX )   const INT i  ) const;


	void ShowGrid( _In_ const bool show ) {
		m_showGrid = show;
		if ( IsWindow( m_hWnd ) ) {
			InvalidateRect( NULL );
			}
		}
	
	void ShowStripes( _In_ const bool show ) {
		m_showStripes = show;
		if ( IsWindow( m_hWnd ) ) {
			InvalidateRect( NULL );
			}
		}
	
	void ShowFullRowSelection( _In_ const bool show ) {
		m_showFullRowSelection = show;
		if ( IsWindow( m_hWnd ) ) {
			InvalidateRect( NULL );
			}
		}


	COLORREF GetHighlightColor( ) const {
		if ( HasFocus( ) ) {
			return GetSysColor( COLOR_HIGHLIGHT );
			}
		return RGB( 190, 190, 190 );
		}
	
	_Success_( return != COLORREF( 0 ) ) COLORREF GetHighlightTextColor( ) const {
		if ( HasFocus( ) ) {
			return GetSysColor( COLOR_HIGHLIGHTTEXT );
			}
		return RGB( 0, 0, 0 );
		}


	_Success_( return != COLORREF( 0 ) ) COLORREF GetItemBackgroundColor( _In_ _In_range_( 0, INT_MAX )   const INT i ) const {
		return ( IsItemStripeColor( i ) ? m_stripeColor : m_windowColor );
		}

	bool IsItemStripeColor( _In_ _In_range_( 0, INT_MAX )   const INT i ) const {
		return ( m_showStripes && ( i % 2 != 0 ) );
		}

	_Success_( return != COLORREF( 0 ) ) COLORREF GetItemBackgroundColor( _In_ const COwnerDrawnListItem* const item ) const {
		auto itemPos = FindListItem( item );
		if ( itemPos != -1 ) {
			return GetItemBackgroundColor( itemPos );
			}
		return COLORREF( 0 );
		}
	_Success_( return != COLORREF( 0 ) ) COLORREF GetItemSelectionBackgroundColor( _In_ const COwnerDrawnListItem* const item ) const {
		auto itemPos = FindListItem( item );
		if ( itemPos != -1 ) {
			return GetItemSelectionBackgroundColor( itemPos );
			}
		return COLORREF( 0 );
		}
	bool IsItemStripeColor( _In_ const COwnerDrawnListItem* const item ) const {
		auto itemPos = FindListItem( item );
		if ( itemPos != -1 ) {
			return IsItemStripeColor( itemPos );
			}
		return COLORREF( 0 );
		}
	bool HasFocus( ) const {
		return ::GetFocus( ) == m_hWnd;
		}
	bool IsShowSelectionAlways( ) const {
		return ( GetStyle( ) & LVS_SHOWSELALWAYS ) != 0;
		}

	CRect GetWholeSubitemRect                ( _In_ const INT item, _In_ const INT subitem  ) const;

protected:
	
	//void OnVscroll( HWND hwnd, HWND hwndCtl, UINT code, int pos );


	virtual void DrawItem                    ( _In_ PDRAWITEMSTRUCT pdis                   );
	void DoDrawSubItemBecauseItCannotDrawItself( _In_ const COwnerDrawnListItem* const item, _In_ _In_range_( 0, INT_MAX ) const INT subitem, _In_ CDC& dcmem, _In_ CRect& rcDraw, _In_ PDRAWITEMSTRUCT& pdis, _In_ bool showSelectionAlways, _In_ bool bIsFullRowSelection ) const;
	void         InitializeColors            (                                              );
	bool         IsColumnRightAligned        ( _In_ const INT col                                ) const;
	_Success_( return >= 0 ) INT          GetSubItemWidth             ( _In_ const COwnerDrawnListItem* const item, _In_ _In_range_( 0, INT_MAX ) const ENUM_COL subitem ) const;

	public:
	bool     m_showGrid             : 1; // Whether to draw a grid
	bool     m_showStripes          : 1; // Whether to show stripes
	bool     m_showFullRowSelection : 1; // Whether to draw full row selection
	_Field_range_( 0, UINT_MAX ) UINT      m_rowHeight;                // Height of an item
	INT      m_yFirstItem;               // Top of a first list item
	COLORREF m_windowColor;              // The default background color if !m_showStripes
	COLORREF m_stripeColor;              // The stripe color, used for every other item if m_showStripes










public:
#ifdef DEBUG
	static int longestString;
#endif

protected:

	void buildArrayFromItemsInHeaderControl( _In_ CArray<INT, INT>& columnOrder, _Inout_ CArray<INT, INT>& vertical );

	DECLARE_MESSAGE_MAP()
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnHdnDividerdblclick(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	//afx_msg void OnVscroll( HWND hwnd, HWND hwndCtl, UINT code, int pos );
	afx_msg void OnHdnItemchanging(NMHDR *pNMHDR, LRESULT *pResult);
	//afx_msg void OnSetRedraw( HWND hwnd, BOOL fRedraw );
	
	};


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
