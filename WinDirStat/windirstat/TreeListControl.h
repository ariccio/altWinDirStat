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
#include "ownerdrawnlistcontrol.h"
//#include "pacman.h"

class CTreeListItem;
class CTreeListControl;
class CSortingListItem;
class CImageList;

//
// CTreeListItem. An item in the CTreeListControl. (CItem is derived from CTreeListItem.)
// In order to save memory, once the item is actually inserted in the List, we allocate the VISIBLEINFO structure (m_vi).
// m_vi is freed as soon as the item is removed from the List.
class CTreeListItem : public COwnerDrawnListItem {
	// Data needed to display the item.
	struct VISIBLEINFO {
		VISIBLEINFO( ) : sizeCache( UINT64_ERROR ), indent( 0 ), isExpanded( 0 ) { }

		SRECT  rcPlusMinus;     // Coordinates of the little +/- rectangle, relative to the upper left corner of the item.
		SRECT  rcTitle;         // Coordinates of the label, relative to the upper left corner of the item.
		// sortedChildren: This member contains our children (the same set of children as in CItem::m_children) and is initialized as soon as we are expanded.
		// In contrast to CItem::m_children, this array is always sorted depending on the current user-defined sort column and -order.
		std::vector<CTreeListItem *> sortedChildren;
		_Field_range_( 0, 18446744073709551615 ) std::uint64_t sizeCache;
		_Field_range_( 0, 32767 )                std::int16_t  indent;  // 0 for the root item, 1 for its children, and so on.
		                                         bool          isExpanded : 1; // Whether item is expanded.
		};

	public:
		CTreeListItem( ) : m_parent( NULL ), m_vi( NULL ) { }
		//CTreeListItem( CTreeListItem&& in ) = delete;
		CTreeListItem( CTreeListItem& in ) = delete;
		virtual ~CTreeListItem( ) {
			delete m_vi;
			m_vi = { NULL };
			m_parent = { NULL };
			}

		virtual size_t         GetChildrenCount( ) const = 0;
		virtual INT            Compare          ( _In_ const COwnerDrawnListItem* const other, _In_ _In_range_( 0, 7 ) const INT subitem                          ) const override final;
		virtual bool           DrawSubitem      ( _In_ _In_range_( 0, 7 ) const ENUM_COL subitem,             _In_ CDC& pdc,         _In_ CRect rc, _In_ const UINT state, _Out_opt_ INT* const width, _Inout_ INT* const focusLeft ) const override;
		
		_Success_( return != NULL ) _Must_inspect_result_ _Ret_maybenull_         CTreeListItem* GetSortedChild   ( _In_ const size_t i                             ) const;
		//_Success_( return != NULL ) _Must_inspect_result_ _Ret_maybenull_         CTreeListItem* GetParent( ) const {
		//	return m_parent;
		//	}

		_Pre_satisfies_( this->m_vi != NULL ) std::int16_t  GetIndent( ) const {
			ASSERT( IsVisible( ) );
			return m_vi->indent;
			}


		size_t  FindSortedChild                 ( _In_ const CTreeListItem* const child                                               ) const;

		_Pre_satisfies_( this->m_vi != NULL ) void SetExpanded( _In_ const bool expanded = true ) {
			ASSERT( IsVisible( ) );
			m_vi->isExpanded = expanded;
			}


		_Pre_satisfies_( this->m_vi != NULL ) void SetPlusMinusRect( _In_ const CRect& rc ) const {
			ASSERT( IsVisible( ) );
			m_vi->rcPlusMinus = SRECT( rc );
			}

		_Pre_satisfies_( this->m_vi != NULL ) void SetTitleRect( _In_ const CRect& rc ) const {
			ASSERT( IsVisible( ) );
			m_vi->rcTitle = SRECT( rc );
			}

		//_At_( this->m_vi, _When_( next_state_visible, _Post_valid_ ) ) _At_( this->m_vi, _When_( ( !next_state_visible ), _Post_ptr_invalid_ ) ) 
		//_At_( this->m_vi, _When_( next_state_visible == false, _Post_ptr_invalid_ ) )
		//_At_( this->m_vi, _When_( next_state_visible == false, _Post_invalid_ ) )
		void SetVisible ( _In_ const bool next_state_visible = true ) const;
		_Pre_satisfies_( this->m_vi != NULL ) void SortChildren                       (                                               );
		_Pre_satisfies_( this->m_parent != NULL ) bool  HasSiblings                       (                                           ) const;
		
