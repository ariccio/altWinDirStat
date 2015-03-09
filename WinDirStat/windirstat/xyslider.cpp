// XySlider.cpp	- Implementation of CXySlider
//
// see `file_header_text.txt` for licensing & contact info.

#pragma once

#include "stdafx.h"

#ifndef WDS_XYSLIDER_CPP
#define WDS_XYSLIDER_CPP

#pragma message( "Including `" __FILE__ "`..." )


#include "datastructures.h"
#include "xyslider.h"
#include "globalhelpers.h"


namespace {
	const int GRIPPER_RADIUS = 8;

	void move_to_coord( _In_ const CDC& pdc, _In_ const int rc_x, _In_ const int rc_y ) {
		//pdc.MoveTo( rc.left,  m_zero.y ); <---Not handling the return value means that WE DO NOT care about the previous "current position", thus the fourth parameter to MoveToEx should be NULL.
		ASSERT( pdc.m_hDC != NULL );
		if ( pdc.m_hDC != pdc.m_hAttribDC ) {
			//If [MoveToEx] succeeds, the return value is nonzero. If [MoveToEx] fails, the return value is zero.
			VERIFY( ::MoveToEx( pdc.m_hDC, rc_x, rc_y, NULL ) );
			}
		if ( pdc.m_hAttribDC != NULL ) {
			//If [MoveToEx] succeeds, the return value is nonzero. If [MoveToEx] fails, the return value is zero.
			VERIFY( ::MoveToEx( pdc.m_hAttribDC, rc_x, rc_y, NULL ) );
			}
		}
	
	void line_to_coord( _In_ const CDC& pdc, _In_ const int rc_x, _In_ const int rc_y ) {
		ASSERT( pdc.m_hDC != NULL );
		if ( ( pdc.m_hAttribDC != NULL ) && ( pdc.m_hDC != pdc.m_hAttribDC ) ) {
			//If [MoveToEx] succeeds, the return value is nonzero. If [MoveToEx] fails, the return value is zero.
			VERIFY( ::MoveToEx( pdc.m_hAttribDC, rc_x, rc_y, NULL ) );
			}

		//If [LineTo] succeeds, the return value is nonzero. If [LineTo] fails, the return value is zero.
		VERIFY( ::LineTo( pdc.m_hDC, rc_x, rc_y ) );
		}


	void fill_solid_rectangle( _In_ const HDC m_hDC, _In_ const RECT rc, _In_ const COLORREF clr ) {
		ASSERT( m_hDC != NULL );

		//If [SetBkColor] fails, the return value is CLR_INVALID.
		const COLORREF bk_color_res_1 = ::SetBkColor( m_hDC, clr );
		ASSERT( bk_color_res_1 != CLR_INVALID );

		if ( bk_color_res_1 == CLR_INVALID ) {
			TRACE( _T( "::SetBkColor( pdc.m_hDC, color ) failed!!\r\n" ) );
			}

		//If the string is drawn, the return value [of ExtTextOutW] is nonzero. However, if the ANSI version of ExtTextOut is called with ETO_GLYPH_INDEX, the function returns TRUE even though the function does nothing.
		VERIFY( ::ExtTextOutW( m_hDC, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL ) );
		}
	}


IMPLEMENT_DYNAMIC(CXySlider, CStatic)

void AFXAPI DDX_XySlider( CDataExchange* pDX, INT nIDC, POINT& value ) {
	pDX->PrepareCtrl(nIDC);
	HWND hWndCtrl;
	pDX->m_pDlgWnd->GetDlgItem( nIDC, &hWndCtrl );
	if ( pDX->m_bSaveAndValidate ) {
		::SendMessageW( hWndCtrl, XY_GETPOS, 0, ( LPARAM ) &value );
		}
	else {
		::SendMessageW( hWndCtrl, XY_SETPOS, 0, ( LPARAM ) &value );
		}
	}

