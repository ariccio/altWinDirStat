// mainframe.h		- Declaration of CMySplitterWnd and CMainFrame
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

#ifndef MAINFRAME_H
#define MAINFRAME_H
#else
#error ass
#endif


#pragma once

#include "stdafx.h"

class CMySplitterWnd;
class CMainFrame;
class CDirstatView;
class CGraphView;
class CTypeView;





// COptionsPropertySheet. The options dialog.
class COptionsPropertySheet : public CPropertySheet {
	DECLARE_DYNAMIC(COptionsPropertySheet)

public:
	COptionsPropertySheet( ) : CPropertySheet( IDS_WINDIRSTAT_SETTINGS ), m_alreadyAsked( false ) { }
	virtual BOOL OnInitDialog (                    ) override final;
protected:
	virtual BOOL OnCommand    ( _In_ WPARAM wParam, _In_ LPARAM lParam ) override final;
	bool m_alreadyAsked : 1;
	};

// CMySplitterWnd. A CSplitterWnd with 2 columns or rows, which knows about the current split ratio and retains it even when resized.
class CMySplitterWnd : public CSplitterWnd {
public:
	CMySplitterWnd::CMySplitterWnd( _In_z_ PCWSTR name );
	
	DOUBLE GetSplitterPos( ) const {
		return m_splitterPos;
		}
	virtual void StopTracking ( _In_       BOOL   bAccept     ) override final;
	void SetSplitterPos       ( _In_ const DOUBLE pos         );
	void RestoreSplitterPos   ( _In_ const DOUBLE posIfVirgin );

protected:
	CString m_persistenceName;	// Name of object for CPersistence
	DOUBLE  m_splitterPos;		// Current split ratio
	bool    m_wasTrackedByUser;	// True as soon as user has modified the splitter position
	DOUBLE  m_userSplitterPos;	// Split ratio as set by the user

	DECLARE_MESSAGE_MAP()
	afx_msg void OnSize( const UINT nType, INT cx, INT cy );
public:
	afx_msg void OnDestroy( );

	};


// CDeadFocusWnd. The focus in Windirstat can be on 
// - the directory list
// - the extension list,
// - or none of them. In this case the focus lies on
//   an invisible (zero-size) child of CMainFrame.
class CDeadFocusWnd : public CWnd {
public:
	void Create( _In_ CWnd *parent );
	~CDeadFocusWnd( ) {
		DestroyWindow( );
		}
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnKeyDown( const UINT nChar, const UINT nRepCnt, const UINT nFlags );
	};


//
// CMainFrame. The main application window.
//
class CMainFrame : public WTL::CFrameWindowImpl<CMainFrame>, public WTL::CUpdateUI<CMainFrame>/*, public WTL::CMessageFilter, public WTL::CIdleHandler*/ {
protected:
	static CMainFrame* _theFrame;
public:
	CMainFrame( ) : m_wndSplitter( _T( "main" ) ), m_wndSubSplitter( _T( "sub" ) ), m_lastSearchTime( -1 ), m_logicalFocus( focus::LF_NONE ) {// Created by MFC only
		_theFrame = this;
		}

	//DECLARE_DYNCREATE(CMainFrame)
	DECLARE_FRAME_WND_CLASS( NULL, IDR_MAINFRAME )
public:
	static CMainFrame* GetTheFrame( );
	virtual ~CMainFrame( ) {
		_theFrame = { NULL };
		}
	
	
	_At_( lf, _Pre_satisfies_( ( lf == focus::LF_NONE ) || ( lf == focus::LF_DIRECTORYLIST ) || ( lf == focus::LF_EXTENSIONLIST ) ) )
	void   MoveFocus                 ( _In_ const focus::LOGICAL_FOCUS lf                                                             );
	_At_( lf, _Pre_satisfies_( ( lf == focus::LF_NONE ) || ( lf == focus::LF_DIRECTORYLIST ) || ( lf == focus::LF_EXTENSIONLIST ) ) )
	void   SetLogicalFocus           ( _In_ const focus::LOGICAL_FOCUS lf                                                             );
	void   InitialShowWindow         (                                                                                                );
	void   RestoreGraphView          (                                                                                                );
	void   RestoreTypeView           (                                                                                                );
	void   SetSelectionMessageText   (                                                                                                );
	void   WriteTimeToStatusBar      ( _In_ const DOUBLE drawTiming, _In_ const DOUBLE searchTiming, _In_ const DOUBLE fileNameLength );
	void   CopyToClipboard           ( _In_z_ _In_reads_( strLen ) const PCWSTR psz, rsize_t strLen                                   ) const;
	size_t getExtDataSize            (                                                                                                ) const;
	_Must_inspect_result_ _Success_(return != NULL) CDirstatView* GetDirstatView   ( );
	_Must_inspect_result_ _Success_(return != NULL) CGraphView*   GetGraphView     ( );
	_Must_inspect_result_ _Success_(return != NULL) CTypeView*    GetTypeView      ( );

public:
	virtual BOOL OnCreateClient    (         LPCREATESTRUCT  lpcs, CCreateContext* pContext ) override final;
	//virtual BOOL PreCreateWindow   (           CREATESTRUCT& cs ) override final {
	//	return CFrameWnd::PreCreateWindow( cs );
	//	}

public:	
	CMySplitterWnd       m_wndSubSplitter;	// Contains the two upper views
	CMySplitterWnd       m_wndSplitter;		// Contains (a) m_wndSubSplitter and (b) the graphview.
protected:
	CStatusBar           m_wndStatusBar;	// Status bar
	//CToolBar             m_wndToolBar;		// Tool bar
public:
	focus::LOGICAL_FOCUS m_logicalFocus;	// Which view has the logical focus
	CString              m_drawTiming;
	DOUBLE               m_lastSearchTime;
protected:	
	CDeadFocusWnd        m_wndDeadFocus;	// Zero-size window which holds the focus if logical focus is "NONE"

