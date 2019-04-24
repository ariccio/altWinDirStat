// mainframe.h		- Declaration of WDSSplitterWnd and CMainFrame
//
// see `file_header_text.txt` for licensing & contact info. If you can't find that file, then assume you're NOT allowed to do whatever you wanted to do.
#pragma once

#include "stdafx.h"


#ifndef WDS_MAINFRAME_H
#define WDS_MAINFRAME_H


WDS_FILE_INCLUDE_MESSAGE


//#include "windirstat.h"
//#include "globalhelpers.h"
#include "LOGICAL_FOCUS_enum.h"
#include "macros_that_scare_small_children.h"

struct WDSSplitterWnd;
class CMainFrame;
class CDirstatView;
class CGraphView;
class CTypeView;
class CDirstatApp;

// WDSOptionsPropertySheet. The options dialog.
struct WDSOptionsPropertySheet final : public CPropertySheet {
	/*
#define DECLARE_DYNAMIC(class_name) \
public: \
	static const CRuntimeClass class##class_name; \
	virtual CRuntimeClass* GetRuntimeClass() const; \
	*/
	//DECLARE_DYNAMIC(WDSOptionsPropertySheet)

	static const CRuntimeClass classWDSOptionsPropertySheet;
	virtual CRuntimeClass* GetRuntimeClass() const;

	DISALLOW_COPY_AND_ASSIGN( WDSOptionsPropertySheet );

	WDSOptionsPropertySheet     (                                        ) : CPropertySheet( IDS_WINDIRSTAT_SETTINGS ) { }
	virtual BOOL OnInitDialog (                                        ) override final;
	virtual BOOL OnCommand    ( _In_ WPARAM wParam, _In_ LPARAM lParam ) override final;
	};

// WDSSplitterWnd. A CSplitterWnd with 2 columns or rows, which knows about the current split ratio and retains it even when resized.
struct WDSSplitterWnd final : public CSplitterWnd {
	DISALLOW_COPY_AND_ASSIGN( WDSSplitterWnd );

	WDSSplitterWnd( _In_z_ PCWSTR const name );

	void RestoreSplitterPos( _In_ const DOUBLE default_pos ) noexcept {
		SetSplitterPos( ( m_wasTrackedByUser ) ? m_userSplitterPos : default_pos );
		}



	virtual void    StopTracking       ( _In_       BOOL   bAccept     ) override final;
	void            SetSplitterPos     ( _In_ const DOUBLE pos         ) noexcept;

	PCWSTR const m_persistenceName;		// Name of object for CPersistence
	DOUBLE       m_splitterPos = 0.5;			// Current split ratio
	DOUBLE       m_userSplitterPos = 0.5;		// Split ratio as set by the user
	//C4820: 'WDSSplitterWnd' : '7' bytes padding added after data member 'WDSSplitterWnd::m_wasTrackedByUser' (dirstatdoc.cpp)
	bool         m_wasTrackedByUser = false;	// True as soon as user has modified the splitter position
	

	DECLARE_MESSAGE_MAP()
	afx_msg void OnSize( const UINT nType, INT cx, INT cy );

	};

namespace dead_focus_wnd {
	enum {
		IDC_DEADFOCUS		// ID of dead-focus window
		};
	}

// CDeadFocusWnd. The focus in Windirstat can be on 
// - the directory list
// - the extension list,
// - or none of them. In this case the focus lies on
//   an invisible (zero-size) child of CMainFrame.
//TODO: convert to ATL
class CDeadFocusWnd final : public ATL::CWindowImpl<CDeadFocusWnd> {
public:

	CDeadFocusWnd( CMainFrame* ptr ) : m_frameptr( ptr ) { }
	DISALLOW_COPY_AND_ASSIGN( CDeadFocusWnd );

	CMainFrame* m_frameptr;

	~CDeadFocusWnd( ) = default;