void CXySlider::Initialize( ) {
	if ( ( !m_inited ) && ( ::IsWindow( m_hWnd ) ) ) {
		// Make size odd, so that zero lines are central
		RECT rc = { 0, 0, 0, 0 };

		//"Return value: If [GetWindowRect] succeeds, the return value is nonzero. If [GetWindowRect] fails, the return value is zero. To get extended error information, call GetLastError."
		VERIFY( ::GetWindowRect( m_hWnd, &rc ) );


		CWnd::GetParent( )->ScreenToClient( &rc );
		if ( ( rc.right - rc.left ) % 2 == 0 ) {
			rc.right--;
			}
		if ( ( rc.bottom - rc.top ) % 2 == 0 ) {
			rc.bottom--;
			}
	
		ASSERT( ::IsWindow( m_hWnd ) );

		//If [MoveWindow] succeeds, the return value is nonzero.
		VERIFY( ::MoveWindow( m_hWnd, rc.left, rc.top, ( rc.right - rc.left ), ( rc.bottom - rc.top ), TRUE ) );

		CalcSizes( );

		m_inited = true;
		}
	}

void CXySlider::CalcSizes( ) {
	ASSERT( ::IsWindow( m_hWnd ) );
	//"Return value: If the function succeeds, the return value is nonzero. If the function fails, the return value is zero. To get extended error information, call GetLastError."
	VERIFY( ::GetClientRect( m_hWnd, &m_rcAll ) );

	ASSERT( m_rcAll.left == 0 );
	ASSERT( m_rcAll.top  == 0 );
	ASSERT( ( m_rcAll.right - m_rcAll.left ) % 2 == 1 );
	ASSERT( ( m_rcAll.bottom - m_rcAll.top ) % 2 == 1 );
	ASSERT( ( m_rcAll.right - m_rcAll.left ) >= GRIPPER_RADIUS * 2 );	// Control must be large enough
	ASSERT( ( m_rcAll.bottom - m_rcAll.top ) >= GRIPPER_RADIUS * 2 );

	m_zero.x = ( m_rcAll.right - m_rcAll.left ) / 2;
	m_zero.y = ( m_rcAll.bottom - m_rcAll.top ) / 2;

	m_radius.cx = ( m_rcAll.right - m_rcAll.left ) / 2 - 1;
	m_radius.cy = ( m_rcAll.bottom - m_rcAll.top ) / 2 - 1;

	m_rcInner = m_rcAll;

	//"Return value: If [InflateRect] succeeds, the return value is nonzero. If [InflateRect] fails, the return value is zero."
	VERIFY( ::InflateRect( &m_rcInner, -( GRIPPER_RADIUS - 3 ), -( GRIPPER_RADIUS - 3 ) ) );
	//m_rcInner.DeflateRect( GRIPPER_RADIUS - 3, GRIPPER_RADIUS - 3 );


	m_gripperRadius.cx = GRIPPER_RADIUS;
	m_gripperRadius.cy = GRIPPER_RADIUS;

	m_range.cx = m_radius.cx - m_gripperRadius.cx;
	m_range.cy = m_radius.cy - m_gripperRadius.cy;

	//m_range = m_radius - m_gripperRadius;
	}

void CXySlider::NotifyParent( ) const {
	NMHDR hdr;
	hdr.hwndFrom = m_hWnd;
	hdr.idFrom   = static_cast<UINT_PTR>( GetDlgCtrlID( ) );
	hdr.code     = XYSLIDER_CHANGED;
	TRACE( _T( "NotifyParent called! Sending WM_NOTIFY!\r\n" ) );
	CWnd::GetParent( )->SendMessageW( WM_NOTIFY, static_cast<WPARAM>( GetDlgCtrlID( ) ), ( LPARAM ) &hdr );
	}

