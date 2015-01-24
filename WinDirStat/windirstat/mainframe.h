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
class COptionsPropertySheet final : public CPropertySheet {
	DECLARE_DYNAMIC(COptionsPropertySheet)
public:
	COptionsPropertySheet& operator=( const COptionsPropertySheet& in ) = delete;
	COptionsPropertySheet( const COptionsPropertySheet& in ) = delete;

	COptionsPropertySheet     (                                        ) : CPropertySheet( IDS_WINDIRSTAT_SETTINGS ) { }
	virtual BOOL OnInitDialog (                                        ) override final;
	virtual BOOL OnCommand    ( _In_ WPARAM wParam, _In_ LPARAM lParam ) override final;
	};

// CMySplitterWnd. A CSplitterWnd with 2 columns or rows, which knows about the current split ratio and retains it even when resized.
class CMySplitterWnd final : public CSplitterWnd {
public:
	CMySplitterWnd& operator=( const CMySplitterWnd& in ) = delete;
	CMySplitterWnd( const CMySplitterWnd& in ) = delete;

	CMySplitterWnd::CMySplitterWnd     ( _In_z_     PCWSTR name );
	virtual void    StopTracking       ( _In_       BOOL   bAccept     ) override final;
	void            SetSplitterPos     ( _In_ const DOUBLE pos         );
	void            RestoreSplitterPos ( _In_ const DOUBLE default_pos );

	const PCWSTR m_persistenceName;		// Name of object for CPersistence
	DOUBLE       m_splitterPos;			// Current split ratio
	DOUBLE       m_userSplitterPos;		// Split ratio as set by the user
	bool         m_wasTrackedByUser;	// True as soon as user has modified the splitter position
	

	DECLARE_MESSAGE_MAP()
	afx_msg void OnSize( const UINT nType, INT cx, INT cy );

	};


// CDeadFocusWnd. The focus in Windirstat can be on 
// - the directory list
// - the extension list,
// - or none of them. In this case the focus lies on
//   an invisible (zero-size) child of CMainFrame.
class CDeadFocusWnd final : public CWnd {
public:
	CDeadFocusWnd( ) { }
	CDeadFocusWnd& operator=( const CDeadFocusWnd& in ) = delete;
	CDeadFocusWnd( const CDeadFocusWnd& in ) = delete;

//#pragma warning( once : 4263 )

#pragma warning( suppress: 4263 )
	void Create( _In_ CWnd* parent );
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
class CMainFrame final : public CFrameWnd {
public:
	static CMainFrame* _theFrame;
	CMainFrame( ) : m_wndSplitter( global_strings::main_split ), m_wndSubSplitter( global_strings::sub_split ), m_lastSearchTime( -1 ), m_logicalFocus( LOGICAL_FOCUS::LF_NONE ) {// Created by MFC only
		_theFrame = this;
		}

	DECLARE_DYNCREATE(CMainFrame)

	CMainFrame& operator=( const CMainFrame& in ) =  delete;
	CMainFrame( const CMainFrame& in ) = delete;

	_Ret_maybenull_ static CMainFrame* GetTheFrame( );
	virtual ~CMainFrame( ) final {
		_theFrame = { NULL };
		}
	
	
	_At_( lf, _Pre_satisfies_( ( lf == LOGICAL_FOCUS::LF_NONE ) || ( lf == LOGICAL_FOCUS::LF_DIRECTORYLIST ) || ( lf == LOGICAL_FOCUS::LF_EXTENSIONLIST ) ) )
	void   MoveFocus                 ( _In_ const LOGICAL_FOCUS lf                                                             );
	
	_At_( lf, _Pre_satisfies_( ( lf == LOGICAL_FOCUS::LF_NONE ) || ( lf == LOGICAL_FOCUS::LF_DIRECTORYLIST ) || ( lf == LOGICAL_FOCUS::LF_EXTENSIONLIST ) ) )
	void   SetLogicalFocus           ( _In_ const LOGICAL_FOCUS lf                                                             );
	void   InitialShowWindow         (                                                                                                );
	void   RestoreGraphView          (                                                                                                );
	void   RestoreTypeView           (                                                                                                );
	void   SetSelectionMessageText   (                                                                                                );
	void   WriteTimeToStatusBar      ( _In_ const DOUBLE drawTiming, _In_ const DOUBLE searchTiming, _In_ const DOUBLE fileNameLength, _In_ const DOUBLE compressed_file_timing );
	void   CopyToClipboard           ( _In_ const std::wstring psz                                   ) const;
	size_t getExtDataSize            (                                                                                                ) const;
	_Must_inspect_result_ _Ret_maybenull_ CDirstatView* GetDirstatView   ( ) const;
	_Must_inspect_result_ _Ret_maybenull_ CGraphView*   GetGraphView     ( ) const;
	_Must_inspect_result_ _Ret_maybenull_ CTypeView*    GetTypeView      ( ) const;

	virtual BOOL OnCreateClient    (         LPCREATESTRUCT  lpcs, CCreateContext* pContext ) override final;
	virtual BOOL PreCreateWindow   (           CREATESTRUCT& cs                             ) override final {
		return CFrameWnd::PreCreateWindow( cs );
		}

public:	
	CMySplitterWnd       m_wndSubSplitter;	// Contains the two upper views
	CMySplitterWnd       m_wndSplitter;		// Contains (a) m_wndSubSplitter and (b) the graphview.
	CStatusBar           m_wndStatusBar;	// Status bar
	LOGICAL_FOCUS        m_logicalFocus;	// Which view has the logical focus
	std::wstring         m_drawTiming;
	DOUBLE               m_lastSearchTime;
	CDeadFocusWnd        m_wndDeadFocus;	// Zero-size window which holds the focus if logical focus is "NONE"

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
	afx_msg void OnSysColorChange();
	};




// $Log$
// Revision 1.5  2004/11/05 16:53:07  assarbad
// Added Date and History tag where appropriate.
//
