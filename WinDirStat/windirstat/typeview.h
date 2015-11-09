// typeview.h	- Declaration of CExtensionListControl and CTypeView
//
// see `file_header_text.txt` for licensing & contact info. If you can't find that file, then assume you're NOT allowed to do whatever you wanted to do.


#pragma once

#include "stdafx.h"

#ifndef WDS_TYPEVIEW_H
#define WDS_TYPEVIEW_H

WDS_FILE_INCLUDE_MESSAGE

#include "ChildrenHeapManager.h"
//#include "dirstatdoc.h"	// SExtensionRecord
#include "macros_that_scare_small_children.h"
#include "datastructures.h"
#include "ownerdrawnlistcontrol.h"


class CTypeView;
class CDirstatDoc;
class CExtensionListControl;
class CListItem;

void trace_on_erase_bkgnd_typeview( );

// CListItem. The items of the CExtensionListControl.
class CListItem final : public COwnerDrawnListItem {
	public:
		CListItem( ) : m_list( NULL ) { }
		CListItem ( _In_ CExtensionListControl* const list, _In_ std::uint32_t files_in, _In_ std::uint64_t bytes_in, _In_ COLORREF color_in, _In_z_ PCWSTR const name, const std::uint16_t length ) : m_list( list ), m_bytes( std::move( bytes_in ) ), m_files( std::move( files_in ) ), color( std::move( color_in ) ), COwnerDrawnListItem( name, length ) { }
			
		CListItem& operator=( const CListItem& in ) = delete;
		CListItem ( CListItem& in ) = delete;
		CListItem( CListItem&& in ) = delete;

		virtual ~CListItem( ) final = default;

		virtual COLORREF     ItemTextColor    ( ) const override final;
	private:
		//concrete_compare is called as a single line INSIDE a single line function. Let's ask for inlining.
		inline  INT          concrete_compare ( _In_ const CListItem* const other, RANGE_ENUM_COL const column::ENUM_COL subitem ) const;
		virtual INT          Compare          ( _In_ const COwnerDrawnListItem* const other, RANGE_ENUM_COL const column::ENUM_COL subitem                               ) const override final;
		virtual bool         DrawSubitem      ( RANGE_ENUM_COL const column::ENUM_COL subitem, _In_ HDC hDC, _In_ RECT rc, _In_ const UINT state, _Out_opt_ INT* const width, _Inout_ INT* const focusLeft, _In_ const COwnerDrawnListCtrl* const list ) const override final;
			
