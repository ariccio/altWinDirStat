// XySlider.cpp	- Implementation of CXySlider
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

#include "stdafx.h"
#include "xyslider.h"

#include "globalhelpers.h"

IMPLEMENT_DYNAMIC(CXySlider, CStatic)

//const UINT CXySlider::XY_SETPOS = WM_USER + 100;
//const UINT CXySlider::XY_GETPOS = WM_USER + 101;


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
	if ( !m_inited && IsWindow( m_hWnd ) ) {
		// Make size odd, so that zero lines are central
		CRect rc;

		//"Return value: If the function succeeds, the return value is nonzero. If the function fails, the return value is zero. To get extended error information, call GetLastError."
		VERIFY( ::GetWindowRect( m_hWnd, rc ) );
		//GetWindowRect( rc );


		GetParent( )->ScreenToClient( rc );
		if ( ( rc.right - rc.left ) % 2 == 0 ) {
			rc.right--;
			}
		if ( ( rc.bottom - rc.top ) % 2 == 0 ) {
			rc.bottom--;
			}
		MoveWindow( rc );

		// Initialize constants
		CalcSizes( );

		m_inited = true;
		}
	}

void CXySlider::CalcSizes( ) {
	//static const INT GRIPPER_RADIUS = 8;
	ASSERT( ::IsWindow( m_hWnd ) );
	//"Return value: If the function succeeds, the return value is nonzero. If the function fails, the return value is zero. To get extended error information, call GetLastError."
	VERIFY( ::GetClientRect( m_hWnd, &m_rcAll ) );
	//GetClientRect( &m_rcAll );


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

	/*
	inline void CRect::DeflateRect(
		_In_ int x,
		_In_ int y) throw()
	{
		::InflateRect(this, -x, -y);
	}	
	*/

	//"Return value: If the function succeeds, the return value is nonzero. If the function fails, the return value is zero."
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
	GetParent( )->SendMessageW( WM_NOTIFY, static_cast<WPARAM>( GetDlgCtrlID( ) ), ( LPARAM ) &hdr );
	}

void CXySlider::PaintBackground( _In_ CDC& pdc ) {
	//ASSERT_VALID( pdc );
	pdc.FillSolidRect( &m_rcAll, GetSysColor( COLOR_BTNFACE ) );

	CRect rc = m_rcInner;
	VERIFY( pdc.DrawEdge( rc, EDGE_SUNKEN, BF_RECT | BF_ADJUST ) );

	pdc.FillSolidRect( rc, RGB( 255, 255, 255 ) );

	CPen pen( PS_SOLID, 1, GetSysColor( COLOR_3DLIGHT ) );
	CSelectObject sopen( pdc, pen );

	pdc.MoveTo( rc.left,  m_zero.y );
	//"Return value: Nonzero if the line is drawn; otherwise 0."
	VERIFY( pdc.LineTo( rc.right, m_zero.y ) );
	pdc.MoveTo( m_zero.x, rc.top );
	VERIFY( pdc.LineTo( m_zero.x, rc.bottom ) );

	CRect circle = m_rcAll;

	/*
inline void CRect::DeflateRect(_In_ SIZE size) throw()
{
	::InflateRect(this, -size.cx, -size.cy);
}
	*/
	//"Return value: If the function succeeds, the return value is nonzero. If the function fails, the return value is zero."
	//circle.DeflateRect( m_gripperRadius );
	VERIFY( ::InflateRect( circle, -( m_gripperRadius.cx ), -( m_gripperRadius.cy ) ) );

	CSelectStockObject sobrush( pdc, NULL_BRUSH );
	VERIFY( pdc.Ellipse( circle ) );

	if ( GetFocus( ) == this ) {
		//"Return value: If the function succeeds, the return value is nonzero. If the function fails, the return value is zero."
		//pdc.DrawFocusRect( &m_rcAll );
		ASSERT(::IsWindow(m_hWnd));
		VERIFY( ::DrawFocusRect( pdc.m_hDC, &m_rcAll ) );
		}
	}

void CXySlider::PaintGripper( _In_ CDC& pdc ) {
	//ASSERT_VALID( pdc );
	RECT rc = GetGripperRect( );

	COLORREF color = GetSysColor( COLOR_BTNFACE );
	if ( m_gripperHighlight ) {
		INT r = GetRValue( color );
		INT g = GetGValue( color );
		INT b = GetBValue( color );;
		r += ( 255 - r ) / 3;
		g += ( 255 - g ) / 3;
		b += ( 255 - b ) / 3;
		color = RGB( r, g, b );
		}
	pdc.FillSolidRect( &rc, color );
	VERIFY( pdc.DrawEdge( &rc, EDGE_RAISED, BF_RECT ) );

	CPen pen( PS_SOLID, 1, GetSysColor( COLOR_3DSHADOW ) );
	CSelectObject sopen( pdc, pen );

	pdc.MoveTo( rc.left, rc.top + ( rc.bottom - rc.top ) / 2 );
	//"Return value: If the function succeeds, the return value is nonzero. If the function fails, the return value is zero."
	VERIFY( pdc.LineTo( rc.right, rc.top + ( rc.bottom - rc.top ) / 2 ) );
	pdc.MoveTo( rc.left + ( rc.right - rc.left ) / 2, rc.top );
	VERIFY( pdc.LineTo( rc.left + ( rc.right - rc.left ) / 2, rc.bottom ) );
	}