void CXySlider::PaintBackground( _In_ CDC& pdc ) {
	ASSERT_VALID( &pdc );
	ASSERT( pdc.m_hDC != NULL );

	//If [SetBkColor] fails, the return value is CLR_INVALID.
	const COLORREF color_res_1 = ::SetBkColor( pdc.m_hDC, GetSysColor( COLOR_BTNFACE ) );
	
	//TODO: check this!
	ASSERT( color_res_1 != CLR_INVALID );
	if ( color_res_1 == CLR_INVALID ) {
		TRACE( _T( "SetBkColor( pdc.m_hDC, GetSysColor( COLOR_BTNFACE ) ) failed!!\r\n" ) );
		}

	//If the string is drawn, the return value [of ExtTextOutW] is nonzero. However, if the ANSI version of ExtTextOut is called with ETO_GLYPH_INDEX, the function returns TRUE even though the function does nothing.
	VERIFY( ::ExtTextOutW( pdc.m_hDC, 0, 0, ETO_OPAQUE, &m_rcAll, NULL, 0u, NULL ) );

	RECT rc = m_rcInner;
	
	ASSERT( pdc.m_hDC != NULL );

	//If [DrawEdge] succeeds, the return value is nonzero. If [DrawEdge] fails, the return value is zero.
	VERIFY( ::DrawEdge( pdc.m_hDC, &rc, EDGE_SUNKEN, BF_RECT | BF_ADJUST ) );

	//pdc.FillSolidRect( &rc, RGB( 255, 255, 255 ) );

	ASSERT_VALID( &pdc );
	ASSERT( pdc.m_hDC != NULL );

	//If [SetBkColor] fails, the return value is CLR_INVALID.
	const COLORREF color_res_2 = ::SetBkColor( pdc.m_hDC, RGB( 255, 255, 255 ) );
	
	//TODO: check this!
	ASSERT( color_res_2 != CLR_INVALID );

	if ( color_res_2 == CLR_INVALID ) {
		TRACE( _T( "::SetBkColor( pdc.m_hDC, RGB( 255, 255, 255 ) ) failed!!\r\n" ) );
		}

	//If the string is drawn, the return value [of ExtTextOutW] is nonzero. However, if the ANSI version of ExtTextOut is called with ETO_GLYPH_INDEX, the function returns TRUE even though the function does nothing.
	VERIFY( ::ExtTextOutW( pdc.m_hDC, 0, 0, ETO_OPAQUE, &rc, NULL, 0u, NULL ) );

	CPen pen( PS_SOLID, 1, GetSysColor( COLOR_3DLIGHT ) );
	CSelectObject sopen( pdc, pen );

	move_to_coord( pdc, rc.left, m_zero.y );

	line_to_coord( pdc, rc.right, m_zero.y );
	
	move_to_coord( pdc, m_zero.x, rc.top );

	line_to_coord( pdc, m_zero.x, rc.bottom );

	RECT circle = m_rcAll;

	//"Return value: If the function succeeds, the return value is nonzero. If the function fails, the return value is zero."
	//circle.DeflateRect( m_gripperRadius );
	VERIFY( ::InflateRect( &circle, -( m_gripperRadius.cx ), -( m_gripperRadius.cy ) ) );

	CSelectStockObject sobrush( pdc, NULL_BRUSH );

	ASSERT( pdc.m_hDC != NULL );
	//If [Ellipse] succeeds, the return value is nonzero. If [Ellipse] fails, the return value is zero.
	VERIFY( ::Ellipse( pdc.m_hDC, circle.left, circle.top, circle.right, circle.bottom ) );

	//--------------------------------
	if ( GetFocus( ) == this ) {
		//TODO: what function?
		//"Return value: If the function succeeds, the return value is nonzero. If the function fails, the return value is zero."
		ASSERT( ::IsWindow( m_hWnd ) );
		VERIFY( ::DrawFocusRect( pdc.m_hDC, &m_rcAll ) );
		}
	}

