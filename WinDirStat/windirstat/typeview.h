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


// CExtensionListControl.
class CExtensionListControl : public COwnerDrawnListControl {
protected:
	// Columns
	enum {
		COL_EXTENSION,
		COL_COLOR,
		COL_DESCRIPTION,
		COL_BYTES,
		COL_BYTESPERCENT,
		COL_FILES
		};

	// CListItem. The items of the CExtensionListControl.
	class CListItem : public COwnerDrawnListItem {
		public:

			CListItem                ( CExtensionListControl* list, _In_z_ PCWSTR extension, SExtensionRecord r ) : m_list( list ), m_extension( extension ), m_record( r ), m_image( -1 ) { }
			
			bool DrawSubitem         ( _In_ _In_range_( 0, INT32_MAX ) const INT subitem, _In_ CDC* pdc, _In_ CRect rc, _In_ const UINT state, _Inout_opt_ INT* width, _Inout_ INT* focusLeft  ) const;
			virtual CString GetText  ( _In_ _In_range_( 0, INT32_MAX ) const INT subitem                                                                    ) const override;
			INT Compare              ( _In_ const CSortingListItem* other, _In_ const INT subitem                           ) const override;
			//CString GetExtension     (                                                                                      ) const { return m_extension; }
#ifdef DRAW_ICONS
			INT GetImage             (                                                                                      ) const;
#endif

			const std::wstring       m_extension;

		private:
			void DrawColor          ( _In_ CDC *pdc, _In_ CRect rc, _In_ const UINT state, _Inout_opt_ INT *width ) const;

#ifdef DRAW_ICONS
			CString GetDescription  (                                                  ) const;
#endif

			CString GetBytesPercent (                                                  ) const;
			DOUBLE  GetBytesFraction (                                                  ) const;

			CExtensionListControl* m_list;
			mutable CString        m_description;
			SExtensionRecord       m_record;
			mutable INT            m_image;
		};

public:
	CExtensionListControl            ( CTypeView* typeView              ) : COwnerDrawnListControl( _T( "types" ), 19 ), m_typeView( typeView ), m_rootSize ( 0 ), adjustedTiming( 0 ), averageExtensionNameLength( ) { }

	virtual bool GetAscendingDefault ( _In_ const INT column            ) const override;
	void Initialize                  (                                  );
	void SetExtensionData            ( _In_ const std::vector<SExtensionRecord>* extData  );
	
	void SelectExtension             ( _In_z_ const PCTSTR ext         );
	CString GetSelectedExtension     (                                  ) const;
	
	
	void SetRootSize                 ( _In_ const LONGLONG totalBytes   ) { m_rootSize = totalBytes; }
	
	std::vector<CListItem> extensionItems;
	DOUBLE adjustedTiming;
	DOUBLE averageExtensionNameLength;

protected:
	CListItem* GetListItem(_In_  const INT i ) const;

	//18446744073709551615 is the maximum theoretical size of an NTFS file according to http://blogs.msdn.com/b/oldnewthing/archive/2007/12/04/6648243.aspx
	_Field_range_( 0, 18446744073709551615 ) LONGLONG   m_rootSize;
	CTypeView* m_typeView;

	DECLARE_MESSAGE_MAP()
	afx_msg void OnDestroy();
	afx_msg void OnLvnDeleteitem(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void MeasureItem(LPMEASUREITEMSTRUCT mis);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnLvnItemchanged(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	};


//
// CTypeView. The upper right view, which shows the extensions and their
// cushion colors.
//
class CTypeView : public CView {
protected:
	CTypeView( ) : m_extensionListControl( this ), m_showTypes( true ) { }
	
	DECLARE_DYNCREATE(CTypeView)

public:
	virtual ~CTypeView( ) { }
	_Must_inspect_result_ CDirstatDoc* GetDocument     (                   ) const;
	void         SysColorChanged (                   );

	virtual BOOL PreCreateWindow ( CREATESTRUCT& cs  ) override;

	void ShowTypes               ( _In_ const bool show   );

	void SetHighlightExtension   ( _In_z_ const LPCTSTR ext );
	_Success_( return > 0 ) DOUBLE getPopulateTiming( )      const { return m_extensionListControl.adjustedTiming; }
	_Success_( return > 0 ) DOUBLE getExtensionNameLength( ) const { return m_extensionListControl.averageExtensionNameLength; }
	
	bool                  m_showTypes;             // Whether this view shall be shown (F8 option)

protected:
	virtual void OnInitialUpdate (                                                    ) override;
	virtual void OnUpdate        ( CView* pSender, LPARAM lHint, CObject* pHint ) override;
	virtual void OnDraw          ( CDC* pDC                                           ) override;
	void SetSelection            (                                                    );

	void OnUpdate0( );
	void OnUpdateHINT_LISTSTYLECHANGED( );
	void OnUpdateHINT_TREEMAPSTYLECHANGED( );
	//void OnSetRedraw( HWND hwnd, BOOL fRedraw );

	
	CExtensionListControl m_extensionListControl;  // The list control

	BOOL g_fRedrawEnabled;

	DECLARE_MESSAGE_MAP()
	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, INT cx, INT cy);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
public:
	#ifdef _DEBUG
		virtual void AssertValid() const;
		virtual void Dump(CDumpContext& dc) const;
	#endif
	};

#ifndef _DEBUG  // Debugversion in typeview.cpp
_Must_inspect_result_ inline CDirstatDoc* CTypeView::GetDocument() const {
	return static_cast< CDirstatDoc* >( m_pDocument );
	}
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
// Revision 1.6  2004/11/05 16:53:08  assarbad
// Added Date and History tag where appropriate.
//
