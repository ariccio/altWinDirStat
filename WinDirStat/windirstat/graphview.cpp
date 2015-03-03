// graphview.cpp: Implementation of CGraphView
//
// see `file_header_text.txt` for licensing & contact info.

#pragma once

#include "stdafx.h"

#ifndef WDS_GRAPHVIEW_CPP
#define WDS_GRAPHVIEW_CPP

#include "graphview.h"

//#include "dirstatview.h"


//encourage inter-procedural optimization (and class-hierarchy analysis!)
//#include "ownerdrawnlistcontrol.h"
//#include "TreeListControl.h"
//#include "item.h"
//#include "typeview.h"


//#include "windirstat.h"
//#include "dirstatdoc.h"
//#include "options.h"
//#include "mainframe.h"

/*

#define IMPLEMENT_DYNCREATE(class_name, base_class_name) \
	CObject* PASCAL class_name::CreateObject() \
		{ return new class_name; } \
	IMPLEMENT_RUNTIMECLASS(class_name, base_class_name, 0xFFFF, \
		class_name::CreateObject, NULL)

#define IMPLEMENT_RUNTIMECLASS(class_name, base_class_name, wSchema, pfnNew, class_init) \
	AFX_COMDAT const CRuntimeClass class_name::class##class_name = { \
		#class_name, sizeof(class class_name), wSchema, pfnNew, \
			RUNTIME_CLASS(base_class_name), NULL, class_init }; \
	CRuntimeClass* class_name::GetRuntimeClass() const \
		{ return RUNTIME_CLASS(class_name); }

#define RUNTIME_CLASS(class_name) _RUNTIME_CLASS(class_name)
#define _RUNTIME_CLASS(class_name) ((CRuntimeClass*)(&class_name::class##class_name))
---------------------------------------------------------------------

	CObject* PASCAL CGraphView::CreateObject( ) {
		return new CGraphView;
		}

	AFX_COMDAT const CRuntimeClass CGraphView::classCGraphView = { "CGraphView", sizeof( class CGraphView ), 0xFFFF, CGraphView::CreateObject, ((CRuntimeClass*)(&CView::classCView)), NULL, NULL };
	
	CRuntimeClass* CGraphView::GetRuntimeClass( ) const {
		return ((CRuntimeClass*)(&CGraphView::classCGraphView));
		}

*/

//IMPLEMENT_DYNCREATE( CGraphView, CView )


	/*
#define BEGIN_MESSAGE_MAP(theClass, baseClass) \
	PTM_WARNING_DISABLE \
	const AFX_MSGMAP* theClass::GetMessageMap() const \
		{ return GetThisMessageMap(); } \
	const AFX_MSGMAP* PASCAL theClass::GetThisMessageMap() \
	{ \
		typedef theClass ThisClass;						   \
		typedef baseClass TheBaseClass;					   \
		static const AFX_MSGMAP_ENTRY _messageEntries[] =  \
		{


#define END_MESSAGE_MAP() \
		{0, 0, 0, 0, AfxSig_end, (AFX_PMSG)0 } \
	}; \
		static const AFX_MSGMAP messageMap = \
		{ &TheBaseClass::GetThisMessageMap, &_messageEntries[0] }; \
		return &messageMap; \
	}								  \
	PTM_WARNING_RESTORE

-------------------------------------------------------------
#define BEGIN_MESSAGE_MAP(CGraphView, CView ) \
	PTM_WARNING_DISABLE \
	const AFX_MSGMAP* CGraphView::GetMessageMap() const {
		return GetThisMessageMap( );
		}
	const AFX_MSGMAP* PASCAL CGraphView::GetThisMessageMap( ) {
		typedef CGraphView ThisClass;
		typedef CView TheBaseClass;
		static const AFX_MSGMAP_ENTRY _messageEntries[ ] = {


#define END_MESSAGE_MAP() \
		{0, 0, 0, 0, AfxSig_end, (AFX_PMSG)0 }
	};
		static const AFX_MSGMAP messageMap = { &TheBaseClass::GetThisMessageMap, &_messageEntries[0] };
		return &messageMap;
	}
	PTM_WARNING_RESTORE

	*/
	/*
BEGIN_MESSAGE_MAP( CGraphView, CView )
	ON_WM_SIZE( )
	ON_WM_LBUTTONDOWN( )
	ON_WM_SETFOCUS( )
	ON_WM_CONTEXTMENU( )
	ON_WM_MOUSEMOVE( )
	ON_WM_DESTROY( )
	ON_WM_TIMER( )
	//ON_COMMAND(ID_POPUP_CANCEL, OnPopupCancel)
END_MESSAGE_MAP( )
	*/

AFX_COMDAT const CRuntimeClass CGraphView::classCGraphView = { "CGraphView", sizeof( class CGraphView ), 0xFFFF, &( CGraphView::CreateObject ), const_cast<CRuntimeClass*>( &CView::classCView ), NULL, NULL };