void CXySlider::PaintGripper( _In_ CDC& pdc ) {
	//ASSERT_VALID( pdc );
	RECT rc = GetGripperRect( );

	COLORREF color_scopeholder = ::GetSysColor( COLOR_BTNFACE );
	if ( m_gripperHighlight ) {
		INT r = GetRValue( color_scopeholder );
		INT g = GetGValue( color_scopeholder );
		INT b = GetBValue( color_scopeholder );;
		r += ( 255 - r ) / 3;
		g += ( 255 - g ) / 3;
		b += ( 255 - b ) / 3;
		color_scopeholder = RGB( r, g, b );
		}

	const COLORREF color = color_scopeholder;

	fill_solid_rectangle( pdc.m_hDC, rc, color );

	ASSERT( pdc.m_hDC != NULL );

	//If [DrawEdge] succeeds, the return value is nonzero. If [DrawEdge] fails, the return value is zero.
	VERIFY( ::DrawEdge( pdc.m_hDC, &rc, EDGE_RAISED, BF_RECT ) );


	CPen pen( PS_SOLID, 1, ::GetSysColor( COLOR_3DSHADOW ) );
	CSelectObject sopen( pdc, pen );

	move_to_coord( pdc, rc.left, ( rc.top + ( rc.bottom - rc.top ) / 2 ) );

	line_to_coord( pdc, rc.right, ( rc.top + ( rc.bottom - rc.top ) / 2 ) );

	move_to_coord( pdc, rc.left + ( rc.right - rc.left ) / 2, rc.top );

	line_to_coord( pdc, rc.left + ( rc.right - rc.left ) / 2, rc.bottom );
	}

void CXySlider::DoMoveBy( _In_ const INT cx, _In_ const INT cy ) {
	m_pos.x += cx;
	CheckMinMax( m_pos.x, -m_range.cx, m_range.cx );

	m_pos.y += cy;
	CheckMinMax( m_pos.y, -m_range.cy, m_range.cy );

	VERIFY( CWnd::RedrawWindow( ) );

	const POINT oldpos = m_externalPos;
	InternToExtern( );
	if ( ( m_externalPos.x != oldpos.x ) || ( m_externalPos.y != oldpos.y ) ) {
		NotifyParent( );
		}
	}

void CXySlider::Handle_WM_MOUSEMOVE( _In_ const POINT& ptMin, _In_ const POINT& ptMax, _In_ const MSG& msg, _Inout_ POINT& pt0 ) {
	POINT pt = msg.pt;
	ASSERT( ::IsWindow( m_hWnd ) );
	//"Return value: If the function succeeds, the return value is nonzero. If the function fails, the return value is zero."
	VERIFY( ::ScreenToClient( m_hWnd, &pt ) );
	//ScreenToClient( &pt );

	CheckMinMax( pt.x, ptMin.x, ptMax.x );
	CheckMinMax( pt.y, ptMin.y, ptMax.y );

	const INT dx = pt.x - pt0.x;
	const INT dy = pt.y - pt0.y;

	DoMoveBy( dx, dy );

	pt0 = pt;
	}

