// TreeListControl.h - Declaration of CTreeListItem and CTreeListControl
//
// see `file_header_text.txt` for licensing & contact info. If you can't find that file, then assume you're NOT allowed to do whatever you wanted to do.

#pragma once

#include "stdafx.h"

#ifndef WDS_TREELISTCONTROL_H
#define WDS_TREELISTCONTROL_H

WDS_FILE_INCLUDE_MESSAGE

#include "datastructures.h"
#include "ownerdrawnlistcontrol.h"
#include "ChildrenHeapManager.h"
#include "macros_that_scare_small_children.h"


class CTreeListItem;
class CTreeListControl;
class CSortingListItem;
class CImageList;
class CDirstatDoc;




struct attribs final {
	bool readonly   : 1;
	bool hidden     : 1;
	bool system     : 1;
  //bool archive    : 1;//Nobody actually cares about the archive attribute!
	bool compressed : 1;
	bool encrypted  : 1;
	bool reparse    : 1;
	bool invalid    : 1;

	//Not a file attribute, but a member of attribs to enable better packing
	bool m_done     : 1;
	};


struct VISIBLEINFO final {
	__forceinline VISIBLEINFO( ) : indent( 0 ), isExpanded { false }, ntfs_compression_ratio { 0.0 } { }
	__forceinline ~VISIBLEINFO( ) = default;

	                          SRECT                        rcPlusMinus;     // Coordinates of the little +/- rectangle, relative to the upper left corner of the item.
	                          SRECT                        rcTitle;         // Coordinates of the label, relative to the upper left corner of the item.
	                          std::vector<const CTreeListItem *> cache_sortedChildren; // cache_sortedChildren: This member contains our children (the same set of children as in CItem::m_children) and is initialized as soon as we are expanded. // In contrast to CItem::m_children, this array is always sorted depending on the current user-defined sort column and -order.
	_Field_range_( 0, 32767 ) std::int16_t                 indent;  // 0 for the root item, 1 for its children, and so on.
		                      bool                         isExpanded : 1; // Whether item is expanded.
							  double                       ntfs_compression_ratio;
	};

// `/d1reportSingleClassLayoutCItemBranch`
// CTreeListItem. An item in the CTreeListControl. (CItem is derived from CTreeListItem.)
// In order to save memory, once the item is actually inserted in the List, we allocate the VISIBLEINFO structure (m_vi).
// m_vi is freed as soon as the item is removed from the List.
class CTreeListItem final : public COwnerDrawnListItem {
	
		virtual bool   DrawSubitem( RANGE_ENUM_COL const column::ENUM_COL subitem, _In_ HDC hDC, _In_ RECT rc, _In_ const UINT state, _Out_opt_ INT* const width, _Inout_ INT* const focusLeft, _In_ const COwnerDrawnListCtrl* const list ) const noexcept override final;
		
		virtual INT Compare( _In_ const COwnerDrawnListItem* const other, RANGE_ENUM_COL const column::ENUM_COL subitem                          ) const noexcept override final;

		//ItemTextColor __should__ be private!
		virtual COLORREF ItemTextColor( ) const noexcept override final {
			return Concrete_ItemTextColor( );
			}

		inline INT     concrete_compare( _In_ const CTreeListItem* const other, RANGE_ENUM_COL const column::ENUM_COL subitem ) const noexcept;
		const bool     set_plusminus_and_title_rects( _In_ const RECT rcLabel, _In_ const RECT rc_const ) const;

		const COLORREF Concrete_ItemTextColor( ) const noexcept;

		CTreeListItem( CTreeListItem& in ) = delete;
		CTreeListItem& operator=( const CTreeListItem& in ) = delete;


		
	public:
		//Unconditionally called only ONCE, so we ask for inlining.
		//Encodes the attributes to fit (in) 1 byte
		__forceinline void SetAttributes( _In_ const DWORD attr ) noexcept {
			if ( attr == INVALID_FILE_ATTRIBUTES ) {
				m_attr.invalid = true;
				return;
				}
			m_attr.readonly   = ( ( attr bitand FILE_ATTRIBUTE_READONLY      ) != 0 );
			m_attr.hidden     = ( ( attr bitand FILE_ATTRIBUTE_HIDDEN        ) != 0 );
			m_attr.system     = ( ( attr bitand FILE_ATTRIBUTE_SYSTEM        ) != 0 );
			m_attr.compressed = ( ( attr bitand FILE_ATTRIBUTE_COMPRESSED    ) != 0 );
			m_attr.encrypted  = ( ( attr bitand FILE_ATTRIBUTE_ENCRYPTED     ) != 0 );
			m_attr.reparse    = ( ( attr bitand FILE_ATTRIBUTE_REPARSE_POINT ) != 0 );
			m_attr.invalid    = false;
			}

