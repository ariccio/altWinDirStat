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
#else
#error ass
#endif

#pragma once
#include "stdafx.h"


class COwnerDrawnListItem;
class COwnerDrawnListCtrl;


// COwnerDrawnListItem. An item in a COwnerDrawnListCtrl.
// Some columns (subitems) may be owner drawn (DrawSubitem() returns true), COwnerDrawnListCtrl draws the texts (GetText()) of all others.
// DrawLabel() draws a standard label (width image, text, selection and focus rect)
class COwnerDrawnListItem {
public:

	COwnerDrawnListItem( _In_z_ PCWSTR name, const std::uint16_t length ) : m_name( name ), m_name_length( length ) { }
	COwnerDrawnListItem( ) { }
	~COwnerDrawnListItem( ) {
		delete[ ] m_name;
		}
	INT          compare_interface            ( _In_ const COwnerDrawnListItem* const other, RANGE_ENUM_COL const column::ENUM_COL subitem ) const;
	INT          CompareS                     ( _In_ const COwnerDrawnListItem* const other, _In_ const SSorting& sorting ) const;
	bool         DrawSubitem_                 ( RANGE_ENUM_COL const column::ENUM_COL subitem, _In_ CDC& pdc, _In_ CRect rc, _In_ const UINT state, _Out_opt_ INT* const width, _Inout_ INT* const focusLeft ) const;
	void         DrawSelection                ( _In_ const COwnerDrawnListCtrl* const list, _In_ CDC& pdc,       _Inout_ CRect rc, _In_ const UINT state                       ) const;
	std::wstring GetText                      ( RANGE_ENUM_COL const column::ENUM_COL subitem ) const; // This text is drawn, if DrawSubitem returns false

	COLORREF    item_text_color( ) const;

	COLORREF     default_item_text_color      ( ) const;
	