	LRESULT OnKeyDown( UINT /*nMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled );

	BEGIN_MSG_MAP( CDeadFocusWnd )
		MESSAGE_HANDLER( WM_KEYDOWN, CDeadFocusWnd::OnKeyDown )
#pragma warning( suppress : 4365 )//C4365: 'argument' : conversion from 'unsigned int' to 'int', signed/unsigned mismatch
	END_MSG_MAP()
	};


//
// CMainFrame. The main application window.
//
class CMainFrame final : public CFrameWnd {
	DISALLOW_COPY_AND_ASSIGN(CMainFrame);
public:
	static CMainFrame* _theFrame;

	//Keeping CMainFrame's constructor in the implementation file means that we don't need to anything about global_strings, in the header.
	CMainFrame( );

	/*
#define DECLARE_DYNAMIC(class_name) \
public: \
	static const CRuntimeClass class##class_name; \
	virtual CRuntimeClass* GetRuntimeClass() const; \

#define DECLARE_DYNCREATE(class_name) \
	DECLARE_DYNAMIC(class_name) \
	static CObject* PASCAL CreateObject();
	*/
	//DECLARE_DYNCREATE(CMainFrame)
	static const CRuntimeClass classCMainFrame;
	virtual CRuntimeClass* GetRuntimeClass() const;
	static CObject* PASCAL CreateObject() noexcept;


	_Ret_maybenull_ static CMainFrame* GetTheFrame( );

	virtual ~CMainFrame( ) final {
		_theFrame = { nullptr };
		}
	
	
	_At_( lf, _Pre_satisfies_( ( lf == LOGICAL_FOCUS::LF_NONE ) || ( lf == LOGICAL_FOCUS::LF_DIRECTORYLIST ) || ( lf == LOGICAL_FOCUS::LF_EXTENSIONLIST ) ) )
	void   MoveFocus                 ( _In_ const LOGICAL_FOCUS lf                                                             ) noexcept;
	
	_At_( lf, _Pre_satisfies_( ( lf == LOGICAL_FOCUS::LF_NONE ) || ( lf == LOGICAL_FOCUS::LF_DIRECTORYLIST ) || ( lf == LOGICAL_FOCUS::LF_EXTENSIONLIST ) ) )
	void   SetLogicalFocus           ( _In_ const LOGICAL_FOCUS lf                                                             ) noexcept;
	void   InitialShowWindow         (                                                                                                ) noexcept;
	void   RestoreGraphView          (                                                                                                ) noexcept;
	void   RestoreTypeView           (                                                                                                ) noexcept;
	void   SetSelectionMessageText   (                                                                                                ) noexcept;
	
	void   valid_timing_to_write( _In_ const double populate_timing, _In_ const double draw_timing, _In_ const double average_extension_length, _In_ const double enum_timing, _In_ const double compressed_file_timing, _In_ const double total_time, _In_ const rsize_t ext_data_size, _In_ const double file_name_length, _Out_ _Post_z_ _Pre_writable_size_( buffer_size_init ) PWSTR buffer_ptr, const rsize_t buffer_size_init ) noexcept;

	void   invalid_timing_to_write( _In_ const double average_extension_length, _In_ const rsize_t ext_data_size, _Out_ _Post_z_ _Pre_writable_size_( buffer_size_init ) PWSTR buffer_ptr, const rsize_t buffer_size_init ) noexcept;

	_Pre_satisfies_( searchTiming >= compressed_file_timing )
	void   WriteTimeToStatusBar      ( _In_ const DOUBLE drawTiming, _In_ const DOUBLE searchTiming, _In_ const DOUBLE fileNameLength, _In_ const DOUBLE compressed_file_timing );
	size_t getExtDataSize            (                                                                                                ) const noexcept;

	_Must_inspect_result_ _Ret_maybenull_ CDirstatView* GetDirstatView   ( ) const noexcept;
	private:
	_Must_inspect_result_ _Ret_maybenull_ CGraphView*   GetGraphView     ( ) const noexcept;
	public:
	_Must_inspect_result_ _Ret_maybenull_ CTypeView*    GetTypeView      ( ) const noexcept;

	public:
	virtual BOOL OnCreateClient    (         LPCREATESTRUCT  lpcs, CCreateContext* pContext ) override final;
	virtual BOOL PreCreateWindow   (           CREATESTRUCT& cs                             ) override final {
		return CFrameWnd::PreCreateWindow( cs );
		}

public:	
	WDSSplitterWnd       m_wndSubSplitter;	// Contains the two upper views (dirstatview & typeview?)
	WDSSplitterWnd       m_wndSplitter;		// Contains (a) m_wndSubSplitter and (b) the graphview.
	CStatusBar           m_wndStatusBar;	// Status bar
	//C4820: 'CMainFrame' : '4' bytes padding added after data member 'CMainFrame::m_logicalFocus'
	LOGICAL_FOCUS        m_logicalFocus;	// Which view has the logical focus
	std::wstring         m_drawTiming;
	DOUBLE               m_lastSearchTime;
	CDeadFocusWnd        m_wndDeadFocus;	// Zero-size window which holds the focus if logical focus is "NONE"
	CDirstatApp*         m_appptr;

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
	afx_msg void OnUpdateViewShowGrid(CCmdUI *pCmdUI);//TODO:
	afx_msg void OnViewShowfiletypes();
	afx_msg void OnViewShowGrid();//TODO:

	afx_msg void OnConfigure();
	afx_msg void OnDestroy();

public:
	afx_msg void OnSysColorChange();
	};




// $Log$
// Revision 1.5  2004/11/05 16:53:07  assarbad
// Added Date and History tag where appropriate.
//
#else

#endif