		_Must_inspect_result_ _Success_( SUCCEEDED( return ) )
		virtual HRESULT Text_WriteToStackBuffer( RANGE_ENUM_COL const column::ENUM_COL subitem, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, _On_failure_( _Post_valid_ ) rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const override final;

		_Pre_satisfies_( subitem == column::COL_BYTES ) _Success_( SUCCEEDED( return ) )
		 inline const HRESULT Text_WriteToStackBuffer_COL_BYTES( RANGE_ENUM_COL const column::ENUM_COL subitem, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const;

		_Pre_satisfies_( subitem == column::COL_FILES_TYPEVIEW ) _Success_( SUCCEEDED( return ) )
		 inline const HRESULT Text_WriteToStackBuffer_COL_FILES_TYPEVIEW( RANGE_ENUM_COL const column::ENUM_COL subitem, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const;

		_Pre_satisfies_( subitem == column::COL_DESCRIPTION ) _Success_( SUCCEEDED( return ) ) _Pre_satisfies_( strSize > 0 )
		 inline const HRESULT Text_WriteToStackBuffer_COL_DESCRIPTION( RANGE_ENUM_COL const column::ENUM_COL subitem, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const;

		_Pre_satisfies_( subitem == column::COL_BYTESPERCENT ) _Success_( SUCCEEDED( return ) )
		 inline const HRESULT Text_WriteToStackBuffer_COL_BYTESPERCENT( RANGE_ENUM_COL const column::ENUM_COL subitem, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const;

			    void         DrawColor        ( _In_ HDC hDC, _In_ RECT rc, _In_ const UINT state, _Out_opt_ INT* const width ) const;
			    DOUBLE       GetBytesFraction (                                                                                 ) const;
	private:
		const CExtensionListControl* m_list;
		_Field_range_( 0, 4294967295 )
		      std::uint32_t          m_files;//save 4 bytes :)
		_Field_range_( 0, 18446744073709551615 )
		      std::uint64_t          m_bytes;
		      COLORREF               color;

	};


// CExtensionListControl.
class CExtensionListControl final : public COwnerDrawnListCtrl {
public:
	CExtensionListControl( CTypeView* const typeView );
	
	CExtensionListControl& operator=( const CExtensionListControl& in ) = delete;
	CExtensionListControl( const CExtensionListControl& in ) = delete;

	virtual ~CExtensionListControl( ) final = default;

public:
	        _Ret_z_ PCWSTR const GetSelectedExtension        (                                                    ) const;

	        void               Initialize                  (                                                    );
	        void               SetExtensionData            ( _In_ const std::vector<SExtensionRecord>* extData  );
	        void               SelectExtension             ( _In_z_ PCWSTR const ext                        );
	
	void SysColorChanged( ) {
		COwnerDrawnListCtrl::InitializeColors( );
		}
	
	//http://msdn.microsoft.com/en-us/library/windows/desktop/aa365247(v=vs.85).aspx : Note  The maximum path of 32,767 characters is approximate, because the "\\?\" prefix may be expanded to a longer string by the system at run time, and this expansion applies to the total length.
	_Field_range_( 0, 33000                ) DOUBLE                                  m_averageExtensionNameLength;

	//18446744073709551615 is the maximum theoretical size of an NTFS file according to http://blogs.msdn.com/b/oldnewthing/archive/2007/12/04/6648243.aspx
	_Field_range_( 0, 18446744073709551615 ) std::uint64_t                           m_rootSize;
	                                         DOUBLE                                  m_adjustedTiming;
											 size_t                                  m_exts_count;
				_Field_size_( m_exts_count ) std::unique_ptr<CListItem[]>            m_exts;
	                                         CTypeView*                        const m_typeView;
											 std::unique_ptr<Children_String_Heap_Manager>            m_name_pool;

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
	void SysColorChanged( );

	CTypeView& operator=( const CTypeView& in ) = delete;
	CTypeView( const CTypeView& in ) = delete;

	_Must_inspect_result_ _Ret_maybenull_ CDirstatDoc* GetDocument           (                             ) const;
	                                      void         SetHighlightExtension ( _In_ const std::wstring ext );



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
		/*
		void CView::OnInitialUpdate()
		{
			OnUpdate(NULL, 0, NULL);        // initial update
		}
		*/
		//OnUpdate(NULL, 0, NULL) calls CTypeView::OnUpdate
		CView::OnInitialUpdate( );
		}
	virtual void OnUpdate        ( CView* pSender, LPARAM lHint, CObject* pHint ) override final;
	
	//Called by CView::OnPaint
	virtual void OnDraw( CDC* pDC ) override final {
		ASSERT_VALID( pDC );
		CView::OnDraw( pDC );
		}

	void SetSelection( );
	void OnUpdate0( );
	void OnUpdateHINT_LISTSTYLECHANGED( );
	void OnUpdateHINT_TREEMAPSTYLECHANGED( );


	DECLARE_MESSAGE_MAP()
	afx_msg INT OnCreate( LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, INT cx, INT cy);

	afx_msg BOOL OnEraseBkgnd( CDC* pDC ) {
		ASSERT_VALID( pDC );
		trace_on_erase_bkgnd_typeview( );
		return CView::OnEraseBkgnd( pDC );
		}
	afx_msg void OnSetFocus( CWnd* pOldWnd );
	};





#else

#endif
