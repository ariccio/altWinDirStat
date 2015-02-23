// layout.cpp	- Implementation of CLayout
//
// see `file_header_text.txt` for licensing & contact info.

#pragma once

#include "stdafx.h"

#ifndef WDS_LAYOUT_CPP
#define WDS_LAYOUT_CPP

#include "options.h"
#include "layout.h"

//void CLayout::OnInitDialog( _In_ const bool centerWindow ) {
//	//m_dialog->SetIcon( GetApp( )->LoadIcon( IDR_MAINFRAME ), false );
//
//	RECT rcDialog;
//	m_dialog->GetWindowRect( &rcDialog );
//	m_originalDialogSize = CRect( rcDialog ).Size( );
//
//	//Not vectorized: 1304, loop includes assignments of different sizes
//	for ( auto& aControl : m_control ) {
//		RECT rc;
//		
//		//If [GetWindowRect] succeeds, the return value is nonzero.
//		VERIFY( ::GetWindowRect( aControl.control, &rc ) );
//		//aControl.control->GetWindowRect( &rc );
//
//		/*
//_AFXWIN_INLINE void CWnd::GetWindowRect(LPRECT lpRect) const
//	{ ASSERT(::IsWindow(m_hWnd)); ::GetWindowRect(m_hWnd, lpRect); }
//		*/
//
//		m_dialog->ScreenToClient( &rc );
//		aControl.originalRectangle = rc;
//		}
//
//	RECT sg;
//	m_dialog->GetClientRect( &sg );
//	sg.left = sg.right  - WDS_CLAYOUT_CSIZEGRIPPER_WIDTH;
//	sg.top  = sg.bottom - WDS_CLAYOUT_CSIZEGRIPPER_WIDTH;
//	//m_dialog->m_hWnd
//	// _In_ const HWND parent_hWnd, rc, L"", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 0, IDC_SIZEGRIPPER, NULL
//	m_sizeGripper.Create( m_dialog->m_hWnd, sg, L"", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 0, IDC_SIZEGRIPPER, NULL );
//	//m_sizeGripper.Create( m_dialog, sg );
//
//	m_control.emplace_back( SControlInfo { m_sizeGripper.m_hWnd, 1, 1, 0, 0, sg } );
//
//	CPersistence::GetDialogRectangle( m_name, rcDialog );
//	
//	ASSERT( ::IsWindow( m_dialog->m_hWnd ) );
//	
//	//If [MoveWindow] succeeds, the return value is nonzero.
//	VERIFY( ::MoveWindow( m_dialog->m_hWnd, rcDialog.left, rcDialog.top, ( rcDialog.right - rcDialog.left ), ( rcDialog.bottom - rcDialog.top ), TRUE ) );
//	if ( centerWindow ) {
//		m_dialog->CenterWindow( );
//		}
//	}

//void CLayout::OnSize( ) {
//	RECT rc_outer_temp;
//	m_dialog->GetWindowRect( &rc_outer_temp );
//	const RECT rc_outer = rc_outer_temp;
//	const auto newDialogSize = CRect( rc_outer ).Size( );
//	const auto diff = newDialogSize - m_originalDialogSize;
//	
//	// The DeferWindowPos-stuff prevents the controls from overwriting each other.
//	auto hdwp = BeginDeferWindowPos( static_cast<int>( m_control.size( ) ) );
//
//	//Not vectorized: 1304, loop includes assignments of different sizes
//	for ( auto& aControl : m_control ) {
//		auto rc = CRect( aControl.originalRectangle );
//		const auto movex = static_cast<int>( aControl.movex );
//		const auto movey = static_cast<int>( aControl.movey );
//		WTL::CSize move( diff.cx * movex, diff.cy * movey );
//		CRect stretch( 0, 0, diff.cx * static_cast<int>( aControl.stretchx ), diff.cy * static_cast<int>( aControl.stretchy ) );
//		rc += move;
//		rc += stretch;
//		hdwp = DeferWindowPos( hdwp, ( aControl.control ), NULL, rc.left, rc.top, rc.Width( ), rc.Height( ), SWP_NOOWNERZORDER | SWP_NOZORDER );
//		}
//	
//	VERIFY( EndDeferWindowPos( hdwp ) );
//	}

/////////////////////////////////////////////////////////////////////////////

//const INT CLayout::CSizeGripper::_width = 14;