		//default constructor DOES NOT initialize jack shit.
		__forceinline CTreeListItem( ) { }
		virtual ~CTreeListItem( ) final = default;

		CTreeListItem( const std::uint64_t size, const FILETIME time, const DWORD attr, const bool done, _In_ CTreeListItem* const parent, _In_z_ _Readable_elements_( length ) PCWSTR const name, const std::uint16_t length ) : COwnerDrawnListItem( name, length ), m_child_info( ), m_parent(parent), m_rect{ 0, 0, 0, 0 }, m_size{ std::move(size) }, m_lastChange(time) {
			SetAttributes( attr );
			m_attr.m_done = done;
			}



		_Must_inspect_result_ _Success_( SUCCEEDED( return ) )
		virtual HRESULT Text_WriteToStackBuffer ( RANGE_ENUM_COL const column::ENUM_COL subitem, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, _On_failure_( _Post_valid_) rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const noexcept override final;

		_Pre_satisfies_( subitem == column::COL_PERCENTAGE ) _Success_( SUCCEEDED( return ) )
		  const HRESULT WriteToStackBuffer_COL_PERCENTAGE( RANGE_ENUM_COL const column::ENUM_COL subitem, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const noexcept;

		_Pre_satisfies_( subitem == column::COL_NTCOMPRESS ) _Success_( SUCCEEDED( return ) )
		  const HRESULT WriteToStackBuffer_COL_NTCOMPRESS( RANGE_ENUM_COL const column::ENUM_COL subitem, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const noexcept;

		_Pre_satisfies_( subitem == column::COL_SUBTREETOTAL ) _Success_( SUCCEEDED( return ) )
		 inline const HRESULT WriteToStackBuffer_COL_SUBTREETOTAL( RANGE_ENUM_COL const column::ENUM_COL subitem, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const noexcept;

		_Pre_satisfies_( ( subitem == column::COL_FILES ) || ( subitem == column::COL_ITEMS ) ) _Success_( SUCCEEDED( return ) )
		 inline const HRESULT WriteToStackBuffer_COL_FILES( RANGE_ENUM_COL const column::ENUM_COL subitem, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const noexcept;

		_Pre_satisfies_( subitem == column::COL_LASTCHANGE ) _Success_( SUCCEEDED( return ) )
		 inline const HRESULT WriteToStackBuffer_COL_LASTCHANGE( RANGE_ENUM_COL const column::ENUM_COL subitem, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, _Out_ _On_failure_( _Post_valid_ ) rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const noexcept;

		_Pre_satisfies_( subitem == column::COL_ATTRIBUTES ) _Success_( SUCCEEDED( return ) )
		 inline const HRESULT WriteToStackBuffer_COL_ATTRIBUTES( RANGE_ENUM_COL const column::ENUM_COL subitem, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const noexcept;

		FILETIME FILETIME_recurse( ) const noexcept;

		_Ret_range_( 0, UINT64_MAX )
		std::uint64_t size_recurse( ) const noexcept;

		//4,294,967,295  (4294967295 ) is the maximum number of files in an NTFS filesystem according to http://technet.microsoft.com/en-us/library/cc781134(v=ws.10).aspx
		_Ret_range_( 0, 4294967295 )
		std::uint32_t files_recurse( ) const noexcept;

		DOUBLE  GetFraction                   (                                                                   ) const noexcept;

		std::vector<const CTreeListItem*> size_sorted_vector_of_children( ) const;

		void    UpwardGetPathWithoutBackslash ( std::wstring& pathBuf ) const;

		void refresh_sizeCache( ) noexcept;

		_Pre_satisfies_( this->m_parent == NULL )
		void AddChildren( _In_ CTreeListControl* const tree_list_control );

		//http://msdn.microsoft.com/en-us/library/windows/desktop/aa365247(v=vs.85).aspx : Note  The maximum path of 32,767 characters is approximate, because the "\\?\" prefix may be expanded to a longer string by the system at run time, and this expansion applies to the total length.
		_Ret_range_( 0, 33000 ) DOUBLE  averageNameLength             (                                                                   ) const noexcept;

		std::wstring GetPath                       ( ) const;

		INT     GetSortAttributes             (                                                                   ) const noexcept;

		INT CompareSibling                           ( _In_ const CTreeListItem* const tlib, _In_ _In_range_( 0, INT32_MAX ) const column::ENUM_COL subitem ) const noexcept;

		_Pre_satisfies_( this->m_child_info.m_child_info_ptr == nullptr ) 
		void    stdRecurseCollectExtensionData_FILE( _Inout_    std::unordered_map<std::wstring, minimal_SExtensionRecord>* const extensionMap ) const;

		void    stdRecurseCollectExtensionData( _Inout_    std::unordered_map<std::wstring, minimal_SExtensionRecord>* const extensionMap ) const;

		_Success_( return != NULL ) _Must_inspect_result_ _Ret_maybenull_ 
		const CTreeListItem* GetSortedChild   ( _In_ const size_t i                             ) const;

		_Success_( return < child_count ) _Pre_satisfies_( child_count > 0 )
		size_t  FindSortedChild                 ( _In_ const CTreeListItem* const child, _In_ const size_t child_count ) const;

		_Pre_satisfies_( this->m_child_info.m_child_info_ptr == nullptr )
			PCWSTR const CStyle_GetExtensionStrPtr( ) const noexcept;


		_Pre_satisfies_( this->m_vi._Mypair._Myval2 != nullptr )
		std::int16_t  GetIndent( ) const noexcept {
			ASSERT( IsVisible( ) );
			return m_vi->indent;
			}

		_Pre_satisfies_( this->m_vi._Mypair._Myval2 != nullptr )
		void SetExpanded( _In_ const bool expanded = true ) noexcept {
			ASSERT( IsVisible( ) );
			m_vi->isExpanded = expanded;
			}

		_Pre_satisfies_( this->m_vi._Mypair._Myval2 != nullptr )
		void SetPlusMinusRect( _In_ const RECT& rc ) const noexcept {
			ASSERT( IsVisible( ) );
			m_vi->rcPlusMinus = SRECT( rc );
			}

		_Pre_satisfies_( this->m_vi._Mypair._Myval2 != nullptr )
		void SetTitleRect( _In_ const RECT& rc ) const noexcept {
			ASSERT( IsVisible( ) );
			m_vi->rcTitle = SRECT( rc );
			}

		void SetVisible ( _In_ const bool next_state_visible = true ) const;

		
		//TODO: This should be reversed (i.e. CTreeListControl::SortChildren( _Inout_ CTreeListItem* const item ) )
		_Pre_satisfies_( this->m_vi._Mypair._Myval2 != nullptr )
		void SortChildren                       ( _In_ const CTreeListControl* const ctrl );

		_Pre_satisfies_( this->m_parent != NULL )
		bool  HasSiblings                       (                                           ) const noexcept;
		
		void childNotNull( _In_ const CTreeListItem* const aTreeListChild, const size_t i );
		
		bool HasChildren ( ) const noexcept {
			return ( m_child_info.m_child_info_ptr != NULL );
			}
		
		_Pre_satisfies_( this->m_vi._Mypair._Myval2 != nullptr )
		bool IsExpanded( ) const noexcept {
			ASSERT( IsVisible( ) );
			return m_vi->isExpanded; 
			}
		
		bool IsVisible( ) const noexcept {
			return ( m_vi != nullptr );
			}
	
		_Pre_satisfies_( this->m_vi._Mypair._Myval2 != nullptr )
		RECT GetPlusMinusRect( ) const;

		_Pre_satisfies_( this->m_vi._Mypair._Myval2 != nullptr )
		RECT GetTitleRect( ) const;

		
		//The compiler will automatically inline if /Ob2 is on, so we'll ask anyways.
		RECT TmiGetRectangle( ) const {
			return BuildRECT( m_rect );
			}


		void TmiSetRectangle( _In_ const RECT& rc          ) const;

	public:
	//data members - DON'T FUCK WITH LAYOUT! It's tweaked for good memory layout!
		                               attribs                          m_attr;
									   //'CTreeListItem' : '7' bytes padding added after data member 'CTreeListItem::m_attr'
		                               child_info_block_manager         m_child_info;
		                         const CTreeListItem*                   m_parent;
		                       mutable std::unique_ptr<VISIBLEINFO>     m_vi = nullptr; // Data needed to display the item.
		                       mutable SRECT                            m_rect;         // Finally, this is our coordinates in the Treemap view. (For GraphView)
		

		//4,294,967,295 ( 4294967295 ) is the maximum number of files in an NTFS filesystem according to http://technet.microsoft.com/en-us/library/cc781134(v=ws.10).aspx
		//We can exploit this fact to use a 4-byte unsigned integer for the size of the array, which saves us 4 bytes on 64-bit architectures!
		//18446744073709551615 is the maximum theoretical size of an NTFS file according to http://blogs.msdn.com/b/oldnewthing/archive/2007/12/04/6648243.aspx
		_Field_range_( 0, 18446744073709551615 ) std::uint64_t          m_size;                // OwnSize
		                               FILETIME                         m_lastChange;          // Last modification time OF SUBTREE

	};



//
// CTreeListControl. A CListCtrl, which additionally behaves an looks like a tree control.
//
class CTreeListControl final : public COwnerDrawnListCtrl {
	DECLARE_DYNAMIC( CTreeListControl )

		void ExpandItemAndScroll( _In_ _In_range_( 0, INT_MAX ) const int i ) {
			ExpandItem( i, true );
			}

		void ExpandItemNoScroll( _In_ _In_range_( 0, INT_MAX ) const int i ) {
			ExpandItem( i, false );
			}

	public:
		CTreeListControl& operator=( const CTreeListControl& in ) = delete;
		CTreeListControl( const CTreeListControl& in ) = delete;

		_Pre_satisfies_( rowHeight % 2 == 0 )
		CTreeListControl( _In_range_( 0, NODE_HEIGHT ) UINT rowHeight, CDirstatDoc* docptr ) : COwnerDrawnListCtrl( global_strings::treelist_str, rowHeight ), m_pDocument( docptr ) {
			ASSERT( rowHeight <= NODE_HEIGHT );     // größer können wir nicht//"larger, we can not"?
			ASSERT( rowHeight % 2 == 0 );           // muss gerade sein//"must be straight"?
			}

		//The compiler will automatically inline if /Ob2 is on, so we'll ask anyways.
		void SysColorChanged( ) noexcept {
			InitializeColors( );
			InitializeNodeBitmaps( );
			}

		void DeleteItem( _In_ _In_range_( 0, INT_MAX ) const int i ) {
			
			/*
			From C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.20.27508\atlmfc\include\afxcmn.inl:
			_AFXCMN_INLINE int CListCtrl::GetItemCount() const
				{ ASSERT(::IsWindow(m_hWnd)); return (int) ::SendMessage(m_hWnd, LVM_GETITEMCOUNT, 0, 0L); }
			*/
			ASSERT( i < CListCtrl::GetItemCount( ) );
			auto const anItem = GetItem( i );
			if ( anItem != NULL ) {
				anItem->SetExpanded( false );
				anItem->SetVisible( false );
				anItem->m_vi.reset( );
				}
			if ( !CListCtrl::DeleteItem( i ) ) {
				std::terminate( );
				}
			}

		//calls CWnd::DestroyWindow( )
		virtual ~CTreeListControl( ) = default;

#pragma warning( suppress: 4263 )
		        BOOL CreateEx( _In_ const DWORD dwExStyle, _In_ DWORD dwStyle, _In_ const RECT& rect, _In_ CWnd* pParentWnd, _In_ const UINT nID ) noexcept;

		_Must_inspect_result_ _Success_( return != -1 ) _Ret_range_( 0, INT_MAX )
				INT  GetSelectedItem       ( ) const;
				bool SelectedItemCanToggle ( ) const;
				void InitializeNodeBitmaps ( ) const noexcept;
				void Sort                  ( );
				void ToggleSelectedItem    ( );

		_Must_inspect_result_ _Success_( return != NULL ) _Ret_maybenull_
				CTreeListItem* GetItem( _In_ _In_range_( 0, INT_MAX ) const int i ) const noexcept;


				int  EnumNode                                  ( _In_     const CTreeListItem* const item ) const;
				void SetRootItem                               ( _In_opt_ const CTreeListItem* const root ) noexcept;
		_Pre_satisfies_( !isDone )
				void OnChildAdded                              ( _In_opt_ const CTreeListItem* const parent,   _In_ CTreeListItem* const child, _In_ const bool isDone );
				INT  find_item_then_show_first_try_failed      ( _In_     const CTreeListItem* const path,     _In_ const int i );
				void find_item_then_show                       ( _In_     const CTreeListItem* const path,     _In_ const int i, int& parent, _In_ const bool showWholePath, _In_ const CTreeListItem* const target_in_path );
				void expand_item_then_scroll_to_it             ( _In_     const CTreeListItem* const pathZero,      _In_range_( 0, INT_MAX ) const int index, _In_ const bool showWholePath );
				void expand_item_no_scroll_then_doWhateverJDoes( _In_     const CTreeListItem* const pathZero,      _In_range_( 0, INT_MAX ) const int parent );
				void handle_VK_RIGHT                           ( _In_     const CTreeListItem* const item,     _In_ _In_range_( 0, INT_MAX ) const int i );
				void adjustColumnSize                          ( _In_     const CTreeListItem* const item_at_index );
				void SelectAndShowItem                         ( _In_     const CTreeListItem* const item,     _In_ const bool showWholePath                                                           );
				void SelectItem                                ( _In_     const CTreeListItem* const item );
				void EnsureItemVisible                         ( _In_     const CTreeListItem* const item                                                                                    );
				void handle_VK_LEFT                            ( _In_     const CTreeListItem* const item,     _In_ _In_range_( 0, INT32_MAX ) const int i );
				
		_Pre_satisfies_( item->m_vi._Mypair._Myval2 != nullptr ) _Success_( return )
				const bool DrawNodeNullWidth                   ( _In_     const CTreeListItem* const item, _In_ HDC hDC,  _In_ const RECT& rcRest, _In_    HDC hDCmem, _In_ const UINT ysrc ) const;
				RECT DrawNode_Indented                         ( _In_     const CTreeListItem* const item, _In_ HDC hDC, _Inout_    RECT& rc,     _Inout_ RECT& rcRest ) const noexcept;
				RECT DrawNode                                  ( _In_     const CTreeListItem* const item, _In_ HDC hDC, _Inout_    RECT& rc            ) const noexcept;

		_Pre_satisfies_( ( parent + 1 ) < index ) _Ret_range_( -1, INT_MAX ) 
				int  collapse_parent_plus_one_through_index    ( _In_     const CTreeListItem*       thisPath, const int index, _In_range_( 0, INT_MAX ) const int parent );
				
				void handle_VK_ESCAPE                          ( const UINT nChar, const UINT nRepCnt, const UINT nFlags );
				void handle_VK_TAB                             ( const UINT nChar, const UINT nRepCnt, const UINT nFlags );
				void handle_remaining_keys                     ( const UINT nChar, const UINT nRepCnt, const UINT nFlags );


	protected:
				void ExpandItemInsertChildren                  ( _In_     const CTreeListItem* const item, _In_ _In_range_( 0, INT32_MAX ) const INT_PTR i, _In_ const bool scroll  );
				void InsertItem                                ( _In_     const CTreeListItem* const item, _In_ _In_range_( 0, INT32_MAX ) const INT_PTR i );
				void insertItemsAdjustWidths                   ( _In_     const CTreeListItem* const item, _In_ _In_range_( 1, SIZE_T_MAX ) const size_t count, _Inout_ _Out_range_( 0, INT_MAX ) INT& maxwidth, _In_ const bool scroll, _In_ _In_range_( 0, INT_MAX ) const INT_PTR i );
				int  countItemsToDelete                        ( _In_     const CTreeListItem* const item, _Inout_ bool& selectNode, _In_ _In_range_( 0, INT_MAX ) const int& i );
				void PrepareDefaultMenu                        ( _In_     const CTreeListItem* const item, _Out_ CMenu* const menu ) const;

				void OnItemDoubleClick                         ( _In_ _In_range_( 0, INT_MAX ) const int i );
				void ExpandItem                                ( _In_ _In_range_( 0, INT_MAX ) const int i, _In_ const bool scroll );
				void ToggleExpansion                           ( _In_ _In_range_( 0, INT_MAX ) const INT i                           );
				void SelectItem                                ( _In_ _In_range_( 0, INT_MAX ) const INT i );

		_Success_( return == true )
				bool CollapseItem                              ( _In_ _In_range_( 0, INT_MAX ) const int i                           );

				void handle_OnContextMenu( CPoint pt ) const;

	   mutable CBitmap           m_bmNodes0;                   // The bitmaps needed to draw the treecontrol-like branches
	   mutable CBitmap           m_bmNodes1;                   // The same bitmaps with stripe-background color
			   INT               m_lButtonDownItem;            // Set in OnLButtonDown(). -1 if not item hit.
			   //C4820: 'CTreeListControl' : '3' bytes padding added after data member 'CTreeListControl::m_lButtonDownOnPlusMinusRect'
			   bool              m_lButtonDownOnPlusMinusRect; // Set in OnLButtonDown(). True, if plus-minus-rect hit.

	public:
			   CDirstatDoc*      m_pDocument;
	protected:
		DECLARE_MESSAGE_MAP()
		afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
		afx_msg void MeasureItem( _In_ PMEASUREITEMSTRUCT mis ) {
			mis->itemHeight = static_cast<UINT>( m_rowHeight );
			}
		afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
		afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
		afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
		afx_msg BOOL OnEraseBkgnd(CDC* pDC);
		afx_msg void OnSetFocus( _In_ CWnd* pOldWnd );
};

//See N4188
//If a class contains an unsized array, it cannot be used as the base class for another class. In addition, a class containing an unsized array cannot be used to declare any member except the last member of another class. A class containing an unsized array cannot have a direct or indirect virtual base class.
//The sizeof operator, when applied to a class containing an unsized array, returns the amount of storage required for all members except the unsized array. Implementors of classes that contain unsized arrays should provide alternate methods for obtaining the correct size of the class.
//You cannot declare arrays of objects that have unsized array components. Also, performing pointer arithmetic on pointers to such objects generates an error message.
//newer, more efficient allocation strategy will use a struct with the child count, child name heap manager, and an UNSIZED ARRAY as the children array.
//See also: "MEM54-CPP. Provide placement new with properly-aligned pointers to sufficient storage capacity"
//           https://www.securecoding.cert.org/confluence/display/cplusplus/MEM54-CPP.+Provide+placement+new+with+properly-aligned+pointers+to+sufficient+storage+capacity
//It'll have to use a struct that'll look something like this:
//struct children_heap_block_allocation final {
//	children_heap_block_allocation( ) : m_childCount { 0u } { }
//	children_heap_block_allocation( const children_heap_block_allocation& in ) = delete;
//	children_heap_block_allocation& operator=( const children_heap_block_allocation& in ) = delete;
//
//	~children_heap_block_allocation( ) {
//		for ( size_t i = 0u; i < m_childCount; ++i ) {
//			m_children[ i ].~CTreeListItem( );
//			}
//		}
//
//	_Field_range_( 0, 4294967295 )
//		std::uint32_t                m_childCount;
//		Children_String_Heap_Manager_Impl m_name_pool;
//		//maybe we could also store the NON-NTFS-compressed folder size here?
//
//#pragma warning( suppress: 4200 )//yes, this is Microsoft-specific
//		CTreeListItem                m_children[ ];//unsized array is a MICROSOFT-SPECIFIC extension to C++ that emulates C's Flexible Array Member.
//		//there are VERY active discussions in the C++ CWG (core working group) to develop some standardized version of array data members of runtime-bound.
//	};

//_At_( return, _Writable_bytes_( bytes_allocated ) )
//_Ret_notnull_ children_heap_block_allocation* allocate_enough_memory_for_children_block( _In_ const std::uint32_t number_of_children, _Out_ size_t& bytes_allocated );




#else

#endif