void CXySlider::DoDrag( _In_ const POINT point ) {
	POINT pt0 = point;

	HighlightGripper( true );

	/*
	inline CPoint CRect::CenterPoint() const throw()
	{
		return CPoint((left+right)/2, (top+bottom)/2);
	}	
	*/
	const RECT grip_rect = GetGripperRect( );
	const auto grip_rect_center_x = ( ( grip_rect.left + grip_rect.right ) / 2 );
	const auto grip_rect_center_y = ( ( grip_rect.top + grip_rect.bottom ) / 2 );
	const auto new_point_x = ( pt0.x - grip_rect_center_x );
	const auto new_point_y = ( pt0.y - grip_rect_center_y );
	const CSize inGripper( new_point_x, new_point_y );
	POINT ptMin_holder;
	ptMin_holder.x = ( m_zero.x - m_range.cx + inGripper.cx );
	ptMin_holder.y = ( m_zero.y - m_range.cy + inGripper.cy );
	const POINT ptMin = ptMin_holder;

	POINT ptMax_holder;
	ptMax_holder.x = ( m_zero.x + m_range.cx + inGripper.cx );
	ptMax_holder.y = ( m_zero.y + m_range.cy + inGripper.cy );

	const POINT ptMax = ptMax_holder;

	CWnd::SetCapture( );
	do {
		MSG msg;
		if ( !GetMessageW( &msg, NULL, 0, 0 ) ) {
			break;
			}

		if ( msg.message == WM_LBUTTONUP ) {
			break;
			}
		if ( CWnd::GetCapture( ) != this ) {
			break;
			}

		if ( msg.message == WM_MOUSEMOVE ) {
			Handle_WM_MOUSEMOVE( ptMin, ptMax, msg, pt0 );
			}
		else {
			DispatchMessageW( &msg );
			}
#pragma warning(suppress:4127)//conditional expression is constant
		} while ( true );

	VERIFY( ReleaseCapture( ) );

	HighlightGripper( false );
	}

void CXySlider::DoPage( _In_ const POINT point ) {
	POINT _m_zero_m_pos_scope_holder;
	_m_zero_m_pos_scope_holder.x = m_zero.x + m_pos.x;
	_m_zero_m_pos_scope_holder.y = m_zero.y + m_pos.y;


	const POINT& _m_zero_m_pos_ = _m_zero_m_pos_scope_holder;

	POINT _point_minus_m_zero_m_pos;
	_point_minus_m_zero_m_pos.x = point.x;
	_point_minus_m_zero_m_pos.y = point.y;

	_point_minus_m_zero_m_pos.x -= _m_zero_m_pos_.x;
	_point_minus_m_zero_m_pos.y -= _m_zero_m_pos_.y;

	SIZE sz_holder;
	sz_holder.cx = _point_minus_m_zero_m_pos.x;
	sz_holder.cy = _point_minus_m_zero_m_pos.y;

	const SIZE& sz = sz_holder;

	ASSERT( sz.cx != 0 || sz.cy != 0 );

	const auto len = sqrt( static_cast<DOUBLE>( sz.cx ) * static_cast<DOUBLE>( sz.cx ) + static_cast<DOUBLE>( sz.cy ) * static_cast<DOUBLE>( sz.cy ) );

	const auto dx = static_cast<INT>( 10 * sz.cx / len );
	const auto dy = static_cast<INT>( 10 * sz.cy / len );

	DoMoveBy( dx, dy );
	}

void CXySlider::HighlightGripper( _In_ const bool on ) {
	m_gripperHighlight = on;
	VERIFY( CWnd::RedrawWindow( ) );
	}

void CXySlider::RemoveTimer( ) {
	if ( m_timer != 0 ) {
		ASSERT( ::IsWindow( m_hWnd ) );
		
		//If [KillTimer] succeeds, the return value is nonzero. If [KillTimer] fails, the return value is zero. To get extended error information, call GetLastError.
		VERIFY( ::KillTimer( m_hWnd, m_timer ) );
		}
	m_timer = 0;
	}

afx_msg void CXySlider::OnSetFocus( CWnd* pOldWnd ) {
	CWnd::OnSetFocus( pOldWnd );
	/*
void CWnd::OnSetFocus( CWnd* ) { 
	BOOL bHandled;
	bHandled = FALSE;
	if ( m_pCtrlCont != NULL ) {
		bHandled = m_pCtrlCont->HandleSetFocus();
		}

	if( !bHandled ) {
		Default();
		}
	}

LRESULT CWnd::Default( ) {
	_AFX_THREAD_STATE* pThreadState = _afxThreadState.GetData( );
	return DefWindowProc( pThreadState->m_lastSentMsg.message, pThreadState->m_lastSentMsg.wParam, pThreadState->m_lastSentMsg.lParam );
	}
	*/


	ASSERT( ::IsWindow( m_hWnd ) );
	//"Return value: If the function succeeds, the return value is nonzero. If the function fails, the return value is zero."
	VERIFY( ::InvalidateRect( m_hWnd, NULL, TRUE ) );
	//Invalidate( );
	}

