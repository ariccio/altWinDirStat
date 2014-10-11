// TreeListControl.h - Declaration of CTreeListItem and CTreeListControl
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


#ifndef TREELISTCONTROL_H
#define TREELISTCONTROL_H
#else
#error ass
#endif

#pragma once

#include "stdafx.h"
//#include "ownerdrawnlistcontrol.h"
//#include "pacman.h"

class CTreeListItem;
class CTreeListControl;


//
// CTreeListItem. An item in the CTreeListControl. (CItem is derived from CTreeListItem.)
// In order to save memory, once the item is actually inserted in the List, we allocate the VISIBLEINFO structure (m_vi).
// m_vi is freed as soon as the item is removed from the List.
class CTreeListItem : public COwnerDrawnListItem {
	// Data needed to display the item.
	struct VISIBLEINFO {
		
		SRECT  rcPlusMinus;     // Coordinates of the little +/- rectangle, relative to the upper left corner of the item.
		SRECT  rcTitle;         // Coordinates of the label, relative to the upper left corner of the item.

#ifdef DRAW_ICONS
		_Field_range_( -1, INT_MAX ) INT    image;		// -1 as long as not needed, >= 0: valid index in MyImageList.
#endif
		// sortedChildren: This member contains our children (the same set of children as in CItem::m_children) and is initialized as soon as we are expanded.
		// In contrast to CItem::m_children, this array is always sorted depending on the current user-defined sort column and -order.
		std::vector<CTreeListItem *> sortedChildren;
		_Field_range_( 0, 32767 ) std::int16_t    indent;  // 0 for the root item, 1 for its children, and so on.

		bool                         isExpanded : 1; // Whether item is expanded.
		


		};

	public:
		CTreeListItem( ) : m_parent( NULL ), m_vi( NULL ) { }
		CTreeListItem( CTreeListItem&& in );
		virtual ~CTreeListItem( );

		virtual INT            Compare          ( _In_ const CSortingListItem* const other, _In_ const INT subitem                          ) const override;
		virtual INT            CompareSibling   ( _In_ const CTreeListItem* const tlib,     _In_ _In_range_( 0, INT32_MAX ) const INT subitem                                                              ) const = 0;
		virtual bool           DrawSubitem      ( _In_ _In_range_( 0, INT_MAX ) const INT subitem,             _In_ CDC* pdc,         _In_ CRect rc, _In_ const UINT state, _Inout_opt_ INT* width, _Inout_ INT* focusLeft ) const;
		virtual size_t         GetChildrenCount (                                                                                     ) const = 0;
		

#ifdef DRAW_ICONS
		virtual INT            GetImageToCache  (                                                                                     ) const = 0;
		virtual INT            GetImage         (                                                                                     ) const;
		void UncacheImage                       (                                                                                     );
#endif
		_Success_( return != NULL ) _Must_inspect_result_ _Ret_maybenull_ virtual CTreeListItem* GetTreeListChild ( _In_ _In_range_( 0, INT32_MAX ) const size_t i  ) const = 0;
		_Success_( return != NULL ) _Must_inspect_result_ _Ret_maybenull_         CTreeListItem* GetSortedChild   ( _In_ const size_t i  );
		_Success_( return != NULL ) _Must_inspect_result_ _Ret_maybenull_         CTreeListItem* GetParent        (                                                 ) const;


		std::int16_t  GetIndent                 (                                                                                     ) const;
		size_t  FindSortedChild                 ( _In_ const CTreeListItem* const child                                                     );

		void SetExpanded                        ( _In_ const bool expanded = true                                                     );
		//void SetParent                          ( _In_ CTreeListItem* parent                                                          );
		void SetPlusMinusRect                   ( _In_ const CRect& rc                                                                ) const;
		void SetTitleRect                       ( _In_ const CRect& rc                                                                ) const;
		void SetVisible                         ( _In_ const bool next_state_visible = true                                           );
		_Pre_satisfies_( m_vi != NULL ) void SortChildren                       (                                                                                     );

	
		bool  HasSiblings                       (                                                                                     ) const;
		bool  HasChildren                       (                                                                                     ) const;
		bool  IsExpanded                        (                                                                                     ) const;
		bool  IsVisible                         (                                                                                     ) const;
	
		CRect GetPlusMinusRect                  (                                                                                     ) const;
		CRect GetTitleRect                      (                                                                                     ) const;
	
	protected:
		//static INT __cdecl  _compareProc( _In_ const void* p1, _In_ const void* p2 );
		static bool _compareProc2( CTreeListItem* lhs, CTreeListItem* rhs );
		
		_Must_inspect_result_ _Ret_maybenull_ static CTreeListControl* GetTreeListControl (                                );
		void SetScrollPosition                                            ( _In_ _In_range_( 0, INT_MAX ) const INT top             );
		_Success_( return != -1 ) INT  GetScrollPosition                  (                                );

	public:
		CTreeListItem*       m_parent;
	private:
		mutable VISIBLEINFO* m_vi;
	};


//
// CTreeListControl. A CListCtrl, which additionally behaves an looks like a tree control.
//
class CTreeListControl : public COwnerDrawnListControl {
	DECLARE_DYNAMIC( CTreeListControl )

	// In order to save memory, and as we have only one CTreeListControl in the application, this is global.
	

	public:
		_Must_inspect_result_ _Ret_maybenull_ static CTreeListControl *GetTheTreeListControl ( );