void CXySlider::DoMoveBy( _In_ const INT cx, _In_ const INT cy ) {
	m_pos.x += cx;
	CheckMinMax( m_pos.x, -m_range.cx, m_range.cx );

	m_pos.y += cy;
	CheckMinMax( m_pos.y, -m_range.cy, m_range.cy );

	VERIFY( RedrawWindow( ) );

	const WTL::CPoint oldpos = m_externalPos;
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

	//const WTL::CPoint ptMin( m_zero - m_range + inGripper );
	POINT ptMax_holder;
	ptMax_holder.x = ( m_zero.x + m_range.cx + inGripper.cx );
	ptMax_holder.y = ( m_zero.y + m_range.cy + inGripper.cy );

	const POINT ptMax = ptMax_holder;

	//const WTL::CPoint ptMax( m_zero + m_range + inGripper );

	SetCapture( );
	do {
		MSG msg;
		if ( !GetMessageW( &msg, NULL, 0, 0 ) ) {
			break;
			}

		if ( msg.message == WM_LBUTTONUP ) {
			break;
			}
		if ( GetCapture( ) != this ) {
			break;
			}

		if ( msg.message == WM_MOUSEMOVE ) {
			//CPoint pt = msg.pt;
			//ScreenToClient( &pt );
			//
			//CheckMinMax( pt.x, ptMin.x, ptMax.x );
			//CheckMinMax( pt.y, ptMin.y, ptMax.y );
			//
			//const INT dx = pt.x - pt0.x;
			//const INT dy = pt.y - pt0.y;
			//
			//DoMoveBy( dx, dy );
			//
			//pt0 = pt;
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
	//const WTL::CSize debugging_sz = point - ( m_zero + m_pos );
	//ASSERT( debugging_sz == sz );

	//const WTL::CSize sz = point - ( m_zero + m_pos );

	ASSERT( sz.cx != 0 || sz.cy != 0 );

	const auto len = sqrt( static_cast<DOUBLE>( sz.cx ) * static_cast<DOUBLE>( sz.cx ) + static_cast<DOUBLE>( sz.cy ) * static_cast<DOUBLE>( sz.cy ) );

	const auto dx = static_cast<INT>( 10 * sz.cx / len );
	const auto dy = static_cast<INT>( 10 * sz.cy / len );

	DoMoveBy( dx, dy );
	}

void CXySlider::HighlightGripper( _In_ const bool on ) {
	m_gripperHighlight = on;
	VERIFY( RedrawWindow( ) );
	}

void CXySlider::RemoveTimer( ) {
	if ( m_timer != 0 ) {
		VERIFY( KillTimer( m_timer ) );
		}
	m_timer = 0;
	}

afx_msg void CXySlider::OnSetFocus( CWnd* pOldWnd ) {
	CStatic::OnSetFocus( pOldWnd );
	ASSERT( ::IsWindow( m_hWnd ) );
	//"Return value: If the function succeeds, the return value is nonzero. If the function fails, the return value is zero."
	VERIFY( ::InvalidateRect( m_hWnd, NULL, TRUE ) );
	//Invalidate( );
	}

afx_msg void CXySlider::OnKillFocus( CWnd* pNewWnd ) {
	CStatic::OnKillFocus( pNewWnd );
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
	INT w = ( m_rcAll.right - m_rcAll.left );
	INT h = ( m_rcAll.bottom - m_rcAll.top );

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

_At_( nChar, _Const_ )
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
	SetFocus( );

	const RECT rc = GetGripperRect( );

	if ( ::PtInRect( &rc, point ) ) {
		return DoDrag( point );
		}
	DoPage( point );
	InstallTimer( );
	}

void CXySlider::OnLButtonDblClk( UINT /*nFlags*/, CPoint point ) {
	SetFocus( );

	/*
	inline BOOL CRect::PtInRect(_In_ POINT point) const throw()
	{
		return ::PtInRect(this, point);
	}
	*/
	const RECT grip_rect = GetGripperRect( );
	if ( ::PtInRect( &grip_rect, point ) ) {
		return DoMoveBy( -m_pos.x, -m_pos.y );
		}
	DoPage( point );
	InstallTimer( );
	}

void CXySlider::OnTimer( UINT_PTR /*nIDEvent*/ ) {
	POINT point;
	VERIFY( GetCursorPos( &point ) );
	ASSERT( ::IsWindow( m_hWnd ) );
	//"Return value: If the function succeeds, the return value is nonzero. If the function fails, the return value is zero."
	VERIFY( ::ScreenToClient( m_hWnd, &point ) );
	//ScreenToClient( &point );

	const RECT rc = GetGripperRect( );
	if ( !::PtInRect( &rc, point ) ) {
		DoPage( point );
		}
	}

void CXySlider::SetPos( const POINT pt ) {
	Initialize( );
	m_externalPos = pt;
	ExternToIntern( );
	Invalidate( );
	}

// $Log$
// Revision 1.4  2004/11/07 00:06:34  assarbad
// - Fixed minor bug with ampersand (details in changelog.txt)
//
// Revision 1.3  2004/11/05 16:53:08  assarbad
// Added Date and History tag where appropriate.
//