//LRESULT CLayout::CSizeGripper::OnPaint( UINT /*nMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/ ) {
//	//CPaintDC dc( this );
//	PAINTSTRUCT ps;
//	HDC hDC = BeginPaint( &ps );
//
//	RECT rc;
//
//	ASSERT( ::IsWindow( m_hWnd ) );
//	VERIFY( ::GetClientRect( m_hWnd, &rc ) );
//	//GetClientRect( &rc );
//
//	ASSERT( CRect( rc ).Width( ) == WDS_CLAYOUT_CSIZEGRIPPER_WIDTH );
//	ASSERT( CRect( rc ).Height( ) == WDS_CLAYOUT_CSIZEGRIPPER_WIDTH );
//
//	WTL::CPoint start;
//	WTL::CPoint end;
//
//	start.x = 1;
//	start.y = WDS_CLAYOUT_CSIZEGRIPPER_WIDTH;
//	end.x   = WDS_CLAYOUT_CSIZEGRIPPER_WIDTH;
//	end.y   = 1;
//
//	//DrawShadowLine( dc, start, end );
//	DrawShadowLine( hDC, start, end );
//
//	start.x += 4;
//	end.y   += 4;
//
//	//DrawShadowLine( dc, start, end );
//	DrawShadowLine( hDC, start, end );
//	
//	start.x += 4;
//	end.y   += 4;
//
//	//DrawShadowLine( dc, start, end );
//	DrawShadowLine( hDC, start, end );
//
//	// Do not call CWnd::OnPaint() for painting messages
//	return 0;
//	}

void CLayout::CSizeGripper::DrawShadowLine( _In_ HDC& hDC, _In_ WTL::CPoint start, _In_ WTL::CPoint end ) {
	//ASSERT_VALID( pdc );
	CPen lightPen( PS_SOLID, 1, GetSysColor( COLOR_3DHIGHLIGHT ) );
	HGDIOBJ old_obj_1 = SelectObject( hDC, lightPen );
	//CSelectObject sopen( pdc, lightPen );

	//pdc.MoveTo( start );
	/*
_AFXWIN_INLINE CPoint CDC::MoveTo(POINT point)
	{ ASSERT(m_hDC != NULL); return MoveTo(point.x, point.y); }
	
CPoint CDC::MoveTo(int x, int y)
{
	ASSERT(m_hDC != NULL);
	CPoint point;

	if (m_hDC != m_hAttribDC)
		VERIFY(::MoveToEx(m_hDC, x, y, &point));
	if (m_hAttribDC != NULL)
		VERIFY(::MoveToEx(m_hAttribDC, x, y, &point));
	return point;
}

	*/

	ASSERT( hDC != NULL );
	POINT point_1 = { 0, 0 };
	VERIFY( ::MoveToEx( hDC, start.x, start.y, &point_1 ) );

	//VERIFY( pdc.LineTo( end ) );
	/*
BOOL CDC::LineTo(int x, int y)
{
	ASSERT(m_hDC != NULL);
	if (m_hAttribDC != NULL && m_hDC != m_hAttribDC)
		::MoveToEx(m_hAttribDC, x, y, NULL);
	return ::LineTo(m_hDC, x, y);
}

	*/
	ASSERT( hDC != NULL );
	VERIFY( ::LineTo( hDC, end.x, end.y ) );

	start.x++;
	end.y++;

	CPen darkPen( PS_SOLID, 1, GetSysColor( COLOR_3DSHADOW ) );
	
	//unselect object
	const auto old_obj_1_unselect_res = SelectObject( hDC, old_obj_1 );
	ASSERT( old_obj_1_unselect_res != NULL );

	HGDIOBJ old_obj_2 = SelectObject( hDC, darkPen );
	//CSelectObject sopen2( pdc, darkPen );

	//pdc.MoveTo( start );
	ASSERT( hDC != NULL );
	POINT point_2 = { 0, 0 };
	VERIFY( ::MoveToEx( hDC, start.x, start.y, &point_2 ) );
	
	//VERIFY( pdc.LineTo( end ) );
	ASSERT( hDC != NULL );
	VERIFY( ::LineTo( hDC, end.x, end.y ) );

	start.x++;
	end.y++;

	//pdc.MoveTo( start );
	ASSERT( hDC != NULL );
	POINT point_3 = { 0, 0 };
	VERIFY( ::MoveToEx( hDC, start.x, start.y, &point_3 ) );

	//VERIFY( pdc.LineTo( end ) );
	ASSERT( hDC != NULL );
	VERIFY( ::LineTo( hDC, end.x, end.y ) );

	//unselect object
	const auto old_obj_2_unselect_res = SelectObject( hDC, old_obj_2 );
	ASSERT( old_obj_2_unselect_res != NULL );
	}

//LRESULT CLayout::CSizeGripper::OnNcHitTest( UINT /*nMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/ ) {
//	POINT point_temp;
//	point_temp.x = GET_X_LPARAM( lParam );
//	point_temp.y = GET_Y_LPARAM( lParam );
//	POINT point = point_temp;
//
//	//If [ScreenToClient] succeeds, the return value is nonzero.
//	VERIFY( ScreenToClient( &point ) );
//	if ( point.x + point.y >= WDS_CLAYOUT_CSIZEGRIPPER_WIDTH ) {
//		return HTBOTTOMRIGHT;
//		}
//	return 0;
//	}

// $Log$
// Revision 1.5  2004/11/05 16:53:07  assarbad
// Added Date and History tag where appropriate.
//
#else

#endif