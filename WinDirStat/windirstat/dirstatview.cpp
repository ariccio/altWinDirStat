// dirstatview.cpp : Implementation of CDirstatView
//
// see `file_header_text.txt` for licensing & contact info.

#pragma once

#include "stdafx.h"

#ifndef WDS_DIRSTATVIEW_CPP
#define WDS_DIRSTATVIEW_CPP

//
#include "dirstatview.h"

//this function exists for the singular purpose of tracing to console, as doing so from a .cpp is cleaner.
inline void trace_OnEraseBkgnd( ) {
	TRACE( _T( "CDirstatView::OnEraseBkgnd!\r\n" ) );
	}

//this function exists for the singular purpose of tracing to console, as doing so from a .cpp is cleaner.
inline void trace_LoadingPerst( ) {
	TRACE( _T( "Loading persistent attributes....\r\n" ) );
	}

//this function exists for the singular purpose of tracing to console, as doing so from a .cpp is cleaner.
inline void trace_ListStyleCha( ) {
	TRACE( _T( "List style has changed, redrawing!\r\n" ) );
	}

//this function exists for the singular purpose of tracing to console, as doing so from a .cpp is cleaner.
inline void trace_SelectionCha( ) {
	TRACE( _T( "CDirstatView::OnUpdateHINT_SELECTIONCHANGED\r\n" ) );
	}

IMPLEMENT_DYNCREATE( CDirstatView, CView )

BEGIN_MESSAGE_MAP(CDirstatView, CView)
	ON_WM_SIZE()
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_DESTROY()
	ON_WM_SETFOCUS()
	ON_NOTIFY(LVN_ITEMCHANGED, _nIdTreeListControl, &( CDirstatView::OnLvnItemchanged ) )
	ON_UPDATE_COMMAND_UI(ID_POPUP_TOGGLE, &( CDirstatView::OnUpdatePopupToggle ) )
	ON_COMMAND(ID_POPUP_TOGGLE, &( CDirstatView::OnPopupToggle ) )
END_MESSAGE_MAP()

#else

#endif