	_Must_inspect_result_ _Success_( SUCCEEDED( return ) )
	HRESULT      GetText_WriteToStackBuffer   ( RANGE_ENUM_COL const column::ENUM_COL subitem, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, _Inout_ rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const;
protected:
	void         DrawLabel                    ( _In_ COwnerDrawnListCtrl* const list, _In_ CDC& pdc, _In_ CRect& rc, _In_ const UINT state, _Out_opt_ INT* const width, _Inout_ INT* const focusLeft, _In_ const bool indent ) const;
	void         DrawPercentage               ( _In_ CDC& pdc, _In_ CRect rc,       _In_ const DOUBLE fraction, _In_ const COLORREF color                                                             ) const;
	void         DrawColorTranspBackground    ( _In_ CRect& rcRest, _In_ CImageList* const il, _In_ CDC& pdc ) const;
	void         DrawHighlightSelectBackground( _In_ const CRect& rcLabel, _In_ const CRect& rc, _In_ const COwnerDrawnListCtrl* const list, _In_ CDC& pdc, _Inout_ COLORREF& textColor ) const;
	void         AdjustLabelForMargin         ( _In_ const CRect& rcRest, _Inout_ CRect& rcLabel ) const;


private:
	virtual INT          Compare                ( _In_ const COwnerDrawnListItem* const other, RANGE_ENUM_COL const column::ENUM_COL subitem ) const;
	
	virtual std::wstring Text                   ( RANGE_ENUM_COL const column::ENUM_COL subitem ) const = 0;

	_Must_inspect_result_ _On_failure_( _Post_satisfies_( sizeBuffNeed == SIZE_T_ERROR ) ) _Success_( SUCCEEDED( return ) )
	virtual HRESULT      Text_WriteToStackBuffer( RANGE_ENUM_COL const column::ENUM_COL subitem, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, _Inout_ rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const = 0;

	virtual COLORREF     ItemTextColor        ( ) const;

	// Return value is true, if the item draws itself. width != NULL -> only determine width, do not draw. If focus rectangle shall not begin leftmost, set *focusLeft to the left edge of the desired focus rectangle.
	virtual bool         DrawSubitem            ( RANGE_ENUM_COL const column::ENUM_COL subitem, _In_ CDC& pdc, _In_ CRect rc, _In_ const UINT state, _Out_opt_ INT* const width, _Inout_ INT* const focusLeft ) const = 0;

	public:
	//std::wstring m_name; // Display name
	_Field_z_ _Field_size_( m_name_length ) PCWSTR        m_name;
	                                        std::uint16_t m_name_length;
	};


//
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
	
	void InsertListItem                      ( _In_ const INT_PTR i, _In_ const COwnerDrawnListItem* const item );

	void AdjustColumnWidth                   ( RANGE_ENUM_COL const column::ENUM_COL col              );
	void OnColumnsInserted                   (                                              );
	void AddExtendedStyle                    ( _In_ const DWORD     exStyle );
	COLORREF GetItemSelectionBackgroundColor ( _In_ _In_range_( 0, INT_MAX )   const INT i  ) const;
	COLORREF GetItemSelectionTextColor       ( _In_ _In_range_( 0, INT_MAX )   const INT i  ) const;
	
	CRect GetWholeSubitemRect                ( _In_ const INT item, _In_ const INT subitem  ) const;

	_Must_inspect_result_ _Success_( return != NULL ) _Ret_maybenull_
	COwnerDrawnListItem* GetItem             ( _In_ _In_range_( 0, INT_MAX )   const int i  ) const;

	virtual void SysColorChanged( ) {
		InitializeColors( );
		}

	void SetSorting( RANGE_ENUM_COL const column::ENUM_COL       sortColumn, _In_ const bool ascending ) {
		m_sorting.ascending2 = m_sorting.ascending1;
		m_sorting.column1    = sortColumn;
		m_sorting.column2    = m_sorting.column1;
		m_sorting.ascending1 = ascending;
		}

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
	
	_Success_( return != COLORREF( 0 ) )
	COLORREF GetHighlightTextColor( ) const {
		if ( HasFocus( ) ) {
			return GetSysColor( COLOR_HIGHLIGHTTEXT );
			}
		return RGB( 0, 0, 0 );
		}


	_Success_( return != COLORREF( 0 ) )
	COLORREF GetItemBackgroundColor( _In_ _In_range_( 0, INT_MAX )   const INT i ) const {
		return ( IsItemStripeColor( i ) ? m_stripeColor : m_windowColor );
		}

	bool IsItemStripeColor( _In_ _In_range_( 0, INT_MAX )   const INT i ) const {
		return ( m_showStripes && ( i % 2 != 0 ) );
		}

	_Success_( return != COLORREF( 0 ) )
	COLORREF GetItemBackgroundColor( _In_ const COwnerDrawnListItem* const item ) const {
		const auto itemPos = FindListItem( item );
		if ( itemPos != -1 ) {
			return GetItemBackgroundColor( itemPos );
			}
		return COLORREF( 0 );
		}
	_Success_( return != COLORREF( 0 ) )
	COLORREF GetItemSelectionBackgroundColor( _In_ const COwnerDrawnListItem* const item ) const {
		const auto itemPos = FindListItem( item );
		if ( itemPos != -1 ) {
			return GetItemSelectionBackgroundColor( itemPos );
			}
		return COLORREF( 0 );
		}
	bool IsItemStripeColor( _In_ const COwnerDrawnListItem* const item ) const {
		const auto itemPos = FindListItem( item );
		if ( itemPos != -1 ) {
			return IsItemStripeColor( itemPos );
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


protected:
	
	//void OnVscroll( HWND hwnd, HWND hwndCtl, UINT code, int pos );

	// Overridables
	virtual bool GetAscendingDefault ( _In_ const column::ENUM_COL column ) const {
		UNREFERENCED_PARAMETER( column );
		return true;
		}


	virtual void DrawItem                    ( _In_ PDRAWITEMSTRUCT pdis                   );
	void         DoDrawSubItemBecauseItCannotDrawItself( _In_ const COwnerDrawnListItem* const item, _In_ _In_range_( 0, INT_MAX ) const column::ENUM_COL subitem, _In_ CDC& dcmem, _In_ CRect& rcDraw, _In_ const PDRAWITEMSTRUCT& pdis, _In_ const bool showSelectionAlways, _In_ const bool bIsFullRowSelection, const std::vector<bool>& is_right_aligned_cache ) const;

	_Success_( SUCCEEDED( return ) )
	HRESULT      drawSubItem_stackbuffer     ( _In_ const COwnerDrawnListItem* const item, _In_ CRect& rcText, const int& align, _In_ _In_range_( 0, INT_MAX ) const column::ENUM_COL subitem, _In_ CDC& dcmem ) const;

	void         DrawText_dynamic            ( _In_ const COwnerDrawnListItem* const item, _In_ CRect& rcText, const int& align, _In_ _In_range_( 0, INT_MAX ) const column::ENUM_COL subitem, _In_ CDC& dcmem ) const;

	void         InitializeColors            (                                              );
	bool         IsColumnRightAligned        ( _In_ const INT col                                ) const;
	
	_Success_( return >= 0 ) _Ret_range_( 0, INT_MAX ) _On_failure_( _Ret_range_( -1, -1 ) )
	INT          GetSubItemWidth             ( _In_ const COwnerDrawnListItem* const item, _In_ _In_range_( 0, INT_MAX ) const column::ENUM_COL subitem ) const;

	public:
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

protected:

	void buildArrayFromItemsInHeaderControl( _In_ CArray<INT, INT>& columnOrder, _Inout_ CArray<INT, INT>& vertical );

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
