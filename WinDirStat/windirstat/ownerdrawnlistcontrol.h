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
class COwnerDrawnListItem: public CSortingListItem {
public:
	//COwnerDrawnListItem();
	//virtual ~COwnerDrawnListItem();

	
	virtual CString GetText                  ( _In_range_( 0, INT32_MAX ) const INT subitem ) const = 0; // This text is drawn, if DrawSubitem returns false
	
	virtual COLORREF GetItemTextColor        ( ) const { return GetSysColor(COLOR_WINDOWTEXT); } // This color is used for the  current item
	
	// Return value is true, if the item draws itself.
	// width != NULL -> only determine width, do not draw.
	// If focus rectangle shall not begin leftmost, set *focusLeft to the left edge of the desired focus rectangle.
	virtual bool DrawSubitem                 ( _In_ _In_range_( 0, INT_MAX ) const INT subitem,            _In_ CDC* pdc,     _In_ CRect rc, _In_ const UINT state, _Inout_opt_ INT* width, _Inout_ INT* focusLeft ) const = 0;

	void DrawSelection                       ( _In_ COwnerDrawnListControl* list, _In_ CDC* pdc,       _In_ CRect rc, _In_ const UINT state                       ) const;
#ifdef DEBUG
	static int longestString;
#endif

protected:
	
	void DrawLabel                           ( _In_ COwnerDrawnListControl* list, _In_opt_ CImageList* il, _In_ CDC* pdc,              _In_ CRect& rc, _In_ const UINT state, _Inout_opt_ INT* width, _Inout_ INT* focusLeft, _In_ const bool indent = true) const;
	void DrawPercentage                      ( _In_ CDC* pdc,                     _In_ CRect rc,       _In_ const DOUBLE fraction, _In_ const COLORREF color                                                             ) const;

	void DrawColorWithTransparentBackground( _In_ CRect& rcRest, _In_ CImageList* il, _In_ CDC* pdc ) const;
	void DrawHighlightedItemSelectionBackground( _In_ const CRect& rcLabel, _In_ const CRect& rc, _In_ COwnerDrawnListControl* list, _In_ CDC* pdc, _Inout_ COLORREF& textColor ) const;
	void AdjustLabelForMargin( _In_ const CRect& rcRest, _Inout_ CRect& rcLabel ) const;
	};


//
// COwnerDrawnListControl. Must be report view. Deals with COwnerDrawnListItems.
// Can have a grid or not (own implementation, don't set LVS_EX_GRIDLINES). Flicker-free.
class COwnerDrawnListControl : public CSortingListControl {
	DECLARE_DYNAMIC(COwnerDrawnListControl)
public:
	COwnerDrawnListControl          ( _In_z_ PCTSTR name, INT rowHeight );
	virtual ~COwnerDrawnListControl ( );
	virtual void SysColorChanged    ( );

	
	_Success_( return != -1 ) INT FindListItem                         ( _In_ const COwnerDrawnListItem* const item   ) const;
	void AdjustColumnWidth                   ( _In_ const INT col                     );
	void OnColumnsInserted                   (                                   );
	void ShowGrid                            ( _In_ const bool show                   );
	void ShowStripes                         ( _In_ const bool show                   );
	void ShowFullRowSelection                ( _In_ const bool show                   );
	
	COLORREF GetHighlightColor               (                                   ) const;
	COLORREF GetHighlightTextColor           (                                   ) const;
	COLORREF GetItemBackgroundColor          ( _In_ _In_range_( 0, INT_MAX ) const INT i                       ) const;
	COLORREF GetItemBackgroundColor          ( _In_ const COwnerDrawnListItem* const item   );
	COLORREF GetItemSelectionBackgroundColor ( _In_ _In_range_( 0, INT_MAX ) const INT i                       ) const;
	COLORREF GetItemSelectionBackgroundColor ( _In_ const COwnerDrawnListItem* const item   );
	COLORREF GetItemSelectionTextColor       ( _In_ _In_range_( 0, INT_MAX ) const INT i                       );
	bool HasFocus                            (                                   ) const;
	
	bool IsItemStripeColor                   ( _In_ _In_range_( 0, INT_MAX ) const INT i                       ) const;
	bool IsItemStripeColor                   ( _In_ const COwnerDrawnListItem* const item   );
	bool IsShowSelectionAlways               (                                   ) const;

	COwnerDrawnListItem *GetItem             ( _In_ _In_range_( 0, INT_MAX ) const INT i                       );
	
	CRect GetWholeSubitemRect                ( _In_ const INT item, _In_ const INT subitem );

protected:
	
	//void OnVscroll( HWND hwnd, HWND hwndCtl, UINT code, int pos );


	virtual void DrawItem                    ( _In_ LPDRAWITEMSTRUCT pdis                   );
	void DoDrawSubItemBecauseItCannotDrawItself( _In_ COwnerDrawnListItem* item, _In_ _In_range_( 0, INT_MAX ) const INT subitem, _In_ CDC& dcmem, _In_ CRect& rcDraw, _In_ LPDRAWITEMSTRUCT& pdis, _In_ bool showSelectionAlways, _In_ bool bIsFullRowSelection );
	void         InitializeColors            (                                              );
	bool         IsColumnRightAligned        ( _In_ const INT col                                ) const;//const?
	_Success_( return >= 0 ) INT          GetSubItemWidth             ( _In_ COwnerDrawnListItem* item, _In_ _In_range_( 0, INT_MAX ) const INT subitem ) const;//const?

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
