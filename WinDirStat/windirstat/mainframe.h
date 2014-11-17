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


//
// The "logical focus" can be 
// - on the Directory List
// - on the Extension List
// Although these windows can loose the real focus, for instance
// when a dialog box is opened, the logical focus will not be lost.
//
enum LOGICAL_FOCUS
{
	LF_NONE,
	LF_DIRECTORYLIST,
	LF_EXTENSIONLIST
};


//
// COptionsPropertySheet. The options dialog.
//
class COptionsPropertySheet : public CPropertySheet {
	DECLARE_DYNAMIC(COptionsPropertySheet)

public:
	COptionsPropertySheet( ) : CPropertySheet( IDS_WINDIRSTAT_SETTINGS ), m_alreadyAsked( false ) { }
	virtual BOOL OnInitDialog (                    ) override final;
protected:
	virtual BOOL OnCommand    ( _In_ WPARAM wParam, _In_ LPARAM lParam ) override final;
	bool m_alreadyAsked : 1;
	};


//
// CMySplitterWnd. A CSplitterWnd with 2 columns or rows, which
// knows about the current split ratio and retains it even when resized.
//
class CMySplitterWnd : public CSplitterWnd {
public:
	//CMySplitterWnd( _In_z_ PCWSTR name );
	CMySplitterWnd::CMySplitterWnd( _In_z_ PCWSTR name ) : m_persistenceName( name ), m_splitterPos( 0.5 ), m_wasTrackedByUser( false ), m_userSplitterPos( 0.5 ) {
		CPersistence::GetSplitterPos( m_persistenceName, m_wasTrackedByUser, m_userSplitterPos );
		}

	virtual void StopTracking ( _In_       BOOL   bAccept     ) override final;
	DOUBLE GetSplitterPos( ) const {
		return m_splitterPos;
		}
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
	afx_msg void OnDestroy( ) {
		CPersistence::SetSplitterPos( m_persistenceName, m_wasTrackedByUser, m_userSplitterPos );
		CSplitterWnd::OnDestroy( );
		}
	};

//
// CDeadFocusWnd. The focus in Windirstat can be on 
// - the directory list
// - the extension list,
// - or none of them. In this case the focus lies on
//   an invisible (zero-size) child of CMainFrame.
// On VK_TAB CDeadFocusWnd moves the focus to the
// directory list then.
//
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
class CMainFrame : public CFrameWnd {
protected:
	static CMainFrame* _theFrame;
	CMainFrame( ) : m_wndSplitter( _T( "main" ) ), m_wndSubSplitter( _T( "sub" ) ), m_lastSearchTime( -1 ) {// Created by MFC only
		_theFrame = this;
		m_logicalFocus = LF_NONE;
		}

	DECLARE_DYNCREATE(CMainFrame)

public:
	static CMainFrame* GetTheFrame( );
	virtual ~CMainFrame( ) {
		_theFrame = { NULL };
		}
	
	void CopyToClipboard           ( _In_z_ _In_reads_( strLen ) const PCWSTR psz, rsize_t strLen );

	//void FirstUpdateProgress( ) {
	//	GetDocument( )->SetTitlePrefix( _T( "Scanning " ) );//gets called far too often. TODO: 
	//	}

	void InitialShowWindow         (                             );
	
	void MinimizeGraphView( ) {
		m_wndSplitter.SetSplitterPos( 1.0 );
		}

	void MinimizeTypeView( ) {
		m_wndSubSplitter.SetSplitterPos( 1.0 );
		}
	
	void MoveFocus                 ( _In_ _Pre_satisfies_( ( lf == LF_NONE ) || ( lf == LF_DIRECTORYLIST ) || ( lf == LF_EXTENSIONLIST ) ) const LOGICAL_FOCUS lf );
	
	void RestoreGraphView          (                             );
	void RestoreTypeView           (                             );
	void SetLogicalFocus           ( _In_ const LOGICAL_FOCUS lf );
	void SetSelectionMessageText   (                             );
	//void UpdateProgress            (                             );
	void WriteTimeToStatusBar      ( _In_ const DOUBLE drawTiming, _In_ const DOUBLE searchTiming, _In_ const DOUBLE fileNameLength );
	void WriteTimeToStatusBar      ( );
	
	_Must_inspect_result_ _Success_(return != NULL) CDirstatView* GetDirstatView   ( );
	_Must_inspect_result_ _Success_(return != NULL) CGraphView*   GetGraphView     ( );
	_Must_inspect_result_ _Success_(return != NULL) CTypeView*    GetTypeView      ( );

	LOGICAL_FOCUS GetLogicalFocus( ) const {
		return m_logicalFocus;
		}
	
protected:
	virtual BOOL OnCreateClient    (         LPCREATESTRUCT lpcs, CCreateContext* pContext ) override final;
	virtual BOOL PreCreateWindow( CREATESTRUCT&  cs ) override final {
		return CFrameWnd::PreCreateWindow( cs );
		}

	void MakeSaneShowCmd( _Inout_ UINT& u_ShowCmd ) {
		if ( u_ShowCmd != SW_SHOWMAXIMIZED ) {
			u_ShowCmd = SW_SHOWNORMAL;
			}
		}

	size_t getExtDataSize( );

	CMySplitterWnd  m_wndSubSplitter;	// Contains the two upper views
	CMySplitterWnd  m_wndSplitter;		// Contains (a) m_wndSubSplitter and (b) the graphview.

	CStatusBar		m_wndStatusBar;	// Status bar
	CToolBar		m_wndToolBar;	// Tool bar

	LOGICAL_FOCUS	m_logicalFocus; // Which view has the logical focus
	CDeadFocusWnd	m_wndDeadFocus;	// Zero-size window which holds the focus if logical focus is "NONE"
	DOUBLE          m_lastSearchTime;
	
	DECLARE_MESSAGE_MAP()
	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg LRESULT OnEnterSizeMove( const WPARAM, const LPARAM );
	afx_msg LRESULT OnExitSizeMove( const WPARAM, const LPARAM );
	afx_msg void OnClose();
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
	afx_msg void OnUpdateMemoryUsage(CCmdUI *pCmdUI);
	afx_msg void OnSize(UINT nType, INT cx, INT cy);
	afx_msg void OnUpdateViewShowtreemap(CCmdUI *pCmdUI);
	afx_msg void OnViewShowtreemap();
	afx_msg void OnUpdateViewShowfiletypes(CCmdUI *pCmdUI);
	afx_msg void OnViewShowfiletypes();
	afx_msg void OnConfigure();
	afx_msg void OnDestroy();

public:
	CString m_drawTiming;
	#ifdef _DEBUG
		virtual void AssertValid( ) const {
			CFrameWnd::AssertValid( );
			}
		virtual void Dump( CDumpContext& dc ) const {
			CFrameWnd::Dump( dc );
			}

	#endif
		afx_msg void OnSysColorChange();
	};




// $Log$
// Revision 1.5  2004/11/05 16:53:07  assarbad
// Added Date and History tag where appropriate.
//
