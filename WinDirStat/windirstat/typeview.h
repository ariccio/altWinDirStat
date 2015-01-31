// typeview.h	- Declaration of CExtensionListControl and CTypeView
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


#ifndef TYPEVIEW_H
#define TYPEVIEW_H
#else
#error ass
#endif

#pragma once

#include "stdafx.h"

//#include "dirstatdoc.h"	// SExtensionRecord

class CTypeView;
class CDirstatDoc;
class CExtensionListControl;
class CListItem;

// CListItem. The items of the CExtensionListControl.
class CListItem final : public COwnerDrawnListItem {
	public:
		CListItem( ) : m_list( NULL ) { }
		CListItem ( _In_ CExtensionListControl* const list, _In_ SExtensionRecord r, _In_z_ PCWSTR name, const std::uint16_t length ) : m_list( list ), m_record( std::move( r ) ), COwnerDrawnListItem( name, length ) { }
			
		CListItem& operator=( const CListItem& in ) = delete;
		CListItem ( CListItem& in ) = delete;

		virtual ~CListItem( ) final = default;

		CListItem ( CListItem&& in );
		
		virtual COLORREF     ItemTextColor    ( ) const override final;
	private:
		virtual INT          Compare          ( _In_ const COwnerDrawnListItem* const other, RANGE_ENUM_COL const column::ENUM_COL subitem                               ) const override final;
		virtual bool         DrawSubitem      ( RANGE_ENUM_COL const column::ENUM_COL subitem, _In_ CDC& pdc, _In_ RECT rc, _In_ const UINT state, _Out_opt_ INT* const width, _Inout_ INT* const focusLeft, _In_ const COwnerDrawnListCtrl* const list ) const override final;
			
