// dirstatview.h	- Declaration of CMyTreeListControl and CDirstatView
//
// see `file_header_text.txt` for licensing & contact info. If you can't find that file, then assume you're NOT allowed to do whatever you wanted to do.


#pragma once

#include "stdafx.h"

#ifndef WDS_DIRSTATVIEW_H
#define WDS_DIRSTATVIEW_H

WDS_FILE_INCLUDE_MESSAGE


#include "TreeListControl.h" //for CTreeListControl m_treeListControl, else we'd need to use PIMPL. I HATE PIMPL.
#include "macros_that_scare_small_children.h"

class CDirstatView;
class CDirstatDoc;

extern const UINT _WDS_nIdTreeListControl;
extern const UINT ITEM_ROW_HEIGHT;

inline void trace_OnEraseBkgnd( ) noexcept;
inline void trace_LoadingPerst( ) noexcept;
inline void trace_ListStyleCha( ) noexcept;
inline void trace_SelectionCha( ) noexcept;

// CDirstatView. The upper left view, which consists of the TreeList.
class CDirstatView final : public CView {
public:
	CTreeListControl m_treeListControl;	// The tree list

protected:
	CDirstatView( ) noexcept : m_treeListControl( ITEM_ROW_HEIGHT, GetDocument( ) ) {// Created by MFC only
		m_treeListControl.SetSorting( column::COL_SUBTREETOTAL, false );
		}

private:
	DISALLOW_COPY_AND_ASSIGN( CDirstatView );

protected:
	/*
	// not serializable, but dynamically constructable
	#define DECLARE_DYNCREATE(class_name) \
		DECLARE_DYNAMIC(class_name) \
		static CObject* PASCAL CreateObject();
	Sooo, DECLARE_DYNCREATE( CDirstatView )
	--becomes--
		DECLARE_DYNAMIC(CDirstatView) \
		static CObject* PASCAL CreateObject();
	--becomes--
public: 
	static const CRuntimeClass classCDirstatView; \
	virtual CRuntimeClass* GetRuntimeClass() const; \
		static CObject* PASCAL CreateObject();

	*/
	//DECLARE_DYNCREATE( CDirstatView )
public:
	static const CRuntimeClass classCDirstatView;
	virtual CRuntimeClass* GetRuntimeClass() const;
	static CObject* PASCAL CreateObject();

public:

	virtual ~CDirstatView( ) final = default;

	void SysColorChanged( ) noexcept {
		m_treeListControl.SysColorChanged( );
		}

protected:
	virtual BOOL PreCreateWindow( CREATESTRUCT& cs ) override final {
		
		/*
		From C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.20.27508\atlmfc\src\mfc\viewcore.cpp:67:
		BOOL CView::PreCreateWindow(CREATESTRUCT & cs)
		{
			ASSERT(cs.style & WS_CHILD);

			if (cs.lpszClass == NULL)
			{
				VERIFY(AfxDeferRegisterClass(AFX_WNDFRAMEORVIEW_REG));
				cs.lpszClass = _afxWndFrameOrView;  // COLOR_WINDOW background
			}

			if (cs.style & WS_BORDER)
			{
				cs.dwExStyle |= WS_EX_CLIENTEDGE;
				cs.style &= ~WS_BORDER;
			}

			return TRUE;
		}

		*/
		return CView::PreCreateWindow( cs );
		}

	virtual void OnInitialUpdate( ) override final {
		/*
		From C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.20.27508\atlmfc\src\mfc\viewcore.cpp:189:
		void CView::OnInitialUpdate()
		{
			OnUpdate(NULL, 0, NULL);        // initial update
		}

		void CView::OnUpdate(CView* pSender, LPARAM, CObject* )
		{
		ASSERT(pSender != this);
		UNUSED(pSender);     // unused in release builds

		// invalidate the entire pane, erase background too
		Invalidate(TRUE);
		}
		_AFXWIN_INLINE void CWnd::Invalidate(BOOL bErase)
			{ ASSERT(::IsWindow(m_hWnd)); ::InvalidateRect(m_hWnd, NULL, bErase); }
		*/

		//OnUpdate(NULL, 0, NULL) calls CGraphView::OnUpdate
		//also CDirstatView::OnUpdate?

		//CView::OnInitialUpdate( );
		//CView::OnUpdate(nullptr, 0, nullptr);
		//CWnd::Invalidate(TRUE);
		//ASSERT(::IsWindow(m_hWnd));
		hwnd::InvalidateErase(m_hWnd);
		m_treeListControl.GetHeaderCtrl()->RedrawWindow();
		}
	//Called by CView::OnPaint
	virtual void OnDraw( CDC* pDC ) override final {
		ASSERT_VALID( pDC );

		/*
		from C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.20.27508\atlmfc\src\mfc\viewcore.cpp:211:
		void CView::OnDraw(CDC*)
		{
		}
		*/
		//CView::OnDraw( pDC );
		m_treeListControl.GetHeaderCtrl()->RedrawWindow();
		}
	