const AFX_MSGMAP* PASCAL CGraphView::GetThisMessageMap( ) {
	typedef CGraphView ThisClass;
	typedef CView TheBaseClass;
	static const AFX_MSGMAP_ENTRY _messageEntries[ ] = {
		{ 
			WM_SIZE, 0, 0, 0,
			AfxSig_vwii,
			(AFX_PMSG)(AFX_PMSGW) (static_cast<void (AFX_MSG_CALL CWnd::*)(UINT, int, int)>( &ThisClass::OnSize))
		},
		{ 
			WM_LBUTTONDOWN, 0, 0, 0,
			AfxSig_vwp,
			(AFX_PMSG)(AFX_PMSGW) (static_cast<void (AFX_MSG_CALL CWnd::*)(UINT, CPoint)>( &ThisClass::OnLButtonDown))
		},
		{ 
			WM_SETFOCUS, 0, 0, 0,
			AfxSig_vW,
			(AFX_PMSG)(AFX_PMSGW)(static_cast<void (AFX_MSG_CALL CWnd::*)(CWnd*)>( &ThisClass::OnSetFocus))
		},
		{
			WM_CONTEXTMENU, 0, 0, 0,
			AfxSig_vWp,
			(AFX_PMSG)(AFX_PMSGW)(static_cast<void (AFX_MSG_CALL CWnd::*)(CWnd*, CPoint)>( &ThisClass::OnContextMenu))
		},
		{
			WM_MOUSEMOVE, 0, 0, 0,
			AfxSig_vwp,
			(AFX_PMSG)(AFX_PMSGW)(static_cast<void (AFX_MSG_CALL CWnd::*)(UINT, CPoint)>( &ThisClass::OnMouseMove))
		},
		{
			WM_DESTROY, 0, 0, 0,
			AfxSig_vv,
			(AFX_PMSG)(AFX_PMSGW)(static_cast<void (AFX_MSG_CALL CWnd::*)(void)>( &ThisClass::OnDestroy))
		},
		{
			WM_TIMER, 0, 0, 0,
			AfxSig_v_up_v,
			(AFX_PMSG)(AFX_PMSGW)(static_cast<void (AFX_MSG_CALL CWnd::*)(UINT_PTR)>( &ThisClass::OnTimer))
		},
		{
			0, 0, 0, 0, AfxSig_end, (AFX_PMSG)0
		}
	};
	static const AFX_MSGMAP messageMap = { &TheBaseClass::GetThisMessageMap, &_messageEntries[ 0 ] };
	return &messageMap;
}

//void CGraphView::OnMouseMove( UINT /*nFlags*/, CPoint point ) {
//	//auto Document = static_cast< CDirstatDoc* >( m_pDocument );
//	const auto Document = STATIC_DOWNCAST( CDirstatDoc, m_pDocument );
//	//Perhaps surprisingly, Document == NULL CAN be a valid condition. We don't have to set the message to anything if there's no document.
//	auto guard = WDS_SCOPEGUARD_INSTANCE( [&]{ reset_timer_if_zero( ); } );
//	if ( Document == NULL ) {
//		//return reset_timer_if_zero( );
//		return;
//		}
//	const auto root = Document->m_rootItem.get( );
//	if ( root == NULL ) {
//		TRACE( _T( "FindItemByPoint CANNOT find a point when given a NULL root! So let's not try.\r\n" ) );
//		//return reset_timer_if_zero( );
//		return;
//		}
//
//	ASSERT( root->m_attr.m_done );
//
//	if ( !( root->m_attr.m_done ) ) {
//		displayWindowsMsgBoxWithMessage( L"CGraphView::OnMouseMove: root item is NOT done! This should never happen!" );
//		std::terminate( );
//		}
//
//	if ( !IsDrawn( ) ) {
//		//return reset_timer_if_zero( );
//		return;
//		}
//	const auto item = static_cast< const CItemBranch* >( m_treemap.FindItemByPoint( root, point ) );
//	if ( item == NULL ) {
//		TRACE( _T( "There's nothing with a path, therefore nothing for which we can set the message text.\r\n" ) );
//		//return reset_timer_if_zero( );
//		return;
//		}
//	ASSERT( m_frameptr != NULL );
//	if ( m_frameptr == NULL ) {
//		//return reset_timer_if_zero( );
//		return;
//		}
//	TRACE( _T( "focused & Mouse on tree map!(x: %ld, y: %ld), %s\r\n" ), point.x, point.y, item->GetPath( ).c_str( ) );
//	m_frameptr->SetMessageText( item->GetPath( ).c_str( ) );
//
//	reset_timer_if_zero( );
//	guard.dismiss( );
//	}

#ifdef DEBUG
//this function exists for the singular purpose of tracing to console, as doing so from a .cpp is cleaner.
inline void trace_empty_view_graphview( ) {
	TRACE( _T( "Drawing Empty view...\r\n" ) );
	}

//this function exists for the singular purpose of tracing to console, as doing so from a .cpp is cleaner.
inline void trace_call_onidle( ) { 
	TRACE( _T( "\"[Causing] OnIdle() to be called once\"\r\n" ) );
	}

//this function exists for the singular purpose of tracing to console, as doing so from a .cpp is cleaner.
inline void trace_mouse_left( ) {
	TRACE( _T( "Mouse has left the tree map area?\r\n" ) );
	}

//this function exists for the singular purpose of tracing to console, as doing so from a .cpp is cleaner.
inline void trace_focused_mouspos( _In_ const LONG x, _In_ const LONG y, _In_z_ PCWSTR const path ) {
	TRACE( _T( "focused & Mouse on tree map!(x: %ld, y: %ld), %s\r\n" ), x, y, path );
	}
#endif

//TweakSizeOfRectangleForHightlight is called once, unconditionally.
inline void TweakSizeOfRectangleForHightlight( _Inout_ RECT& rc, _Inout_ RECT& rcClient, _In_ const bool grid ) {
	if ( grid ) {
		rc.right++;
		rc.bottom++;
		}
	if ( rcClient.left < rc.left ) {
		rc.left--;
		}
	if ( rcClient.top < rc.top ) {
		rc.top--;
		}
	if ( rc.right < rcClient.right ) {
		rc.right++;
		}
	if ( rc.bottom < rcClient.bottom ) {
		rc.bottom++;
		}
	}

#else

#endif