		_Must_inspect_result_ _Success_( SUCCEEDED( return ) )
		virtual HRESULT Text_WriteToStackBuffer( RANGE_ENUM_COL const column::ENUM_COL subitem, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, _Out_ _On_failure_( _Post_valid_ ) rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const override final;

		//_Pre_satisfies_( subitem == column::COL_EXTENSION ) _Success_( SUCCEEDED( return ) )
		//	    HRESULT Text_WriteToStackBuffer_COL_EXTENSION( RANGE_ENUM_COL const column::ENUM_COL subitem, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, _Out_ _On_failure_( _Post_valid_ ) rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const;

		_Pre_satisfies_( subitem == column::COL_COLOR ) _Success_( SUCCEEDED( return ) )
			    HRESULT Text_WriteToStackBuffer_COL_COLOR( RANGE_ENUM_COL const column::ENUM_COL subitem, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const;

		_Pre_satisfies_( subitem == column::COL_BYTES ) _Success_( SUCCEEDED( return ) )
		 inline HRESULT Text_WriteToStackBuffer_COL_BYTES( RANGE_ENUM_COL const column::ENUM_COL subitem, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const;

		_Pre_satisfies_( subitem == column::COL_FILES_TYPEVIEW ) _Success_( SUCCEEDED( return ) )
			    HRESULT Text_WriteToStackBuffer_COL_FILES_TYPEVIEW( RANGE_ENUM_COL const column::ENUM_COL subitem, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const;

		_Pre_satisfies_( subitem == column::COL_DESCRIPTION ) _Success_( SUCCEEDED( return ) )
		 inline HRESULT Text_WriteToStackBuffer_COL_DESCRIPTION( RANGE_ENUM_COL const column::ENUM_COL subitem, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const;

		_Pre_satisfies_( subitem == column::COL_BYTESPERCENT ) _Success_( SUCCEEDED( return ) )
			    HRESULT Text_WriteToStackBuffer_COL_BYTESPERCENT( RANGE_ENUM_COL const column::ENUM_COL subitem, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const;

																_Success_( SUCCEEDED( return ) )
			    HRESULT WriteToStackBuffer_default( WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const;

		
		//virtual std::wstring Text             ( _In_ _In_range_( 0, INT32_MAX ) const column::ENUM_COL subitem                                                                    ) const override final;
			    void         DrawColor        ( _In_ CDC& pdc, _In_ RECT rc, _In_ const UINT state, _Out_opt_ INT* const width ) const;
			  //std::wstring GetBytesPercent  (                                                                                 ) const;
			    DOUBLE       GetBytesFraction (                                                                                 ) const;

	private:
		CExtensionListControl* m_list;
		SExtensionRecord       m_record;
	};


// CExtensionListControl.
class CExtensionListControl final : public COwnerDrawnListCtrl {
public:
	CExtensionListControl( CTypeView* const typeView );
	
	CExtensionListControl& operator=( const CExtensionListControl& in ) = delete;
	CExtensionListControl( const CExtensionListControl& in ) = delete;

	virtual ~CExtensionListControl( ) final = default;

private:
	virtual bool               GetAscendingDefault         ( _In_ const column::ENUM_COL column                 ) const override final;
public:
	        const std::wstring GetSelectedExtension        (                                                    ) const;
	        void               Initialize                  (                                                    );
	        void               SetExtensionData            ( _In_ const std::vector<SExtensionRecord>* extData  );
	        void               SelectExtension             ( _In_ const std::wstring ext                        );
	
	void SysColorChanged( ) {
		InitializeColors( );
		}
	
	//http://msdn.microsoft.com/en-us/library/windows/desktop/aa365247(v=vs.85).aspx : Note  The maximum path of 32,767 characters is approximate, because the "\\?\" prefix may be expanded to a longer string by the system at run time, and this expansion applies to the total length.
	_Field_range_( 0, 33000                ) DOUBLE                                  m_averageExtensionNameLength;

	//18446744073709551615 is the maximum theoretical size of an NTFS file according to http://blogs.msdn.com/b/oldnewthing/archive/2007/12/04/6648243.aspx
	_Field_range_( 0, 18446744073709551615 ) std::uint64_t                           m_rootSize;
	                                         DOUBLE                                  m_adjustedTiming;
	                                       //std::vector<CListItem*>                 m_extensionItems;
											 size_t                                  m_exts_count;
				_Field_size_( m_exts_count ) std::unique_ptr<CListItem[]>            m_exts;
	                                         CTypeView*                              m_typeView;

	_Ret_notnull_ CListItem* GetListItem( _In_ const INT i ) const;

	DECLARE_MESSAGE_MAP()
	afx_msg void OnDestroy();
	afx_msg void OnLvnDeleteitem(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void MeasureItem( PMEASUREITEMSTRUCT mis );
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnLvnItemchanged(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	};


//
// CTypeView. The upper right view, which shows the extensions and their
// cushion colors.
//
class CTypeView final : public CView {
protected:
	CTypeView( );
	
	DECLARE_DYNCREATE(CTypeView)

public:
	virtual ~CTypeView( ) final = default;

	CTypeView& operator=( const CTypeView& in ) = delete;
	CTypeView( const CTypeView& in ) = delete;

	_Must_inspect_result_ _Ret_maybenull_ CDirstatDoc* GetDocument           (                             ) const;
	                                      void         SetHighlightExtension ( _In_ const std::wstring ext );

										  void SysColorChanged( );

	virtual BOOL PreCreateWindow( CREATESTRUCT& cs ) override final {
		return CView::PreCreateWindow( cs );
		}

	void ShowTypes( _In_ const bool show ) {
		m_showTypes = show;
		OnUpdate( NULL, 0, NULL );
		}
	//C4820: 'CTypeView' : '7' bytes padding added after data member 'CTypeView::m_showTypes'
	bool                  m_showTypes;             // Whether this view shall be shown (F8 option)
	CExtensionListControl m_extensionListControl;  // The list control

protected:
	//C4820: 'CTypeView' : '4' bytes padding added after data member 'CTypeView::g_fRedrawEnabled'
	BOOL                  g_fRedrawEnabled;

	virtual void OnInitialUpdate( ) override final {
		CView::OnInitialUpdate( );
		}
	virtual void OnUpdate        ( CView* pSender, LPARAM lHint, CObject* pHint ) override final;
	
	virtual void OnDraw( CDC* pDC ) override final {
		ASSERT_VALID( pDC );
		CView::OnDraw( pDC );
		}

	void SetSelection            (                                                    );

	void OnUpdate0( );
	void OnUpdateHINT_LISTSTYLECHANGED( );
	void OnUpdateHINT_TREEMAPSTYLECHANGED( );
	//void OnSetRedraw( HWND hwnd, BOOL fRedraw );

	

	DECLARE_MESSAGE_MAP()
	afx_msg INT OnCreate( LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, INT cx, INT cy);

	afx_msg BOOL OnEraseBkgnd( CDC* pDC ) {
		ASSERT_VALID( pDC );
		TRACE( _T( "CTypeView::OnEraseBkgnd!\r\n" ) );
		return CView::OnEraseBkgnd( pDC );
		}
	afx_msg void OnSetFocus( CWnd* pOldWnd );
public:
	#ifdef _DEBUG
		virtual void AssertValid() const;
		virtual void Dump(CDumpContext& dc) const;
	#endif
	};

//#ifndef _DEBUG  // Debugversion in typeview.cpp
//_Must_inspect_result_ _Ret_maybenull_ inline CDirstatDoc* CTypeView::GetDocument() const {
//	return static_cast< CDirstatDoc* >( m_pDocument );
//	}
//#endif


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
// Revision 1.6  2004/11/05 16:53:08  assarbad
// Added Date and History tag where appropriate.
//