	/*
	//return DYNAMIC_DOWNCAST( CDirstatDoc, m_pDocument );
	//--becomes--
	//(CDirstatDoc*)AfxDynamicDownCast(RUNTIME_CLASS(CDirstatDoc), m_pDocument)
	//--becomes--
	//(CDirstatDoc*)AfxDynamicDownCast(_RUNTIME_CLASS(CDirstatDoc), m_pDocument)
	//--becomes--
	//(CDirstatDoc*)AfxDynamicDownCast(((CRuntimeClass*)(&CDirstatDoc::classCDirstatDoc)), m_pDocument)
	
	*/

	//Keeping GetDocument in the implementation file means that we don't need to anything about CDirstatDoc in the header.
	_Must_inspect_result_ CDirstatDoc* GetDocument( ) noexcept;

	virtual void OnUpdate( CView* pSender, LPARAM lHint, CObject* pHint ) override final {
		switch ( lHint )
		{
			case UpdateAllViews_ENUM::HINT_NEWROOT:
				return OnUpdateHINT_NEWROOT( );

			case UpdateAllViews_ENUM::HINT_SELECTIONCHANGED:
				return OnUpdateHINT_SELECTIONCHANGED( );

			case UpdateAllViews_ENUM::HINT_SHOWNEWSELECTION:
				return OnUpdateHINT_SHOWNEWSELECTION( );

			case UpdateAllViews_ENUM::HINT_REDRAWWINDOW:
				VERIFY( m_treeListControl.RedrawWindow( ) );
				break;

			case UpdateAllViews_ENUM::HINT_LISTSTYLECHANGED:
				return OnUpdateHINT_LISTSTYLECHANGED( );
			case 0:
				return CView::OnUpdate( pSender, lHint, pHint );

			default:
				return;
			}
		}

	//Keeping OnUpdateHINT_NEWROOT in the implementation file means that we don't need to anything about CDirstatDoc in the header.
	void OnUpdateHINT_NEWROOT( ) noexcept;

	//Keeping OnUpdateHINT_SELECTIONCHANGED in the implementation file means that we don't need to anything about CDirstatDoc in the header.
	void OnUpdateHINT_SELECTIONCHANGED( );

	//Keeping OnUpdateHINT_SHOWNEWSELECTION in the implementation file means that we don't need to anything about CDirstatDoc in the header.
	void OnUpdateHINT_SHOWNEWSELECTION( );

	void OnUpdateHINT_LISTSTYLECHANGED( ) noexcept {
		
		trace_ListStyleCha( );
		const COptions* const Options = GetOptions( );
		m_treeListControl.ShowGrid( Options->m_listGrid );
		m_treeListControl.ShowStripes( Options->m_listStripes );
		m_treeListControl.ShowFullRowSelection( Options->m_listFullRowSelection );
		}

	void SetTreeListControlOptions( ) noexcept {
		const COptions* const Options = GetOptions( );
		m_treeListControl.ShowGrid            ( Options->m_listGrid             );
		m_treeListControl.ShowStripes         ( Options->m_listStripes          );
		m_treeListControl.ShowFullRowSelection( Options->m_listFullRowSelection );
		return;
		}

protected:

	/*
	#define DECLARE_MESSAGE_MAP() \
	protected: \
		static const AFX_MSGMAP* PASCAL GetThisMessageMap(); \
		virtual const AFX_MSGMAP* GetMessageMap() const; \

	*/
	//DECLARE_MESSAGE_MAP()
protected:
		static const AFX_MSGMAP* PASCAL GetThisMessageMap(); \
		virtual const AFX_MSGMAP* GetMessageMap() const; \

