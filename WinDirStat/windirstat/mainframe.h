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
	COptionsPropertySheet();
	void SetLanguageChanged   ( _In_ const bool changed );
	virtual BOOL OnInitDialog (                    );

	bool m_restartApplication;	// [out]

protected:
	virtual BOOL OnCommand    ( _In_ WPARAM wParam, _In_ LPARAM lParam );

	bool m_languageChanged;
	bool m_alreadyAsked;
	};


//
// CMySplitterWnd. A CSplitterWnd with 2 columns or rows, which
// knows about the current split ratio and retains it even when resized.
//
class CMySplitterWnd : public CSplitterWnd {
public:
	CMySplitterWnd( _In_z_ LPCTSTR name );
	virtual void StopTracking ( _In_       BOOL   bAccept     );
	DOUBLE GetSplitterPos     (                               ) const;
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
	afx_msg void OnDestroy();
	};

#ifdef DRAW_PACMAN
//
// CPacmanControl. Pacman on the status bar.
//
class CPacmanControl : public CStatic {
public:
	CPacmanControl();
	void Drive ( _In_ const LONGLONG readJobs );
	void Start ( _In_ const bool start        );

protected:
	CPacman m_pacman;

	DECLARE_MESSAGE_MAP()
	afx_msg void OnPaint();
	afx_msg INT OnCreate( const LPCREATESTRUCT lpCreateStruct );
	};
#endif

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
	CDeadFocusWnd();
	void Create( _In_ CWnd *parent );
	~CDeadFocusWnd();

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnKeyDown( const UINT nChar, const UINT nRepCnt, const UINT nFlags );
	};


//
// CMainFrame. The main application window.
//
class CMainFrame : public CFrameWnd {
protected:
	static CMainFrame *_theFrame;
	CMainFrame();		// Created by MFC only
	DECLARE_DYNCREATE(CMainFrame)

public:
	static CMainFrame *GetTheFrame();
	virtual ~CMainFrame    ( );
	

	//void AppendUserDefinedCleanups ( CMenu *menu            );
	void CopyToClipboard           ( _In_z_ const LPCTSTR psz      );
	//void DrivePacman               (                             );
	void FirstUpdateProgress       (                             );
	void HideProgress              (                             );
	void InitialShowWindow         (                             );
	void MinimizeGraphView         (                             );
	void MinimizeTypeView          (                             );
	void MoveFocus                 ( _In_ const LOGICAL_FOCUS lf );
	void RestoreGraphView          (                             );
	void RestoreTypeView           (                             );
	void SetLogicalFocus           ( _In_ const LOGICAL_FOCUS lf );
	void SetProgressPos            ( _In_ LONGLONG pos           );
	void SetProgressPos100         (                             );
	void SetSelectionMessageText   (                             );
	void ShowProgress              ( _In_ LONGLONG range         );
	void UpdateProgress            (                             );
	//void UpdateRB                  (                             );
	void WriteTimeToStatusBar      ( _In_ const DOUBLE drawTiming, _In_ const DOUBLE searchTiming, _In_ const DOUBLE fileNameLength );
	void WriteTimeToStatusBar      ( );
	
	_Must_inspect_result_ _Success_(return != NULL) CDirstatView *GetDirstatView   ( );
	_Must_inspect_result_ _Success_(return != NULL) CGraphView   *GetGraphView     ( );
	_Must_inspect_result_ _Success_(return != NULL) CTypeView    *GetTypeView      ( );

#ifdef SUSPEND_BUTTON
	bool IsProgressSuspended       ( );
#endif

	LOGICAL_FOCUS GetLogicalFocus  ( ) const;
	
protected:
	virtual BOOL OnCreateClient    (         LPCREATESTRUCT lpcs, CCreateContext* pContext );
	virtual BOOL PreCreateWindow   (         CREATESTRUCT&  cs                             );
	
	void CreateStatusProgress      (                                                       );
#ifdef DRAW_PACMAN
	void CreatePacmanProgress      (                                                       );
#endif

#ifdef SUSPEND_BUTTON
	void CreateSuspendButton       ( _Inout_ CRect&         rc                             );
#endif

	void DestroyProgress           (                                                       );
	void MakeSaneShowCmd           ( _Inout_ UINT&          u                              );
	size_t getExtDataSize( );
	//void MyQueryRecycleBin         ( CRecycleBinApi& rb, LONGLONG& items, LONGLONG& bytes );
	//void UpdateCleanupMenu         ( CMenu *menu                                          );

	bool            m_progressVisible;		// True while progress must be shown (either pacman or progressbar)
	LONGLONG        m_progressRange;	// Progress range. A range of 0 means that we have no range available. In this case we should display pacman.
	LONGLONG        m_progressPos;		// Progress position (<= progressRange, or an item count in case of m_progressRang == 0)

	//LONGLONG        m_rbLastKnownItems;
	//LONGLONG        m_rbLastKnownbytes;

	CMySplitterWnd  m_wndSubSplitter;	// Contains the two upper views
	CMySplitterWnd  m_wndSplitter;		// Contains (a) m_wndSubSplitter and (b) the graphview.

	CStatusBar		m_wndStatusBar;	// Status bar
	CToolBar		m_wndToolBar;	// Tool bar
	CProgressCtrl	m_progress;		// Progress control. Is Create()ed and Destroy()ed again every time.
#ifdef DRAW_PACMAN
	CPacmanControl	m_pacman;		// Static control for Pacman.
#endif

#ifdef SUSPEND_BUTTON
	CButton			m_suspendButton;// Progress-Suspend-Button
#endif

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
	//afx_msg void OnUpdateSendmailtoowner(CCmdUI *pCmdUI);
	//afx_msg void OnSendmailtoowner();
#ifdef SUSPEND_BUTTON
	afx_msg void OnBnClickedSuspend();
#endif
	afx_msg void OnTreemapHelpabouttreemaps();

public:
	CString m_drawTiming;
	#ifdef _DEBUG
		virtual void AssertValid() const;
		virtual void Dump(CDumpContext& dc) const;

	#endif
		afx_msg void OnSysColorChange();
	};




// $Log$
// Revision 1.5  2004/11/05 16:53:07  assarbad
// Added Date and History tag where appropriate.
//