		void childNotNull( CItemBranch* const aTreeListChild, const size_t i );
		bool  HasChildren( ) const {
			return GetChildrenCount( ) > 0;
			}
		
		_Pre_satisfies_( this->m_vi != NULL ) bool IsExpanded( ) const {
			ASSERT( IsVisible( ) );
			return m_vi->isExpanded; 
			}
		bool  IsVisible( ) const {
			return ( m_vi != NULL );
			}
	
		_Pre_satisfies_( this->m_vi != NULL ) CRect GetPlusMinusRect( ) const;

		_Pre_satisfies_( this->m_vi != NULL ) CRect GetTitleRect( ) const;
	public:
		static bool _compareProc2( const CTreeListItem* const lhs, const CTreeListItem* const rhs );
		
		_Ret_notnull_ static CTreeListControl* GetTreeListControl( );

		//void SetScrollPosition                                            ( _In_ _In_range_( 0, INT_MAX ) const INT top             );
		//_Success_( return != -1 ) INT  GetScrollPosition                  (                                );

	public:
		CTreeListItem*       m_parent;
	//private:
		mutable VISIBLEINFO* m_vi = NULL;
	};


//
// CTreeListControl. A CListCtrl, which additionally behaves an looks like a tree control.
//
class CTreeListControl : public COwnerDrawnListControl {
	DECLARE_DYNAMIC( CTreeListControl )

	// In order to save memory, and as we have only one CTreeListControl in the application, this is global.
	

	public:
		_Pre_satisfies_( _theTreeListControl != NULL )
		static CTreeListControl *GetTheTreeListControl( ) {
			ASSERT( _theTreeListControl != NULL );
			return _theTreeListControl;
			}

		_Pre_satisfies_( rowHeight % 2 == 0 )
		CTreeListControl( _In_range_( 0, NODE_HEIGHT ) UINT rowHeight ) : COwnerDrawnListControl( _T( "treelist" ), rowHeight ) {
			ASSERT( _theTreeListControl == NULL );
			_theTreeListControl = this;
			ASSERT( rowHeight <= NODE_HEIGHT );     // größer können wir nicht//"larger, we can not"?
			ASSERT( rowHeight % 2 == 0 );           // muss gerade sein//"must be straight"?
			}
		
		virtual ~CTreeListControl( ) {
			_theTreeListControl = { NULL };
			}
#pragma warning( suppress: 4263 )
		        BOOL CreateEx( _In_ const DWORD dwExStyle, _In_ DWORD dwStyle, _In_ const RECT& rect, _In_ CWnd* pParentWnd, _In_ const UINT nID );

		virtual void SysColorChanged       ( ) override final;
				bool SelectedItemCanToggle ( ) const;
				void Sort                  ( );
				void ToggleSelectedItem    ( );


		_Must_inspect_result_ _Success_( return != NULL ) _Ret_maybenull_
				CTreeListItem* GetItem( _In_ _In_range_( 0, INT_MAX ) const INT_PTR i ) const;
				void SetRootItem                               ( _In_opt_ const CTreeListItem* const root                     );
		_Pre_satisfies_( !isDone )
				void OnChildAdded                              ( _In_opt_ const CTreeListItem* const parent, _In_ CTreeListItem* const child, _In_ const bool isDone );
		_Success_( return != -1 ) _Ret_range_( -1, INT_MAX )
				INT  FindTreeItem                              ( _In_     const CTreeListItem* const item ) const;
				INT  GetItemScrollPosition                     ( _In_     const CTreeListItem* const item ) const;
				int  EnumNode                                  ( _In_     const CTreeListItem* const item ) const;	
				void thisPathNotNull                           ( _In_     const CTreeListItem* const thisPath, const std::int64_t i, int& parent, _In_ const bool showWholePath, const std::vector<const CTreeListItem *>& path );
				void pathZeroNotNull                           ( _In_     const CTreeListItem* const pathZero, _In_range_( 0, INT_MAX ) const int     index, _In_ const bool showWholePath );
				void doWhateverJDoes                           ( _In_     const CTreeListItem* const pathZero, _In_range_( 0, INT_MAX ) const INT_PTR parent );
				void adjustColumnSize                          ( _In_     const CTreeListItem* const item_at_index );
				void SelectAndShowItem                         ( _In_     const CTreeListItem* const item, _In_ const bool showWholePath                                                           );
				void SelectItem                                ( _In_     const CTreeListItem* const item );
				void EnsureItemVisible                         ( _In_     const CTreeListItem* const item                                                                                    );
				void ExpandItem                                ( _In_     const CTreeListItem* const item                                                                                          );
				void handle_VK_LEFT                            ( _In_     const CTreeListItem* const item, _In_ _In_range_( 0, INT32_MAX ) const int i );
				void handle_VK_RIGHT                           ( _In_     const CTreeListItem* const item, _In_ _In_range_( 0, INT32_MAX ) const INT_PTR i );
				void SetItemScrollPosition                     ( _In_     const CTreeListItem* const item, _In_ const INT top );