	//DECLARE_MESSAGE_MAP()
	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg LRESULT OnEnterSizeMove( const WPARAM, const LPARAM );
	afx_msg LRESULT OnExitSizeMove( const WPARAM, const LPARAM );
	afx_msg void OnClose();
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
	afx_msg void OnUpdateMemoryUsage(CCmdUI *pCmdUI);
	//afx_msg void OnSize(UINT nType, INT cx, INT cy);
	afx_msg void OnUpdateViewShowtreemap(CCmdUI *pCmdUI);
	afx_msg void OnViewShowtreemap();
	afx_msg void OnUpdateViewShowfiletypes(CCmdUI *pCmdUI);
	afx_msg void OnViewShowfiletypes();
	afx_msg void OnConfigure();
	afx_msg void OnDestroy();

//BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
//	ON_WM_CREATE()
//	ON_MESSAGE(WM_ENTERSIZEMOVE, OnEnterSizeMove)
//	ON_MESSAGE(WM_EXITSIZEMOVE, OnExitSizeMove)
//	ON_WM_CLOSE()
//	ON_WM_INITMENUPOPUP()
//	ON_UPDATE_COMMAND_UI(ID_INDICATOR_MEMORYUSAGE, OnUpdateMemoryUsage)
//	ON_WM_SIZE()
//	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOWTREEMAP, OnUpdateViewShowtreemap)
//	ON_COMMAND(ID_VIEW_SHOWTREEMAP, OnViewShowtreemap)
//	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOWFILETYPES, OnUpdateViewShowfiletypes)
//	ON_COMMAND(ID_VIEW_SHOWFILETYPES, OnViewShowfiletypes)
//	ON_COMMAND(ID_CONFIGURE, OnConfigure)
//	ON_WM_DESTROY()
//	//ON_COMMAND(ID_TREEMAP_HELPABOUTTREEMAPS, OnTreemapHelpabouttreemaps)
//	ON_WM_SYSCOLORCHANGE()
//END_MESSAGE_MAP()

	BEGIN_MSG_MAP( CMainFrame )
		MESSAGE_HANDLER( WM_CREATE, OnCreate )
		MESSAGE_HANDLER( WM_ENTERSIZEMOVE, OnEnterSizeMove )
		MESSAGE_HANDLER( WM_EXITSIZEMOVE, OnExitSizeMove )
		MESSAGE_HANDLER( WM_CLOSE, OnClose )
		MESSAGE_HANDLER( WM_INITMENUPOPUP, OnInitMenuPopup )
		ON_UPDATE_COMMAND_UI( ID_INDICATOR_MEMORYUSAGE, OnUpdateMemoryUsage )
		MESSAGE_HANDLER( WM_SIZE, OnSize )
		ON_UPDATE_COMMAND_UI( ID_VIEW_SHOWTREEMAP, OnUpdateViewShowtreemap )
		COMMAND_ID_HANDLER( ID_VIEW_SHOWTREEMAP, OnViewShowtreemap )
		ON_UPDATE_COMMAND_UI( ID_VIEW_SHOWFILETYPES, OnUpdateViewShowfiletypes )
		COMMAND_ID_HANDLER( ID_VIEW_SHOWFILETYPES, OnViewShowfiletypes )
		COMMAND_ID_HANDLER( ID_CONFIGURE, OnConfigure )
		MESSAGE_HANDLER( WM_DESTROY, OnDestroy )
		MESSAGE_HANDLER( WM_SYSCOLORCHANGE, OnSysColorChange )
	END_MSG_MAP()


public:
	
	#ifdef _DEBUG
		//virtual void AssertValid( ) const {
		//	CFrameWnd::AssertValid( );
		//	}
		//virtual void Dump( CDumpContext& dc ) const {
		//	CFrameWnd::Dump( dc );
		//	}

	#endif
		afx_msg void OnSysColorChange();
	};




// $Log$
// Revision 1.5  2004/11/05 16:53:07  assarbad
// Added Date and History tag where appropriate.
//