afx_msg void CXySlider::OnKillFocus( CWnd* pNewWnd ) {
	CWnd::OnKillFocus( pNewWnd );
	ASSERT( ::IsWindow( m_hWnd ) );
	//"Return value: If the function succeeds, the return value is nonzero. If the function fails, the return value is zero."
	VERIFY( ::InvalidateRect( m_hWnd, NULL, TRUE ) );
	//Invalidate( );
	}



BEGIN_MESSAGE_MAP(CXySlider, CStatic)
	ON_WM_DESTROY()
	ON_WM_GETDLGCODE()
	ON_WM_NCHITTEST()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_PAINT()
	ON_WM_KEYDOWN()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONUP()
	ON_WM_TIMER()
	ON_MESSAGE(XY_SETPOS, &( CXySlider::OnSetPos ) )
	ON_MESSAGE(XY_GETPOS, &( CXySlider::OnGetPos ) )
END_MESSAGE_MAP()

void CXySlider::OnPaint( ) {
	Initialize( );
	const INT w = ( m_rcAll.right - m_rcAll.left );
	const INT h = ( m_rcAll.bottom - m_rcAll.top );

	CPaintDC dc( this );
	CDC dcmem;
	VERIFY( dcmem.CreateCompatibleDC( &dc ) );
	CBitmap bm;
	VERIFY( bm.CreateCompatibleBitmap( &dc, w, h ) );
	CSelectObject sobm( dcmem, bm );

	PaintBackground( dcmem );
	// PaintValues(&dcmem); This is too noisy
	PaintGripper( dcmem );

	VERIFY( dc.BitBlt( 0, 0, w, h, &dcmem, 0, 0, SRCCOPY ) );
	//VERIFY( dcmem.DeleteDC( ) );
	}


void CXySlider::OnKeyDown( UINT nChar, UINT /*nRepCnt*/, UINT /*nFlags*/ ) {
	switch ( nChar )
	{
		case VK_LEFT:
			return DoMoveBy( -1,  0 );
		case VK_RIGHT:
			return DoMoveBy(  1,  0 );
		case VK_UP:
			return DoMoveBy(  0, -1 );
		case VK_DOWN:
			return DoMoveBy(  0,  1 );
	}
	}

void CXySlider::OnLButtonDown( UINT /*nFlags*/, CPoint point ) {
	CWnd::SetFocus( );

	const RECT rc = GetGripperRect( );

	if ( ::PtInRect( &rc, point ) ) {
		return DoDrag( point );
		}
	DoPage( point );
	InstallTimer( );
	}

void CXySlider::OnLButtonDblClk( UINT /*nFlags*/, CPoint point ) {
	CWnd::SetFocus( );

	const RECT grip_rect = GetGripperRect( );
	if ( ::PtInRect( &grip_rect, point ) ) {
		return DoMoveBy( -m_pos.x, -m_pos.y );
		}
	DoPage( point );
	InstallTimer( );
	}

void CXySlider::OnTimer( UINT_PTR /*nIDEvent*/ ) {
	POINT point;
	VERIFY( ::GetCursorPos( &point ) );
	ASSERT( ::IsWindow( m_hWnd ) );
	//"Return value: If [ScreenToClient] succeeds, the return value is nonzero. If [ScreenToClient] fails, the return value is zero."
	VERIFY( ::ScreenToClient( m_hWnd, &point ) );

	const RECT rc = GetGripperRect( );
	if ( !::PtInRect( &rc, point ) ) {
		DoPage( point );
		}
	}

void CXySlider::SetPos( const POINT pt ) {
	Initialize( );
	m_externalPos = pt;
	ExternToIntern( );
	CWnd::Invalidate( );
	}


#else

#endif