				void DrawNode                                  ( _In_ CDC& pdc,                  _Inout_ CRect& rc,              _Inout_ CRect& rcPlusMinus, _In_ const CTreeListItem* const item );
				void DrawNodeNullWidth                         ( _In_ CDC& pdc, _In_ const CRect& rcRest, _In_ const CTreeListItem* const item, _Inout_ bool& didBitBlt, _In_ CDC& dcmem, _In_ const unsigned int ysrc );
		_Pre_satisfies_( ( parent + 1 ) < index )
				void CollapseKThroughIndex                     ( _Inout_ _Out_range_( -1, INT_MAX ) int& index, const int parent, const std::wstring text, const std::int64_t i, _In_ const CTreeListItem* thisPath );
	protected:
		
		virtual void OnItemDoubleClick                         ( _In_ _In_range_( 0, INT_MAX )   const INT i ) { ToggleExpansion( i ); }
				void ExpandItemInsertChildren                  ( _In_ _In_range_( 0, INT32_MAX ) const INT_PTR i, _In_ const bool scroll, _In_ const CTreeListItem* const item );
				void InsertItem                                ( _In_ _In_range_( 0, INT32_MAX ) const INT_PTR i, _In_ const CTreeListItem* const item );
				void ExpandItem                                ( _In_ _In_range_( 0, INT32_MAX ) const INT_PTR i, _In_ const bool scroll = true );
				void DeleteItem                                ( _In_ _In_range_( 0, INT_MAX )   const INT i                           );
				void ToggleExpansion                           ( _In_ _In_range_( 0, INT_MAX )   const INT i                           );
				void SelectItem                                ( _In_ _In_range_( 0, INT_MAX )   const INT i );
				void insertItemsAdjustWidths                   ( _In_ _In_range_( 1, SIZE_T_MAX ) const size_t count, _In_ const CTreeListItem* const item, _Inout_ _Out_range_( 0, INT_MAX ) INT& maxwidth, _In_ const bool scroll, _In_ _In_range_( 0, INT_MAX ) const INT_PTR i );
				INT  countItemsToDelete                        ( bool& selectNode, _In_ _In_range_( 0, INT_MAX ) const INT& i, _In_ const CTreeListItem* const item );
		_Success_( return == true )
				bool CollapseItem                              ( _In_ _In_range_( 0, INT_MAX )   const INT i                           );
				
				
		
				

		_Must_inspect_result_ _Success_( return != -1 )
				INT  GetSelectedItem( ) const;
				void InitializeNodeBitmaps                     (             );
		static CTreeListControl* _theTreeListControl;
			   CBitmap           m_bmNodes0;                   // The bitmaps needed to draw the treecontrol-like branches
			   CBitmap           m_bmNodes1;                   // The same bitmaps with stripe-background color
			   CImageList*       m_imageList;                  // We don't use the system-supplied SetImageList(), but MySetImageList().
			   INT               m_lButtonDownItem;            // Set in OnLButtonDown(). -1 if not item hit.
			   bool              m_lButtonDownOnPlusMinusRect; // Set in OnLButtonDown(). True, if plus-minus-rect hit.


		DECLARE_MESSAGE_MAP()

		afx_msg void MeasureItem( _In_ PMEASUREITEMSTRUCT mis ) {
			mis->itemHeight = static_cast<UINT>( m_rowHeight );
			}
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
