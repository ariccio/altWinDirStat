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

#pragma once

#include "sortinglistcontrol.h"

class COwnerDrawnListItem;
class COwnerDrawnListControl;


//
// COwnerDrawnListItem. An item in a COwnerDrawnListControl.
// Some columns (subitems) may be owner drawn (DrawSubitem() returns true), COwnerDrawnListControl draws the texts (GetText()) of all others.
// DrawLabel() draws a standard label (width image, text, selection and focus rect)
//
class COwnerDrawnListItem: public CSortingListItem
{
public:
	COwnerDrawnListItem();
	virtual ~COwnerDrawnListItem();

	
	virtual CString GetText                  ( const int subitem ) const = 0; // This text is drawn, if DrawSubitem returns false
	
	virtual COLORREF GetItemTextColor        ( ) const { return GetSysColor(COLOR_WINDOWTEXT); } // This color is used for the  current item
	
	// Return value is true, if the item draws itself.
	// width != NULL -> only determine width, do not draw.
	// If focus rectangle shall not begin leftmost, set *focusLeft to the left edge of the desired focus rectangle.
	virtual bool DrawSubitem                 ( int subitem,                  CDC *pdc,     CRect rc, UINT state, int *width, int *focusLeft ) const = 0;

	virtual void DrawAdditionalState         ( CDC * /*pdc*/,                const CRect& /*rcLabel*/                                       ) const {}

	void DrawSelection                       ( COwnerDrawnListControl *list, CDC *pdc,     CRect rc, const UINT state                       ) const;

protected:
	
	void DrawLabel                           ( COwnerDrawnListControl *list, CImageList *il, CDC *pdc,              CRect& rc, const UINT state, int *width, int *focusLeft, const bool indent = true) const;
	void DrawPercentage                      ( CDC *pdc,                     CRect rc,       const double fraction, const COLORREF color                                                             ) const;
};


//
// COwnerDrawnListControl. Must be report view. Deals with COwnerDrawnListItems.
// Can have a grid or not (own implementation, don't set LVS_EX_GRIDLINES). Flicker-free.
//
class COwnerDrawnListControl: public CSortingListControl
{
	DECLARE_DYNAMIC(COwnerDrawnListControl)
public:
	COwnerDrawnListControl          ( LPCTSTR name, int rowHeight );
	virtual ~COwnerDrawnListControl ( );
	virtual void SysColorChanged    ( );

	
	int FindListItem                         ( const COwnerDrawnListItem *item   );
	int GetGeneralLeftIndent                 (                                   );
	int GetRowHeight                         (                                   );
	int GetTextXMargin                       (                                   );
	
	void AdjustColumnWidth                   ( const int col                     );
	void OnColumnsInserted                   (                                   );
	void ShowGrid                            ( const bool show                   );
	void ShowStripes                         ( const bool show                   );
	void ShowFullRowSelection                ( const bool show                   );
	
	COLORREF GetHighlightColor               (                                   );
	COLORREF GetHighlightTextColor           (                                   );
	COLORREF GetItemBackgroundColor          ( const int i                       );
	COLORREF GetItemBackgroundColor          ( const COwnerDrawnListItem *item   );
	COLORREF GetItemSelectionBackgroundColor ( const int i                       );
	COLORREF GetItemSelectionBackgroundColor ( const COwnerDrawnListItem *item   );
	COLORREF GetItemSelectionTextColor       ( const int i                       );
	COLORREF GetNonFocusHighlightColor       (                                   ) const;
	COLORREF GetNonFocusHighlightTextColor   (                                   ) const;
	COLORREF GetStripeColor                  (                                   ) const;
	COLORREF GetWindowColor                  (                                   ) const;

	bool HasFocus                            (                                   );
	bool IsFullRowSelection                  (                                   ) const;
	bool IsItemStripeColor                   ( const int i                       ) const;
	bool IsItemStripeColor                   ( const COwnerDrawnListItem *item   );
	bool IsShowSelectionAlways               (                                   );

	COwnerDrawnListItem *GetItem             ( const int i                       );
	
	CRect GetWholeSubitemRect                ( const int item, const int subitem );

protected:

	virtual void DrawItem                    ( _In_ LPDRAWITEMSTRUCT pdis                   );
	void         InitializeColors            (                                              );
	bool         IsColumnRightAligned        ( const int col                                );//const?
	int          GetSubItemWidth             ( COwnerDrawnListItem *item, const int subitem );//const?

	int      m_rowHeight;	          // Height of an item
	bool     m_showGrid;	          // Whether to draw a grid
	bool     m_showStripes;	          // Whether to show stripes
	bool     m_showFullRowSelection;  // Whether to draw full row selection
	int      m_yFirstItem;	          // Top of a first list item
	COLORREF m_windowColor;	          // The default background color if !m_showStripes
	COLORREF m_stripeColor;	          // The stripe color, used for every other item if m_showStripes

	DECLARE_MESSAGE_MAP()
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnHdnDividerdblclick(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnHdnItemchanging(NMHDR *pNMHDR, LRESULT *pResult);
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