	afx_msg void OnSize( UINT nType, INT cx, INT cy ) {
		CWnd::OnSize( nType, cx, cy );
		//IsWindow function: https://msdn.microsoft.com/en-us/library/windows/desktop/ms633528.aspx
		//If the window handle identifies an existing window, the return value is nonzero.
		//If the window handle does not identify an existing window, the return value is zero.
		if ( ::IsWindow( m_treeListControl.m_hWnd ) ) {
			const RECT rc = { 0, 0, cx, cy };
			//MoveWindow function: https://msdn.microsoft.com/en-us/library/windows/desktop/ms633534.aspx
			//If [MoveWindow] succeeds, the return value is nonzero.
			//If the function fails, the return value is zero. To get extended error information, call GetLastError.
			VERIFY( ::MoveWindow( m_treeListControl.m_hWnd, rc.left, rc.top, ( rc.right - rc.left ), ( rc.bottom - rc.top ), TRUE ) );
			m_treeListControl.RedrawWindow( );
			}
		}
	afx_msg INT OnCreate( LPCREATESTRUCT lpCreateStruct ) {
		if ( CView::OnCreate( lpCreateStruct ) == -1 ){
			return -1;
			}
		const RECT rect = { 0, 0, 0, 0 };
		VERIFY( m_treeListControl.CreateEx( 0, WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SHOWSELALWAYS, rect, this, _WDS_nIdTreeListControl ) );
		m_treeListControl.AddExtendedStyle( LVS_EX_HEADERDRAGDROP );
		SetTreeListControlOptions( );
		m_treeListControl.InsertColumn( column::COL_NAME,         _T( "Name" ),                   LVCFMT_LEFT,  200, column::COL_NAME );
		m_treeListControl.InsertColumn( column::COL_PERCENTAGE,   _T( "Percentage" ),             LVCFMT_RIGHT,  55, column::COL_PERCENTAGE );
		m_treeListControl.InsertColumn( column::COL_SUBTREETOTAL, _T( "Size" ),                   LVCFMT_RIGHT,  90, column::COL_SUBTREETOTAL );
		m_treeListControl.InsertColumn( column::COL_ITEMS,        _T( "Items" ),                  LVCFMT_RIGHT,  55, column::COL_ITEMS );
		m_treeListControl.InsertColumn( column::COL_NTCOMPRESS,   _T( "NTFS compression ratio" ), LVCFMT_RIGHT, 100, column::COL_NTCOMPRESS );
		m_treeListControl.InsertColumn( column::COL_LASTCHANGE,   _T( "Last Change" ),            LVCFMT_LEFT,  120, column::COL_LASTCHANGE );
		m_treeListControl.InsertColumn( column::COL_ATTRIBUTES,   _T( "Attributes" ),             LVCFMT_LEFT,   50, column::COL_ATTRIBUTES );

		
		trace_LoadingPerst( );
		m_treeListControl.OnColumnsInserted( );
		hwnd::InvalidateErase(m_hWnd);
		return 0;
		}
	afx_msg BOOL OnEraseBkgnd( CDC* pDC ) {
		trace_OnEraseBkgnd( );
		//UNREFERENCED_PARAMETER( pDC );
		//return TRUE;
		return CWnd::OnEraseBkgnd( pDC );
		}
	afx_msg void OnDestroy( ) {
		CView::OnDestroy( );
		}
	afx_msg void OnSetFocus( CWnd* pOldWnd ) {
		UNREFERENCED_PARAMETER( pOldWnd );
		m_treeListControl.SetFocus( );
		}

	//Keeping OnLvnItemchanged in the implementation file means that we don't need to anything about CDirstatDoc in the header.
	afx_msg void OnLvnItemchanged( NMHDR* pNMHDR, LRESULT* pResult );

	afx_msg void OnUpdatePopupToggle( _In_ CCmdUI* pCmdUI ) {
		pCmdUI->Enable( m_treeListControl.SelectedItemCanToggle( ) );
		}
	afx_msg void OnPopupToggle( ) {
		m_treeListControl.ToggleSelectedItem( );
		}

public:
	#ifdef _DEBUG
	virtual void AssertValid( ) const {
		CView::AssertValid( );
		}
	virtual void Dump( CDumpContext& dc ) const {
		TRACE( _T( "CDirstatView::Dump\r\n" ) );
		AfxCheckMemory( );
		CView::Dump( dc );
		}
	#endif
	};

#else


#endif