		_Pre_satisfies_( rowHeight % 2 == 0 ) CTreeListControl( INT rowHeight = -1 );
		
		virtual ~CTreeListControl( );
		virtual BOOL CreateEx                          ( _In_ const DWORD dwExStyle, _In_ DWORD dwStyle, _In_ const RECT& rect, _In_ CWnd* pParentWnd, _In_ const UINT nID );
		virtual void SysColorChanged                   ( );
		//virtual bool HasImages( ) const;


		void MySetImageList                            ( _In_opt_ CImageList* il                      ) { m_imageList = il; }
		void SetItemScrollPosition                     ( _In_ const CTreeListItem* const item, _In_ const INT top );
		void SetRootItem                               ( _In_opt_ CTreeListItem* root                 );
		//void OnChildAdded                              ( _In_ const CTreeListItem* const parent, _In_ CTreeListItem* child     );
		void OnChildAdded                              ( _In_ const CTreeListItem* const parent, _In_ CTreeListItem* child, _In_ bool isDone );
		//void OnChildRemoved                            ( _In_ CTreeListItem* parent, _In_ CTreeListItem* childdata );
		//void OnRemovingAllChildren                     ( _In_ CTreeListItem* parent                           );
		
		_Must_inspect_result_ _Success_( return != NULL ) _Ret_maybenull_ CTreeListItem *GetItem                         ( _In_ _In_range_( 0, INT_MAX ) const INT_PTR i         );

		INT  GetItemScrollPosition                     ( _In_ const CTreeListItem* const item );
		
		void DeselectAll                               (                                                                                                              );
		void Sort                                      (                                                                                                              );
		void ToggleSelectedItem                        (                                                                                                              );
		
		void SelectAndShowItem                         ( _In_ const CTreeListItem* item, _In_ const bool showWholePath                                                           );
		void DrawNode                                  ( _In_ CDC* pdc,                  _In_ CRect& rc,              _Inout_ CRect& rcPlusMinus, _In_ const CTreeListItem* item );
		void SelectItem                                ( _In_ const CTreeListItem* item                                                                                    );
		void EnsureItemVisible                         ( _In_ const CTreeListItem* item                                                                                    );
		void ExpandItem                                ( _In_ CTreeListItem* item                                                                                          );
		_Success_( return != -1 ) INT  FindTreeItem                              ( _In_ const CTreeListItem* item                                                                                    );
		bool SelectedItemCanToggle                     (                                                                                                              );
		

		void DrawNodeNullWidth( _In_ CDC* pdc, _In_ CRect& rcRest, _In_ const CTreeListItem* const item, _Inout_ bool& didBitBlt, _In_ CDC& dcmem, _In_ unsigned int ysrc );
		int  EnumNode( _In_ const CTreeListItem* const item );


	protected:
		
		virtual void OnItemDoubleClick                 ( _In_ _In_range_( 0, INT_MAX ) const INT i ) { ToggleExpansion( i ); }
		void         InitializeNodeBitmaps             (             );


		void ExpandItemInsertChildren( _In_ _In_range_( 0, INT_MAX ) const INT_PTR i, _In_ const bool scroll, _In_ CTreeListItem* item );

		void InsertItem                                ( _In_ _In_range_( 0, INT_MAX ) const INT_PTR i, _In_ CTreeListItem* item      );
		void DeleteItem                                ( _In_ _In_range_( 0, INT_MAX ) const INT i                           );
		_Success_( return == true ) bool CollapseItem                              ( _In_ _In_range_( 0, INT_MAX ) const INT i                           );
		void ExpandItem                                ( _In_ _In_range_( 0, INT_MAX ) const INT_PTR i, _In_ const bool scroll = true );
		void ToggleExpansion                           ( _In_ _In_range_( 0, INT_MAX ) const INT i                           );
		void SelectItem                                ( _In_ _In_range_( 0, INT_MAX ) const INT i                           );
		_Must_inspect_result_ _Success_( return != -1 ) INT GetSelectedItem( ) const;

		static CTreeListControl* _theTreeListControl;

		CBitmap     m_bmNodes0;			            // The bitmaps needed to draw the treecontrol-like branches
		CBitmap     m_bmNodes1;			            // The same bitmaps with stripe-background color
		CImageList* m_imageList;	                // We don't use the system-supplied SetImageList(), but MySetImageList().
		INT         m_lButtonDownItem;		        // Set in OnLButtonDown(). -1 if not item hit.
		bool        m_lButtonDownOnPlusMinusRect;	// Set in OnLButtonDown(). True, if plus-minus-rect hit.

		DECLARE_MESSAGE_MAP()

		afx_msg void MeasureItem(LPMEASUREITEMSTRUCT mis);
		afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
		afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
		afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
		afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};



// $Log$
// Revision 1.8  2004/11/13 08:17:07  bseifert
// Remove blanks in Unicode Configuration names.
//
// Revision 1.7  2004/11/12 22:14:16  bseifert
// Eliminated CLR_NONE. Minor corrections.
//
// Revision 1.6  2004/11/12 00:47:42  assarbad
// - Fixed the code for coloring of compressed/encrypted items. Now the coloring spans the full row!
//
// Revision 1.5  2004/11/08 00:46:26  assarbad
// - Added feature to distinguish compressed and encrypted files/folders by color as in the Windows 2000/XP explorer.
//   Same rules apply. (Green = encrypted / Blue = compressed)
//
// Revision 1.4  2004/11/05 16:53:07  assarbad
// Added Date and History tag where appropriate.
//
